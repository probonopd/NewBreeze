/*
	*
	* NBTrashDelegate.hpp - NBTrashDelegate.cpp Header
	*
*/

#pragma once

#include "Global.hpp"

class NBTrashDelegate : public QItemDelegate {
    Q_OBJECT

	public:
		void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

	private:
		void paintExtraDetails( QPainter *painter, QList<QRect> &textRectList, const QModelIndex &index ) const;
};
