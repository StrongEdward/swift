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

#include <QStyledItemDelegate>
#include <QColor>
#include <QFont>

#include "AccountItemDelegate.h"
#include "GroupItemDelegate.h"
#include "DelegateCommons.h"

namespace Swift {
	class QtTreeWidget;
	class RosterDelegate : public QStyledItemDelegate {
	public:
		RosterDelegate(QtTreeWidget* tree, bool compact, bool multiaccountGui);
		~RosterDelegate();
		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	public slots:
		void setCompact(bool compact);
	private:
		QSize contactSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
		void paintAccount(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
		void paintGroup(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const; 
		void paintContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const; 
		bool compact_;
		bool multiaccountGui_;
		DelegateCommons common_;
		GroupItemDelegate* groupDelegate_;
		AccountItemDelegate* accountDelegate_;
		QtTreeWidget* tree_;
	};
}
