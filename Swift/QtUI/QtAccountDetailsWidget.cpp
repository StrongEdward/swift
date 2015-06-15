/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <Swift/QtUI/QtAccountDetailsWidget.h>

namespace Swift {

QtAccountDetailsWidget::QtAccountDetailsWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::QtAccountDetailsWidget)
{
	ui->setupUi(this);
}

QtAccountDetailsWidget::~QtAccountDetailsWidget()
{
	delete ui;
}

}
