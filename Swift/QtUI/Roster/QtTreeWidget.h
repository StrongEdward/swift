/*
 * Copyright (c) 2010-2014 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QModelIndex>
#include <QTreeView>
 
#include <Swift/QtUI/Roster/RosterDelegate.h>
#include <Swift/QtUI/Roster/MultipleRosterProxyModel.h>

#include <Swift/Controllers/UIInterfaces/ChatWindow.h>

namespace Swift {
class UIEventStream;
class SettingsProvider;

class QtTreeWidget : public QTreeView {
	Q_OBJECT
	public:
		enum MessageTarget {MessageDefaultJID, MessageDisplayJID};

		QtTreeWidget(UIEventStream* eventStream, SettingsProvider* settings, MessageTarget messageTarget, QWidget* parent = 0);
		~QtTreeWidget();
		void show();
		QtTreeWidgetItem* getRoot();
		void setRosterModel(Roster* roster);
		Roster* getRoster() {return roster_;}
		void refreshTooltip();
		void setMessageTarget(MessageTarget messageTarget);
		JID jidFromIndex(const QModelIndex& index) const;
		JID selectedJID() const;
		void setOnline(bool isOnline);

	public:
		boost::signal<void (RosterItem*)> onSomethingSelectedChanged;

	private slots:
		void handleItemActivated(const QModelIndex&);
		void handleModelItemExpanded(const QModelIndex&, bool expanded);
		void handleExpanded(const QModelIndex&);
		void handleCollapsed(const QModelIndex&);
		void handleClicked(const QModelIndex&);
		void handleSettingChanged(const std::string& setting);
		void handleRefreshTooltip();

	protected:
		void dragEnterEvent(QDragEnterEvent* event);
		void dropEvent(QDropEvent* event);
		void dragMoveEvent(QDragMoveEvent* event);
		bool event(QEvent* event);
		QModelIndexList getSelectedIndexes() const;
		bool isOnline() const;

	private:
		void drawBranches(QPainter*, const QRect&, const QModelIndex&) const;

	protected slots:
		virtual void currentChanged(const QModelIndex& current, const QModelIndex& previous);
	protected:
		UIEventStream* eventStream_;

	private:
		MultipleRosterProxyModel* model_;
		//RosterModel* model_;
		Roster* roster_;
		RosterDelegate* delegate_;
		QtTreeWidgetItem* treeRoot_;
		SettingsProvider* settings_;
		bool tooltipShown_;
		MessageTarget messageTarget_;
		bool isOnline_;
};

}
