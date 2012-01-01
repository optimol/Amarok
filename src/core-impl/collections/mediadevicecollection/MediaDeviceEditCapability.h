/****************************************************************************************
 * Copyright (c) 2011 Matěj Laitl <matej@laitl.cz>                                      *
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

#ifndef MEDIADEVICE_EDIT_CAPABILITY_H
#define MEDIADEVICE_EDIT_CAPABILITY_H

#include "core/capabilities/EditCapability.h"
#include "MediaDeviceMeta.h"

namespace Meta
{

/**
 * This capability is different from other media device capabilities - it implements core
 * EditCapability and has nothing to do with Hander::Capability
 */
class MediaDeviceEditCapability : public Capabilities::EditCapability
{
    Q_OBJECT

    public:
        MediaDeviceEditCapability( MediaDeviceTrack *track );
        virtual ~MediaDeviceEditCapability();

        virtual bool isEditable() const { return m_track->isEditable(); }
        virtual void setAlbum( const QString &newAlbum );
        virtual void setAlbumArtist( const QString &newAlbumArtist );
        virtual void setArtist( const QString &newArtist );
        virtual void setComposer( const QString &newComposer );
        virtual void setGenre( const QString &newGenre );
        virtual void setYear( int newYear );
        virtual void setBpm( const qreal newBpm );
        virtual void setTitle( const QString &newTitle );
        virtual void setComment( const QString &newComment );
        virtual void setTrackNumber( int newTrackNumber );
        virtual void setDiscNumber( int newDiscNumber );
        virtual void setUidUrl( const QString &newUidUrl );

        virtual void beginMetaDataUpdate();
        virtual void endMetaDataUpdate();

    private:
        /**
         * Tells the underlying track to write back changes if and only if current update
         * is not a part of a larger batch (initiated by beginMetaDataUpdate())
         */
        void commitIfInNonBatchUpdate();

        bool m_inBatchUpdate;
        MediaDeviceTrackPtr m_track;
};

}

#endif
