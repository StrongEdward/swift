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

#include <3rdParty/Boost/src/boost/make_shared.hpp>
#include <3rdParty/Boost/src/boost/scoped_ptr.hpp>
#include <3rdParty/Boost/src/boost/lexical_cast.hpp>

#include <Swiften/Network/NetworkFactories.h>
#include <Swiften/TLS/CertificateWithKey.h>

#include <SwifTools/Dock/Dock.h>
#include <SwifTools/Notifier/TogglableNotifier.h>
#include <SwifTools/Idle/IdleDetector.h>


#include <Swift/Controllers/MainController.h>
#include <Swift/Controllers/Settings/SettingsProviderHierachy.h>
#include <Swift/Controllers/UIInterfaces/UIFactory.h>
#include <Swift/Controllers/UIInterfaces/LoginWindow.h>
#include <Swift/Controllers/UIEvents/UIEventStream.h>
#include <Swift/Controllers/XMPPEvents/EventController.h>
#include <Swift/Controllers/SystemTrayController.h>
#include <Swift/Controllers/UIInterfaces/LoginWindow.h>
#include <Swift/Controllers/SoundPlayer.h>
#include <Swift/Controllers/Intl.h>
#include <Swift/Controllers/ProfileSettingsProvider.h>
#include <Swift/Controllers/Storages/StoragesFactory.h>
#include <Swift/Controllers/Settings/SettingsProvider.h>
#include <Swift/Controllers/SettingConstants.h>


namespace Swift {

AccountsManager::AccountsManager(EventLoop* eventLoop,
								 UIEventStream* uiEventStream,
								 EventController* eventController,
								 NetworkFactories* networkFactories,
								 UIFactory* uiFactory,
								 SettingsProvider* settings,
								 SystemTrayController* systemTrayController,
								 SoundPlayer* soundPlayer,
								 StoragesFactory* storagesFactory,
								 CertificateStorageFactory* certificateStorageFactory,
								 Dock* dock,
								 Notifier* notifier,
								 TogglableNotifier* togglableNotifier,
								 URIHandler* uriHandler,
								 IdleDetector* idleDetector,
								 const std::map<std::string, std::string> emoticons,
								 bool useDelayForLatency) :
	defaultAccount_(boost::shared_ptr<Account>()),
	uiEventStream_(uiEventStream),
	settings_(settings),
	eventLoop_(eventLoop),
	eventController_(eventController),
	networkFactories_(networkFactories),
	uiFactory_(uiFactory),
	systemTrayController_(systemTrayController),
	soundPlayer_(soundPlayer),
	storagesFactory_(storagesFactory),
	certificateStorageFactory_(certificateStorageFactory),
	dock_(dock),
	//notifier_(notifier),
	togglableNotifier_(togglableNotifier),
	uriHandler_(uriHandler),
	idleDetector_(idleDetector),
	emoticons_(emoticons),
	useDelayForLatency_(useDelayForLatency)
{
	loginWindow_ = uiFactory->createLoginWindow(uiEventStream_);
	loginWindow_->setShowNotificationToggle(!notifier->isExternallyConfigured());
	loginWindow_->onLoginRequest.connect(boost::bind(&AccountsManager::handleLoginRequestTriggeredByCombobox, this, _1, _2, _3, _4, _5, _6));
	loginWindow_->onPurgeSavedLoginRequest.connect(boost::bind(&AccountsManager::handlePurgeSavedLoginRequest, this, _1));

	const std::string lastLoginJID = settings_->getSetting(SettingConstants::LAST_LOGIN_JID);

	std::vector<std::string> profiles = settings_->getAvailableProfiles();

	// Search for bad or lacking indexes in profiles
	bool indicesOK = true;
	std::set<int> indices;
	foreach (std::string profile, profiles) {
		ProfileSettingsProvider provider(profile, settings_);
		int index = provider.getIntSetting("index", -1);
		if (index < 0) { // if there was no index or index was not correct
			indicesOK = false;
		} else {
			indices.insert(index); // putting into set to check if every index is unique
		}
	}
	if (indices.size() < profiles.size() ) {
		indicesOK = false;
	}

	// Create Main Controllers
	maxAccountIndex_ = 0;
	foreach (std::string profile, profiles) {
		boost::shared_ptr<Account> account;
		if (indicesOK) {
			account = boost::make_shared<Account>(profile, settings_);
			if (account->getIndex() >= maxAccountIndex_) {
				maxAccountIndex_ = account->getIndex() + 1;
			}
		} else {
			account = boost::make_shared<Account>(profile, settings_, maxAccountIndex_);
			maxAccountIndex_++;
		}

		createMainController(account, false);

		// For now: default account = last login account
		//if (account->getJID() == lastLoginJID) defaultAccount_ = account;

		/*if (loginAutomatically) {
			mainControllers_[0]->profileSettings_ = new ProfileSettingsProvider(getDefaultJID(), settings_);
			// Below code will be changed soon
			boost::shared_ptr<Account> account = getAccountByJID(getDefaultJID());
			 FIXME: deal with autologin with a cert
			mainControllers_[0]->handleLoginRequest(getDefaultJID(), account->password_, account->certificatePath_, CertificateWithKey::ref(), account->options_, true, true);
		} else {
			mainController->profileSettings_ = NULL;
		}*/
	}

	// Ensure that accounts are sorted by index
	std::sort(mainControllers_.begin(), mainControllers_.end(), AccountsManager::compareAccounts);

	// Ensure that all indices will be from 0 to n
	for (unsigned int i = 0; i < mainControllers_.size(); i++) {
		mainControllers_[i]->getAccount()->setIndex(i);
	}

	loginWindow_->onDefaultAccountChanged.connect(boost::bind(&AccountsManager::handleDefaultAccountChanged, this, _1));
	defaultAccount_ = getAccountByJIDString(settings_->getSetting(SettingConstants::DEFAULT_ACCOUNT));
	if (!defaultAccount_ && mainControllers_.size() > 0) {
		defaultAccount_ = mainControllers_.at(0)->getAccount();
	}
	setDefaultAccount(defaultAccount_);

	loginWindow_->setAccountsManager(this);

	//bool loginAutomatically = settings_->getSetting(SettingConstants::LOGIN_AUTOMATICALLY);
	bool eagle = settings_->getSetting(SettingConstants::FORGET_PASSWORDS);
	if (!eagle) {
		loginWindow_->selectUser(getDefaultAccount()->getJID());
		loginWindow_->setLoginAutomatically(getDefaultAccount()->getLoginAutomatically());
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
}

bool AccountsManager::compareAccounts (MainController* a, MainController* b) {
	return a->getAccount()->getIndex() < b->getAccount()->getIndex();
}

int AccountsManager::maxAccountIndex() {
	return (mainControllers_.size() > 0 ? mainControllers_.back()->getAccount()->getIndex() : 0);
}

void AccountsManager::createMainController(boost::shared_ptr<Account> account, bool triggeredByCombobox) {
	MainController* mainController = new MainController (account,
														 eventLoop_,
														 uiEventStream_,
														 eventController_,
														 networkFactories_,
														 uiFactory_,
														 loginWindow_,
														 settings_,
														 systemTrayController_,
														 soundPlayer_,
														 storagesFactory_,
														 certificateStorageFactory_,
														 dock_,
														 togglableNotifier_,
														 uriHandler_,
														 idleDetector_,
														 emoticons_,
														 useDelayForLatency_,
														 triggeredByCombobox);
	mainControllers_.push_back(mainController);
	mainController->onShouldBeDeleted.connect(boost::bind(&AccountsManager::removeAccount, this, _1));
}

JID AccountsManager::getDefaultJID() {
	return defaultAccount_->getJID();
}

boost::shared_ptr<Account> AccountsManager::getDefaultAccount() {
	return defaultAccount_;
}

boost::shared_ptr<Account> AccountsManager::getAccountByJIDString(std::string jid) {
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
	if (!account) {
		account = boost::shared_ptr<Account>(new Account(maxAccountIndex_,
														 "account" + boost::lexical_cast<std::string>(maxAccountIndex_),
														 "",
														 "",
														 "",
														 ClientOptions(),
														 false,
														 false,
														 false,
														 settings_));
	}
	createMainController(account, false);
	if (mainControllers_.size() == 1) {
		setDefaultAccount(account);
	}
	maxAccountIndex_++;
	loginWindow_->addAvailableAccount(account);
}

void AccountsManager::removeAccount(const std::string& username) {
	boost::shared_ptr<Account> account = getAccountByJIDString(username);
	if (account) {
		// Disconnect
		//account->setEnabled(false);

		// Disconnect, erase from vector and repair indices
		MainController* controllerToDelete = mainControllers_[account->getIndex()];
		controllerToDelete->signOut();
		mainControllers_.erase(mainControllers_.begin() + account->getIndex());
		for (unsigned int i = 0; i < mainControllers_.size(); i++) {
			mainControllers_[i]->getAccount()->setIndex(i);
		}
		maxAccountIndex_ = mainControllers_.back()->getAccount()->getIndex();

		settings_->removeProfile(username);
		loginWindow_->removeAvailableAccount(account->getIndex());

		delete controllerToDelete;
	}
}

void AccountsManager::clearAutoLogins() {
	foreach (MainController * controller, mainControllers_) {
		controller->getAccount()->setLoginAutomatically(false);
	}
}

// Handlers
void AccountsManager::handleLoginRequestTriggeredByCombobox(const std::string &username, const std::string &password, const std::string& certificatePath, const ClientOptions& options, bool remember, bool loginAutomatically) {

	JID usernameJID = JID(username);
	if (!usernameJID.isValid() || usernameJID.getNode().empty()) {
		loginWindow_->setMessage(QT_TRANSLATE_NOOP("", "User address invalid. User address should be of the form 'alice@wonderland.lit'"));
		loginWindow_->setIsLoggingIn(false);
	} else {
		loginWindow_->setMessage("");
		loginWindow_->setIsLoggingIn(true);

		boost::shared_ptr<Account> account = getAccountByJIDString(username);

		if (!account) { // Login to new account
			account = boost::shared_ptr<Account>(new Account(maxAccountIndex_ + 1,
															 username,
															 username,
															 password,
															 certificatePath,
															 options,
															 remember,
															 loginAutomatically,
															 false,
															 //(mainControllers_.size() > 0 ? false : true), // Set as default if there's no other accounts
															 settings_));
			createMainController(account, true);
			if (mainControllers_.size() == 1) {
				setDefaultAccount(account);
			}
			account->setEnabled(true);
			maxAccountIndex_++;
			loginWindow_->addAvailableAccount(account);

		} else { // Login to existing account

			//if (!settings_->getSetting(SettingConstants::FORGET_PASSWORDS)) {
				account->setCertificatePath(certificatePath);
				account->setPassword(password);
				account->setRememberPassword(remember);
				if (loginAutomatically) { // We want only one account (for now) to log in automatically
					clearAutoLogins();
				}
				account->setLoginAutomatically(loginAutomatically);
				account->setClientOptions(options);
				//settings_->storeSetting(SettingConstants::LOGIN_AUTOMATICALLY, loginAutomatically);
				account->setEnabled(true);
			//}
		}
		settings_->storeSetting(SettingConstants::LAST_LOGIN_JID, username);

	}
}

void AccountsManager::handleDefaultAccountChanged(int index) {
	/*defaultAccount_->setDefault(false);
	defaultAccount_ = this->getAccountAt(index);
	defaultAccount_->setDefault(true);*/

	setDefaultAccount(getAccountAt(index));
}

void AccountsManager::handlePurgeSavedLoginRequest(const std::string& username) {
	removeAccount(username);
}

void AccountsManager::setDefaultAccount(boost::shared_ptr<Account> account) {
	if (account) {
		defaultAccount_ = account;
		settings_->storeSetting(SettingConstants::DEFAULT_ACCOUNT, defaultAccount_->getJID());
	}
}


} // namespace Swift
