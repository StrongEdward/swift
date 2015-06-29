/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <QFrame>
#include <QPixmap>
#include <QMenu>
#include <QLabel>
#include <QButtonGroup>

#include <Swift/Controllers/Account.h>
#include <Swift/QtUI/ui_QtAccountDetailsWidget.h>
#include <Swift/QtUI/QtTreeviewTriangle.h>
#include <Swift/QtUI/QtAccountColorWidget.h>

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

		std::string getUserAddress();
		void setDefaultAccountLook(bool isDefault);

		boost::signal <void(int)> onWantsToBeDefault;
		boost::signal <void(std::string)> onWantsToBeDeleted;

	protected:
		void mousePressEvent(QMouseEvent*);

	private:
		void triangleClicked();

	private slots:
		void handleCogwheelClicked();
		void handleCertificateChecked(bool checked);
		void handleColorChanged();
		void handleAccountNameEdited(const QString& text);
		void handleUserAddressEdited(const QString& text);
		void handlePasswordEdited(const QString& text);
		void handlePasswordReturnPressed();
		void handleRememberPasswordToggled(bool checked);
		void handleAccountEnabledChanged(bool checked);
		void handleAccountEnabledValueChanged(bool checked);
		void handleSetAsDefault();
		void handleDeleteButtonClicked();

	private:
		Ui::QtAccountDetailsWidget *ui;
		QtTreeviewTriangle* triangle_;
		QtAccountColorWidget* color_;
		QMenu* contextMenu_;

		boost::shared_ptr<Account> account_;

};

}
