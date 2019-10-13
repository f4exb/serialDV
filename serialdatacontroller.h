///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#ifndef SERIALDATACONTROLLER_H_
#define SERIALDATACONTROLLER_H_

#if defined(__WINDOWS__)
#include <windows.h>
#endif

#include "datacontroller.h"

namespace SerialDV
{
class SERIALDV_API SerialDataController : public DataController {
public:
    SerialDataController();
    virtual ~SerialDataController();

    virtual bool open(const std::string& device, SERIAL_SPEED speed);

    virtual bool initResponse();
    virtual int  read(unsigned char* buffer, unsigned int lengthInBytes);
    virtual int  write(const unsigned char* buffer, unsigned int lengthInBytes);

    virtual void closeIt();

private:
    std::string    m_device;
    SERIAL_SPEED   m_speed;
#if defined(__WINDOWS__)
    HANDLE         m_handle;
    OVERLAPPED     m_readOverlapped;
    OVERLAPPED     m_writeOverlapped;
    unsigned char* m_readBuffer;
    unsigned int   m_readLength;
    bool           m_readPending;
#else
    int            m_fd;
#endif

#if defined(__WINDOWS__)
    int readNonblock(unsigned char* buffer, unsigned int length);
#endif
};

} // namespace SerialDV

#endif /* SERIALDATACONTROLLER_H_ */
