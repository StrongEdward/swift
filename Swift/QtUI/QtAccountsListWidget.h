/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <vector>

#include <QScrollArea>

//#include <3rdParty/Boost/src/boost/shared_ptr.hpp>

#include <Swift/QtUI/QtAccountDetailsWidget.h>

namespace Ui {
class QtAccountsListWidget;
}

namespace Swift {

class AccountsManager;

class QtAccountsListWidget : public QScrollArea
{
		Q_OBJECT

	public:
		explicit QtAccountsListWidget(QWidget *parent = 0);
		~QtAccountsListWidget();

		void setManager(AccountsManager* manager);

	private:
		Ui::QtAccountsListWidget *ui;
		QLayout* accountsLayout_;

		std::vector<QtAccountDetailsWidget*> accounts_;
		AccountsManager* manager_;
};

}
