/*
 * Copyright (c) 2011 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include "Swift/QtUI/Roster/QtTreeWidget.h"

namespace Swift {
class QtUIPreferences;

class QtRosterWidget : public QtTreeWidget {
	Q_OBJECT
	public:
		QtRosterWidget(UIEventStream* eventStream, SettingsProvider* settings, QWidget* parent = 0);
		virtual ~QtRosterWidget();

		void setRosterModel(Roster* roster); // override
		void addRoster(Roster* roster);
		void removeRoster(Roster* roster);
		AccountRosterItem* getAccountItem(const std::string accountDisplayName) const;

	public slots:
		void handleEditUserActionTriggered(bool checked);
	protected:
		void contextMenuEvent(QContextMenuEvent* event);
	private:
		void renameGroup(GroupRosterItem* group);
};

}
