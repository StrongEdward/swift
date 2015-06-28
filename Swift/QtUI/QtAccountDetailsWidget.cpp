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

#include <QtSwiftUtil.h>

#include <Swift/QtUI/QtAccountDetailsWidget.h>
#include <Swift/QtUI/QtConnectionSettingsWindow.h>
#include <Swift/QtUI/CAPICertificateSelector.h>

#include <QPixmap>

namespace Swift {

QtAccountDetailsWidget::QtAccountDetailsWidget(boost::shared_ptr<Account> account, QButtonGroup* buttonGroup, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::QtAccountDetailsWidget),
	triangle_(new QtTreeviewTriangle),
	color_(new QtAccountColorWidget),
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
	connect(ui->enabledCheck_, &QCheckBox::toggled, this, &QtAccountDetailsWidget::handleAccountEnabled);

	buttonGroup->addButton(ui->defaultRadio_);
	buttonGroup->setId(ui->defaultRadio_, account_->getIndex());

	connect(ui->connectionOptions_, &QPushButton::clicked, this, &QtAccountDetailsWidget::handleCogwheelClicked);

	if (account_->getCertificatePath() != "") {
		ui->certificateButton_->setChecked(true);
	}
	connect(ui->certificateButton_, &QPushButton::clicked, this, &QtAccountDetailsWidget::handleCertificateChecked);

	ui->accountLayout_->insertWidget(1, color_);
	color_->setColor(account_->getColor());
	connect(color_, &QtAccountColorWidget::colorChanged, this, &QtAccountDetailsWidget::handleColorChanged);
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

void QtAccountDetailsWidget::setDefault() {
	ui->defaultRadio_->setChecked(true);
}

std::string QtAccountDetailsWidget::getUserAddress() {
	return account_->getJID();
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

void QtAccountDetailsWidget::handleRememberPasswordToggled(bool checked) {
	account_->setRememberPassword(checked);
}

void QtAccountDetailsWidget::handleAccountEnabled(bool checked) {
	account_->setEnabled(checked);
}

}
