/*
 *  Copyright (c) 2007 Jeff Mitchell <kde-dev@emailgoeshere.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef PMP_DEVICE_H
#define PMP_DEVICE_H

class PMPBackend;
class PMPProtocol;

class PMPDevice
{
    public:
        PMPDevice() { m_slave = 0; m_device = Solid::Device(); m_backend = 0; }
        PMPDevice( PMPProtocol *slave, const Solid::Device &device ) { m_slave = slave; m_device = device; m_backend = 0; }
        ~PMPDevice() { if( m_backend ) delete m_backend; }

        void initialize();
        bool isValid() const;
        PMPBackend* backend() const { return m_backend; }

    private:
        Solid::Device m_device;
        PMPBackend* m_backend;
        PMPProtocol* m_slave;
};

#endif /*PMP_DEVICE_H*/
