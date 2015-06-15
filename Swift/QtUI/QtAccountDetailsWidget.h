/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <QWidget>
#include <Swift/QtUI/ui_QtAccountDetailsWidget.h>

namespace Ui {
class QtAccountDetailsWidget;
}

namespace Swift {
class QtAccountDetailsWidget : public QWidget
{
		Q_OBJECT

	public:
		explicit QtAccountDetailsWidget(QWidget *parent = 0);
		~QtAccountDetailsWidget();

	private:
		Ui::QtAccountDetailsWidget *ui;
};

}
