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

#include <Swiften/Base/String.h>
#include <Swiften/StringCodecs/Base64.h>
#include <Swiften/Network/NetworkFactories.h>

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
#include <Swift/Controllers/ProfileSettingsProvider.h>
#include <Swift/Controllers/Storages/StoragesFactory.h>
#include <Swift/Controllers/Storages/CertificateStorageFactory.h>
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

	std::string lastLoginJID = settings_->getSetting(SettingConstants::LAST_LOGIN_JID);

	std::vector<std::string> profiles = settings_->getAvailableProfiles();
	foreach (std::string profile, profiles) {
		ProfileSettingsProvider profileSettings(profile, settings_);
		std::string jid = profileSettings.getStringSetting("jid");
		std::string password = profileSettings.getStringSetting("pass");
		std::string certificate = profileSettings.getStringSetting("certificate");
		ClientOptions clientOptions = parseClientOptions(profileSettings.getStringSetting("options"));

		loginWindow_->addAvailableAccount(jid, password, certificate, clientOptions);

		accounts_.push_back(Account(jid, jid, password, certificate, clientOptions, false, false));

		// TODO: support remember password and autologin
		// TODO: first parameter is temporary - finally: add setting "accountname"

		if (jid == lastLoginJID) defaultAccountJid_ = lastLoginJID;
	}


	MainController* mainController = new MainController(eventLoop,
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

	bool loginAutomatically = settings_->getSetting(SettingConstants::LOGIN_AUTOMATICALLY);
	bool eagle = settings_->getSetting(SettingConstants::FORGET_PASSWORDS);
	if (!eagle) {
		loginWindow_->selectUser(getDefaultJid());
		loginWindow_->setLoginAutomatically(loginAutomatically);
	}

	if (loginAutomatically) {
		mainController->profileSettings_ = new ProfileSettingsProvider(getDefaultJid(), settings_);
		// Below code will be changed soon
		Account account = getAccountByJid(getDefaultJid());
		/* FIXME: deal with autologin with a cert*/
		mainController->handleLoginRequest(getDefaultJid(), account.password_, account.certificatePath_, CertificateWithKey::ref(), account.options_, true, true);
	} else {
		mainController->profileSettings_ = NULL;
	}

}

AccountsManager::~AccountsManager() {
	foreach (MainController* controller, mainControllers_) {
		delete controller;
	}
}

JID AccountsManager::getDefaultJid() {
	return defaultAccountJid_;
}

Account AccountsManager::getAccountByJid(std::string jid) {
	foreach (Account account, accounts_) {
		if (account.jid_ == jid)
			return account;
	}
	return Account();
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
