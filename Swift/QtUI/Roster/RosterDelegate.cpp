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

#include "RosterDelegate.h"

#include <QApplication>
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QFontMetrics>
#include <QPainterPath>
#include <QPolygon>
#include <qdebug.h>
#include <QBitmap>

#include "Swift/Controllers/Roster/ContactRosterItem.h"
#include "Swift/Controllers/Roster/GroupRosterItem.h"
#include "Swift/QtUI/Roster/AccountRosterItem.h"
#include <Swift/QtUI/QtSwiftUtil.h>

#include "QtTreeWidget.h"
#include "RosterModel.h"

namespace Swift {

RosterDelegate::RosterDelegate(QtTreeWidget* tree, bool compact) : compact_(compact) {
	tree_ = tree;
	groupDelegate_ = new GroupItemDelegate();
	accountDelegate_ = new AccountItemDelegate();
}

RosterDelegate::~RosterDelegate() {
	delete groupDelegate_;
	delete accountDelegate_;
}

void RosterDelegate::setCompact(bool compact) {
	compact_ = compact;
	emit sizeHintChanged(QModelIndex());
}
	
QSize RosterDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const {
	RosterItem* item = static_cast<RosterItem*>(index.internalPointer());
	if (dynamic_cast<AccountRosterItem*>(item)) {
		return accountDelegate_->sizeHint(option, index);
	}
	if (dynamic_cast<GroupRosterItem*>(item)) {
		return groupDelegate_->sizeHint(option, index);
	}
	return contactSizeHint(option, index);
}

QSize RosterDelegate::contactSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index ) const {
	return common_.contactSizeHint(option, index, compact_);
}

void RosterDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	RosterItem* item = static_cast<RosterItem*>(index.internalPointer());
	//qDebug() << "Paint " << P2QSTRING(item->getDisplayName());
	if (dynamic_cast<AccountRosterItem*>(item)) {
		paintAccount(painter, option, index);
	}
	else if (dynamic_cast<GroupRosterItem*>(item)) {
		paintGroup(painter, option, index);
	}
	else {
		paintContact(painter, option, index);
	}
}

void RosterDelegate::paintAccount(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	if (index.isValid()) {
		accountDelegate_->paint(painter, option, index.data(Qt::DisplayRole).toString(), 99, tree_->isExpanded(index));
	}
}

void RosterDelegate::paintGroup(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	if (index.isValid()) {
		groupDelegate_->paint(painter, option, index.data(Qt::DisplayRole).toString(), index.data(ChildCountRole).toInt(), tree_->isExpanded(index));
	}
}

void RosterDelegate::paintContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	QColor nameColor = index.data(Qt::TextColorRole).value<QColor>();
	QString avatarPath;
	if (index.data(AvatarRole).isValid() && !index.data(AvatarRole).value<QString>().isNull()) {
		avatarPath = index.data(AvatarRole).value<QString>();
	}
	QIcon presenceIcon = index.data(PresenceIconRole).isValid() && !index.data(PresenceIconRole).value<QIcon>().isNull()
			? index.data(PresenceIconRole).value<QIcon>()
			: QIcon(":/icons/offline.png");
	bool isIdle = index.data(IdleRole).isValid() ? index.data(IdleRole).toBool() : false;
	QString name = index.data(Qt::DisplayRole).toString();
	QString statusText = index.data(StatusTextRole).toString();
	common_.paintContact(painter, option, nameColor, avatarPath, presenceIcon, name, statusText, isIdle, 0, compact_);
}

}

