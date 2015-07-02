/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <Swift/QtUI/QtTreeviewTriangle.h>

namespace Swift {

QtTreeviewTriangle::QtTreeviewTriangle(QWidget* parent) :
	QLabel(parent),
	expanded_(false),
	expandedPixmap_(QPixmap(":/icons/triangleDown.png")),
	collapsedPixmap_(QPixmap(":/icons/triangle.png")) {

	setMinimumSize(QSize(14,13));
	setPixmap(collapsedPixmap_);
}

bool QtTreeviewTriangle::isExpanded() {
	return expanded_;
}

void QtTreeviewTriangle::mousePressEvent(QMouseEvent*) {
	if (expanded_) {
		setPixmap(collapsedPixmap_);
	}
	else {
		setPixmap(expandedPixmap_);
	}
	expanded_ = !expanded_;
	emit clicked();
}

}
