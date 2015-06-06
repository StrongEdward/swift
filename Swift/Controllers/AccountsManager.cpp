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

#include <3rdParty/Boost/src/boost/make_shared.hpp>
#include <3rdParty/Boost/src/boost/scoped_ptr.hpp>

#include <Swiften/Base/String.h>
#include <Swiften/StringCodecs/Base64.h>
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
								 const std::map<std::string, std::string>& emoticons,
								 bool useDelayForLatency) :
	uiEventStream_(uiEventStream),
	settings_(settings)
{
	loginWindow_ = uiFactory->createLoginWindow(uiEventStream_);
	loginWindow_->onLoginRequest.connect(boost::bind(&AccountsManager::handleLoginRequest, this, _1, _2, _3, _4, _5, _6, _7));

	std::string lastLoginJID = settings_->getSetting(SettingConstants::LAST_LOGIN_JID);

	std::vector<std::string> profiles = settings_->getAvailableProfiles();
	foreach (std::string profile, profiles) {
		ProfileSettingsProvider profileSettings(profile, settings_);
		std::string jid = profileSettings.getStringSetting("jid");
		std::string password = profileSettings.getStringSetting("pass");
		std::string certificate = profileSettings.getStringSetting("certificate");
		ClientOptions clientOptions = parseClientOptions(profileSettings.getStringSetting("options"));

		loginWindow_->addAvailableAccount(jid, password, certificate, clientOptions);

		accounts_.push_back(boost::make_shared<Account>(jid, jid, password, certificate, clientOptions, false, false));

		if (accounts_.back()->getJID() == "test1@jabster.pl") {
			// Creating only one MainController for now
			MainController* mainController = new MainController (accounts_.back(),
																 this,
																 eventLoop,
																 uiEventStream_,
																 eventController,
																 networkFactories,
																 uiFactory,
																 loginWindow_,
																 settings_,
																 systemTrayController,
																 soundPlayer,
																 storagesFactory,
																 certificateStorageFactory,
																 dock,
																 notifier,
																 togglableNotifier,
																 uriHandler,
																 idleDetector,
																 emoticons,
																 useDelayForLatency);
			mainControllers_.push_back(mainController);
		}

		// TODO: support remember password and autologin
		// TODO: first parameter is temporary - finally: add setting "accountname"

		if (jid == lastLoginJID) defaultAccountJid_ = lastLoginJID;

		bool loginAutomatically = settings_->getSetting(SettingConstants::LOGIN_AUTOMATICALLY);
		bool eagle = settings_->getSetting(SettingConstants::FORGET_PASSWORDS);
		if (!eagle) {
			loginWindow_->selectUser(getDefaultJID());
			loginWindow_->setLoginAutomatically(loginAutomatically);
		}

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





}

AccountsManager::~AccountsManager() {
	foreach (MainController* controller, mainControllers_) {
		delete controller;
	}
}

JID AccountsManager::getDefaultJID() {
	return defaultAccountJid_;
}

boost::shared_ptr<Account> AccountsManager::getAccountByJID(std::string jid) {
	foreach (boost::shared_ptr<Account> account, accounts_) {
		if (account->jid_ == jid) {
			return account;
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

// Handlers
void AccountsManager::handleLoginRequest(const std::string &username, const std::string &password, const std::string& certificatePath, CertificateWithKey::ref certificate, const ClientOptions& options, bool remember, bool loginAutomatically) {

	// Future: loginWindow will send reference to MainController or Account to avoid searching by username/jid (when new GUI is implemented)
	// Now: Assuming that Account pointer below is the one sent by loginWindow, (when testing username is not changed, I use only "test1@jabster.pl")
	MainController* controller = getMainControllerByJIDString(username);
	boost::shared_ptr<Account> account = controller->getAccount();

	//JID jid = account_->getJID();
	if (!account->getJID().isValid() || account->getJID().getNode().empty()) {
		loginWindow_->setMessage(account->getAccountName() + ": " + QT_TRANSLATE_NOOP("", "User address invalid. User address should be of the form 'alice@wonderland.lit'"));
		loginWindow_->setIsLoggingIn(false);
	} else {
		loginWindow_->setMessage("");
		loginWindow_->setIsLoggingIn(true);
		// I think we can update account details with every single change in widgets so below code could be removed later.
		ProfileSettingsProvider* profileSettings = new ProfileSettingsProvider(username, settings_);

		if (!settings_->getSetting(SettingConstants::FORGET_PASSWORDS)) { // change this case to get value from profile setting
			profileSettings->storeString("jid", username);
			profileSettings->storeString("certificate", certificatePath);
			profileSettings->storeString("pass", (remember || loginAutomatically) ? password : "");
			profileSettings->storeInt("remember", remember);
			profileSettings->storeInt("enabled", loginAutomatically);

			std::string optionString = serializeClientOptions(options);
			profileSettings->storeString("options", optionString);
			settings_->storeSetting(SettingConstants::LAST_LOGIN_JID, username);
			//settings_->storeSetting(SettingConstants::LOGIN_AUTOMATICALLY, loginAutomatically);
			loginWindow_->addAvailableAccount(profileSettings->getStringSetting("jid"), profileSettings->getStringSetting("pass"), profileSettings->getStringSetting("certificate"), options);
		}

		onLoginRequest(controller, password, certificate, options, profileSettings);
		//password_ = password;
		//certificate_ = certificate;
		//clientOptions_ = options;
		//performLoginFromCachedCredentials();
	}

}



#define SERIALIZE_BOOL(option) result += options.option ? "1" : "0"; result += ",";
#define SERIALIZE_INT(option) result += boost::lexical_cast<std::string>(options.option); result += ",";
#define SERIALIZE_STRING(option) result += Base64::encode(createByteArray(options.option)); result += ",";
#define SERIALIZE_SAFE_STRING(option) result += safeByteArrayToString(Base64::encode(options.option)); result += ",";
#define SERIALIZE_URL(option) SERIALIZE_STRING(option.toString())

std::string AccountsManager::serializeClientOptions(const ClientOptions& options) {
	std::string result;
	SERIALIZE_BOOL(useStreamCompression);
	switch (options.useTLS) {
		case ClientOptions::NeverUseTLS: result += "1";break;
		case ClientOptions::UseTLSWhenAvailable: result += "2";break;
		case ClientOptions::RequireTLS: result += "3";break;
	}
	result += ",";
	SERIALIZE_BOOL(allowPLAINWithoutTLS);
	SERIALIZE_BOOL(useStreamResumption);
	SERIALIZE_BOOL(useAcks);
	SERIALIZE_STRING(manualHostname);
	SERIALIZE_INT(manualPort);
	switch (options.proxyType) {
		case ClientOptions::NoProxy: result += "1";break;
		case ClientOptions::SystemConfiguredProxy: result += "2";break;
		case ClientOptions::SOCKS5Proxy: result += "3";break;
		case ClientOptions::HTTPConnectProxy: result += "4";break;
	}
	result += ",";
	SERIALIZE_STRING(manualProxyHostname);
	SERIALIZE_INT(manualProxyPort);
	SERIALIZE_URL(boshURL);
	SERIALIZE_URL(boshHTTPConnectProxyURL);
	SERIALIZE_SAFE_STRING(boshHTTPConnectProxyAuthID);
	SERIALIZE_SAFE_STRING(boshHTTPConnectProxyAuthPassword);
	return result;
}

#define CHECK_PARSE_LENGTH if (i >= segments.size()) {return result;}
#define PARSE_INT_RAW(defaultValue) CHECK_PARSE_LENGTH intVal = defaultValue; try {intVal = boost::lexical_cast<int>(segments[i]);} catch(const boost::bad_lexical_cast&) {};i++;
#define PARSE_STRING_RAW CHECK_PARSE_LENGTH stringVal = byteArrayToString(Base64::decode(segments[i]));i++;

#define PARSE_BOOL(option, defaultValue) PARSE_INT_RAW(defaultValue); result.option = (intVal == 1);
#define PARSE_INT(option, defaultValue) PARSE_INT_RAW(defaultValue); result.option = intVal;
#define PARSE_STRING(option) PARSE_STRING_RAW; result.option = stringVal;
#define PARSE_SAFE_STRING(option) PARSE_STRING_RAW; result.option = SafeString(createSafeByteArray(stringVal));
#define PARSE_URL(option) {PARSE_STRING_RAW; result.option = URL::fromString(stringVal);}


ClientOptions AccountsManager::parseClientOptions(const std::string& optionString) {
	ClientOptions result;
	size_t i = 0;
	int intVal = 0;
	std::string stringVal;
	std::vector<std::string> segments = String::split(optionString, ',');

	PARSE_BOOL(useStreamCompression, 1);
	PARSE_INT_RAW(-1);
	switch (intVal) {
	case 1: result.useTLS = ClientOptions::NeverUseTLS;break;
	case 2: result.useTLS = ClientOptions::UseTLSWhenAvailable;break;
	case 3: result.useTLS = ClientOptions::RequireTLS;break;
	default:;
	}
	PARSE_BOOL(allowPLAINWithoutTLS, 0);
	PARSE_BOOL(useStreamResumption, 0);
	PARSE_BOOL(useAcks, 1);
	PARSE_STRING(manualHostname);
	PARSE_INT(manualPort, -1);
	PARSE_INT_RAW(-1);
	switch (intVal) {
	case 1: result.proxyType = ClientOptions::NoProxy;break;
	case 2: result.proxyType = ClientOptions::SystemConfiguredProxy;break;
	case 3: result.proxyType = ClientOptions::SOCKS5Proxy;break;
	case 4: result.proxyType = ClientOptions::HTTPConnectProxy;break;
	}
	PARSE_STRING(manualProxyHostname);
	PARSE_INT(manualProxyPort, -1);
	PARSE_URL(boshURL);
	PARSE_URL(boshHTTPConnectProxyURL);
	PARSE_SAFE_STRING(boshHTTPConnectProxyAuthID);
	PARSE_SAFE_STRING(boshHTTPConnectProxyAuthPassword);

	return result;
}

} // namespace Swift
