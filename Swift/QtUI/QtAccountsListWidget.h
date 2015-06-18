/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <QScrollArea>

namespace Ui {
class QtAccountsListWidget;
}

namespace Swift {
class QtAccountsListWidget : public QScrollArea
{
		Q_OBJECT

	public:
		explicit QtAccountsListWidget(QWidget *parent = 0);
		~QtAccountsListWidget();

	private:
		Ui::QtAccountsListWidget *ui;
		QLayout* accountsLayout_;
};

}
