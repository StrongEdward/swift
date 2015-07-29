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

#include "Swift/Controllers/Roster/RosterItem.h"

#include <boost/algorithm/string.hpp>

#include "Swift/Controllers/Roster/GroupRosterItem.h"

namespace Swift {

RosterItem::RosterItem(const std::string& name, RosterItem* parent, int rosterId) : name_(name), sortableDisplayName_(boost::to_lower_copy(name_)), parent_(parent), rosterIndex_(rosterId) {
	/* The following would be good, but because of C++'s inheritance not working in constructors, it's not going to work. */
	//if (parent) {
	//	parent_->addChild(this);
	//}
}

RosterItem::~RosterItem() {

}

RosterItem* RosterItem::getParent() const {
	return parent_;
}

void RosterItem::setDisplayName(const std::string& name) {
	name_ = name;
	sortableDisplayName_ = boost::to_lower_copy(name_);
	onDataChanged();
}

const std::string& RosterItem::getDisplayName() const {
	return name_;
}

const std::string& RosterItem::getSortableDisplayName() const {
	return sortableDisplayName_;
}

int RosterItem::getRosterIndex() {
	return rosterIndex_;
}


}

