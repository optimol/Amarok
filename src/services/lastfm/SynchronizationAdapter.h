/****************************************************************************************
 * Copyright (c) 2012 Matěj Laitl <matej@laitl.cz>                                      *
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

#ifndef SYNCHRONIZATIONADAPTER_H
#define SYNCHRONIZATIONADAPTER_H

#include "statsyncing/Provider.h"

#include <QSemaphore>

class SynchronizationAdapter : public StatSyncing::Provider
{
    Q_OBJECT

    public:
        /**
         * @param user Last.fm username
         */
        SynchronizationAdapter( const QString &user );
        virtual ~SynchronizationAdapter();

        virtual QString id() const;
        virtual QString prettyName() const;
        virtual KIcon icon() const;
        virtual qint64 reliableTrackMetaData() const;
        virtual qint64 writableTrackStatsData() const;
        virtual Preference defaultPreference();
        virtual QSet<QString> artists();
        virtual StatSyncing::TrackList artistTracks( const QString &artistName );

    signals:
        /// hacks to create and start Last.fm queries in main eventloop
        // Last.fm indexes from 1!
        void startArtistSearch( int page );
        void startTrackSearch( QString artistName, int page );
        void startTagSearch( QString artistName, QString trackName );

    private slots:
        /// @see startArtistSearch
        void slotStartArtistSearch( int page );
        void slotStartTrackSearch( QString artistName, int page );
        void slotStartTagSearch( QString artistName, QString trackName );

        void slotArtistsReceived();
        void slotTracksReceived();
        void slotTagsReceived();

    private:
        QString m_user;

        QSet<QString> m_artists;
        StatSyncing::TrackList m_tracks;
        StatSyncing::TrackList m_tagQueue; // tracks waiting to be assigned tags
        QSemaphore m_semaphore;
};

#endif // SYNCHRONIZATIONADAPTER_H
