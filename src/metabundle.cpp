// Max Howell <max.howell@methylblue.com>, (C) 2004
// Alexandre Pereira de Oliveira <aleprj@gmail.com>, (C) 2005
// License: GNU General Public License V2


#define DEBUG_PREFIX "MetaBundle"

#include "amarok.h"
#include "debug.h"
#include "collectiondb.h"
#include <kfilemetainfo.h>
#include <kio/global.h>
#include <kmimetype.h>
#include <qdom.h>
#include <qfile.h> //decodePath()
#include <taglib/fileref.h>
#include <taglib/id3v1genres.h> //used to load genre list
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v1tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/mpegfile.h>
#include <taglib/oggfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/flacfile.h>
#include <taglib/textidentificationframe.h>
#include <taglib/xiphcomment.h>
#include <taglib/tbytevector.h>

#include <config.h>
#ifdef HAVE_MP4V2
#include "metadata/mp4/mp4file.h"
#include "metadata/mp4/mp4tag.h"
#else
#include "metadata/m4a/mp4file.h"
#include "metadata/m4a/mp4itunestag.h"
#endif

#include "metabundle.h"
#include "podcastbundle.h"

/// These are untranslated and used for storing/retrieving XML playlist
const QString MetaBundle::exactColumnName( int c ) //static
{
    switch( c )
    {
        case Filename:   return "Filename";
        case Title:      return "Title";
        case Artist:     return "Artist";
        case Composer:   return "Composer";
        case Year:       return "Year";
        case Album:      return "Album";
        case DiscNumber: return "DiscNumber";
        case Track:      return "Track";
        case Genre:      return "Genre";
        case Comment:    return "Comment";
        case Directory:  return "Directory";
        case Type:       return "Type";
        case Length:     return "Length";
        case Bitrate:    return "Bitrate";
        case SampleRate: return "SampleRate";
        case Score:      return "Score";
        case Rating:     return "Rating";
        case PlayCount:  return "PlayCount";
        case LastPlayed: return "LastPlayed";
        case Filesize:   return "Filesize";
        case Mood:       return "Mood";
    }
    return "<ERROR>";
}

const QString MetaBundle::prettyColumnName( int index ) //static
{
    switch( index )
    {
        case Filename:   return i18n( "Filename"    );
        case Title:      return i18n( "Title"       );
        case Artist:     return i18n( "Artist"      );
        case Composer:   return i18n( "Composer"    );
        case Year:       return i18n( "Year"        );
        case Album:      return i18n( "Album"       );
        case DiscNumber: return i18n( "Disc Number" );
        case Track:      return i18n( "Track"       );
        case Genre:      return i18n( "Genre"       );
        case Comment:    return i18n( "Comment"     );
        case Directory:  return i18n( "Directory"   );
        case Type:       return i18n( "Type"        );
        case Length:     return i18n( "Length"      );
        case Bitrate:    return i18n( "Bitrate"     );
        case SampleRate: return i18n( "Sample Rate" );
        case Score:      return i18n( "Score"       );
        case Rating:     return i18n( "Rating"      );
        case PlayCount:  return i18n( "Play Count"  );
        case LastPlayed: return i18n( "Last Played" );
        case Mood:       return i18n( "Mood"        );
        case Filesize:   return i18n( "File Size"   );
    }
    return "This is a bug.";
}

int MetaBundle::columnIndex( const QString &name )
{
    for( int i = 0; i < NUM_COLUMNS; ++i )
        if( exactColumnName( i ).lower() == name.lower() )
            return i;
    return -1;
}

MetaBundle::MetaBundle()
        : m_year( Undetermined )
        , m_discNumber( Undetermined )
        , m_track( Undetermined )
        , m_bitrate( Undetermined )
        , m_length( Undetermined )
        , m_sampleRate( Undetermined )
        , m_score( Undetermined )
        , m_rating( Undetermined )
        , m_playCount( Undetermined )
        , m_lastPlay( abs( Undetermined ) )
        , m_filesize( Undetermined )
        , m_type( other )
        , m_exists( true )
        , m_isValidMedia( true )
        , m_podcastBundle( 0 )
{
    init();
}

MetaBundle::MetaBundle( const KURL &url, bool noCache, TagLib::AudioProperties::ReadStyle readStyle )
    : m_url( url )
    , m_year( Undetermined )
    , m_discNumber( Undetermined )
    , m_track( Undetermined )
    , m_bitrate( Undetermined )
    , m_length( Undetermined )
    , m_sampleRate( Undetermined )
    , m_score( Undetermined )
    , m_rating( Undetermined )
    , m_playCount( Undetermined )
    , m_lastPlay( abs( Undetermined ) )
    , m_filesize( Undetermined )
    , m_type( other )
    , m_exists( url.protocol() == "file" && QFile::exists( url.path() ) )
    , m_isValidMedia( false ) //will be updated
    , m_podcastBundle( 0 )
{
    if ( m_exists )
    {
        if ( !noCache )
            m_isValidMedia = CollectionDB::instance()->bundleForUrl( this );

        if ( !m_isValidMedia || m_length <= 0 )
            readTags( readStyle );
    }
    else
    {
        // if it's a podcast we might get some info this way
        CollectionDB::instance()->bundleForUrl( this );
        m_bitrate = m_length = m_sampleRate = Unavailable;
    }
}

//StreamProvider ctor
MetaBundle::MetaBundle( const QString& title,
                        const QString& streamUrl,
                        const int      bitrate,
                        const QString& genre,
                        const QString& streamName,
                        const KURL& url )
        : m_url       ( url )
        , m_genre     ( genre )
        , m_streamName( streamName )
        , m_streamUrl ( streamUrl )
        , m_year( 0 )
        , m_discNumber( 0 )
        , m_track( 0 )
        , m_bitrate( bitrate )
        , m_length( Irrelevant )
        , m_sampleRate( Unavailable )
        , m_score( Undetermined )
        , m_rating( Undetermined )
        , m_playCount( Undetermined )
        , m_lastPlay( abs( Undetermined ) )
        , m_filesize( Undetermined )
        , m_type( other )
        , m_exists( true )
        , m_isValidMedia( true )
        , m_podcastBundle( 0 )
{
    if( title.contains( '-' ) )
    {
        m_title  = title.section( '-', 1, 1 ).stripWhiteSpace();
        m_artist = title.section( '-', 0, 0 ).stripWhiteSpace();
    }
    else
    {
        m_title  = title;
        m_artist = streamName; //which is sort of correct..
    }
}

MetaBundle::MetaBundle( const MetaBundle &bundle )
    : m_url( bundle.m_url )
    , m_title( bundle.m_title )
    , m_artist( bundle.m_artist )
    , m_composer( bundle.m_composer )
    , m_album( bundle.m_album )
    , m_comment( bundle.m_comment )
    , m_genre( bundle.m_genre )
    , m_streamName( bundle.m_streamName )
    , m_streamUrl( bundle.m_streamUrl )
    , m_year( bundle.m_year )
    , m_discNumber( bundle.m_discNumber )
    , m_track( bundle.m_track )
    , m_bitrate( bundle.m_bitrate )
    , m_length( bundle.m_length )
    , m_sampleRate( bundle.m_sampleRate )
    , m_score( bundle.m_score )
    , m_rating( bundle.m_rating )
    , m_playCount( bundle.m_playCount )
    , m_lastPlay( bundle.m_lastPlay )
    , m_filesize( bundle.m_filesize )
    , m_type( bundle.m_type )
    , m_exists( bundle.m_exists )
    , m_isValidMedia( bundle.m_isValidMedia )
    , m_podcastBundle( 0 )
{
    if( bundle.m_podcastBundle )
        setPodcastBundle( *bundle.m_podcastBundle );
}

MetaBundle::~MetaBundle()
{
    delete m_podcastBundle;
}

MetaBundle&
MetaBundle::operator=( const MetaBundle& bundle )
{
    m_url = bundle.m_url;
    m_title = bundle.m_title;
    m_artist = bundle.m_artist;
    m_composer = bundle.m_composer;
    m_album = bundle.m_album;
    m_comment = bundle.m_comment;
    m_genre = bundle.m_genre;
    m_streamName = bundle.m_streamName;
    m_streamUrl = bundle.m_streamUrl;
    m_year = bundle.m_year;
    m_discNumber = bundle.m_discNumber;
    m_track = bundle.m_track;
    m_bitrate = bundle.m_bitrate;
    m_length = bundle.m_length;
    m_sampleRate = bundle.m_sampleRate;
    m_score = bundle.m_score;
    m_rating = bundle.m_rating;
    m_playCount = bundle.m_playCount;
    m_lastPlay = bundle.m_lastPlay;
    m_filesize = bundle.m_filesize;
    m_type = bundle.m_type;
    m_exists = bundle.m_exists;
    m_isValidMedia = bundle.m_isValidMedia;
    m_podcastBundle = 0;
    if( bundle.m_podcastBundle )
        setPodcastBundle( *bundle.m_podcastBundle );

    return *this;
}


bool
MetaBundle::checkExists()
{
    m_exists = isStream() || ( url().protocol() == "file" && QFile::exists( url().path() ) );

    return m_exists;
}

bool
MetaBundle::operator==( const MetaBundle& bundle ) const
{
    return artist()     == bundle.artist() &&
           title()      == bundle.title() &&
           composer()   == bundle.composer() &&
           album()      == bundle.album() &&
           year()       == bundle.year() &&
           comment()    == bundle.comment() &&
           genre()      == bundle.genre() &&
           track()      == bundle.track() &&
           discNumber() == bundle.discNumber() &&
           length()     == bundle.length() &&
           bitrate()    == bundle.bitrate() &&
           sampleRate() == bundle.sampleRate();
    // FIXME: check for size equality?
}

void
MetaBundle::clear()
{
    *this = MetaBundle();
}

void
MetaBundle::init( TagLib::AudioProperties *ap )
{
    if ( ap )
    {
        m_bitrate    = ap->bitrate();
        m_length     = ap->length();
        m_sampleRate = ap->sampleRate();
    }
    else
        m_bitrate = m_length = m_sampleRate = Undetermined;
}

void
MetaBundle::init( const KFileMetaInfo& info )
{
    if( info.isValid() && !info.isEmpty() )
    {
        m_artist     = info.item( "Artist" ).string();
        m_album      = info.item( "Album" ).string();
        m_comment    = info.item( "Comment" ).string();
        m_genre      = info.item( "Genre" ).string();
        m_year       = info.item( "Year" ).string().toInt();
        m_track      = info.item( "Track" ).string().toInt();
        m_bitrate    = info.item( "Bitrate" ).value().toInt();
        m_length     = info.item( "Length" ).value().toInt();
        m_sampleRate = info.item( "Sample Rate" ).value().toInt();

        // For title, check if it is valid. If not, use prettyTitle.
        // @see bug:83650
        const KFileMetaInfoItem item = info.item( "Title" );
        m_title = item.isValid() ? item.string() : prettyTitle( m_url.fileName() );

        // because whoever designed KMetaInfoItem is a donkey
        #define makeSane( x ) if( x == "---" ) x = null;
        QString null;
        makeSane( m_artist );
        makeSane( m_album );
        makeSane( m_comment );
        makeSane( m_genre  );
        makeSane( m_title );
        #undef makeSane

        m_isValidMedia = true;
    }
    else
    {
        m_bitrate = m_length = m_sampleRate = m_filesize = Undetermined;
        m_isValidMedia = false;
    }
}

void
MetaBundle::readTags( TagLib::AudioProperties::ReadStyle readStyle )
{
    if( url().protocol() != "file" )
        return;

    const QString path = url().path();
    TagLib::FileRef fileref;
    TagLib::Tag *tag = 0;


    fileref = TagLib::FileRef( QFile::encodeName( path ), true, readStyle );

    if( !fileref.isNull() )
    {
        tag = fileref.tag();

        if ( tag )
        {
            #define strip( x ) TStringToQString( x ).stripWhiteSpace()
            setTitle( strip( tag->title() ) );
            setArtist( strip( tag->artist() ) );
            setAlbum( strip( tag->album() ) );
            setComment( strip( tag->comment() ) );
            setGenre( strip( tag->genre() ) );
            setYear( tag->year() );
            setTrack( tag->track() );
            #undef strip

            m_isValidMedia = true;

            m_filesize = QFile( path ).size();
        }


    /* As mpeg implementation on TagLib uses a Tag class that's not defined on the headers,
       we have to cast the files, not the tags! */

        QString disc;
        if ( TagLib::MPEG::File *file = dynamic_cast<TagLib::MPEG::File *>( fileref.file() ) )
        {
            m_type = mp3;
            if ( file->ID3v2Tag() )
            {
                if ( !file->ID3v2Tag()->frameListMap()["TPOS"].isEmpty() )
                    disc = TStringToQString( file->ID3v2Tag()->frameListMap()["TPOS"].front()->toString() ).stripWhiteSpace();

                if ( !file->ID3v2Tag()->frameListMap()["TCOM"].isEmpty() )
                    setComposer( TStringToQString( file->ID3v2Tag()->frameListMap()["TCOM"].front()->toString() ).stripWhiteSpace() );
            }
        }
        else if ( TagLib::Ogg::Vorbis::File *file = dynamic_cast<TagLib::Ogg::Vorbis::File *>( fileref.file() ) )
        {
            m_type = ogg;
            if ( file->tag() )
            {
                if ( !file->tag()->fieldListMap()[ "COMPOSER" ].isEmpty() )
                    setComposer( TStringToQString( file->tag()->fieldListMap()["COMPOSER"].front() ).stripWhiteSpace() );

                if ( !file->tag()->fieldListMap()[ "DISCNUMBER" ].isEmpty() )
                    disc = TStringToQString( file->tag()->fieldListMap()["DISCNUMBER"].front() ).stripWhiteSpace();
            }
        }
        else if ( TagLib::FLAC::File *file = dynamic_cast<TagLib::FLAC::File *>( fileref.file() ) )
        {
            m_type = flac;
            if ( file->xiphComment() )
            {
                if ( !file->xiphComment()->fieldListMap()[ "COMPOSER" ].isEmpty() )
                    setComposer( TStringToQString( file->xiphComment()->fieldListMap()["COMPOSER"].front() ).stripWhiteSpace() );

                if ( !file->xiphComment()->fieldListMap()[ "DISCNUMBER" ].isEmpty() )
                    disc = TStringToQString( file->xiphComment()->fieldListMap()["DISCNUMBER"].front() ).stripWhiteSpace();
            }
        }
        else if ( TagLib::MP4::File *file = dynamic_cast<TagLib::MP4::File *>( fileref.file() ) )
        {
            m_type = mp4;
            TagLib::MP4::Tag *mp4tag = dynamic_cast<TagLib::MP4::Tag *>( file->tag() );
            if( mp4tag )
            {
                setComposer( TStringToQString( mp4tag->composer() ) );
                disc = QString::number( mp4tag->disk() );
            }
        }

        if ( !disc.isEmpty() )
        {
            int i = disc.find ('/');
            if ( i != -1 )
                // disc.right( i ).toInt() is total number of discs, we don't use this at the moment
                setDiscNumber( disc.left( i ).toInt() );
            else
                setDiscNumber( disc.toInt() );
        }

        init( fileref.audioProperties() );
    }

    //FIXME disabled for beta4 as it's simpler to not got 100 bug reports
    //else if( KMimeType::findByUrl( m_url )->is( "audio" ) )
    //    init( KFileMetaInfo( m_url, QString::null, KFileMetaInfo::Everything ) );
}

void MetaBundle::updateFilesize()
{
    if( url().protocol() != "file" )
    {
        m_filesize = Undetermined;
        return;
    }

    const QString path = url().path();
    m_filesize = QFile( path ).size();

    debug() << "filesize = " << m_filesize << endl;
}

int MetaBundle::score() const
{
    if( m_score == Undetermined )
        //const_cast is ugly, but other option was mutable, and then we lose const correctness checking
        //everywhere else
        *const_cast<int*>(&m_score) = CollectionDB::instance()->getSongPercentage( m_url.path() );
    return m_score;
}

int MetaBundle::rating() const
{
    if( m_rating == Undetermined )
        *const_cast<int*>(&m_rating) = CollectionDB::instance()->getSongRating( m_url.path() );
    return m_rating;
}

int MetaBundle::playCount() const
{
    if( m_playCount == Undetermined )
        *const_cast<int*>(&m_playCount) = CollectionDB::instance()->getPlayCount( m_url.path() );
    return m_playCount;
}

uint MetaBundle::lastPlay() const
{
    if( (int)m_lastPlay == abs(Undetermined) )
        *const_cast<uint*>(&m_lastPlay) = CollectionDB::instance()->getLastPlay( m_url.path() ).toTime_t();
    return m_lastPlay;
}

void MetaBundle::copyFrom( const MetaBundle &bundle )
{
    setTitle( bundle.title() );
    setArtist( bundle.artist() );
    setComposer( bundle.composer() );
    setAlbum( bundle.album() );
    setYear( bundle.year() );
    setDiscNumber( bundle.discNumber() );
    setComment( bundle.comment() );
    setGenre( bundle.genre() );
    setTrack( bundle.track() );
    setLength( bundle.length() );
    setBitrate( bundle.bitrate() );
    setSampleRate( bundle.sampleRate() );
    setScore( bundle.score() );
    setRating( bundle.rating() );
    setPlayCount( bundle.playCount() );
    setLastPlay( bundle.lastPlay() );
    setFileType( bundle.fileType() );
    setFilesize( bundle.filesize() );
    if( bundle.m_podcastBundle )
        setPodcastBundle( *bundle.m_podcastBundle );
    else
    {
        delete m_podcastBundle;
        m_podcastBundle = 0;
    }
}

void MetaBundle::setExactText( int column, const QString &newText )
{
    switch( column )
    {
        case Title:      setTitle(      newText );         break;
        case Artist:     setArtist(     newText );         break;
        case Composer:   setComposer(   newText );         break;
        case Year:       setYear(       newText.toInt() ); break;
        case Album:      setAlbum(      newText );         break;
        case DiscNumber: setDiscNumber( newText.toInt() ); break;
        case Track:      setTrack(      newText.toInt() ); break;
        case Genre:      setGenre(      newText );         break;
        case Comment:    setComment(    newText );         break;
        case Length:     setLength(     newText.toInt() ); break;
        case Bitrate:    setBitrate(    newText.toInt() ); break;
        case SampleRate: setSampleRate( newText.toInt() ); break;
        case Score:      setScore(      newText.toInt() ); break;
        case Rating:     setRating(     newText.toInt() ); break;
        case PlayCount:  setPlayCount(  newText.toInt() ); break;
        case LastPlayed: setLastPlay(   newText.toInt() ); break;
        case Filesize:   setFilesize(   newText.toInt() ); break;
        case Type:       setFileType(   newText.toInt() ); break;
        default: warning() << "Tried to set the text of an immutable or nonexistent column! [" << column << endl;
   }
}

QString MetaBundle::exactText( int column ) const
{
    switch( column )
    {
        case Filename:   return filename();
        case Title:      return title();
        case Artist:     return artist();
        case Composer:   return composer();
        case Year:       return QString::number( year() );
        case Album:      return album();
        case DiscNumber: return QString::number( discNumber() );
        case Track:      return QString::number( track() );
        case Genre:      return genre();
        case Comment:    return comment();
        case Directory:  return directory();
        case Type:       return QString::number( fileType() );
        case Length:     return QString::number( length() );
        case Bitrate:    return QString::number( bitrate() );
        case SampleRate: return QString::number( sampleRate() );
        case Score:      return QString::number( score() );
        case Rating:     return QString::number( rating() );
        case PlayCount:  return QString::number( playCount() );
        case LastPlayed: return QString::number( lastPlay() );
        case Filesize:   return QString::number( filesize() );
        case Mood:       return QString::null;
        default: warning() << "Tried to get the text of a nonexistent column! [" << column << endl;
    }

    return QString::null; //shouldn't happen
}

QString MetaBundle::prettyText( int column ) const
{
    QString text;
    switch( column )
    {
        case Filename:   text = isStream() ? url().prettyURL() : filename();                         break;
        case Title:      text = title().isEmpty() ? MetaBundle::prettyTitle( filename() ) : title(); break;
        case Artist:     text = artist();                                                            break;
        case Composer:   text = composer();                                                          break;
        case Year:       text = year() ? QString::number( year() ) : QString::null;                  break;
        case Album:      text = album();                                                             break;
        case DiscNumber: text = discNumber() ? QString::number( discNumber() ) : QString::null;      break;
        case Track:      text = track() ? QString::number( track() ) : QString::null;                break;
        case Genre:      text = genre();                                                             break;
        case Comment:    text = comment();                                                           break;
        case Directory:  text = url().isEmpty() ? QString() : directory();                           break;
        case Type:       text = url().isEmpty() ? QString() : type();                                break;
        case Length:     text = prettyLength( length(), true );                                      break;
        case Bitrate:    text = prettyBitrate( bitrate() );                                          break;
        case SampleRate: text = prettySampleRate();                                                  break;
        case Score:      text = QString::number( score() );                                          break;
        case Rating:     text = prettyRating();                                                      break;
        case PlayCount:  text = QString::number( playCount() );                                      break;
        case LastPlayed: text = amaroK::verboseTimeSince( lastPlay() );                              break;
        case Filesize:   text = prettyFilesize();                                                    break;
        case Mood:       text = QString::null;                                                       break;
        default: warning() << "Tried to get the text of a nonexistent column!" << endl;              break;
    }

    return text.stripWhiteSpace();
}

bool MetaBundle::isAdvancedExpression( const QString &expression ) //static
{
    return ( expression.contains( "\""  ) ||
             expression.contains( ":"   ) ||
             expression.contains( "-"   ) ||
             expression.contains( "AND" ) ||
             expression.contains( "OR"  ) );
}

bool MetaBundle::matchesSimpleExpression( const QString &expression, QValueList<int> columns ) const
{
    const QStringList terms = QStringList::split( ' ', expression.lower() );
    bool matches = true;
    for( uint x = 0; matches && x < terms.count(); ++x )
    {
        uint y = 0, n = columns.count();
        for(; y < n; ++y )
            if ( prettyText( columns[y] ).lower().contains( terms[x] ) )
                break;
        matches = ( y < n );
    }

    return matches;
}

bool MetaBundle::matchesExpression( const QString &expression, QValueList<int> defaultColumns ) const
{
    return matchesParsedExpression( parseExpression( expression ), defaultColumns );
}

MetaBundle::ParsedExpression MetaBundle::parseExpression( QString expression ) //static
{
    if( expression.contains( "\"" ) % 2 == 1 ) expression += "\""; //make an even number of "s

    //something like thingy"bla"stuff -> thingy "bla" stuff
    bool odd = false;
    for( int pos = expression.find( "\"" );
         pos >= 0 && pos <= (int)expression.length();
         pos = expression.find( "\"", pos + 1 ) )
    {
        expression = expression.insert( odd ? ++pos : pos++, " " );
        odd = !odd;
    }
    expression = expression.simplifyWhiteSpace();

    int x; //position in string of the end of the next element
    bool OR = false, minus = false; //whether the next element is to be OR, and/or negated
    QString tmp, s = "", field = ""; //the current element, a tempstring, and the field: of the next element
    QStringList tmpl; //list of elements of which at least one has to match (OR)
    QValueList<QStringList> allof; //list of all the tmpls, of which all have to match
    while( !expression.isEmpty() )  //seperate expression into parts which all have to match
    {
        if( expression.startsWith( " " ) )
            expression = expression.mid( 1 ); //cuts off the first character
        if( expression.startsWith( "\"" ) ) //take stuff in "s literally (basically just ends up ignoring spaces)
        {
            expression = expression.mid( 1 );
            x = expression.find( "\"" );
        }
        else
            x = expression.find( " " );
        if( x < 0 )
            x = expression.length();
        s = expression.left( x ); //get the element
        expression = expression.mid( x + 1 ); //move on

        if( !field.isEmpty() || ( s != "-" && s != "AND" && s != "OR" &&
                                  !s.endsWith( ":" ) && !s.endsWith( ":>" ) && !s.endsWith( ":<" ) ) )
        {
            if( !OR && !tmpl.isEmpty() ) //add the OR list to the AND list
            {
                allof += tmpl;
                tmpl.clear();
            }
            else
                OR = false;
            tmp = field + s;
            if( minus )
            {
                tmp = "-" + tmp;
                minus = false;
            }
            tmpl += tmp;
            tmp = field = "";
        }
        else if( s.endsWith( ":" ) || s.endsWith( ":>" ) || s.endsWith( ":<" ) )
            field = s;
        else if( s == "OR" )
            OR = true;
        else if( s == "-" )
            minus = true;
        else
            OR = false;
    }
    if( !tmpl.isEmpty() )
        allof += tmpl;

    return allof;
}

bool MetaBundle::matchesParsedExpression( ParsedExpression data, QValueList<int> defaults ) const
{
    for( uint i = 0, n = data.count(); i < n; ++i ) //check each part for matchiness
    {
        bool b = false; //whether at least one matches
        for( uint ii = 0, count = data[i].count(); ii < count; ++ii )
        {
            QString s = data[i][ii];
            bool neg = s.startsWith( "-" );
            if ( neg )
                s = s.mid( 1 ); //cut off the -
            int x = s.find( ":" ); //where the field ends and the thing-to-match begins
            int column = -1;
            if( x > 0 )
                column = columnIndex( s.left( x ).lower() );
            if( column >= 0 ) //a field was specified and it exists
            {
                QString q = s.mid(x + 1), v = prettyText( column ).lower(), w = q.lower();
                //q = query, v = contents of the field, w = match against it
                bool condition; //whether it matches, not taking negation into account

                bool numeric;
                switch( column )
                {
                    case Year:
                    case DiscNumber:
                    case Track:
                    case Bitrate:
                    case SampleRate:
                    case Score:
                    case Rating:
                    case PlayCount:
                    case LastPlayed:
                    case Filesize:
                        numeric = true;
                        break;
                    default:
                        numeric = false;
                }

                if( column == Filesize )
                    v = QString::number( filesize() / ( 1024 * 1024 ) );

                if( q.startsWith( ">" ) )
                {
                    w = w.mid( 1 );
                    if( numeric )
                        condition = v.toInt() > w.toInt();
                    else if( column == Length )
                    {
                        int g = v.find( ":" ), h = w.find( ":" );
                        condition = v.left( g ).toInt() > w.left( h ).toInt() ||
                                    ( v.left( g ).toInt() == w.left( h ).toInt() &&
                                      v.mid( g + 1 ).toInt() > w.mid( h + 1 ).toInt() );
                    }
                    else
                        condition = v > w; //compare the strings
                }
                else if( q.startsWith( "<" ) )
                {
                    w = w.mid(1);
                    if( numeric )
                        condition = v.toInt() < w.toInt();
                    else if( column == Length )
                    {
                        int g = v.find( ":" ), h = w.find( ":" );
                        condition = v.left( g ).toInt() < w.left( h ).toInt() ||
                                    ( v.left( g ).toInt() == w.left( h ).toInt() &&
                                      v.mid( g + 1 ).toInt() < w.mid( h + 1 ).toInt() );
                    }
                    else
                        condition = v < w;
                }
                else
                {
                    if( numeric )
                        condition = v.toInt() == w.toInt();
                    else if( column == Length )
                    {
                        int g = v.find( ":" ), h = w.find( ":" );
                        condition = v.left( g ).toInt() == w.left( h ).toInt() &&
                                    v.mid( g + 1 ).toInt() == w.mid( h + 1 ).toInt();
                    }
                    else
                        condition = v.contains( q, false );
                }
                if( condition == ( neg ? false : true ) )
                {
                    b = true;
                    break;
                }
            }
            else //check just the default fields
            {
                for( int it = 0, end = defaults.size(); it != end; ++it )
                {
                    b = prettyText( defaults[it] ).contains( s, false ) == ( neg ? false : true );
                    if( ( neg && !b ) || ( !neg && b ) )
                        break;
                }
                if( b )
                    break;
            }
        }
        if( !b )
            return false;
    }

    return true;
}

QString
MetaBundle::prettyTitle() const
{
    QString s = artist();

    //NOTE this gets regressed often, please be careful!
    //     whatever you do, handle the stream case, streams have no artist but have an excellent title

    //FIXME doesn't work for resume playback

    if( s.isEmpty() )
        s = title();
    else
        s = i18n("%1 - %2").arg( artist(), title() );

    if( s.isEmpty() ) s = prettyTitle( filename() );

    return s;
}

QString
MetaBundle::veryNiceTitle() const
{
    QString s;
    //NOTE I'm not sure, but the notes and FIXME's in the prettyTitle function should be fixed now.
    //     If not then they do apply to this function also!
    if( !title().isEmpty() )
    {
        if( !artist().isEmpty() )
            s = i18n( "%1 by %2" ).arg( title() ).arg( artist() );
        else
            s = title();
    }
    else
    {
        s = prettyTitle( filename() );
    }
    return s;
}

QString
MetaBundle::prettyTitle( const QString &filename ) //static
{
    QString s = filename; //just so the code is more readable

    //remove file extension, s/_/ /g and decode %2f-like sequences
    s = s.left( s.findRev( '.' ) ).replace( '_', ' ' );
    s = KURL::decode_string( s );

    return s;
}

QString
MetaBundle::prettyLength( int seconds, bool showHours ) //static
{
    if( seconds > 0 ) return prettyTime( seconds, showHours );
    if( seconds == Undetermined ) return "?";
    if( seconds == Irrelevant  ) return "-";

    return QString::null; //Unavailable = ""
}

QString
MetaBundle::prettyTime( uint seconds, bool showHours ) //static
{
    QString s = QChar( ':' );
    s.append( zeroPad( seconds % 60 ) ); //seconds
    seconds /= 60;

    if( showHours && seconds >= 60)
    {
        s.prepend( zeroPad( seconds % 60 ) ); //minutes
        s.prepend( ':' );
        seconds /= 60;
    }

    //don't zeroPad the last one, as it can be greater than 2 digits
    s.prepend( QString::number( seconds ) ); //hours or minutes depending on above if block

    return s;
}

QString
MetaBundle::prettyBitrate( int i )
{
    //the point here is to force sharing of these strings returned from prettyBitrate()
    static const QString bitrateStore[9] = {
            "?", "32", "64", "96", "128", "160", "192", "224", "256" };

    return (i >=0 && i <= 256 && i % 32 == 0)
                ? bitrateStore[ i / 32 ]
                : prettyGeneric( "%1", i );
}

QString
MetaBundle::prettyFilesize( int s )
{
    return KIO::convertSize( s );
}

QString
MetaBundle::prettyRating( int r )
{
    switch( r )
    {
        case 1: return i18n( "1 - Crap" );
        case 2: return i18n( "2 - Tolerable" );
        case 3: return i18n( "3 - Good" );
        case 4: return i18n( "4 - Excellent" );
        case 5: return i18n( "5 - Inconceivable!" );
        case 0: default: return i18n( "Not rated" ); // assume weird values as not rated
    }
}

QStringList
MetaBundle::ratingList()
{
    QStringList list;
    for ( int i = 0; i<=5; i++ )
        list += prettyRating( i );
    return list;
}

QStringList
MetaBundle::genreList() //static
{
    QStringList list;

    TagLib::StringList genres = TagLib::ID3v1::genreList();
    for( TagLib::StringList::ConstIterator it = genres.begin(), end = genres.end(); it != end; ++it )
        list += TStringToQString( (*it) );

    list.sort();

    return list;
}

void
MetaBundle::setExtendedTag( TagLib::File *file, int tag, const QString value )
{
    char *id = 0;

    if ( m_type == mp3 )
    {
        switch( tag )
        {
            case ( composerTag ): id = "TCOM"; break;
            case ( discNumberTag ): id = "TPOS"; break;
        }
        TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File *>( file );
        if ( mpegFile && mpegFile->ID3v2Tag() )
        {
            if ( value.isEmpty() )
                mpegFile->ID3v2Tag()->removeFrames( id );
            else
            {
                if( !mpegFile->ID3v2Tag()->frameListMap()[id].isEmpty() )
                    mpegFile->ID3v2Tag()->frameListMap()[id].front()->setText( QStringToTString( value ) );
                else
                {
                    TagLib::ID3v2::TextIdentificationFrame *frame = new TagLib::ID3v2::TextIdentificationFrame( id, TagLib::ID3v2::FrameFactory::instance()->defaultTextEncoding() );
                    frame->setText( QStringToTString( value ) );
                    mpegFile->ID3v2Tag()->addFrame( frame );
                }
            }
        }
    }
    else if ( m_type == ogg )
    {
        switch( tag )
        {
            case ( composerTag ): id = "COMPOSER"; break;
            case ( discNumberTag ): id = "DISCNUMBER"; break;
        }
        TagLib::Ogg::Vorbis::File *oggFile = dynamic_cast<TagLib::Ogg::Vorbis::File *>( file );
        if ( oggFile && oggFile->tag() )
        {
            value.isEmpty() ?
                oggFile->tag()->removeField( id ):
                oggFile->tag()->addField( id, QStringToTString( value ), true );
        }
    }
    else if ( m_type == flac )
    {
        switch( tag )
        {
            case ( composerTag ): id = "COMPOSER"; break;
            case ( discNumberTag ): id = "DISCNUMBER"; break;
        }
        TagLib::FLAC::File *flacFile = dynamic_cast<TagLib::FLAC::File *>( file );
        if ( flacFile && flacFile->xiphComment() )
        {
            value.isEmpty() ?
            flacFile->xiphComment()->removeField( id ):
            flacFile->xiphComment()->addField( id, QStringToTString( value ), true );
        }
    }
    else if ( m_type == mp4 )
    {
        TagLib::MP4::Tag *mp4tag = dynamic_cast<TagLib::MP4::Tag *>( file->tag() );
        if( mp4tag )
        {
            switch( tag )
            {
                case ( composerTag ): mp4tag->setComposer( QStringToTString( value ) ); break;
                case ( discNumberTag ): mp4tag->setDisk( value.toInt() );
            }
        }
    }
}

void
MetaBundle::setPodcastBundle( const PodcastEpisodeBundle &peb )
{
    delete m_podcastBundle;
    m_podcastBundle = new PodcastEpisodeBundle;
    *m_podcastBundle = peb;
}

bool
MetaBundle::save()
{
    //Set default codec to UTF-8 (see bugs 111246 and 111232)
    TagLib::ID3v2::FrameFactory::instance()->setDefaultTextEncoding(TagLib::String::UTF8);

    QCString path = QFile::encodeName( url().path() );
    TagLib::FileRef f( path, false );

    if ( !f.isNull() )
    {
        TagLib::Tag * t = f.tag();
        t->setTitle( QStringToTString( title() ) );
        t->setArtist( QStringToTString( artist().string() ) );
        t->setAlbum( QStringToTString( album().string() ) );
        t->setTrack( track() );
        t->setYear( year() );
        t->setComment( QStringToTString( comment().string() ) );
        t->setGenre( QStringToTString( genre().string() ) );

        if ( hasExtendedMetaInformation() )
        {
            setExtendedTag( f.file(), composerTag, composer() );
            setExtendedTag( f.file(), discNumberTag, discNumber() ? QString::number( discNumber() ) : QString() );
        }
        return f.save();
    }
    return false;
}

bool MetaBundle::save( QTextStream &stream, const QStringList &attributes, int indent ) const
{
    QDomDocument QDomSucksItNeedsADocument;
    QDomElement item = QDomSucksItNeedsADocument.createElement( "item" );
    item.setAttribute( "url", url().url() );

    for( int i = 0, n = attributes.count(); i < n; i += 2 )
        item.setAttribute( attributes[i], attributes[i+1] );

    for( int i = 0; i < NUM_COLUMNS; ++i )
    {
        QDomElement tag = QDomSucksItNeedsADocument.createElement( exactColumnName( i ) );
        QDomText text = QDomSucksItNeedsADocument.createTextNode( exactText( i ) );
        tag.appendChild( text );

        item.appendChild( tag );
    }

    item.save( stream, indent );
    return true;
}
