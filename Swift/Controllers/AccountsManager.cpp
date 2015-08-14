/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

/*
 * Copyright (c) 2010-2015 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#include <Swift/Controllers/AccountsManager.h>

#include <set>
#include <vector>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

#include <Swiften/Base/format.h>
#include <Swiften/Network/NetworkFactories.h>

#ifdef SWIFTEN_PLATFORM_WIN32
#include <Swiften/SASL/WindowsAuthentication.h>
#endif

#include <SwifTools/Dock/Dock.h>
#include <SwifTools/Idle/IdleDetector.h>
#include <SwifTools/Notifier/TogglableNotifier.h>

#include <Swift/Controllers/Account.h>
#include <Swift/Controllers/FileTransferListController.h>
#include <Swift/Controllers/HighlightEditorController.h>
#include <Swift/Controllers/HighlightManager.h>
#include <Swift/Controllers/Intl.h>
#include <Swift/Controllers/MainController.h>
#include <Swift/Controllers/Roster/CollapsedRosterItemsSet.h>
#include <Swift/Controllers/SettingConstants.h>
#include <Swift/Controllers/Settings/SettingsProvider.h>
#include <Swift/Controllers/SoundPlayer.h>
#include <Swift/Controllers/Storages/StoragesFactory.h>
#include <Swift/Controllers/SystemTray.h>
#include <Swift/Controllers/SystemTrayController.h>
#include <Swift/Controllers/UIEvents/UIEventStream.h>
#include <Swift/Controllers/UIInterfaces/LoginWindow.h>
#include <Swift/Controllers/UIInterfaces/MainWindow.h>
#include <Swift/Controllers/UIInterfaces/UIFactory.h>
#include <Swift/Controllers/XMLConsoleController.h>
#include <Swift/Controllers/XMPPEvents/EventController.h>


namespace Swift {

AccountsManager::AccountsManager(EventLoop* eventLoop, NetworkFactories* networkFactories, UIFactory* uiFactory, SettingsProvider* settings, SystemTray* systemTray, SoundPlayer* soundPlayer, StoragesFactory* storagesFactory, CertificateStorageFactory* certificateStorageFactory, Dock* dock, Notifier* notifier, URIHandler* uriHandler, IdleDetector* idleDetector, const std::map<std::string, std::string> emoticons, bool useDelayForLatency) :
	defaultAccount_(boost::shared_ptr<Account>()),
	settings_(settings),
	eventLoop_(eventLoop),
	networkFactories_(networkFactories),
	uiFactory_(uiFactory),
	soundPlayer_(soundPlayer),
	storagesFactory_(storagesFactory),
	certificateStorageFactory_(certificateStorageFactory),
	dock_(dock),
	uriHandler_(uriHandler),
	idleDetector_(idleDetector),
	emoticons_(emoticons),
	useDelayForLatency_(useDelayForLatency) {

	uiEventStream_ = new UIEventStream();
	eventController_ = new EventController();

	togglableNotifier_ = new TogglableNotifier(notifier);
	togglableNotifier_->setPersistentEnabled(settings_->getSetting(SettingConstants::SHOW_NOTIFICATIONS));

	systemTrayController_ = new SystemTrayController(eventController_, systemTray);

	highlightManager_ = new HighlightManager(settings_);
	highlightEditorController_ = new HighlightEditorController(uiEventStream_, uiFactory_, highlightManager_);

	fileTransferListController_ = new FileTransferListController(uiEventStream_, uiFactory_);

	xmlConsoleController_ = new XMLConsoleController(uiEventStream_, uiFactory_);

	collapsedRosterItems_ = new CollapsedRosterItemsSet();

	loginWindow_ = uiFactory->createLoginWindow(uiEventStream_);
	loginWindow_->setShowNotificationToggle(!notifier->isExternallyConfigured());
	loginWindow_->onLoginRequest.connect(boost::bind(&AccountsManager::handleLoginRequestTriggeredByCombobox, this, _1, _2, _3, _4, _5, _6));
	loginWindow_->onPurgeSavedLoginRequest.connect(boost::bind(&AccountsManager::handlePurgeSavedLoginRequest, this, _1));
	loginWindow_->onCancelLoginRequest.connect(boost::bind(&AccountsManager::handleCancelLoginRequest, this, _1));
	loginWindow_->onQuitRequest.connect(boost::bind(&AccountsManager::handleQuitRequest, this));

	mainWindow_ = uiFactory->createMainWindow(uiEventStream_);

	loadAccounts();


	loginWindow_->onDefaultAccountChanged.connect(boost::bind(&AccountsManager::handleDefaultAccountChanged, this, _1));
	defaultAccount_ = getAccountByJIDString(settings_->getSetting(SettingConstants::DEFAULT_ACCOUNT));
	if (!defaultAccount_ && mainControllers_.size() > 0) {
		defaultAccount_ = mainControllers_.at(0)->getAccount();
		settings_->storeSetting(SettingConstants::DEFAULT_ACCOUNT, defaultAccount_->getJID());
	}

	loginWindow_->setAccountsManager(this);

	bool eagle = settings_->getSetting(SettingConstants::FORGET_PASSWORDS);
	if (!eagle) {
		boost::shared_ptr<Account> lastAccount = getAccountByJIDString(settings_->getSetting(SettingConstants::LAST_LOGIN_JID));
		if (lastAccount) {
			loginWindow_->selectUser(lastAccount->getJID());
		}
		else {
			if (mainControllers_.size() > 0) {
				loginWindow_->selectUser( defaultAccount_ ? defaultAccount_->getJID() : mainControllers_.at(0)->getAccount()->getJID() );
			}
		}
	}

	foreach (MainController* c, mainControllers_) {
		if (c->getAccount()->getLoginAutomatically()) {
			loginWindow_->setIsLoggingIn(true);
			c->getAccount()->setEnabled(true);
			return; // Only one account can be enabled for now.
		}
	}
}

AccountsManager::~AccountsManager() {
	foreach (MainController* controller, mainControllers_) {
		delete controller;
	}

	delete xmlConsoleController_;
	delete fileTransferListController_;
	delete highlightManager_;
	delete highlightEditorController_;
	delete systemTrayController_;
	delete togglableNotifier_;

	eventController_->disconnectAll();
	delete eventController_;
	delete uiEventStream_;
}

void AccountsManager::loadAccounts() {
	std::string serializedAccountsString = settings_->getSetting(SettingConstants::SERIALIZED_ACCOUNTS);
	std::vector< boost::shared_ptr<Account> > accounts;

	if (serializedAccountsString != "") {
		std::stringstream stream;
		stream << serializedAccountsString;
		try {
			boost::archive::text_iarchive archive(stream);
			while (!stream.eof()) {
				Account* account;
				archive >> account;
				account->setProfileSettings(settings_); //Temporary, until get rid of profile settings at all
				accounts.push_back(boost::shared_ptr<Account>(account));
			}
		}
		catch (boost::archive::archive_exception&) {}
	}
	else { // Read from SettingsProvider and remove from there
		std::vector<std::string> profiles = settings_->getAvailableProfiles();
		foreach (std::string profile, profiles) {
			accounts.push_back(boost::make_shared<Account>(profile, settings_));
			//settings_->removeProfile(profile); // Not removing yet because we need to have profile settings stored (until full transition to serialization is done)
		}
	}

	foreach(boost::shared_ptr<Account> account, accounts) {
		createMainController(account, false);
	}

	// Ensure that all indices will be from 0 to n
	for (unsigned int i = 0; i < mainControllers_.size(); i++) {
		mainControllers_[i]->getAccount()->setIndex(i);
	}
	storeAccounts(); // In case we have read data from ProfileSettingsProvider
}

void AccountsManager::storeAccounts() {
	std::stringstream stream;
	boost::archive::text_oarchive archive(stream);
	foreach (MainController* controller, mainControllers_) {
		Account* account = static_cast<Account*>(controller->getAccount().get());
		archive << account;
	}
	settings_->storeSetting(SettingConstants::SERIALIZED_ACCOUNTS, stream.str());
}

void AccountsManager::createMainController(boost::shared_ptr<Account> account, bool triggeredByCombobox) {
	MainController* mainController = new MainController (account, eventLoop_, uiEventStream_, eventController_, networkFactories_, uiFactory_, highlightManager_, highlightEditorController_, fileTransferListController_, loginWindow_, mainWindow_, xmlConsoleController_, settings_, systemTrayController_, soundPlayer_, storagesFactory_, certificateStorageFactory_, dock_, togglableNotifier_, uriHandler_, idleDetector_, emoticons_, collapsedRosterItems_, useDelayForLatency_, triggeredByCombobox);

	mainControllers_.push_back(mainController);
	mainController->onShouldBeDeleted.connect(boost::bind(&AccountsManager::removeAccount, this, _1));
	mainController->onConnected.connect(boost::bind(&AccountsManager::handleMainControllerConnected, this, _1));
	account->onAccountDataChanged.connect(boost::bind(&AccountsManager::handleAccountDataChanged, this));
	account->onJIDChanged.connect(boost::bind(&AccountsManager::handleAccountJIDChanged, this, _1, _2));

	storeAccounts();
}

void AccountsManager::setDefaultAccount(boost::shared_ptr<Account> account) {
	if (account) {
		defaultAccount_ = account;
		settings_->storeSetting(SettingConstants::DEFAULT_ACCOUNT, defaultAccount_->getJID());
	}
}

void AccountsManager::clearAutoLogins() {
	foreach (MainController * controller, mainControllers_) {
		controller->getAccount()->setLoginAutomatically(false);
	}
}

int AccountsManager::getMaxAccountIndex() {
	return (mainControllers_.size() > 0 ? mainControllers_.back()->getAccount()->getIndex() : -1);
}

JID AccountsManager::getDefaultJID() {
	return defaultAccount_->getJID();
}

boost::shared_ptr<Account> AccountsManager::getDefaultAccount() {
	return defaultAccount_;
}

boost::shared_ptr<Account> AccountsManager::getAccountByJIDString(const std::string& jid) {
	foreach (MainController* controller, mainControllers_) {
		if (controller->getJIDString() == jid) {
			return controller->getAccount();
		}
	}
	return boost::shared_ptr<Account>();
}

MainController* AccountsManager::getMainControllerByJIDString(const std::string& jid) {
	foreach (MainController* controller, mainControllers_) {
		if (controller->getJIDString() == jid) {
			return controller;
		}
	}
	return NULL;
}

boost::shared_ptr<Account> AccountsManager::getAccountAt(unsigned int index) {
	if (index < mainControllers_.size()) {
		return mainControllers_.at(index)->getAccount();
	}
	return boost::shared_ptr<Account>();
}

int AccountsManager::accountsCount() {
	return mainControllers_.size();
}

void AccountsManager::addAccount(boost::shared_ptr<Account> account) {
	loginWindow_->setMessage("");
	if (!account) {
		account = boost::shared_ptr<Account>(new Account(getMaxAccountIndex() + 1, "account" + boost::lexical_cast<std::string>(getMaxAccountIndex() + 1), "", "", "", ClientOptions(), false, false, settings_));
	}

	if (!getAccountByJIDString(account->getJID())) {
		createMainController(account, false);
		if (mainControllers_.size() == 1) {
			setDefaultAccount(account);
		}
		loginWindow_->addAvailableAccount(account);
	}
	else {
		loginWindow_->setMessage("Account with such address already exists");
	}
}

void AccountsManager::removeAccount(const std::string& username) {
	boost::shared_ptr<Account> account = getAccountByJIDString(username);
	if (account) {
		//account->setEnabled(false);

		// Disconnect, erase from vector and repair indices
		MainController* controllerToDelete = mainControllers_[account->getIndex()];
		//controllerToDelete->signOut();
		mainControllers_.erase(mainControllers_.begin() + account->getIndex());
		for (unsigned int i = 0; i < mainControllers_.size(); i++) {
			mainControllers_[i]->getAccount()->setIndex(i);
		}
		//maxAccountIndex_ = (mainControllers_.size() > 0 ? mainControllers_.back()->getAccount()->getIndex() : 0);

		settings_->removeProfile(username);
		loginWindow_->removeAvailableAccount(account->getIndex());

		delete controllerToDelete;
		storeAccounts();
	}
}

MainWindow*AccountsManager::getMainWindow() const {
	return mainWindow_;
}

void AccountsManager::handleLoginRequestTriggeredByCombobox(const std::string &username, const std::string &password, const std::string& certificatePath, const ClientOptions& options, bool remember, bool loginAutomatically) {
	JID tempJid = JID(username);
	if (options.singleSignOn && (!tempJid.isValid() || !tempJid.getNode().empty())) {
		loginWindow_->setMessage(QT_TRANSLATE_NOOP("", "User address invalid. User address should be of the form 'wonderland.lit'"));
		loginWindow_->setIsLoggingIn(false);
	} else if (!options.singleSignOn && (!tempJid.isValid() || tempJid.getNode().empty())) {
		loginWindow_->setMessage(QT_TRANSLATE_NOOP("", "User address invalid. User address should be of the form 'alice@wonderland.lit'"));
		loginWindow_->setIsLoggingIn(false);
	} else {
#ifdef SWIFTEN_PLATFORM_WIN32
		if (options.singleSignOn) {
			std::string userName;
			std::string clientName;
			std::string serverName;
			boost::shared_ptr<boost::system::error_code> errorCode = getUserNameEx(userName, clientName, serverName);

			if (!errorCode) {
				/* Create JID using the Windows logon name and user provided domain name */
				tempJid = JID(clientName, username);
			}
			else {
				loginWindow_->setMessage(str(format(QT_TRANSLATE_NOOP("", "Error obtaining Windows user name (%1%)")) % errorCode->message()));
				loginWindow_->setIsLoggingIn(false);
				return;
			}
		}
#endif

		loginWindow_->setMessage("");
		loginWindow_->setIsLoggingIn(true);

		boost::shared_ptr<Account> account = getAccountByJIDString(username);

		if (!account) { // Login to new account
			account = boost::shared_ptr<Account>(new Account(getMaxAccountIndex() + 1, username, username, password, certificatePath, options, remember, loginAutomatically, /*false,*/ settings_));
			if (options.singleSignOn) {
				account->setJID(tempJid);
			}
			createMainController(account, true);
			if (mainControllers_.size() == 1) {
				setDefaultAccount(account);
			}
			account->setEnabled(true);
			//++maxAccountIndex_;
			loginWindow_->addAvailableAccount(account);
		}
		else { // Login to existing account
			//if (!settings_->getSetting(SettingConstants::FORGET_PASSWORDS)) {
			if (options.singleSignOn) {
				account->setJID(tempJid);
			}
			account->setCertificatePath(certificatePath);
			account->setRememberPassword(remember);
			if (loginAutomatically) { // We want only one account (for now) to log in automatically
				clearAutoLogins();
			}
			account->setLoginAutomatically(loginAutomatically);
			account->setPassword(password);
			account->setClientOptions(options);
			account->setEnabled(true);
			//}
		}
		settings_->storeSetting(SettingConstants::LAST_LOGIN_JID, username);
	}
}

void AccountsManager::handleCancelLoginRequest(const std::string currentUsername) {
	getMainControllerByJIDString(currentUsername)->signOut();
}

void AccountsManager::handleDefaultAccountChanged(int index) {
	setDefaultAccount(getAccountAt(index));
}

void AccountsManager::handleAccountDataChanged() {
	storeAccounts();
}

void AccountsManager::handleAccountJIDChanged(std::string oldJID, std::string newJID) {
	loginWindow_->updateUsernamesListInCombobox();
}

void AccountsManager::handleMainControllerConnected(const MainController* controller) {
	loginWindow_->setIsLoggingIn(false);
	if (!controller->getAccount()->getRememberPassword()) {
		loginWindow_->clearPassword();
	}
}

void AccountsManager::handlePurgeSavedLoginRequest(const std::string& username) {
	removeAccount(username);
}

void AccountsManager::handleQuitRequest() {
	bool allAccountsWasSignedOut = true;
	foreach (MainController* controller, mainControllers_) {
		if (controller->isActive()) {
			allAccountsWasSignedOut = false;
		}
		controller->quit();
	}
	if (allAccountsWasSignedOut) {
		loginWindow_->quit();
	}
}

bool AccountsManager::compareAccounts (MainController* a, MainController* b) {
	return a->getAccount()->getIndex() < b->getAccount()->getIndex();
}

} // namespace Swift
