/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <utility>
#include <vector>

#include <QAbstractItemModel>

#include <Swift/Controllers/Account.h>
#include <Swift/Controllers/Roster/AccountRosterItem.h>
#include <Swift/QtUI/Roster/RosterModel.h>

namespace Swift {
class QtTreeWidget;

class MultipleRosterProxyModel : public QAbstractItemModel {
		Q_OBJECT

	public:
		MultipleRosterProxyModel(QtTreeWidget* view, bool screenReaderMode);
		~MultipleRosterProxyModel();
		void addRoster(Roster* roster);
		void removeRoster(Roster* roster);
		AccountRosterItem* getAccountItem(const std::string& accountDisplayName) const;
		Qt::ItemFlags flags(const QModelIndex& index) const;
		int columnCount(const QModelIndex& parent = QModelIndex()) const;
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
		QModelIndex index(AccountRosterItem* item) const;
		QModelIndex parent(const QModelIndex& child) const;
		int rowCount(const QModelIndex& parent = QModelIndex()) const;
		QMimeData* mimeData(const QModelIndexList& indexes) const;

	signals:
		void itemExpanded(const QModelIndex& item, bool expanded);

	private slots:
		void handleItemExpandedInRosterModel(const QModelIndex& index, bool expanded);

		void handleDataChangedInRosterModel(const QModelIndex&topLeft, const QModelIndex&bottomRight);
		void handleLayoutAboutToBeChangedInRosterModel();
		void handleLayoutChangedInRosterModel();
		void handleModelAboutToBeResetInRosterModel();
		void handleModelResetInRosterModel();
		void handleAccountItemExpandedChanged();

	private:
		RosterItem* getItem(const QModelIndex& index) const;
		QModelIndex createThisModelIndex(const QModelIndex& index) const;
		void reLayout();
		int findPairIndexByRoster(Roster* roster);

	private:
		typedef std::pair<AccountRosterItem*, RosterModel*> AccountModelPair;
		std::vector<AccountModelPair> accounts_;

		QtTreeWidget* view_;
		bool screenReaderMode_;
};

}
