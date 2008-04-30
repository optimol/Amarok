/***************************************************************************
 *   Copyright (c) 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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

#ifndef AMAROKSERVICEBASE_H
#define AMAROKSERVICEBASE_H


#include "Amarok.h"

#include "InfoParserBase.h"
#include "ServiceMetaBase.h"
#include "collection/CollectionManager.h"

#include "amarok_export.h"
#include "../collectionbrowser/SingleCollectionTreeItemModel.h"
#include "../collectionbrowser/CollectionTreeItem.h"
#include "ServiceCollectionTreeView.h"
#include "plugin/plugin.h"

#include <khtml_part.h>
#include <kvbox.h>

#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>

#include <KPluginInfo>

class ServiceBase;
class SearchWidget;
class KMenuBar;


/**
A virtual base class for factories for creating and handeling the different types of service plugins

@author Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>
 */
class AMAROK_EXPORT ServiceFactory : public QObject, public Amarok::Plugin, public TrackProvider
{
    Q_OBJECT
    public:
        /**
         * Constructor
         */
        ServiceFactory();
        
        /**
         * Destructor
         */
        virtual ~ServiceFactory();

        /**
         * Initialize the service plugins of this type. Reimplemented by subclasses
         */
        virtual void init() = 0;
        
        /**
         * Get the name of this service type. Reimplemented by subclasses
         * @return The name
         */
        virtual QString name() = 0;
        
        /**
         * Get a KConfigGroup object containing the config for this tyoe of service. Reimplemented by subclasses
         * @return 
         */
        virtual KConfigGroup config() = 0;
        
        /**
         * Get a KPluginInfo object containing the info about this plugin. Reimplemented by subclasses
         * @return 
         */
        virtual KPluginInfo info() = 0;

        /**
         * Get a best guess if a service of the type generated by this factory will be likely to be able to provide tracks
         * for a given url. This is needed in order to allow on.demand loading of service plugins to handle a url. Reimplemented by subclasses
         * @param url The url to test
         * @return A bool representing wheter the ServiceFactory believes that a service of this kind can process the given url.
         */
        virtual bool possiblyContainsTrack( const KUrl &url ) const { Q_UNUSED( url ); return false; }
        
        /**
         * Attempt to create a Meta::Track object from a given url. This method is meant as a proxy that will forward this call to one or more
         * services managed by this factory. If init has not been called ( no services of this kind has been loaded ) they can now be loaded on
         * demand.
         * @param url The url to test
         * @return A Meta::TrackPtr based one the url, or empty if nothing was known about the url
         */
        virtual Meta::TrackPtr trackForUrl( const KUrl &url );

        /**
         * Clear the list of active services created by this factory. Used when unloading services.
         */
        void clearActiveServices();

    signals:
        /**
         * This signal is emmited whenever a new service has been loaded.
         * @param newService The service that has been loaded
         */
        void newService( class ServiceBase *newService );

    protected:
        QList<ServiceBase *> m_activeServices;
};


/**
A very basic composite widget used as a base for building service browsers.

@author Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>
*/
class AMAROK_EXPORT ServiceBase : public KVBox
{
    Q_OBJECT

public:
     

     /**
      * Constructor
      */
    ServiceBase( const QString &name );
    
    /**
     * Destructor
     */
    ~ServiceBase();


    QString getName();

    void setShortDescription( const QString &shortDescription );
    QString getShortDescription();
    void setLongDescription( const QString &longDescription );
    QString getLongDescription();
    void setIcon( const QIcon &icon );
    QIcon getIcon();
    void setModel( SingleCollectionTreeItemModel * model );
    SingleCollectionTreeItemModel * getModel();

    void setPlayableTracks( bool playable );
    void setInfoParser( InfoParserBase * infoParser );
    InfoParserBase * infoParser();
    
    virtual Collection * collection() = 0;
    
    virtual void polish() = 0;
    virtual bool updateContextView() { return false; }

    void setFilter( const QString &filter );

    /**
     * Returns a list of the messages that the current service accepts. Default impelentation does not
     * accept any.
     * @return A string containing a description of accepted messages.
     */
    virtual QString messages();

    /**
     * Send amessage to this service. Default implementation returns an error as no messages are
     * accepted
     * @param message The message to send to the service
     * @return The reply to the message
     */
    virtual QString sendMessage( const QString &message );

    //virtual void reset() = 0;

public slots:
    //void treeViewSelectionChanged( const QItemSelection & selected );
    void infoChanged ( const QString &infoHtml );

    void sortByArtistAlbum();

signals:
    void home();
    void selectionChanged ( CollectionTreeItem * );
    
protected slots:
    void homeButtonClicked();
    void itemActivated ( const QModelIndex & index );
    void itemSelected( CollectionTreeItem * item  );

    void sortByArtist();

    void sortByAlbum();
    void sortByGenreArtist();
    void sortByGenreArtistAlbum();

protected:
    virtual void generateWidgetInfo( const QString &html = QString() ) const;
    
    static ServiceBase *s_instance;
    ServiceCollectionTreeView *m_contentView;

    QPushButton *m_homeButton;

    KVBox       *m_topPanel;
    KVBox       *m_bottomPanel;
    bool         m_polished;

    QString      m_name;
    QString      m_shortDescription;
    QString      m_longDescription;
    QIcon        m_icon;

    KUrl::List   m_urlsToInsert;

    InfoParserBase * m_infoParser;

    KMenuBar *m_menubar;
    QMenu *m_filterMenu;
    SearchWidget * m_searchWidget;

    //void addToPlaylist( CollectionTreeItem * item );

private: // need to move stuff here
     SingleCollectionTreeItemModel * m_model;
     QSortFilterProxyModel * m_filterModel;
};


#endif
