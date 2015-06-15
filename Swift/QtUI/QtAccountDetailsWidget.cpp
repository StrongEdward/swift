/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <Swift/QtUI/QtAccountDetailsWidget.h>

#include <QPixmap>

namespace Swift {

QtAccountDetailsWidget::QtAccountDetailsWidget(QWidget *parent) :
	QWidget(parent),
	ui_(new Ui::QtAccountDetailsWidget),
	triangle_(new QtTreeviewTriangle)
{
	ui_->setupUi(this);
	ui_->extendingWidget_->hide();

	ui_->connectionOptions_->setIcon(QIcon(":/icons/actions.png"));
	ui_->certificateButton_->setIcon(QIcon(":/icons/certificate.png"));
	ui_->accountLayout_->insertWidget(0, triangle_);

	connect(triangle_, &QtTreeviewTriangle::clicked, this, &QtAccountDetailsWidget::triangleClicked);
}

QtAccountDetailsWidget::~QtAccountDetailsWidget()
{
	delete triangle_;
	delete ui_;
}

void QtAccountDetailsWidget::triangleClicked() {
	if (triangle_->isExpanded()) {
		ui_->extendingWidget_->show();
	} else {
		ui_->extendingWidget_->hide();
	}
}

}
