/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include "Account.h"

namespace Swift {

	Account::Account(const std::string& accountName,
					 const std::string& jid,
					 const std::string& password,
					 const std::string& certificatePath,
					 const ClientOptions& options,
					 bool rememberPassword,
					 bool enabledAutoLogin)
		: accountName_(accountName),
		  jid_(jid),
		  password_(password),
		  certificatePath_(certificatePath),
		  options_(options),
		  rememberPassword_(rememberPassword),
		  enabledAutoLogin_(enabledAutoLogin) {

	}

} // namespace Swift
