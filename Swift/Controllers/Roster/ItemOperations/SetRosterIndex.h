/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <Swift/Controllers/Roster/ItemOperations/RosterItemOperation.h>

namespace Swift {

class SetRosterIndex : public RosterItemOperation
{
	public:
		SetRosterIndex(const int rosterIndex) : RosterItemOperation(), rosterIndex_(rosterIndex) {}

		virtual void operator() (RosterItem* item) const {
			item->setRosterIndex(rosterIndex_);
		}

	private:
		const int rosterIndex_;
};

}
