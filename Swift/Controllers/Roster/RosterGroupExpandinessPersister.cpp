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

#include <Swift/Controllers/Roster/RosterGroupExpandinessPersister.h>

#include <vector>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/bind.hpp>

#include <Swiften/Base/foreach.h>
#include <Swiften/Base/String.h>
#include <Swift/Controllers/Roster/GroupRosterItem.h>
#include <Swift/Controllers/SettingConstants.h>

namespace Swift {

RosterGroupExpandinessPersister::RosterGroupExpandinessPersister(Roster* roster, CollapsedRosterItemsSet* collapsed, SettingsProvider* settings) : roster_(roster), collapsed_(collapsed), accountItem_(NULL), settings_(settings) {
	load();
	roster_->onGroupAdded.connect(boost::bind(&RosterGroupExpandinessPersister::handleGroupAdded, this, _1));
}

void RosterGroupExpandinessPersister::handleGroupAdded(GroupRosterItem* item) {
	if (!accountItem_) {
		return;
	}
	if (collapsed_->containsGroup(accountItem_->getDisplayName(), item->getDisplayName())) {
		item->setExpanded(false);
	} else {
		item->setExpanded(true);
	}
	item->onExpandedChanged.connect(boost::bind(&RosterGroupExpandinessPersister::handleExpandedChanged, this, item, _1));
}

void RosterGroupExpandinessPersister::setAccountItem(AccountRosterItem* accountItem) {
	accountItem_ = accountItem;
	bool accountIsExpanded = settings_->getSetting(SettingConstants::MULTIACCOUNT_ENABLED) ? collapsed_->getAccountExpanded(accountItem_->getDisplayName()) : true; // Ensure account item is expanded in single account mode. Otherwise contacts may not be shown.
	collapsed_->addAccount(accountItem_->getDisplayName(), accountIsExpanded);
	accountItem_->setExpanded( collapsed_->getAccountExpanded(accountItem_->getDisplayName()) ); // To relayout
	accountItem_->onExpandedChanged.connect(boost::bind(&RosterGroupExpandinessPersister::handleExpandedChanged, this, accountItem, _1));
	save();
}

void RosterGroupExpandinessPersister::handleExpandedChanged(GroupRosterItem* item, bool expanded) {
	if (!accountItem_) {
		return;
	}

	if (dynamic_cast<AccountRosterItem*>(item)) {
		collapsed_->setAccountExpanded(item->getDisplayName(), expanded);
		save();
		return;
	}

	if (expanded) {
		collapsed_->removeGroup(accountItem_->getDisplayName(), item->getDisplayName());
	} else {
		collapsed_->addGroup(accountItem_->getDisplayName(), item->getDisplayName());
	}
	save();
}

void RosterGroupExpandinessPersister::save() {
	std::stringstream stream;
	boost::archive::text_oarchive archive(stream);
	archive << collapsed_;

	settings_->storeSetting(SettingConstants::COLLAPSED_ROSTER_ITEMS, stream.str());
}

void RosterGroupExpandinessPersister::load() {
	std::string collapsedSerialized = settings_->getSetting(SettingConstants::COLLAPSED_ROSTER_ITEMS);
	std::stringstream stream;
	stream << collapsedSerialized;
	try {
		boost::archive::text_iarchive archive(stream);
		archive >> collapsed_;
	}
	catch (...) {}
}

}
