/* 
   Copyright (C) 2008 Daniel Winter <dw@danielwinter.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include "NepomukAlbum.h"

#include "NepomukArtist.h"
#include "NepomukCollection.h"
#include "NepomukRegistry.h"

#include "BlockingQuery.h"
#include "Debug.h"
#include "Meta.h"

#include <QDir>
#include <QFile>
#include <QPixmap>
#include <QString>

#include <KMD5>
#include <KUrl>
#include <Nepomuk/ResourceManager>
#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/XMLSchema>

using namespace Meta;

NepomukAlbum::NepomukAlbum( NepomukCollection *collection, const QString &name, const QString &artist )
        : Album()
        , m_collection( collection )
        , m_name( name )
        , m_artist( artist )
        , m_tracksLoaded( false )
        , m_hasImage( false )
        , m_hasImageChecked( false )
{
}

QString
NepomukAlbum::name() const
{
    return m_name;
}

QString
NepomukAlbum::prettyName() const
{
    return m_name;
}

TrackList
NepomukAlbum::tracks()
{
    if( m_tracksLoaded )
    {
        return m_tracks;
    }
    else if( m_collection )
    {
        QueryMaker *qm = m_collection->queryMaker();
        qm->setQueryType( QueryMaker::Track );
        addMatchTo( qm );
        BlockingQuery bq( qm );
        bq.startQuery();
        m_tracks = bq.tracks( m_collection->collectionId() );
        m_tracksLoaded = true;
        return m_tracks;
    }
    else
        return TrackList(); 
}

bool
NepomukAlbum::isCompilation() const
{
    return false;
}

bool
NepomukAlbum::hasAlbumArtist() const
{
    return true;
}

ArtistPtr
NepomukAlbum::albumArtist() const
{
    return m_collection->registry()->artistForArtistName( m_artist );
}

bool
NepomukAlbum::hasImage( int size ) const
{
    DEBUG_BLOCK
    if( !m_hasImageChecked )
        m_hasImage = ! const_cast<NepomukAlbum*>( this )->image( size ).isNull();
    debug() << "nepo has image: returning :" << m_hasImageChecked << endl;
    return m_hasImage;
}

QPixmap
NepomukAlbum::image( int size, bool withShadow )
{
    DEBUG_BLOCK
    if ( !m_hasImageChecked )
    {
        m_hasImageChecked = true;
        m_imagePath = findImage();
        if ( !m_imagePath.isEmpty() )
            m_hasImage = true;
    }
    if( !m_hasImage )
        return Meta::Album::image( size, withShadow );

    if( m_images.contains( size ) )
        return QPixmap( m_images.value( size ) );
    
    QString path = findOrCreateScaledImage( m_imagePath, size );
    if ( !path.isEmpty() )
    {
        m_images.insert( size, path );
        return QPixmap( path );
    }
 
    return Meta::Album::image( size, withShadow );
}

void
NepomukAlbum::emptyCache()
{
    // FIXME: Add proper locks

    m_tracks.clear();
    m_tracksLoaded = false;
}

QString
NepomukAlbum::findImage() const
{
    DEBUG_BLOCK
    // TODO: Query for Image set in Nepomuk
    return findImageInDir();
}

QString
NepomukAlbum::findOrCreateScaledImage( QString path, int size ) const
{
    DEBUG_BLOCK
    if( size <= 1 )
        return QString();

    QByteArray widthKey = QString::number( size ).toLocal8Bit() + "@nepo@"; // nepo 
    QString album = m_name;
    QString artist = hasAlbumArtist() ? albumArtist()->name() : QString();

    if( artist.isEmpty() && album.isEmpty() )
        return QString();

    KMD5 context( artist.toLower().toLocal8Bit() + album.toLower().toLocal8Bit() );
    QByteArray key = context.hexDigest();

    QDir cacheCoverDir( Amarok::saveLocation( "albumcovers/cache/" ) );
    QString cachedImagePath = cacheCoverDir.filePath( widthKey + key );

    // create if it not already there
    if ( !QFile::exists( cachedImagePath ) )
    {
         if( QFile::exists( path ) )
        {
            QImage img( path );
            if( img.isNull() )
                return QString();
            
            // resize and save the image
            img.scaled( size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation ).save( cachedImagePath, "JPG" );
        }
    }
    return cachedImagePath;
}

QString
NepomukAlbum::findImageInDir() const
{
    // test if all files are in one directory
    QString path;
    QString query = QString("SELECT DISTINCT ?path WHERE {"
                "?r <%1> \"%2\"^^<%3> ."  // only for current artist
                "?r <%4> \"%5\"^^<%6> ." // only for current album
                "?r <http://strigi.sf.net/ontologies/0.9#parentUrl> ?path . " // we want to know the parenturl
                "} LIMIT 2") // we need not more than 2
                .arg( m_collection->getUrlForValue( QueryMaker::valArtist ) )
                .arg( m_artist )
                .arg( Soprano::Vocabulary::XMLSchema::string().toString() )
                .arg( m_collection->getUrlForValue( QueryMaker::valAlbum ) )
                .arg( m_name )
                .arg( Soprano::Vocabulary::XMLSchema::string().toString() );
                        
    Soprano::Model* model = Nepomuk::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it
            = model->executeQuery( query,
                                    Soprano::Query::QueryLanguageSparql );
    if ( it.next() )
    {
        path = it.binding( "path" ).toString();
        // if we have another result the files are in more than one dir 
        if ( it.next() )
            return QString();
    }
    else
    {
        // should not happen
        return QString();
    }

    query = QString("SELECT ?r WHERE {"
                  "?r <http://strigi.sf.net/ontologies/0.9#parentUrl> \"%1\"^^<%2> . " // only from path
                  "?r <%3> ?mime  FILTER regex(STR(?mime), '^image') } "  // only images
                  "LIMIT 1") // only one
                  .arg( path )
                  .arg( Soprano::Vocabulary::XMLSchema::string().toString() )
                  .arg( Soprano::Vocabulary::Xesam::mimeType().toString() );
    it = model->executeQuery( query,
                                     Soprano::Query::QueryLanguageSparql );
    if( it.next() )
    {
        Soprano::Node node = it.binding( "r" ) ;
        QUrl url( node.toString() );
        debug() << "nepo image found: " << url << endl;
        if ( QFile::exists( url.toLocalFile() ) )
            return url.toLocalFile();
    }
   return QString();
}
