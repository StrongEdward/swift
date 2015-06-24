/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <QLabel>

namespace Swift {

class QtAccountColourWidget : public QLabel
{
	public:
		QtAccountColourWidget(QWidget* parent = 0);

	protected:
		void mousePressEvent(QMouseEvent*);

	private:
		void updateBackgroundColour();

		QColor colour;
};
}
