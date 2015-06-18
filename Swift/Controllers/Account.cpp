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

#include <string>

//#include <3rdParty/Boost/src/boost/shared_ptr.hpp>

#include <Swiften/Base/Algorithm.h>
#include <Swiften/Base/String.h>
#include <Swiften/StringCodecs/Base64.h>

#include <Swift/Controllers/Account.h>
#include <Swift/Controllers/ProfileSettingsProvider.h>

namespace Swift {

//int Account::maxIndex_ = 0;

Account::Account(ProfileSettingsProvider* profileSettings, int index) : index_(index), profileSettings_(profileSettings) {

	if (index_ >= 0) {
		profileSettings->storeInt("index", index_);
	} else {
		index_ = profileSettings_->getIntSetting("index", 0);
	}

	jid_ = profileSettings_->getStringSetting("jid");
	accountName_ = profileSettings->getStringSetting("accountname");
	if (accountName_.empty()) {
		accountName_ = JID(jid_).getNode();
	}
	password_ = profileSettings_->getStringSetting("pass");
	certificatePath_ = profileSettings_->getStringSetting("certificate");
	clientOptions_ = parseClientOptions(profileSettings->getStringSetting("options"));
	rememberPassword_ = profileSettings_->getIntSetting("remember", 0);
	autoLogin_ = profileSettings_->getIntSetting("autologin", 0);
	enabled_ = autoLogin_;
}

Account::Account(int index,
				 const std::string accountName,
				 const std::string jid,
				 const std::string password,
				 const std::string certificatePath,
				 const ClientOptions options,
				 bool rememberPassword,
				 bool autoLogin,
				 bool enabled,
				 ProfileSettingsProvider* profileSettings)
	: index_(index),
	  accountName_(accountName),
	  jid_(JID(jid)),
	  password_(password),
	  certificatePath_(certificatePath),
	  clientOptions_(options),
	  rememberPassword_(rememberPassword),
	  autoLogin_(autoLogin),
	  enabled_(enabled),
	  profileSettings_(profileSettings) {

	/*if (index_ == maxIndex_) {
		maxIndex_++;
	}*/
}

Account::~Account() {
	delete profileSettings_; // TODO: profileSettings as boost::shared_ptr, now it's created when passing to Account ctor
}

void Account::clearPassword() {
	safeClear(password_);
}

// Getters

int Account::getIndex() {
	return index_;
}

std::string Account::getAccountName() {
	return accountName_;
}

const JID& Account::getJID() {
	return jid_;
}

const std::string& Account::getPassword() {
	return password_;
}

const std::string& Account::getCertificatePath() {
	return certificatePath_;
}

const ClientOptions& Account::getClientOptions() {
	return clientOptions_;
}

bool Account::forgetPassword() {
	return !rememberPassword_;
}

bool Account::getLoginAutomatically() {
	return autoLogin_;
}

bool Account::isEnabled() {
	return enabled_;
}

ProfileSettingsProvider* Account::getProfileSettings() {
	return profileSettings_;
}

// Setters

void Account::setIndex(int newIndex) {
	if (newIndex >= 0) {
		index_ = newIndex;
	}
}

void Account::setAccountName(const std::string& newName) {
	accountName_ = newName;
	profileSettings_->storeString("accountname", newName);
}

void Account::setJID(const std::string& newJID) {
	jid_ = newJID;
	profileSettings_->storeString("jid", newJID);
}

void Account::setPassword(const std::string& newPassword) {
	password_ = newPassword;
	if (rememberPassword_ || autoLogin_) {
		profileSettings_->storeString("pass", newPassword);
	}
}

void Account::setCertificatePath(const std::string& newPath) {
	certificatePath_ = newPath;
	profileSettings_->storeString("certificate", newPath);
}

void Account::setClientOptions(const ClientOptions& newOptions) {
	clientOptions_ = newOptions;
	profileSettings_->storeString("options", serializeClientOptions(newOptions));
}

void Account::setRememberPassword(bool remember) {
	if (!remember) {
		clearPassword();
		profileSettings_->storeString("pass", "");
	} else {
		profileSettings_->storeString("pass", password_);
	}
	rememberPassword_ = remember;
	profileSettings_->storeInt("remember", rememberPassword_);
}

void Account::setLoginAutomatically(bool autoLogin) {
	autoLogin_ = autoLogin;
	profileSettings_->storeInt("autologin", autoLogin_);
	if (autoLogin) {
		rememberPassword_ = true;
	}
}

void Account::setEnabled(bool enabled) {


	if (enabled) {
		onEnabled();
	}
	if (enabled_ && !enabled) {
		// send disabled signal
	}
	enabled_ = enabled;
}


#define SERIALIZE_BOOL(option) result += options.option ? "1" : "0"; result += ",";
#define SERIALIZE_INT(option) result += boost::lexical_cast<std::string>(options.option); result += ",";
#define SERIALIZE_STRING(option) result += Base64::encode(createByteArray(options.option)); result += ",";
#define SERIALIZE_SAFE_STRING(option) result += safeByteArrayToString(Base64::encode(options.option)); result += ",";
#define SERIALIZE_URL(option) SERIALIZE_STRING(option.toString())

std::string Account::serializeClientOptions(const ClientOptions& options) {
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


ClientOptions Account::parseClientOptions(const std::string& optionString) {
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
