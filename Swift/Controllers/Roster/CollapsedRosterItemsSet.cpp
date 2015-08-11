/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <utility>

#include <Swift/Controllers/Roster/CollapsedRosterItemsSet.h>

namespace Swift {

CollapsedRosterItemsSet::CollapsedRosterItemsSet() {

}

void CollapsedRosterItemsSet::addGroup(const std::string& accountDisplayName, const std::string& group) {
	int accountIndex = findAccount(accountDisplayName);
	if (accountIndex == -1) {
		accountIndex = addAccount(accountDisplayName);
	}
	items_[accountIndex].second.insert(group);
}

void CollapsedRosterItemsSet::removeGroup(const std::string& accountDisplayName, const std::string& group) {
	int accountIndex = findAccount(accountDisplayName);
	if (accountIndex == -1) {
		return;
	}
	items_[accountIndex].second.erase(group);
}

bool CollapsedRosterItemsSet::containsGroup(const std::string& accountDisplayName, const std::string& group) const {
	int accountIndex = findAccount(accountDisplayName);
	if (accountIndex == -1) {
		return false;
	}
	return items_[accountIndex].second.find(group) != items_[accountIndex].second.end();
}

int CollapsedRosterItemsSet::addAccount(const std::string& accountDisplayName, bool expanded) {
	int accountIndex = findAccount(accountDisplayName);
	if (accountIndex != -1) {
		accountExpandiness_[accountIndex] = expanded;
		return accountIndex;
	}
	items_.push_back(std::make_pair(accountDisplayName, std::set<std::string>()));
	accountExpandiness_.push_back(expanded);
	return items_.size() - 1;
}

void CollapsedRosterItemsSet::removeAccount(const std::string& accountDisplayName) {
	int accountIndex = findAccount(accountDisplayName);
	if (accountIndex != -1) {
		items_.erase(items_.begin() + accountIndex);
		accountExpandiness_.erase(accountExpandiness_.begin() + accountIndex);
	}
}

void CollapsedRosterItemsSet::setAccountExpanded(const std::string& accountDisplayName, bool expanded) {
	int accountIndex = findAccount(accountDisplayName);
	if (accountIndex != -1) {
		accountExpandiness_[accountIndex] = expanded;
	}
}

bool CollapsedRosterItemsSet::getAccountExpanded(const std::string& accountDisplayName) const {
	int accountIndex = findAccount(accountDisplayName);
	if (accountIndex != -1) {
		return accountExpandiness_[accountIndex];
	}
	return false; //Default: collapsed
}

int CollapsedRosterItemsSet::findAccount(const std::string& accountName) const {
	for (unsigned int i = 0; i < items_.size(); i++) {
		if (items_[i].first == accountName) {
			return i;
		}
	}
	return -1;
}

}
