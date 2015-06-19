/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <Swiften/Base/boost_bsignals.h>

#include <Swift/Controllers/AccountsManager.h>

namespace Swift {
	class AccountsManager;

	class AccountsList {
		public:
			virtual ~AccountsList() {}

			virtual void setManager(AccountsManager* manager) = 0;
			virtual void setDefaultAccount(int index) = 0;

			boost::signal<void (int)> onDefaultButtonClicked;
	};
}
