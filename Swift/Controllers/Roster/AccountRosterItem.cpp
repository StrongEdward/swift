/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include "AccountRosterItem.h"

#include <Swift/Controllers/Roster/Roster.h>

namespace Swift {

AccountRosterItem::AccountRosterItem(boost::shared_ptr<Account> account, const std::string& name, GroupRosterItem* parent, int rosterId) : GroupRosterItem(name, parent, false, rosterId), account_(account) {

}

}
