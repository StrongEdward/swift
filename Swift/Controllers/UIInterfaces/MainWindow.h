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

#include <string>

#include <boost/shared_ptr.hpp>

#include <Swiften/JID/JID.h>
#include <Swiften/Elements/StatusShow.h>
#include <Swiften/Elements/DiscoItems.h>
#include <Swiften/TLS/Certificate.h>
#include <Swiften/Base/boost_bsignals.h>
#include <Swift/Controllers/Roster/ContactRosterItem.h>

namespace Swift {
	class Roster;

	class MainWindow {
		public:
			MainWindow(bool candelete = true) : canDelete_(candelete) {}
			virtual ~MainWindow() {}

			bool canDelete() const {
				return canDelete_;
			}

			virtual void setMyNick(const std::string& name) = 0;
			virtual void setMyJID(const JID& jid) = 0;
			virtual void setMyAvatarPath(const std::string& path) = 0;
			virtual void setMyStatusText(const std::string& status) = 0;
			virtual void setMyStatusType(StatusShow::Type type) = 0;
			virtual void setMyContactRosterItem(boost::shared_ptr<ContactRosterItem> contact) = 0;
			/** Must be able to cope with NULL to clear the roster */
			virtual void addRoster(Roster* roster) = 0;
			virtual void removeRoster(Roster* roster) = 0;
			virtual void setConnecting() = 0;
			virtual void setBlockingCommandAvailable(bool isAvailable) = 0;
			virtual void setAvailableAdHocCommands(const std::vector<DiscoItems::Item>& commands) = 0;
			virtual void setStreamEncryptionStatus(bool tlsInPlaceAndValid) = 0;
			virtual void openCertificateDialog(const std::vector<Certificate::ref>& chain) = 0;
			
			boost::signal<void (StatusShow::Type, const std::string&)> onChangeStatusRequest;
			boost::signal<void ()> onSignOutRequest;
			boost::signal<void ()> onShowCertificateRequest;

		private:
			bool canDelete_;
	};
}
