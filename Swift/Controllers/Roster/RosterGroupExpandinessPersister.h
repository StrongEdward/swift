/*
 * Copyright (c) 2010-2012 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <set>
#include <Swift/Controllers/Roster/CollapsedRosterItemsSet.h>
#include "Swift/Controllers/Roster/Roster.h"
#include "Swift/Controllers/Settings/SettingsProvider.h"
#include <Swift/Controllers/Roster/AccountRosterItem.h>

namespace Swift {
	class RosterGroupExpandinessPersister {
		public:
			RosterGroupExpandinessPersister(Roster* roster, CollapsedRosterItemsSet* collapsed, SettingsProvider* settings);
			void setAccountItem(AccountRosterItem* accountItem);

		private:
			void handleExpandedChanged(GroupRosterItem* item, bool expanded);
			void handleGroupAdded(GroupRosterItem* item);

			void load();
			void save();

			Roster* roster_;
			CollapsedRosterItemsSet* collapsed_;
			AccountRosterItem* accountItem_;

			SettingsProvider* settings_;
	};
}
