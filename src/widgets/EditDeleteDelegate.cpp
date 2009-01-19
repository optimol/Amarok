/***************************************************************************
 *   Copyright (c) 2009  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
 
#include "EditDeleteDelegate.h"

#include "Debug.h"

#include <KIcon>

#include <QPainter>
#include <QPixmap>

#define ICON_WIDTH 16
#define MARGIN 3


EditDeleteDelegate::EditDeleteDelegate( QObject * parent )
    : QStyledItemDelegate( parent )
{
}


EditDeleteDelegate::~EditDeleteDelegate()
{
}

void EditDeleteDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    int y = option.rect.y();

    //use normal painting, sizeHint has ensured that we have enough room for both text and icons.
    QStyledItemDelegate::paint( painter, option, index );

    int iconOffset = option.rect.width() - ( MARGIN * 3 + ICON_WIDTH * 2 );


    if ( option.state & QStyle::State_Selected ) {
        //paint our custom stuff in the leftover space
        //but only if this is the item that the mouse is over...

        KIcon editIcon( "configure" );
        KIcon deleteIcon( "edit-delete" );

        QPixmap editPixmap = editIcon.pixmap( ICON_WIDTH, ICON_WIDTH );
        QPixmap deletePixmap = deleteIcon.pixmap( ICON_WIDTH, ICON_WIDTH );

        painter->drawPixmap( iconOffset + MARGIN, y, ICON_WIDTH, ICON_WIDTH, editPixmap );
        painter->drawPixmap( iconOffset + MARGIN *2 + ICON_WIDTH, y, ICON_WIDTH, ICON_WIDTH, deletePixmap );

    }

}

QSize EditDeleteDelegate::sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QSize orgSize = QStyledItemDelegate::sizeHint( option, index );
    QSize addSize( MARGIN * 3 + ICON_WIDTH * 2, 0 );

    return orgSize + addSize;
    
}



