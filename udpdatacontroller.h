///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2019 Edouard Griffiths, F4EXB.                                  //
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

#ifndef UDPDATACONTROLLER_H_
#define UDPDATACONTROLLER_H_

#include <string>
#include "datacontroller.h"

struct sockaddr_in;

namespace SerialDV
{

class SERIALDV_API UDPDataController : public DataController {
public:
    UDPDataController();
    virtual ~UDPDataController();

    virtual bool open(const std::string& ipAndPort, SERIAL_SPEED speed);

    virtual bool initResponse();
    virtual int  read(unsigned char* buffer, unsigned int lengthInBytes);
    virtual int  write(const unsigned char* buffer, unsigned int lengthInBytes);

    virtual void closeIt();

private:
    void openSocket(int port);
    void closeSocket();
    void setSendAddress(std::string& address, int port);
    int timeout_recvfrom(char *buf, int length, struct sockaddr_in *connection, int timeoutinmicroseconds);

    std::string m_ipAddress;
    int m_port;
    int m_sockFd;
    fd_set m_fds;
    struct sockaddr_in *m_sa;
    struct sockaddr_in *m_ra;
    unsigned char m_responseBuffer[2000];
    int m_responseSize;
    int m_responseIndex;
};

} // namespace SerialDV

#endif // UDPDATACONTROLLER_H_
