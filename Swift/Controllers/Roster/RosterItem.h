/*
 * Copyright (c) 2010 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include "Swiften/Base/boost_bsignals.h"
#include <boost/shared_ptr.hpp>

#include <string>

namespace Swift {
class GroupRosterItem;
class RosterItem {
	public:
		RosterItem(const std::string& name, RosterItem* parent, int rosterId);
		virtual ~RosterItem();
		boost::signal<void ()> onDataChanged;
		RosterItem* getParent() const;
		void setDisplayName(const std::string& name);
		const std::string& getDisplayName() const;
		virtual const std::string& getSortableDisplayName() const;
		int getRosterIndex();
	private:
		std::string name_;
		std::string sortableDisplayName_;
		RosterItem* parent_;
		int rosterIndex_;
};

}

