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

#include <Swift/Controllers/Account.h>

#include <string>

#include <Swiften/Base/Algorithm.h>
#include <Swiften/Base/String.h>
#include <Swiften/StringCodecs/Base64.h>

#include <Swift/Controllers/ProfileSettingsProvider.h>
#include <Swift/Controllers/Settings/SettingsProvider.h>

namespace Swift {

Account::Account() : profileSettings_(NULL) {

}

Account::Account(const std::string& profile, SettingsProvider* settings, int index) : index_(index), settings_(settings), profileSettings_(new ProfileSettingsProvider(profile, settings)) {

	if (index_ >= 0) {
		profileSettings_->storeInt("index", index_);
	}
	else {
		index_ = profileSettings_->getIntSetting("index", 0);
	}

	jid_ = profileSettings_->getStringSetting("jid");
	accountName_ = profileSettings_->getStringSetting("accountname");
	if (accountName_.empty()) {
		accountName_ = jid_.getNode();
	}
	rememberPassword_ = profileSettings_->getIntSetting("remember", 0);
	if (rememberPassword_) {
		cachedPassword_ = profileSettings_->getStringSetting("pass");
	}
	else {
		cachedPassword_ = "";
	}
	certificatePath_ = profileSettings_->getStringSetting("certificate");
	clientOptions_ = parseClientOptions(profileSettings_->getStringSetting("options"));
	autoLogin_ = profileSettings_->getIntSetting("autologin", 0);
	show_ = static_cast<StatusShow::Type>(profileSettings_->getIntSetting("lastShow", StatusShow::Online));
	status_ = profileSettings_->getStringSetting("lastStatus");
	recentChatsSerialized_ = profileSettings_->getStringSetting("recent_chats");
	enabled_ = false;

	determineColor();
	storeAllSettings();
	onAccountDataChanged();
}

Account::Account(int index, const std::string& accountName, const std::string& jid, const std::string& password, const std::string& certificatePath, const ClientOptions& options, bool rememberPassword, bool autoLogin, /*bool enabled,*/ SettingsProvider* settings)
	: index_(index),
	  accountName_(accountName),
	  jid_(JID(jid)),
	  cachedPassword_(password),
	  certificatePath_(certificatePath),
	  clientOptions_(options),
	  rememberPassword_(rememberPassword),
	  autoLogin_(autoLogin),
	  show_(StatusShow::Online),
	  status_(""),
	  //enabled_(enabled),
	  settings_(settings),
	  profileSettings_(new ProfileSettingsProvider(accountName, settings_)) {

	//determineColor();
	storeAllSettings();
	onAccountDataChanged();
}

void Account::storeAllSettings() {
	profileSettings_->storeInt("index", index_);
	profileSettings_->storeString("accountname", accountName_);
	profileSettings_->storeString("jid", jid_);
	if (rememberPassword_) {
		profileSettings_->storeString("pass", cachedPassword_);
	}
	else {
		profileSettings_->storeString("pass", "");
	}
	profileSettings_->storeString("certificate", certificatePath_);
	profileSettings_->storeString("options", serializeClientOptions(clientOptions_));
	profileSettings_->storeInt("remember", rememberPassword_);
	profileSettings_->storeInt("autologin", autoLogin_);
	profileSettings_->storeInt("lastShow", show_);
	profileSettings_->storeString("lastStatus", status_);
	profileSettings_->storeInt("red", color_.red);
	profileSettings_->storeInt("green", color_.green);
	profileSettings_->storeInt("blue", color_.blue);
}

void Account::determineColor() {
	color_.red = profileSettings_->getIntSetting("red", -1);
	color_.green = profileSettings_->getIntSetting("green", -1);
	color_.blue = profileSettings_->getIntSetting("blue", -1);
	if (!color_.isValid()) {
		color_ = RGBColor();
	}
}

Account::~Account() {
	clearPassword();
	if (profileSettings_ != NULL) {
		delete profileSettings_;
	}
}

void Account::clearPassword() {
	safeClear(cachedPassword_);
}

int Account::getIndex() const {
	return index_;
}

std::string Account::getAccountName() const {
	return accountName_;
}

const JID& Account::getJID() const {
	return jid_;
}

const std::string& Account::getPassword() const {
	return cachedPassword_;
}

const std::string& Account::getCertificatePath() const {
	return certificatePath_;
}

const ClientOptions& Account::getClientOptions() const {
	return clientOptions_;
}

bool Account::getRememberPassword() const {
	return rememberPassword_;
}

bool Account::getLoginAutomatically() const {
	return autoLogin_;
}

StatusShow::Type Account::getShow() const {
	return show_;
}

std::string Account::getStatus() const {
	return status_;
}

RGBColor Account::getColor() const {
	return color_;
}

const std::string& Account::getRecentChatsSerialized() const {
	return recentChatsSerialized_;
}

bool Account::isEnabled() const {
	return enabled_;
}

ProfileSettingsProvider* Account::getProfileSettings() const {
	return profileSettings_;
}

void Account::setIndex(int newIndex) {
	if (newIndex >= 0) {
		index_ = newIndex;
	}
}

void Account::setAccountName(const std::string& newName) {
	accountName_ = newName;
	onAccountDataChanged();
	profileSettings_->storeString("accountname", newName);
}

void Account::setJID(const std::string& newJID) {
	jid_ = newJID;
	onAccountDataChanged();
	profileSettings_->storeString("jid", newJID);
}

void Account::setPassword(const std::string& newPassword) {
	cachedPassword_ = newPassword;
	if (rememberPassword_ || autoLogin_) {
		profileSettings_->storeString("pass", cachedPassword_);
	}
	onAccountDataChanged();
}

void Account::setCertificatePath(const std::string& newPath) {
	certificatePath_ = newPath;
	onAccountDataChanged();
	profileSettings_->storeString("certificate", newPath);
}

void Account::setClientOptions(const ClientOptions& newOptions) {
	clientOptions_ = newOptions;
	onAccountDataChanged();
	profileSettings_->storeString("options", serializeClientOptions(newOptions));
}

void Account::setRememberPassword(bool remember) {
	if (!remember) {
		clearPassword();
		profileSettings_->storeString("pass", "");
	}
	else {
		profileSettings_->storeString("pass", cachedPassword_);
	}
	rememberPassword_ = remember;
	onAccountDataChanged();
	profileSettings_->storeInt("remember", rememberPassword_);
}

void Account::setLoginAutomatically(bool autoLogin) {
	autoLogin_ = autoLogin;
	profileSettings_->storeInt("autologin", autoLogin_);
	if (autoLogin) {
		rememberPassword_ = true;
		profileSettings_->storeInt("remember", rememberPassword_);
	}
	onAccountDataChanged();
}

void Account::setShow(StatusShow::Type show) {
	show_ = show;
	onAccountDataChanged();
}

void Account::setStatus(std::string& status) {
	status_ = status;
	onAccountDataChanged();
}

void Account::setColor(RGBColor color) {
	color_ = color;
	onAccountDataChanged();
	profileSettings_->storeInt("red", color_.red);
	profileSettings_->storeInt("green", color_.green);
	profileSettings_->storeInt("blue", color_.blue);
}

void Account::setRecentChatsSerialized(const std::string& recentChatsSerialized) {
	recentChatsSerialized_ = recentChatsSerialized;
	profileSettings_->storeString("recent_chats", recentChatsSerialized_);
	onAccountDataChanged();
}

void Account::setEnabled(bool enabled) {
	bool wasEnabled = enabled_;
	enabled_ = enabled;
	if (!wasEnabled && enabled) {
		onEnabledChanged(true);
	}
	if (wasEnabled && !enabled) {
		// send disabled signal?
		onEnabledChanged(false);
	}
}

void Account::setProfileSettings(SettingsProvider* settings) {
	settings_ = settings;
	profileSettings_ = new ProfileSettingsProvider(jid_, settings_);
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
	SERIALIZE_BOOL(tlsOptions.schannelTLS1_0Workaround);
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
	PARSE_BOOL(tlsOptions.schannelTLS1_0Workaround, false);

	return result;
}


} // namespace Swift
