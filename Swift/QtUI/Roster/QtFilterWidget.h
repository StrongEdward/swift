/*
 * Copyright (c) 2013 Tobias Markmann
 * Licensed under the simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <vector>

#include <QBoxLayout>
#include <QWidget>

#include <Swift/Controllers/Roster/RosterFilter.h>
#include <Swift/Controllers/Roster/FuzzyRosterFilter.h>

#include <Swift/QtUI/Roster/MultipleRosterProxyModel.h>
#include <Swift/QtUI/Roster/QtRosterWidget.h>

namespace Swift {
class UIEventStream;
class QtClosableLineEdit;
class QtFilterWidget : public QWidget {
	Q_OBJECT
	public:
		QtFilterWidget(QWidget* parent, QtRosterWidget* treeView, UIEventStream* eventStream, QBoxLayout* layout = 0);
		virtual ~QtFilterWidget();

	protected:
		bool eventFilter(QObject*, QEvent* event);

	private:
		void popAllFilters();
		void pushAllFilters();

		void updateRosterFilters();
		void updateSearchFilter();

		void handleFilterAdded(RosterFilter* filter);
		void handleFilterRemoved(RosterFilter* filter);

	private:
		QtClosableLineEdit* filterLineEdit_;
		QtRosterWidget* treeView_;
		UIEventStream* eventStream_;
		std::vector<RosterFilter*> filters_;
		MultipleRosterProxyModel* sourceModel_;
		FuzzyRosterFilter* fuzzyRosterFilter_;
		bool isModifierSinglePressed_;
};

}
