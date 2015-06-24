/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <Swift/QtUI/QtAccountColorWidget.h>

#include <QColorDialog>

namespace Swift {

QtAccountColorWidget::QtAccountColorWidget(QWidget* parent) : QLabel(parent) {
	color_ = QColor(100,100,100);
	setFixedSize(16,16);
	updateBackgroundColor();
}

QColor QtAccountColorWidget::getColor() {
	return color_;
}

void QtAccountColorWidget::setColor(RGBColor color) {
	color_ = QColor(color.red, color.green, color.blue);
	updateBackgroundColor();
}

void QtAccountColorWidget::mousePressEvent(QMouseEvent*) {
	QColorDialog dialog(color_);
	QColor newColor = dialog.getColor(color_);
	if (newColor.isValid()) {
		color_ = newColor;
		updateBackgroundColor();
		emit colorChanged();
	}
}

void QtAccountColorWidget::updateBackgroundColor() {
	setStyleSheet("background-color: rgb(" + QString::number(color_.red()) + ", " + QString::number(color_.green()) + ", " + QString::number(color_.blue()) + ");");
}

}
