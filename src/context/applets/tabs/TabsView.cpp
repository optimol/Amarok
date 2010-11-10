/****************************************************************************************
 * Copyright (c) 2010 Rainer Sigle <rainer.sigle@web.de>                                *
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

#include "TabsView.h"
#include "TabsItem.h"
#include "core/support/Debug.h"
#include "SvgHandler.h"
#include "widgets/PrettyTreeView.h"
#include "PaletteHandler.h"

#include <Plasma/TextBrowser>

#include <QHeaderView>
#include <QTreeView>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>

#include <KTextBrowser>

// Subclassed to override the access level of some methods.
// The TabsTreeView and the TabsView are so highly coupled that this is acceptable, imo.
class TabsTreeView : public Amarok::PrettyTreeView
{
    public:
        TabsTreeView( QWidget *parent = 0 )
            : Amarok::PrettyTreeView( parent )
        {
            setAttribute( Qt::WA_NoSystemBackground );
            viewport()->setAutoFillBackground( false );

            setHeaderHidden( true );
            setIconSize( QSize( 36, 36 ) );
            setDragDropMode( QAbstractItemView::DragOnly );
            setSelectionMode( QAbstractItemView::SingleSelection );
            setSelectionBehavior( QAbstractItemView::SelectItems );
            setAnimated( true );
            setRootIsDecorated( false );
            setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
            setVerticalScrollMode( QAbstractItemView::ScrollPerPixel ); // Scrolling per item is really not smooth and looks terrible
        }
    protected:

        // Override access level to make it public. Only visible to the TabsView.
        // Used for context menu methods.
        QModelIndexList selectedIndexes() const { return PrettyTreeView::selectedIndexes(); }
};


TabsView::TabsView( QGraphicsWidget *parent )
    : QGraphicsProxyWidget( parent )
{
    // tree view which holds the collection of fetched tabs
    m_treeView = new TabsTreeView( 0 );
    connect( m_treeView, SIGNAL( clicked( const QModelIndex & ) ), this, SLOT( itemClicked( const QModelIndex & ) ) );
    m_treeView->setFixedWidth( 48 );

    m_treeProxy = new QGraphicsProxyWidget( this );
    m_treeProxy->setWidget( m_treeView );

    // the textbrowser widget to display the tabs
    m_tabTextBrowser = new Plasma::TextBrowser( );
    KTextBrowser *browserWidget = m_tabTextBrowser->nativeWidget();
    browserWidget->setFrameShape( QFrame::StyledPanel );
    browserWidget->setAttribute( Qt::WA_NoSystemBackground );
    browserWidget->setOpenExternalLinks( true );
    browserWidget->setUndoRedoEnabled( true );
    browserWidget->setAutoFillBackground( false );
    browserWidget->setWordWrapMode( QTextOption::NoWrap );
    browserWidget->viewport()->setAutoFillBackground( true );
    browserWidget->viewport()->setAttribute( Qt::WA_NoSystemBackground );
    browserWidget->setTextInteractionFlags( Qt::TextBrowserInteraction | Qt::TextSelectableByKeyboard );

    // arrange textbrowser and treeview in a horizontal layout
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout( Qt::Horizontal );
    layout->addItem( m_treeProxy );
    layout->addItem( m_tabTextBrowser );
    layout->setSpacing( 2 );
    layout->setContentsMargins( 0, 0, 0, 0 );
    setLayout( layout );
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

TabsView::~TabsView()
{

}

void
TabsView::setModel( QAbstractItemModel *model )
{
    tabsListView()->setModel( model );
}

QAbstractItemModel*
TabsView::model()
{
    return tabsListView()->model();
}

QTreeView*
TabsView::tabsListView() const
{
    return static_cast<QTreeView*>( m_treeView );
}

void
TabsView::setTabTextContent(const QString tabText )
{
    m_tabTextBrowser->nativeWidget()->setPlainText( tabText );
}

void
TabsView::showTab( TabsItem *tab )
{
    if( tab )
    {
        const QString htmlCr =  "<br></br>";
        QString tabText = tab->getTabData();
        if( tabText.length() > 0 )
        {
            tabText.replace( "\n", htmlCr, Qt::CaseInsensitive);

            QString linkColor =The::paletteHandler()->palette().link().color().name();
            int fontSize = QFont().pointSize();
            int captionWeight = 600;

            QString htmlData = "<html>";
                    htmlData += "<body style=\" font-family:'Monospace'; font-size:" + QString::number( fontSize ) + "pt;";
                    htmlData += "font-weight:" + QString::number( QFont::Normal ) + "; font-style:normal;\">";

                    // tab heading + tab source
                    htmlData += "<p><span style=\" font-family:'Sans Serif';";
                    htmlData += "font-size:" + QString::number( fontSize + 2 ) + "pt;";
                    htmlData += "font-weight:" + QString::number( captionWeight ) + ";\">";
                    htmlData += tab->getTabTitle();
                    htmlData += " (" + i18n( "tab provided from: ") + "<a href=\"" + tab->getTabUrl() + "\">";
                    htmlData += "<span style=\" text-decoration: underline; color:" + linkColor + ";\">";
                    htmlData += tab->getTabSource() + "</a>";
                    htmlData += ")</span></p>";

                    // tab data
                    htmlData += tabText + "</p></body></html>";

            m_tabTextBrowser->nativeWidget()->setHtml( htmlData );
        }
    }
}

void
TabsView::itemClicked( const QModelIndex &index )
{
    const QStandardItemModel *itemModel = static_cast<QStandardItemModel*>( const_cast<TabsView*>( this )->model() );

    QStandardItem *item = itemModel->itemFromIndex( index );
    TabsItem *tab = dynamic_cast<TabsItem*>( item );
    if( tab )
        showTab( tab );
}

void
TabsView::resizeEvent( QGraphicsSceneResizeEvent *event )
{
    QGraphicsProxyWidget::resizeEvent( event );

    const int newWidth = size().width() / tabsListView()->header()->count();

    for( int i = 0; i < tabsListView()->header()->count(); ++i )
        tabsListView()->header()->resizeSection( i, newWidth );

    tabsListView()->setColumnWidth( 0, 100 );
}

#include <TabsView.moc>
