/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include "AccountRosterItem.h"

#include <Swift/Controllers/Roster/Roster.h>
#include <Swift/QtUI/Roster/RosterModel.h>

namespace Swift {

AccountRosterItem::AccountRosterItem(boost::shared_ptr<Account> account, RosterModel* model, const std::string& name, GroupRosterItem* parent, int rosterId) : GroupRosterItem(name, parent, false, rosterId), account_(account), model_(model) {


}

RosterModel* AccountRosterItem::getModel() const {
	return model_;
}

Roster* AccountRosterItem::getRoster() const {
	return model_->getRoster();
}

}
