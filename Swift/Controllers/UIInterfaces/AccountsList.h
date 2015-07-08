/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include <Swiften/Base/boost_bsignals.h>

#include <Swift/Controllers/Account.h>
#include <Swift/Controllers/AccountsManager.h>

namespace Swift {

	class AccountsList {
		public:
			virtual ~AccountsList() {}

			virtual void setManager(AccountsManager* manager) = 0;
			virtual void addAccountToList(boost::shared_ptr<Account> account) = 0;
			virtual void removeAccountFromList(int index) = 0;
			virtual void reloadAccounts() = 0;
			virtual void setDefaultAccount(int index) = 0;

			boost::signal<void (int)> onAccountWantsToBeDefault;
	};
}