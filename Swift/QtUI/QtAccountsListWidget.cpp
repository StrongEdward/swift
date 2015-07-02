/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <Swift/QtUI/QtAccountsListWidget.h>

#include <QFrame>

#include <boost/bind.hpp>

#include <Swift/Controllers/AccountsManager.h>
#include <Swift/QtUI/QtAccountDetailsWidget.h>
#include <Swift/QtUI/ui_QtAccountsListWidget.h>

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
	reloadAccounts();
}

void QtAccountsListWidget::reloadAccounts() {
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
		//account->setIndex(i);

		accounts_.push_back(new QtAccountDetailsWidget(account, defaultGroup_, this));
		accountsLayout_->addWidget(accounts_.back());
		accounts_.back()->onWantsToBeDefault.connect(boost::bind(&QtAccountsListWidget::handleAccountWantsToBeDefault, this, _1));
		accounts_.back()->onWantsToBeDeleted.connect(boost::bind(&QtAccountsListWidget::handleAccountWantsToBeDeleted, this, _1));
	}

	connect(defaultGroup_, SIGNAL(buttonClicked(int)), this, SLOT(handleAccountWantsToBeDefault(int)));
}

void QtAccountsListWidget::addAccountToList(boost::shared_ptr<Account> account) {
	// need to check if account is on the list?
	bool found = false;
	for (int i = 0; i < accountsLayout_->count(); i++) {
		if (static_cast<QtAccountDetailsWidget*>(accountsLayout_->itemAt(i)->widget())->getUserAddress() == account->getJID().toString() ) {
			found = true;
		}
	}

	if (!found) {
		accounts_.push_back(new QtAccountDetailsWidget(account, defaultGroup_, this));
		accountsLayout_->addWidget(accounts_.back());
		accounts_.back()->onWantsToBeDefault.connect(boost::bind(&QtAccountsListWidget::handleAccountWantsToBeDefault, this, _1));
		accounts_.back()->onWantsToBeDeleted.connect(boost::bind(&QtAccountsListWidget::handleAccountWantsToBeDeleted, this, _1));
	}
}

void QtAccountsListWidget::removeAccountFromList(int index) {
	if (index > 0 && static_cast<unsigned int>(index) < accounts_.size()) {
		QtAccountDetailsWidget* widget = accounts_[index];

		widget->onWantsToBeDefault.disconnect(boost::bind(&QtAccountsListWidget::handleAccountWantsToBeDefault, this, _1));
		widget->onWantsToBeDeleted.disconnect(boost::bind(&QtAccountsListWidget::handleAccountWantsToBeDeleted, this, _1));

		accountsLayout_->removeWidget(widget);
		delete accounts_[index];
		accounts_.erase(accounts_.begin() + index);
	}
}

void QtAccountsListWidget::setDefaultAccount(int index) {
	if (index >= 0) {
		accounts_.at(index)->setDefaultAccountLook(true);
	}
}

void QtAccountsListWidget::handleAccountWantsToBeDefault(int index) {
	accounts_[ manager_->getDefaultAccount()->getIndex() ]->setDefaultAccountLook(false);
	accounts_[index]->setDefaultAccountLook(true);

	onAccountWantsToBeDefault(index);
}

void QtAccountsListWidget::handleAccountWantsToBeDeleted(const std::string& jid) {
	manager_->removeAccount(jid);
}

}
