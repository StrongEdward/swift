/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <set>
#include <utility>
#include <vector>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/utility.hpp>

#include <Swift/Controllers/Roster/GroupRosterItem.h>
#include <Swift/Controllers/Settings/SettingsProvider.h>

namespace Swift {

class CollapsedRosterItemsSet
{
	public:
		CollapsedRosterItemsSet();

		void addGroup(const std::string& accountDisplayName, const std::string& group);
		void removeGroup(const std::string& accountDisplayName, const std::string& group);
		bool containsGroup(const std::string& accountDisplayName, const std::string& group) const;
		int addAccount(const std::string& accountDisplayName, bool expanded = true);
		void removeAccount(const std::string& accountDisplayName);

		void setAccountExpanded(const std::string& accountDisplayName, bool expanded);
		bool getAccountExpanded(const std::string& accountDisplayName) const;

	private:
		int findAccount(const std::string& accountName) const;

	private:
		typedef std::pair<std::string, std::set<std::string> > AccountGroupsPair;
		std::vector<AccountGroupsPair> items_;
		std::vector<bool> accountExpandiness_;

		SettingsProvider* settings_;

		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			ar & items_;
			ar & accountExpandiness_;
		}
};

}
