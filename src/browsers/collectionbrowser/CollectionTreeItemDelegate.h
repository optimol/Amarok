/****************************************************************************************
 * Copyright (c) 2007 Nikolaj Hald Nielsen <nhn@kde.org>                                *
 * Copyright (c) 2008 Mark Kretschmann <kretschmann@kde.org>                            *
 * Copyright (c) 2009 Seb Ruiz <ruiz@kde.org>                                           *
 * Copyright (c) 2013 Ralf Engels <ralf-engels@gmx.de>                                  *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef AMAROK_COLLECTION_TREE_ITEM_DELEGATE_H
#define AMAROK_COLLECTION_TREE_ITEM_DELEGATE_H

#include <QAction>
#include <QFont>
#include <QPersistentModelIndex>
#include <QRect>
#include <QStyledItemDelegate>
#include <QTreeView>

class QFontMetrics;

class CollectionTreeItemDelegate : public QStyledItemDelegate
{
    public:
        CollectionTreeItemDelegate( QTreeView *view );
        ~CollectionTreeItemDelegate();

        void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
        QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;

        /** Returns the rectangle where the action icons are located. */
        static QRect decoratorRect( const QModelIndex &index );

    private:
        /** Verify and if needed update the buffered fonts and font metrics. */
        void updateFonts( const QStyleOptionViewItem &option ) const;

        QTreeView *m_view;

        mutable QFont m_originalFont;
        mutable QFont m_bigFont;
        mutable QFont m_smallFont;

        mutable QFontMetrics *m_normalFm;
        mutable QFontMetrics *m_bigFm;
        mutable QFontMetrics *m_smallFm;

        static QHash<QPersistentModelIndex, QRect> s_indexDecoratorRects;
};

#endif
