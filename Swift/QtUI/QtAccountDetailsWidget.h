/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <QFrame>
#include <QPixmap>
#include <QLabel>
#include <QButtonGroup>

#include <Swift/Controllers/Account.h>
#include <Swift/QtUI/ui_QtAccountDetailsWidget.h>
#include <Swift/QtUI/QtTreeviewTriangle.h>
#include <Swift/QtUI/QtAccountColourWidget.h>

namespace Ui {
class QtAccountDetailsWidget;
}

namespace Swift {

class QtAccountDetailsWidget : public QFrame {
		Q_OBJECT

	public:
		explicit QtAccountDetailsWidget(boost::shared_ptr<Account> account, QButtonGroup* buttonGroup, QWidget* parent = 0);
		~QtAccountDetailsWidget();

		virtual QSize sizeHint() const;
		virtual QSize minimumSizeHint() const;

		void setDefault();
		std::string getUserAddress();

	private:
		void triangleClicked();

	private slots:
		void handleCogwheelClicked();
		void handleCertificateChecked(bool checked);
		void handleColourClicked();

	private:
		Ui::QtAccountDetailsWidget *ui;
		QtTreeviewTriangle* triangle_;
		QtAccountColourWidget* colour_;

		boost::shared_ptr<Account> account_;

};

}
