/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <Swift/Controllers/Account.h>
#include <Swift/Controllers/Roster/RosterItem.h>
#include <Swift/Controllers/Roster/GroupRosterItem.h>

namespace Swift {

class Roster;
class RosterModel;

class AccountRosterItem : public GroupRosterItem
{
	public:
		AccountRosterItem(boost::shared_ptr<Account> account, const std::string& name, GroupRosterItem* parent, int rosterId);

	private:
		boost::shared_ptr<Account> account_;
};

}
