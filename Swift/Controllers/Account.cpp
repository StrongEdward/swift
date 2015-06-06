/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <Swiften/Base/Algorithm.h>

#include <Swift/Controllers/Account.h>

namespace Swift {

Account::Account(const std::string accountName,
				 const std::string jid,
				 const std::string password,
				 const std::string certificatePath,
				 const ClientOptions options,
				 bool rememberPassword,
				 bool enabledAutoLogin)
	: accountName_(accountName),
	  jid_(JID(jid)),
	  password_(password),
	  certificatePath_(certificatePath),
	  options_(options),
	  rememberPassword_(rememberPassword),
	  enabledAutoLogin_(enabledAutoLogin) {

}

void Account::clearPassword() {
	safeClear(password_);
}

std::string Account::getAccountName() {
	return accountName_;
}

const JID& Account::getJID() {
	return jid_;
}


} // namespace Swift
