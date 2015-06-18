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

	/*(QtAccountDetailsWidget* details = new QtAccountDetailsWidget(this);
	QtAccountDetailsWidget* details2 = new QtAccountDetailsWidget(this);

	accountsLayout_->addWidget(details);
	accountsLayout_->addWidget(details2);*/
}

QtAccountsListWidget::~QtAccountsListWidget()
{
	delete ui;
}

void QtAccountsListWidget::setManager(AccountsManager* manager) {
	manager_ = manager;

	// Clear layout
	QLayoutItem* item;
	while ( (item = accountsLayout_->takeAt(0)) ) {
		delete item;
	}

	int size = manager_->accountsCount();
	for (int i = 0; i < size; i++) {
		boost::shared_ptr<Account> account = manager_->getAccountAt(i);

		accounts_.push_back(new QtAccountDetailsWidget(account, this));
		accountsLayout_->addWidget(accounts_.back());

	}
}



}
