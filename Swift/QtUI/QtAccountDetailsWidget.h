/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <QFrame>
#include <QPixmap>
#include <QLabel>

#include <Swift/Controllers/Account.h>
#include <Swift/QtUI/ui_QtAccountDetailsWidget.h>
#include <Swift/QtUI/QtTreeviewTriangle.h>

namespace Ui {
class QtAccountDetailsWidget;
}

namespace Swift {

class QtAccountDetailsWidget : public QFrame {
		Q_OBJECT

	public:
		explicit QtAccountDetailsWidget(boost::shared_ptr<Account> account, QWidget* parent = 0);
		~QtAccountDetailsWidget();

		virtual QSize sizeHint() const;
		virtual QSize minimumSizeHint() const;

	private:
		void triangleClicked();

	private:
		Ui::QtAccountDetailsWidget *ui;
		QtTreeviewTriangle* triangle_;

		boost::shared_ptr<Account> account_;

};

}
