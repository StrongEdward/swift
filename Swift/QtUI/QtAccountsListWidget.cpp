/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include "QtAccountsListWidget.h"
#include "ui_QtAccountsListWidget.h"

#include <Swift/Controllers/AccountsManager.h>
#include <Swift/QtUI/QtAccountDetailsWidget.h>
#include <QFrame>

namespace Swift {

QtAccountsListWidget::QtAccountsListWidget(QWidget *parent)
	: QScrollArea(parent),
	  ui(new Ui::QtAccountsListWidget) {

	ui->setupUi(this);
	accountsLayout_ = ui->areaWidget_->layout();
	accountsLayout_->setAlignment(Qt::AlignTop);

	defaultGroup_ = NULL;
}

QtAccountsListWidget::~QtAccountsListWidget()
{
	delete ui;
	delete defaultGroup_;
}

void QtAccountsListWidget::setManager(AccountsManager* manager) {
	manager_ = manager;

	// Clear layout
	QLayoutItem* item;
	while ( (item = accountsLayout_->takeAt(0)) ) {
		delete item;
	}
	if (defaultGroup_) {
		delete defaultGroup_;
	}
	defaultGroup_ = new QButtonGroup();

	// Fill list with accounts
	int size = manager_->accountsCount();
	for (int i = 0; i < size; i++) {
		boost::shared_ptr<Account> account = manager_->getAccountAt(i);
		account->setIndex(i); // This ensures that all indices will be from 0 to n, not just sorted

		accounts_.push_back(new QtAccountDetailsWidget(account, defaultGroup_, this));
		accountsLayout_->addWidget(accounts_.back());

	}

	connect(defaultGroup_, SIGNAL(buttonClicked(int)), this, SLOT(handleDefaultButtonClicked(int)));

}

void QtAccountsListWidget::setDefaultAccount(int index) {
	accounts_.at(index)->setDefault();
}

void QtAccountsListWidget::handleDefaultButtonClicked(int id) {
	onDefaultButtonClicked(id);
}


}
