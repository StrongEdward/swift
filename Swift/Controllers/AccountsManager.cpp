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

#include <set>

#include <Swift/Controllers/AccountsManager.h>

#include <3rdParty/Boost/src/boost/make_shared.hpp>
#include <3rdParty/Boost/src/boost/scoped_ptr.hpp>

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
	loginWindow_->onLoginRequest.connect(boost::bind(&AccountsManager::handleLoginRequest, this, _1, _2, _3, _4, _5, _6));

	std::string lastLoginJID = settings_->getSetting(SettingConstants::LAST_LOGIN_JID);

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

	// Creating Main Controllers
	maxAccountIndex_ = 0;
	foreach (std::string profile, profiles) {

		boost::shared_ptr<Account> account;
		if (indicesOK) {
			account = boost::make_shared<Account>(new ProfileSettingsProvider(profile, settings_));
			if (account->getIndex() >= maxAccountIndex_) {
				maxAccountIndex_ = account->getIndex() + 1;
			}
		} else {
			account = boost::make_shared<Account>(new ProfileSettingsProvider(profile, settings_), maxAccountIndex_);
			maxAccountIndex_++;
		}

		loginWindow_->addAvailableAccount(account->getJID(), account->getPassword(), account->getCertificatePath(), account->getClientOptions());  // to be deleted after accounts list works
		createMainController(account);

		// For now: default account = last login account
		if (account->getJID() == lastLoginJID) defaultAccount_ = account;

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

	//bool loginAutomatically = settings_->getSetting(SettingConstants::LOGIN_AUTOMATICALLY);
	bool eagle = settings_->getSetting(SettingConstants::FORGET_PASSWORDS);
	if (!eagle) {
		loginWindow_->selectUser(getDefaultJID());
		loginWindow_->setLoginAutomatically(getAccountByJID(getDefaultJID())->getLoginAutomatically());
	}

	if (!defaultAccount_ && mainControllers_.size() > 0) {
		defaultAccount_ = mainControllers_.at(0)->getAccount();
	}
	accountsList_ = loginWindow_->getAccountsList();
	accountsList_->setManager(this);
	accountsList_->setDefaultAccount(defaultAccount_->getIndex());
	accountsList_->onDefaultButtonClicked.connect(boost::bind(&AccountsManager::handleDefaultButtonClicked, this, _1));

	foreach (MainController* c, mainControllers_) {
		if (c->getAccount()->getLoginAutomatically()) {
			loginWindow_->setIsLoggingIn(true);
			c->getAccount()->setEnabled(true);
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

void AccountsManager::createMainController(boost::shared_ptr<Account> account) {
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
														 useDelayForLatency_);
	mainControllers_.push_back(mainController);
}

JID AccountsManager::getDefaultJID() {
	return defaultAccount_->getJID();
}

boost::shared_ptr<Account> AccountsManager::getAccountByJID(std::string jid) {
	/*foreach (boost::shared_ptr<Account> account, accounts_) {
		if (account->getJID() == jid) {
			return account;
		}
	}
	return boost::shared_ptr<Account>();*/

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

void AccountsManager::clearAutoLogins() {
	foreach (MainController * controller, mainControllers_) {
		controller->getAccount()->setLoginAutomatically(false);
	}
}

// Handlers
void AccountsManager::handleLoginRequest(const std::string &username, const std::string &password, const std::string& certificatePath, const ClientOptions& options, bool remember, bool loginAutomatically) {

	//MainController* controller = getMainControllerByJIDString(username);

	JID usernameJID = JID(username);
	if (!usernameJID.isValid() || usernameJID.getNode().empty()) {
		loginWindow_->setMessage(QT_TRANSLATE_NOOP("", "User address invalid. User address should be of the form 'alice@wonderland.lit'"));
		loginWindow_->setIsLoggingIn(false);
	} else {
		loginWindow_->setMessage("");
		loginWindow_->setIsLoggingIn(true);

		// I think we can update account details with every single change in widgets so below code could be removed later.
		boost::shared_ptr<Account> account = getAccountByJID(username);

		if (!account) { // Login to new account
			// First parameter will not be 'username' after implementing new GUI with account name input
			account = boost::shared_ptr<Account>(new Account(maxAccountIndex_,
															 username,
															 username,
															 password,
															 certificatePath,
															 options,
															 remember,
															 loginAutomatically,
															 false,
															 (mainControllers_.size() > 0 ? false : true), // Set as default if there's no other accounts
															 new ProfileSettingsProvider(username, settings_)));
			createMainController(account);
			account->setEnabled(true);
			maxAccountIndex_++;
		} else { // Login to existing account


			//ProfileSettingsProvider* profileSettings = account->getProfileSettings();

			//if (!settings_->getSetting(SettingConstants::FORGET_PASSWORDS)) {
				/*profileSettings->storeString("jid", username);
				profileSettings->storeString("certificate", certificatePath);
				profileSettings->storeString("pass", (remember || loginAutomatically) ? password : "");
				profileSettings->storeInt("remember", remember);
				profileSettings->storeInt("enabled", loginAutomatically);
				std::string optionString = serializeClientOptions(options);
				profileSettings->storeString("options", optionString);*/

				account->setCertificatePath(certificatePath);
				account->setPassword(password);
				account->setRememberPassword(remember);
				if (loginAutomatically) { // We want only one account (for now) to log in automatically
					clearAutoLogins();
				}
				account->setLoginAutomatically(loginAutomatically);
				account->setClientOptions(options);

				settings_->storeSetting(SettingConstants::LAST_LOGIN_JID, username);
				//settings_->storeSetting(SettingConstants::LOGIN_AUTOMATICALLY, loginAutomatically);
				loginWindow_->addAvailableAccount(account->getJID(), account->getPassword(), account->getCertificatePath(), account->getClientOptions());

				account->setEnabled(true);
			//}
		}
	}

}

void AccountsManager::handleDefaultButtonClicked(int id) {
	defaultAccount_->setDefault(false);
	defaultAccount_ = this->getAccountAt(id);
	defaultAccount_->setDefault(true);
}


} // namespace Swift
