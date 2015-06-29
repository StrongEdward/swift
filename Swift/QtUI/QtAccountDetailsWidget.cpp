/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

/*
 * Copyright (c) 2010 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#include <QFileDialog>
#include <QMessageBox>

#include <3rdParty/Boost/src/boost/bind.hpp>

#include <QtSwiftUtil.h>

#include <Swift/QtUI/QtAccountDetailsWidget.h>
#include <Swift/QtUI/QtConnectionSettingsWindow.h>
#include <Swift/QtUI/CAPICertificateSelector.h>

namespace Swift {

QtAccountDetailsWidget::QtAccountDetailsWidget(boost::shared_ptr<Account> account, QButtonGroup* buttonGroup, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::QtAccountDetailsWidget),
	triangle_(new QtTreeviewTriangle),
	color_(new QtAccountColorWidget),
	contextMenu_(new QMenu(this)),
	account_(account)
{
	ui->setupUi(this);
	ui->accountLayout_->setAlignment(Qt::AlignVCenter);
	ui->accountNameLabel_->show();
	ui->accountName_->hide();
	ui->extendingWidget_->hide();	

	ui->connectionOptions_->setIcon(QIcon(":/icons/actions.png"));

	ui->certificateButton_->setIcon(QIcon(":/icons/certificate.png"));
	ui->certificateButton_->setToolTip(tr("Click if you have a personal certificate used for login to the service."));
	ui->certificateButton_->setWhatsThis(tr("Click if you have a personal certificate used for login to the service."));
	ui->certificateButton_->setAccessibleName(tr("Login with certificate"));
	ui->certificateButton_->setAccessibleDescription(tr("Click if you have a personal certificate used for login to the service."));

	ui->statusIcon_->setPixmap(QPixmap(":/icons/offline.png"));
	ui->deleteButton_->setIcon(QIcon(":/icons/delete.ico"));
	connect(ui->deleteButton_, &QPushButton::clicked, this, &QtAccountDetailsWidget::handleDeleteButtonClicked);
	ui->accountLayout_->insertWidget(0, triangle_);

	connect(triangle_, &QtTreeviewTriangle::clicked, this, &QtAccountDetailsWidget::triangleClicked);

	// Setting editor
	ui->accountNameLabel_->setText(P2QSTRING(account_->getAccountName()));
	ui->accountName_->setText(P2QSTRING(account->getAccountName()));
	ui->userAddress_->setText(P2QSTRING(account_->getJID().toString()));
	ui->password_->setText(P2QSTRING(account_->getPassword()));
	ui->rememberCheck_->setChecked(!account_->forgetPassword());
	ui->enabledCheck_->setChecked(account_->getLoginAutomatically());

	connect(ui->accountName_, &QLineEdit::textEdited, this, &QtAccountDetailsWidget::handleAccountNameEdited);
	connect(ui->userAddress_, &QLineEdit::textEdited, this, &QtAccountDetailsWidget::handleUserAddressEdited);
	connect(ui->password_, &QLineEdit::textEdited, this, &QtAccountDetailsWidget::handlePasswordEdited);
	connect(ui->rememberCheck_, &QCheckBox::toggled, this, &QtAccountDetailsWidget::handleRememberPasswordToggled);
	connect(ui->enabledCheck_, &QCheckBox::toggled, this, &QtAccountDetailsWidget::handleAccountEnabledChanged);
	connect(ui->password_, &QLineEdit::returnPressed, this, &QtAccountDetailsWidget::handlePasswordReturnPressed);
	account_->onEnabledChanged.connect(boost::bind(&QtAccountDetailsWidget::handleAccountEnabledValueChanged, this, _1));

	buttonGroup->addButton(ui->defaultRadio_);
	buttonGroup->setId(ui->defaultRadio_, account_->getIndex());
	ui->defaultRadio_->hide(); // Because of new idea: bold default account name

	connect(ui->connectionOptions_, &QPushButton::clicked, this, &QtAccountDetailsWidget::handleCogwheelClicked);

	if (account_->getCertificatePath() != "") {
		ui->certificateButton_->setChecked(true);
	}
	connect(ui->certificateButton_, &QPushButton::clicked, this, &QtAccountDetailsWidget::handleCertificateChecked);

	ui->accountLayout_->insertWidget(1, color_);
	color_->setColor(account_->getColor());
	connect(color_, &QtAccountColorWidget::colorChanged, this, &QtAccountDetailsWidget::handleColorChanged);

	contextMenu_->addAction(tr("Set as default"), this, SLOT(handleSetAsDefault()));
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

std::string QtAccountDetailsWidget::getUserAddress() {
	return account_->getJID();
}

void QtAccountDetailsWidget::setDefaultAccountLook(bool isDefault) {
	if (isDefault) {
		ui->defaultRadio_->setChecked(true);
		ui->accountNameLabel_->setStyleSheet("*{font-weight: bold}");
	} else {
		ui->defaultRadio_->setChecked(false);
		ui->accountNameLabel_->setStyleSheet("");
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

void QtAccountDetailsWidget::handleCogwheelClicked() {
	QtConnectionSettingsWindow connectionSettings(account_->getClientOptions());
	if (connectionSettings.exec() == QDialog::Accepted) {
		account_->setClientOptions(connectionSettings.getOptions());
	}
}

void QtAccountDetailsWidget::handleCertificateChecked(bool checked) {
	QString certificateFile;
	if (checked) {
#ifdef HAVE_SCHANNEL
		certificateFile = P2QSTRING(selectCAPICertificate());
		if (certificateFile.isEmpty()) {
			ui->certificateButton_->setChecked(false);
		}
#else
		certificateFile = QFileDialog::getOpenFileName(this, tr("Select an authentication certificate"), QString(), tr("P12 files (*.cert *.p12 *.pfx);;All files (*.*)"));
		if (certificateFile.isEmpty()) {
			ui->certificateButton_->setChecked(false);
		}
#endif
	}
	else {
		certificateFile = "";
	}
	account_->setCertificatePath(Q2PSTRING(certificateFile));
}

void QtAccountDetailsWidget::handleColorChanged() {
	QColor qtColor = color_->getColor();
	RGBColor newColor(qtColor.red(), qtColor.green(), qtColor.blue());
	account_->setColor(newColor);
}

void QtAccountDetailsWidget::handleAccountNameEdited(const QString& text) {
	account_->setAccountName(Q2PSTRING(text));
}

void QtAccountDetailsWidget::handleUserAddressEdited(const QString& text) {
	account_->setJID(Q2PSTRING(text));
}

void QtAccountDetailsWidget::handlePasswordEdited(const QString& text) {
	account_->setPassword(Q2PSTRING(text));
}

void QtAccountDetailsWidget::handlePasswordReturnPressed() {
	ui->enabledCheck_->setChecked(true);
}

void QtAccountDetailsWidget::handleRememberPasswordToggled(bool checked) {
	account_->setRememberPassword(checked);
}

void QtAccountDetailsWidget::handleAccountEnabledChanged(bool checked) {
	account_->setJID(Q2PSTRING(ui->userAddress_->text()));
	account_->setPassword(Q2PSTRING(ui->password_->text()));
	account_->setEnabled(checked);
}

void QtAccountDetailsWidget::handleAccountEnabledValueChanged(bool checked) {
	ui->enabledCheck_->setChecked(checked);
}

void QtAccountDetailsWidget::handleSetAsDefault() {
	ui->defaultRadio_->setChecked(true);
	onWantsToBeDefault(account_->getIndex());
}

void QtAccountDetailsWidget::handleDeleteButtonClicked() {
	int result = QMessageBox::question(this, tr("Remove profile"), tr("Remove the profile '%1'?").arg(P2QSTRING(account_->getJID().toString())), QMessageBox::Yes | QMessageBox::No);
	if (result == QMessageBox::Yes) {
		onWantsToBeDeleted(account_->getJID());
	}
}

void QtAccountDetailsWidget::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::RightButton) {
		contextMenu_->exec(QCursor::pos());
	}
}



}
