/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include "QtAccountsListWidget.h"
#include "ui_QtAccountsListWidget.h"

#include <Swift/QtUI/QtAccountDetailsWidget.h>
#include <QFrame>

namespace Swift {

QtAccountsListWidget::QtAccountsListWidget(QWidget *parent) :
	QScrollArea(parent),
	ui(new Ui::QtAccountsListWidget)
{
	ui->setupUi(this);
	accountsLayout_ = ui->areaWidget_->layout();
	accountsLayout_->setAlignment(Qt::AlignTop);

	QtAccountDetailsWidget* details = new QtAccountDetailsWidget(this);
	QtAccountDetailsWidget* details2 = new QtAccountDetailsWidget(this);

	/*QFrame* r1 = new QFrame();
	QFrame* r2 = new QFrame();
	r1->setFrameShape(QFrame::Box);
	r2->setFrameShape(QFrame::Box);
	QHBoxLayout* l1 = new QHBoxLayout();
	QHBoxLayout* l2 = new QHBoxLayout();

	l1->addWidget(details);
	r1->setLayout(l1);
	l2->addWidget(details2);
	r2->setLayout(l2);*/

	accountsLayout_->addWidget(details);
	accountsLayout_->addWidget(details2);

}

QtAccountsListWidget::~QtAccountsListWidget()
{
	delete ui;
}

}
