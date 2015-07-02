/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>

namespace Swift {

class QtTreeviewTriangle : public QLabel {
		Q_OBJECT

	public:
		explicit QtTreeviewTriangle(QWidget* parent = 0);
		~QtTreeviewTriangle() {}
		bool isExpanded();

	signals:
		void clicked();

	protected:
		void mousePressEvent(QMouseEvent*);

	private:
		bool expanded_;
		QPixmap expandedPixmap_;
		QPixmap collapsedPixmap_;
};

}
