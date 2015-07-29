/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

/*
 * Copyright (c) 2010 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#pragma once

#include <QStyledItemDelegate>
#include <QColor>
#include <QFont>

#include "DelegateCommons.h"

namespace Swift {
	class QtTreeWidgetItem;
	class AccountItemDelegate {
	public:
		AccountItemDelegate();
		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QString& name, int rowCount, bool expanded) const;
	private:
		void paintShadowText(QPainter* painter, const QRect& region, const QString& text) const;
		void paintExpansionTriangle(QPainter* painter, const QRect& region, int width, int height, bool expanded) const;
		QFont groupFont_;
		static const int groupCornerRadius_;
		DelegateCommons common_;
	};
}
