/*
 * Copyright (c) 2010-2013 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <map>
#include <set>
#include <string>

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include <Swiften/Base/boost_bsignals.h>
#include <Swiften/Elements/Presence.h>
#include <Swiften/Elements/StatusShow.h>
#include <Swiften/Elements/MUCOccupant.h>
#include <Swiften/Elements/VCard.h>
#include <Swiften/JID/JID.h>

#include <Swift/Controllers/Roster/RosterItem.h>

namespace Swift {

class GroupRosterItem;
class ContactRosterItem : public RosterItem {
	public:
		enum Feature {
			FileTransferFeature,
			WhiteboardFeature
		};

		enum BlockState {
			BlockingNotSupported,
			IsBlocked,
			IsUnblocked,
			IsDomainBlocked
		};
		
	public:
		ContactRosterItem(const JID& jid, const JID& displayJID, const std::string& name, GroupRosterItem* parent, int rosterId);
		virtual ~ContactRosterItem();

		StatusShow::Type getStatusShow() const;
		StatusShow::Type getSimplifiedStatusShow() const;
		std::string getStatusText() const;
		std::string getIdleText() const;
		std::string getOfflineSinceText() const;
		void setAvatarPath(const boost::filesystem::path& path);
		const boost::filesystem::path& getAvatarPath() const;
		const JID& getJID() const;
		void setDisplayJID(const JID& jid);
		const JID& getDisplayJID() const;
		void applyPresence(const std::string& resource, boost::shared_ptr<Presence> presence);
		void clearPresence();
		void calculateShownPresence();
		const std::vector<std::string>& getGroups() const;
		/** Only used so a contact can know about the groups it's in*/
		void addGroup(const std::string& group);
		void removeGroup(const std::string& group);

		MUCOccupant::Role getMUCRole() const;
		void setMUCRole(const MUCOccupant::Role& role);
		MUCOccupant::Affiliation getMUCAffiliation() const;
		void setMUCAffiliation(const MUCOccupant::Affiliation& affiliation);
		std::string getMUCAffiliationText() const;

		void setSupportedFeatures(const std::set<Feature>& features);
		bool supportsFeature(Feature feature) const;

		void setBlockState(BlockState state);
		BlockState blockState() const;

		VCard::ref getVCard() const;
		void setVCard(VCard::ref vcard);

		boost::signal<void ()> onVCardRequested;

	private:
		JID jid_;
		JID displayJID_;
		boost::posix_time::ptime lastAvailableTime_;
		boost::filesystem::path avatarPath_;
		std::map<std::string, boost::shared_ptr<Presence> > presences_;
		boost::shared_ptr<Presence> offlinePresence_;
		boost::shared_ptr<Presence> shownPresence_;
		std::vector<std::string> groups_;
		MUCOccupant::Role mucRole_;
		MUCOccupant::Affiliation mucAffiliation_;
		std::set<Feature> features_;
		BlockState blockState_;
		VCard::ref vcard_;
};

}

