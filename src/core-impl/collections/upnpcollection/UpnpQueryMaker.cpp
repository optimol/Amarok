/****************************************************************************************
 * Copyright (c) 2010 Nikhil Marathe <nsm.nikhil@gmail.com>                             *
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
#define DEBUG_PREFIX "UpnpQueryMaker"

#include "UpnpQueryMaker.h"

#include <kdatetime.h>
#include <kio/upnptypes.h>
#include <kio/scheduler.h>
#include <kio/jobclasses.h>

#include "core/support/Debug.h"
#include "UpnpSearchCollection.h"
#include "UpnpMeta.h"
#include "UpnpCache.h"

namespace Collections {

/*
 * #define emitProperResult( PointerType, list ) \
    do { \
            if ( m_asDataPtrs ) { \
                Meta::DataList data; \
                foreach( PointerType p, list ) { \
                    data << Meta::DataPtr::staticCast( p ); \
                } \
                //emit newResultReady( m_collection->collectionId(), data ); \
            } \
            else { \
                //emit newResultReady( m_collection->collectionId(), list ); \
            } \
    } while ( 0 )*/

#define emitProperResult( PointerType, list ) \
do {\
    foreach( PointerType##Ptr p, list ) \
        m_cacheEntries << Meta::DataPtr::staticCast( p ); \
    if ( m_asDataPtrs ) { \
        emit newResultReady( m_collection->collectionId(), m_cacheEntries ); \
    } \
    else { \
        PointerType##List list; \
        foreach( Meta::DataPtr ptr, m_cacheEntries ) \
            list << PointerType##Ptr::staticCast( ptr ); \
        emit newResultReady( m_collection->collectionId(), list ); \
    } \
} while( 0 )

bool UpnpQueryMaker::m_runningJob = false;
int UpnpQueryMaker::m_count = 0;
QHash<QString, KIO::ListJob *> UpnpQueryMaker::m_inProgressQueries = QHash<QString, KIO::ListJob*>();

UpnpQueryMaker::UpnpQueryMaker( UpnpSearchCollection *collection )
    : QueryMaker()
    , m_collection( collection )
{
    m_count++;
    reset();
}

UpnpQueryMaker::~UpnpQueryMaker()
{
    m_count--;
}


QueryMaker* UpnpQueryMaker::reset()
{
    // TODO kill all jobs here too
    m_queryType = None;
    m_albumMode = AllAlbums;
    m_asDataPtrs = false;
    m_query.reset();
    m_jobCount = 0;

// the Amarok Collection Model expects atleast one entry
// otherwise it will harass us continuously for more entries.
// of course due to the poor quality of UPnP servers I've
// had experience with :P, some may not have sub-results
// for something ( they may have a track with an artist, but
// not be able to give any album for it )
    m_noResults = true;
    return this;
}

void UpnpQueryMaker::run()
{
DEBUG_BLOCK
    QStringList queryList = m_query.queries();
    if( queryList.isEmpty() ) {
        emit queryDone();
        return;
    }

    // and experiment in using the filter only for the query
    // and checking the returned upnp:class
    // based on your query types.
    for( int i = 0; i < queryList.length() ; i++ ) {
        if( queryList[i].isEmpty() )
            continue;
        
        QString url = m_collection->collectionId() + "?search=1&query=";

        url += queryList[i];

        debug() << this << "Running query" << url;

        KIO::ListJob *job = 0;
        if( m_inProgressQueries.contains( url ) ) {
            debug() << "Already have a running job with the same query";
            job = m_inProgressQueries[url];
        }
        else {
            job = KIO::listDir( url, KIO::HideProgressInfo );
            m_inProgressQueries[url] = job;
            m_jobCount++;
        }

        connect( job, SIGNAL( entries( KIO::Job *, const KIO::UDSEntryList & ) ),
                this, SLOT( slotEntries( KIO::Job *, const KIO::UDSEntryList & ) ) );
        connect( job, SIGNAL( result(KJob *) ), this, SLOT( slotDone(KJob *) ) );
    }
    m_runningJob = true;
}

void UpnpQueryMaker::abortQuery()
{
DEBUG_BLOCK
    Q_ASSERT( false );
// TODO implement this to kill job
}

QueryMaker* UpnpQueryMaker::setQueryType( QueryType type )
{
DEBUG_BLOCK
// TODO allow all, based on search capabilities
// which should be passed on by the factory
    m_queryType = type;
/*    QString typeString;
    switch( type ) {
        case Artist:
            debug() << this << "Query type Artist";
            typeString = "( upnp:class derivedfrom \"object.container.person.musicArtist\" )";
            break;
        case Album:
            debug() << this << "Query type Album";
            typeString = "( upnp:class derivedfrom \"object.container.album.musicAlbum\" )";
            break;
        case Track:
            debug() << this << "Query type Track";
            typeString = "( upnp:class derivedfrom \"object.item.audioItem\" )";
            break;
        case Genre:
            debug() << this << "Query type Genre";
            typeString = "( upnp:class derivedfrom \"object.container.genre.musicGenre\" )";
            break;
        case Custom:
            debug() << this << "Query type Custom";
// TODO
            break;
        default:
            debug() << this << "Default case: Query type" << typeString;
            break;
    }
    if( !typeString.isNull() )
        m_query.setType( typeString );*/

    m_query.setType( "( upnp:class derivedfrom \"object.item.audioItem\" )" );
    return this;
}

QueryMaker* UpnpQueryMaker::setReturnResultAsDataPtrs( bool resultAsDataPtrs )
{
DEBUG_BLOCK
    m_asDataPtrs = resultAsDataPtrs;
    return this;
}

QueryMaker* UpnpQueryMaker::addReturnValue( qint64 value )
{
DEBUG_BLOCK
    debug() << this << "Add return value" << value;
    return this;
}

QueryMaker* UpnpQueryMaker::addReturnFunction( ReturnFunction function, qint64 value )
{
DEBUG_BLOCK
    Q_UNUSED( function )
    debug() << this << "Return function with value" << value;
    return this;
}

QueryMaker* UpnpQueryMaker::orderBy( qint64 value, bool descending )
{
DEBUG_BLOCK
    debug() << this << "Order by " << value << "Descending?" << descending;
    return this;
}

QueryMaker* UpnpQueryMaker::orderByRandom()
{
DEBUG_BLOCK
    return this;
}

QueryMaker* UpnpQueryMaker::includeCollection( const QString &collectionId )
{
DEBUG_BLOCK
    debug() << this << "Including collection" << collectionId;
    return this;
}

QueryMaker* UpnpQueryMaker::excludeCollection( const QString &collectionId )
{
DEBUG_BLOCK
    debug() << this << "Excluding collection" << collectionId;
    return this;
}

QueryMaker* UpnpQueryMaker::addMatch( const Meta::TrackPtr &track )
{
DEBUG_BLOCK
    debug() << this << "Adding track match" << track->name();
    // TODO: CHECK query type before searching by dc:title?
    m_query.addMatch( "( dc:title = \"" + track->name() + "\" )" );
    return this;
}

QueryMaker* UpnpQueryMaker::addMatch( const Meta::ArtistPtr &artist )
{
DEBUG_BLOCK
    debug() << this << "Adding artist match" << artist->name();
    m_query.addMatch( "( upnp:artist = \"" + artist->name() + "\" )" );
    return this;
}

QueryMaker* UpnpQueryMaker::addMatch( const Meta::AlbumPtr &album )
{
DEBUG_BLOCK
    debug() << this << "Adding album match" << album->name();
    m_query.addMatch( "( upnp:album = \"" + album->name() + "\" )" );
    return this;
}

QueryMaker* UpnpQueryMaker::addMatch( const Meta::ComposerPtr &composer )
{
DEBUG_BLOCK
    debug() << this << "Adding composer match" << composer->name();
// NOTE unsupported
    return this;
}

QueryMaker* UpnpQueryMaker::addMatch( const Meta::GenrePtr &genre )
{
DEBUG_BLOCK
    debug() << this << "Adding genre match" << genre->name();
    m_query.addMatch( "( upnp:genre = \"" + genre->name() + "\" )" );
    return this;
}

QueryMaker* UpnpQueryMaker::addMatch( const Meta::YearPtr &year )
{
DEBUG_BLOCK
    debug() << this << "Adding year match" << year->name();
// TODO 
    return this;
}

QueryMaker* UpnpQueryMaker::addMatch( const Meta::DataPtr &data )
{
DEBUG_BLOCK
    debug() << this << "Adding dataptr match" << data->name();
    ( const_cast<Meta::DataPtr&>(data) )->addMatchTo( this );
    return this;
}

QueryMaker* UpnpQueryMaker::addMatch( const Meta::LabelPtr &label )
{
DEBUG_BLOCK
    debug() << this << "Adding label match" << label->name();
// NOTE how?
    return this;
}

QueryMaker* UpnpQueryMaker::addFilter( qint64 value, const QString &filter, bool matchBegin, bool matchEnd )
{
DEBUG_BLOCK
    debug() << this << "Adding filter" << value << filter << matchBegin << matchEnd;

// theoretically this should be '=' I think and set to contains below if required
    QString cmpOp = "contains";
    //TODO should we add filters ourselves
    // eg. we always query for audioItems, but how do we decide
    // whether to add a dc:title filter or others.
    // for example, for the artist list
    // our query should be like ( pseudocode )
    // ( upnp:class = audioItem ) and ( dc:title contains "filter" )
    // OR
    // ( upnp:class = audioItem ) and ( upnp:artist contains "filter" );
    // ...
    // so who adds the second query?
// TODO add generic FILTER
    QString property = "dc:title";
    switch( value ) {
        case Meta::valTitle:
            break;
        case Meta::valArtist:
        {
            //if( m_queryType != Artist )
                property = "upnp:artist";
            break;
        }
        case Meta::valAlbum:
        {
            //if( m_queryType != Album )
                property = "upnp:album";
            break;
        }
        case Meta::valGenre:
            property = "upnp:genre";
            break;
        default:
            debug() << "UNSUPPORTED QUERY TYPE" << value;
            break;
    }

    if( matchBegin || matchEnd )
        cmpOp = "contains";

    QString filterString = "( " + property + " " + cmpOp + " \"" + filter + "\" ) ";
    m_query.addFilter( filterString );
    return this;
}

QueryMaker* UpnpQueryMaker::excludeFilter( qint64 value, const QString &filter, bool matchBegin, bool matchEnd )
{
DEBUG_BLOCK
    debug() << this << "Excluding filter" << value << filter << matchBegin << matchEnd;
    return this;
}

QueryMaker* UpnpQueryMaker::addNumberFilter( qint64 value, qint64 filter, NumberComparison compare )
{
DEBUG_BLOCK
    debug() << this << "Adding number filter" << value << filter << compare;
    return this;
}

QueryMaker* UpnpQueryMaker::excludeNumberFilter( qint64 value, qint64 filter, NumberComparison compare )
{
DEBUG_BLOCK
    debug() << this << "Excluding number filter" << value << filter << compare;
    return this;
}

QueryMaker* UpnpQueryMaker::limitMaxResultSize( int size )
{
DEBUG_BLOCK
    debug() << this << "Limit max results to" << size;
    return this;
}

QueryMaker* UpnpQueryMaker::setAlbumQueryMode( AlbumQueryMode mode )
{
DEBUG_BLOCK
    debug() << this << "Set album query mode" << mode;
    m_albumMode = mode;
    return this;
}

QueryMaker* UpnpQueryMaker::setLabelQueryMode( LabelQueryMode mode )
{
DEBUG_BLOCK
    debug() << this << "Set label query mode" << mode;
    return this;
}

QueryMaker* UpnpQueryMaker::beginAnd()
{
DEBUG_BLOCK
    m_query.beginAnd();
    return this;
}

QueryMaker* UpnpQueryMaker::beginOr()
{
DEBUG_BLOCK
    m_query.beginOr();
    return this;
}

QueryMaker* UpnpQueryMaker::endAndOr()
{
DEBUG_BLOCK
    debug() << this << "End AND/OR";
    m_query.endAndOr();
    return this;
}

QueryMaker* UpnpQueryMaker::setAutoDelete( bool autoDelete )
{
DEBUG_BLOCK
    debug() << this << "Auto delete" << autoDelete;
    return this;
}

int UpnpQueryMaker::validFilterMask()
{
DEBUG_BLOCK
// TODO return based on our collections search capabilities!
    return TitleFilter | AlbumFilter | ArtistFilter | GenreFilter;
}

void UpnpQueryMaker::slotEntries( KIO::Job *job, const KIO::UDSEntryList &list )
{
    debug() << "RESULT OF " << job << job->error();
    if( job->error() ) {
        debug() << this << "JOB has error" << job->errorString();
        return;
    }
    debug() << this << "SLOT ENTRIES" << list.length() << m_queryType;

    switch( m_queryType ) {
        case Artist:
            handleArtists( list );
            break;
        case Album:
            handleAlbums( list );
            break;
        case Track:
            handleTracks( list );
            break;
        default:
            break;
    // TODO handle remaining cases
    }

    if( !list.empty() ) {
        debug() << "_______________________       RESULTS!  ____________________________";
        m_noResults = false;
    }

}

void UpnpQueryMaker::handleArtists( const KIO::UDSEntryList &list )
{
    Meta::ArtistList ret;
    foreach( KIO::UDSEntry entry, list ) {
        if( entry.stringValue( KIO::UPNP_CLASS ) == "object.container.person.musicArtist" ) {
            debug() << this << "ARTIST" << entry.stringValue( KIO::UDSEntry::UDS_DISPLAY_NAME );
            ret << m_collection->cache()->getArtist( entry.stringValue( KIO::UDSEntry::UDS_DISPLAY_NAME ) );
        }
        else {
            debug() << this << entry.stringValue( KIO::UDSEntry::UDS_DISPLAY_NAME ) << "ARTIST" << entry.stringValue( KIO::UPNP_ARTIST );
            ret << m_collection->cache()->getArtist( entry.stringValue( KIO::UPNP_ARTIST ) );
        }
    }
    emitProperResult( Meta::Artist, ret );
}

void UpnpQueryMaker::handleAlbums( const KIO::UDSEntryList &list )
{
DEBUG_BLOCK
    debug() << "HANDLING ALBUMS" << list.length();
    Meta::AlbumList ret;
    foreach( KIO::UDSEntry entry, list ) {
        if( entry.stringValue( KIO::UPNP_CLASS ) == "object.container.album.musicAlbum" ) {
            debug() << this << "ALBUM" << entry.stringValue( KIO::UDSEntry::UDS_DISPLAY_NAME );
            ret << m_collection->cache()->getAlbum( entry.stringValue( KIO::UDSEntry::UDS_DISPLAY_NAME ) );
        }
        else {
            debug() << this << "ALBUM" << entry.stringValue( KIO::UPNP_ALBUM );
            ret << m_collection->cache()->getAlbum( entry.stringValue( KIO::UPNP_ALBUM ) );
        }
    }
    emitProperResult( Meta::Album, ret );
}

void UpnpQueryMaker::handleTracks( const KIO::UDSEntryList &list )
{
DEBUG_BLOCK
    debug() << "HANDLING TRACKS" << list.length();
    Meta::TrackList ret;
    foreach( KIO::UDSEntry entry, list ) {
        debug() << this << "TRACK as data ptr?" << m_asDataPtrs << entry.stringValue( KIO::UDSEntry::UDS_DISPLAY_NAME );
        ret << m_collection->cache()->getTrack( entry );
    }
    emitProperResult( Meta::Track, ret );
}

void UpnpQueryMaker::slotDone( KJob *job )
{
DEBUG_BLOCK
    m_runningJob = false;
    m_jobCount--;
    KIO::ListJob *ljob = static_cast<KIO::ListJob*>( job );
    KIO::ListJob *actual = m_inProgressQueries[QUrl::fromPercentEncoding( ljob->url().prettyUrl().toAscii() )];
    debug() << "!!!!!!!! DONE" << ljob << ljob->url();
    m_inProgressQueries.remove( QUrl::fromPercentEncoding( ljob->url().prettyUrl().toAscii() ) );


    if( m_jobCount <= 0 ) {
        if( m_noResults ) {
            debug() << "++++++++++++++++++++++++++++++++++++ NO RESULTS ++++++++++++++++++++++++";
            // TODO proper data types not just DataPtr
            Meta::DataList ret;
            Meta::UpnpTrack *fake = new Meta::UpnpTrack( m_collection );
            fake->setTitle( "No results" );
            fake->setYear( Meta::UpnpYearPtr( new Meta::UpnpYear( "2010" ) ) );
            Meta::DataPtr ptr( fake );
            ret << ptr;
            //emit newResultReady( m_collection->collectionId(), ret );
        }

        if ( m_asDataPtrs ) {
            emit newResultReady( m_collection->collectionId(), m_cacheEntries );
        }
        else {
            switch( m_queryType ) {
                case Artist:
                {
                    Meta::ArtistList list;
                    foreach( Meta::DataPtr ptr, m_cacheEntries )
                        list << Meta::ArtistPtr::staticCast( ptr );
                    emit newResultReady( m_collection->collectionId(), list );
                    break;
                }
                
                case Album:
                {
                    Meta::AlbumList list;
                    foreach( Meta::DataPtr ptr, m_cacheEntries )
                        list << Meta::AlbumPtr::staticCast( ptr );
                    emit newResultReady( m_collection->collectionId(), list );
                    break;
                }
                
                case Track:
                {
                    Meta::TrackList list;
                    foreach( Meta::DataPtr ptr, m_cacheEntries )
                        list << Meta::TrackPtr::staticCast( ptr );
                    emit newResultReady( m_collection->collectionId(), list );
                    break;
                }
            }
            //emit newResultReady( m_collection->collectionId(), list );
        }
        debug() << "ALL JOBS DONE< TERMINATING THIS QM" << this;
        emit queryDone();
    }
}

} //namespace Collections
