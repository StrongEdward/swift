/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <Swift/QtUI/QtAccountColourWidget.h>

#include <QColorDialog>

namespace Swift {

QtAccountColourWidget::QtAccountColourWidget(QWidget* parent) : QLabel(parent) {
	colour = QColor(100,100,100);
	setFixedSize(16,16);
	updateBackgroundColour();
}

void QtAccountColourWidget::mousePressEvent(QMouseEvent*) {
	QColorDialog dialog(colour);
	colour = dialog.getColor(colour);
	updateBackgroundColour();
}

void QtAccountColourWidget::updateBackgroundColour() {
	setStyleSheet("background-color: rgb(" + QString::number(colour.red()) + ", " + QString::number(colour.green()) + ", " + QString::number(colour.blue()) + ");");
}

}
