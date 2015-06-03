/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include "AccountsManager.h"

#include <Swiften/Base/String.h>
#include <Swiften/StringCodecs/Base64.h>

#include <Swift/Controllers/Settings/SettingsProviderHierachy.h>
#include <Swift/Controllers/ProfileSettingsProvider.h>
#include <Swift/Controllers/SettingConstants.h>

#include <Swift/Controllers/UIInterfaces/LoginWindow.h>

namespace Swift {

	AccountsManager::AccountsManager(SettingsProviderHierachy* settings, LoginWindow* loginWindow) : settings_(settings), loginWindow_(loginWindow) {

		std::string lastLoginJID = settings_->getSetting(SettingConstants::LAST_LOGIN_JID);

		foreach (std::string profile, settings_->getAvailableProfiles()) {
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
	}

	std::string AccountsManager::getDefaultJid() {
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
