/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <QLabel>

#include <Swift/Controllers/Account.h>

namespace Swift {

class QtAccountColorWidget : public QLabel
{
		Q_OBJECT

	public:
		QtAccountColorWidget(QWidget* parent = 0);

		QColor getColor();

		void setColor(RGBColor newColor);

	signals:
		void colorChanged();

	protected:
		void mousePressEvent(QMouseEvent*);

	private:
		void updateBackgroundColor();

		QColor color_;
};
}
