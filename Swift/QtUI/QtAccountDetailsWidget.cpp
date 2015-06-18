/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <QtSwiftUtil.h>

#include <Swift/QtUI/QtAccountDetailsWidget.h>

#include <QPixmap>

namespace Swift {

QtAccountDetailsWidget::QtAccountDetailsWidget(boost::shared_ptr<Account> account, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::QtAccountDetailsWidget),
	triangle_(new QtTreeviewTriangle),
	account_(account)
{
	ui->setupUi(this);
	ui->accountLayout_->setAlignment(Qt::AlignVCenter);
	ui->accountNameLabel_->show();
	ui->accountName_->hide();
	ui->extendingWidget_->hide();

	ui->connectionOptions_->setIcon(QIcon(":/icons/actions.png"));
	ui->certificateButton_->setIcon(QIcon(":/icons/certificate.png"));
	ui->statusIcon_->setPixmap(QPixmap(":/icons/offline.png"));
	ui->accountLayout_->insertWidget(0, triangle_);

	connect(triangle_, &QtTreeviewTriangle::clicked, this, &QtAccountDetailsWidget::triangleClicked);

	// Setting editor
	ui->accountNameLabel_->setText(P2QSTRING(account_->getAccountName()));
	ui->accountName_->setText(P2QSTRING(account->getAccountName()));
	ui->userAddress_->setText(P2QSTRING(account_->getJID().toString()));
	ui->password_->setText(P2QSTRING(account_->getPassword()));
	ui->rememberCheck_->setChecked(!account_->forgetPassword());
	ui->enabledCheck_->setChecked(account_->getLoginAutomatically());
	//ui->defaultRadio_->setChecked(account_->);
	//Connection options
	//certificate
	//colour

}

QtAccountDetailsWidget::~QtAccountDetailsWidget()
{
	delete triangle_;
	delete ui;
}

QSize QtAccountDetailsWidget::sizeHint() const {
	return minimumSizeHint();
}

QSize QtAccountDetailsWidget::minimumSizeHint() const {
	if (triangle_->isExpanded()) {
		return QSize(220, 28 + 3*22 + 2*2);
	} else {
		return QSize(220, 28);
	}
}

void QtAccountDetailsWidget::triangleClicked() {
	if (triangle_->isExpanded()) { // show expanded
		ui->extendingWidget_->show();

		ui->accountName_->show();
		ui->accountNameLabel_->hide();

		//QWidget* par = parentWidget();
		//par->updateGeometry();
	} else { // show collapsed

		ui->accountNameLabel_->show();
		ui->extendingWidget_->hide();
		ui->accountNameLabel_->setText(ui->accountName_->text());
		ui->accountName_->hide();

		//parentWidget()->updateGeometry();
	}

}

}
