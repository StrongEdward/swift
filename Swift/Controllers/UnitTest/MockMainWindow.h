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

#include <Swift/Controllers/UIInterfaces/MainWindow.h>

namespace Swift {
	class Roster;
	class MockMainWindow : public MainWindow {
		public:
			MockMainWindow() : roster(NULL) {}
			virtual ~MockMainWindow() {}
			virtual void addRoster(Roster* roster) {this->roster = roster;}
			virtual void removeRoster(Roster*) {}
			virtual void setMyNick(const std::string& /*name*/) {}
			virtual void setMyJID(const JID& /*jid*/) {}
			virtual void setMyAvatarPath(const std::string& /*path*/) {}
			virtual void setMyStatusText(const std::string& /*status*/) {}
			virtual void setMyStatusType(StatusShow::Type /*type*/) {}
			virtual void setMyContactRosterItem(boost::shared_ptr<ContactRosterItem> /*contact*/) {}
			virtual void setAvailableAdHocCommands(const std::vector<DiscoItems::Item>& /*commands*/) {}
			virtual void setConnecting() {}
			virtual void setStreamEncryptionStatus(bool /*tlsInPlaceAndValid*/) {}
			virtual void openCertificateDialog(const std::vector<Certificate::ref>& /*chain*/) {}
			virtual void setBlockingCommandAvailable(bool /*isAvailable*/) {}
			Roster* roster;

	};
}
