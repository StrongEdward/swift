/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <vector>

#include <QScrollArea>
#include <QButtonGroup>

//#include <3rdParty/Boost/src/boost/shared_ptr.hpp>

#include <Swift/Controllers/UIInterfaces/AccountsList.h>
#include <Swift/QtUI/QtAccountDetailsWidget.h>

namespace Ui {
class QtAccountsListWidget;
}

namespace Swift {

class AccountsManager;

class QtAccountsListWidget : public QScrollArea, public AccountsList
{
		Q_OBJECT

	public:
		explicit QtAccountsListWidget(QWidget *parent = 0);
		~QtAccountsListWidget();

		virtual void setManager(AccountsManager* manager);
		virtual void reloadAccounts();
		virtual void addAccountToList(boost::shared_ptr<Account> account);
		virtual void removeAccountFromList(int index);
		virtual void setDefaultAccount(int index);

	private slots:
		void handleAccountWantsToBeDefault(int index);
		void handleAccountWantsToBeDeleted(const std::string& jid);

	private:
		Ui::QtAccountsListWidget *ui;
		QLayout* accountsLayout_;
		QButtonGroup* defaultGroup_;

		std::vector<QtAccountDetailsWidget*> accounts_; // Can it be removed so we will use accountsLayout_->itemAt() to get account widgets?
		AccountsManager* manager_;


};

}
