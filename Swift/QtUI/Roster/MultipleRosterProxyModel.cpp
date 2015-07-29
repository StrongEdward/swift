/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

/*
 * Copyright (c) 2010-2014 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#include <Swift/QtUI/Roster/MultipleRosterProxyModel.h>

#include <qdebug.h>

#include <Swift/QtUI/Roster/QtTreeWidget.h>
#include <Swift/QtUI/QtSwiftUtil.h>
#include <Swift/Controllers/Roster/GroupRosterItem.h>

namespace Swift {

MultipleRosterProxyModel::MultipleRosterProxyModel(QtTreeWidget* view, bool screenReaderMode) : view_(view), screenReaderMode_(screenReaderMode) {
	root_ = new GroupRosterItem("Main Root", NULL, true, -1); //sortbystatus default true
}

MultipleRosterProxyModel::~MultipleRosterProxyModel() {
	delete root_;
	// Delete rosters!
}

void MultipleRosterProxyModel::addRoster(Roster* roster) {
	// Make sure it won't fail when doing some add/remove roster operations (possible solution: repair indices after removing from middle).
	if (!roster) {
		return;
	}

	roster->setIndex(accounts_.size());
	//roster->onChildrenChanged.connect(boost::bind(&MultipleRosterProxyModel::handleChildrenChanged, this, _1));
	//roster->onDataChanged.connect(boost::bind(&MultipleRosterProxyModel::handleDataChanged, this, _1));

	RosterModel* rosterModel = new RosterModel(view_, screenReaderMode_);
	rosterModel->setRoster(roster);
	connect(rosterModel, SIGNAL(itemExpanded(const QModelIndex&, bool)), this, SLOT(handleItemExpandedInRosterModel(const QModelIndex&, bool)));

	connect(rosterModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(handleDataChangedInRosterModel(const QModelIndex&, const QModelIndex&)));
	connect(rosterModel, SIGNAL(layoutAboutToBeChanged()), this, SLOT(handleLayoutAboutToBeChangedInRosterModel()));
	connect(rosterModel, SIGNAL(layoutChanged()), this, SLOT(handleLayoutChangedInRosterModel()));
	connect(rosterModel, SIGNAL(modelAboutToBeReset()), this, SLOT(handleModelAboutToBeResetInRosterModel()));
	connect(rosterModel, SIGNAL(modelReset()), this, SLOT(handleModelResetInRosterModel()));

	AccountRosterItem* accountItem = new AccountRosterItem(roster->getAccount(), rosterModel, roster->getAccount()->getAccountName(), static_cast<GroupRosterItem*>(root_), roster->getIndex());

	accounts_.push_back(accountItem);

	//reLayout();
}

Qt::ItemFlags MultipleRosterProxyModel::flags(const QModelIndex& index) const {
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);
	RosterItem* item = getItem(index);
	if (item && static_cast<size_t>(item->getRosterIndex()) < accounts_.size()) {
		flags |= accounts_[item->getRosterIndex()]->getModel()->flags(index);
	}
	return flags;
}

int MultipleRosterProxyModel::columnCount(const QModelIndex&) const {
	return 1;
}

QVariant MultipleRosterProxyModel::data(const QModelIndex& index, int role) const {
	RosterItem* item = getItem(index);
	if (!item) {
		return QVariant();
	}

	Q_ASSERT(static_cast<size_t>(item->getRosterIndex()) < accounts_.size());
	return accounts_[item->getRosterIndex()]->getModel()->data(index, role);
}

QModelIndex MultipleRosterProxyModel::index(int row, int column, const QModelIndex& parent) const {
	if (!accounts_.size()) {
		return QModelIndex();
	}

	RosterItem* parentItem;
	if (!parent.isValid()) {
		// Top level
		Q_ASSERT(row >= 0 && static_cast<size_t>(row) < accounts_.size());
		return createIndex(row, column, accounts_[row]);
	}
	else {
		parentItem = getItem(parent);
		Q_ASSERT(parentItem);
		/*if (!parentItem) {
			return QModelIndex();
		}*/

		int rosterIndex = parentItem->getRosterIndex();
		Q_ASSERT(static_cast<size_t>(rosterIndex) < accounts_.size());

		if (dynamic_cast<AccountRosterItem*>(parentItem)) {
			return createThisModelIndex(accounts_[rosterIndex]->getModel()->index(row, column, QModelIndex()));
		}
		return createThisModelIndex(accounts_[rosterIndex]->getModel()->index(row, column, parent));
	}
}

QModelIndex MultipleRosterProxyModel::index(AccountRosterItem* item) const {
	if (item == dynamic_cast<AccountRosterItem*>(root_)) {
		return QModelIndex();
	}
	return createIndex(item->getRosterIndex(), 0, item);
}

QModelIndex MultipleRosterProxyModel::parent(const QModelIndex& child) const {
	if (!accounts_.size() || !child.isValid() || dynamic_cast<AccountRosterItem*>(getItem(child)) ) {
		return QModelIndex();
	}

	RosterItem* childItem = getItem(child);
	Q_ASSERT(childItem);
	Q_ASSERT(static_cast<size_t>(childItem->getRosterIndex()) < accounts_.size());

	// Assuming childItem is GroupRosterItem or ContactRosterItem...
	QModelIndex returnedParent = createThisModelIndex(accounts_[childItem->getRosterIndex()]->getModel()->parent(child));
	if (returnedParent.isValid()) {
		return returnedParent;
	}
	else {
		// child's parent is RosterModel root so we connect child (GroupRosterItem) to corresponding account item
		return createIndex(childItem->getRosterIndex(), 0, accounts_[childItem->getRosterIndex()]);
	}
}

int MultipleRosterProxyModel::rowCount(const QModelIndex& parent) const {
	if (!accounts_.size()) {
		return 0;
	}

	if (!parent.isValid()) {
		return accounts_.size();
	}

	RosterItem* item = getItem(parent);
	Q_ASSERT(item);

	int rosterIndex = item->getRosterIndex();
	Q_ASSERT(static_cast<size_t>(rosterIndex) < accounts_.size());

	if (dynamic_cast<AccountRosterItem*>(item)) {
		return accounts_[rosterIndex]->getModel()->rowCount();
	}
	return accounts_[rosterIndex]->getModel()->rowCount(parent);
}

QMimeData* MultipleRosterProxyModel::mimeData(const QModelIndexList& indexes) const {
	QMimeData* data = QAbstractItemModel::mimeData(indexes);

	if (accounts_.empty()) {
		return data;
	}

	ContactRosterItem *item = dynamic_cast<ContactRosterItem*>(getItem(indexes.first()));
	if (item == NULL) {
		return data;
	}

	/* only a single JID in this list */
	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);
	dataStream << P2QSTRING(item->getJID().toString());
	data->setData("application/vnd.swift.contact-jid-list", itemData);
	return data;
}

RosterItem* MultipleRosterProxyModel::getItem(const QModelIndex& index) const {
	return index.isValid() ? static_cast<RosterItem*>(index.internalPointer()) : NULL;
}

QModelIndex MultipleRosterProxyModel::createThisModelIndex(const QModelIndex& index) const {
	if (index.isValid()) {
		return createIndex(index.row(), index.column(), index.internalPointer());
	}
	return QModelIndex();
}

void MultipleRosterProxyModel::handleItemExpandedInRosterModel(const QModelIndex& index, bool expanded) {
	emit itemExpanded(createThisModelIndex(index), expanded);
}

void MultipleRosterProxyModel::handleDataChangedInRosterModel(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
	emit dataChanged(createThisModelIndex(topLeft), createThisModelIndex(bottomRight));
}

void MultipleRosterProxyModel::handleLayoutAboutToBeChangedInRosterModel() {
	emit layoutAboutToBeChanged();
}

void MultipleRosterProxyModel::handleLayoutChangedInRosterModel() {
	emit layoutChanged();
}

void MultipleRosterProxyModel::handleModelAboutToBeResetInRosterModel() {
	beginResetModel();
}

void MultipleRosterProxyModel::handleModelResetInRosterModel() {
	endResetModel();
}



void MultipleRosterProxyModel::reLayout() {
	/*beginResetModel();

	foreach (AccountRosterItem* account, accounts_) {
		//AccountRosterItem* child = dynamic_cast<AccountRosterItem*>(model.second);
		//if (!child) continue;
		emit itemExpanded(index(account), account->isExpanded());
	}

	endResetModel();*/
}


}
