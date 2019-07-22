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

#include <regex>
#include <iostream>
#include <algorithm>

#ifdef __WINDOWS__
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include "udpdatacontroller.h"

namespace SerialDV
{

UDPDataController::UDPDataController() :
    m_responseSize(0),
    m_responseIndex(0)
{
#ifdef __WINDOWS__
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(1, 1), &wsa_data);
#endif
    m_sa = new sockaddr_in;
    m_ra = new sockaddr_in;
}

UDPDataController::~UDPDataController()
{
    delete m_ra;
    delete m_sa;
#ifdef __WINDOWS__
    WSACleanup();
#endif
}

bool UDPDataController::open(const std::string& ipAndPort, SERIAL_SPEED speed)
{
    (void) speed;
    std::regex ip_port_regex("(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}):(\\d{4,5})");
    std::smatch ip_port_match;
    std::regex_search(ipAndPort, ip_port_match, ip_port_regex);

    if (ip_port_match.size() == 3)
    {
        m_ipAddress = ip_port_match[1];
        std::string m_port_str = ip_port_match[2];
        m_port = atoi(m_port_str.c_str());

        if (m_port < 1024)
        {
            std::cerr << "UDPDataController::open: not a valid port: " << m_port << std::endl;
            return false;
        }

        openSocket(m_port);

        if (m_sockFd < 0)
        {
            std::cerr << "UDPDataController::open: could not open socket at port: " << m_port << std::endl;
            return false;
        }

        setSendAddress(m_ipAddress, m_port);

        std::cout << "UDPDataController::open: ip: " << m_ipAddress << " port: " << m_port << std::endl;
        return true;
    }
    else
    {
        std::cerr << "UDPDataController::open: not a valid IP address and port: " << ipAndPort << std::endl;
        return false;
    }
}

bool UDPDataController::initResponse()
{
    m_responseSize = timeout_recvfrom(m_sockFd, (char *) m_responseBuffer, 2000, m_ra, 50000);
    m_responseIndex = 0;
    //std::cerr << "UDPDataController::initResponse: read: " << m_responseSize << std::endl;
    return m_responseSize > 0;
}

int UDPDataController::read(unsigned char* buffer, unsigned int lengthInBytes)
{
    int remain = m_responseSize - m_responseIndex;

    if (remain > 0)
    {
        if (lengthInBytes >= (unsigned int) remain)
        {
            std::copy(m_responseBuffer+m_responseIndex, m_responseBuffer+m_responseSize, buffer);
            m_responseIndex = m_responseSize;
            return remain;
        }
        else
        {
            std::copy(m_responseBuffer+m_responseIndex, m_responseBuffer+m_responseIndex+lengthInBytes, buffer);
            m_responseIndex += lengthInBytes;
            return lengthInBytes;
        }
    }
    else
    {
        return 0;
    }
}

int UDPDataController::write(const unsigned char* buffer, unsigned int lengthInBytes)
{
#ifdef __WINDOWS__
    int nbytes = sendto(m_sockFd, (const char *) buffer, lengthInBytes, 0, (const sockaddr *) m_sa, sizeof(struct sockaddr_in));
#else
    int nbytes = sendto(m_sockFd, buffer, lengthInBytes, 0, (const sockaddr *) m_sa, sizeof(struct sockaddr_in));
#endif
    //std::cerr << "UDPDataController::write: length: " << lengthInBytes << " written: " << nbytes << std::endl;
    return nbytes;
}

void UDPDataController::closeIt()
{
    if (m_sockFd >= 0) {
        closeSocket();
    }
}

void UDPDataController::openSocket(int port)
{
    m_sockFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (m_sockFd < 0)
    {
#ifdef __WINDOWS__
        std::cerr << "UDPDataController::openSocket: error when creating the socket: " << WSAGetLastError() << std::endl;
#else
        std::cerr << "UDPDataController::openSocket: error when creating the socket: " << strerror(errno) << std::endl;
#endif
        return;
    }

    m_ra->sin_family = AF_INET;
    m_ra->sin_port = htons(port);
    m_ra->sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(m_sockFd, (struct sockaddr *) m_ra, sizeof(struct sockaddr_in)) < 0)
    {
#ifdef __WINDOWS__
        std::cerr << "UDPDataController::openSocket: error when binding the socket to port " << port << ": " <<  WSAGetLastError() << std::endl;
#else
        std::cerr << "UDPDataController::openSocket: error when binding the socket to port " << port << ": " <<  strerror(errno) << std::endl;
#endif
        m_sockFd = -1;
    }
}

void UDPDataController::closeSocket()
{
#ifdef __WINDOWS__
    int rc = closesocket(m_sockFd);

    if (rc < 0) {
        std::cerr << "UDPDataController::close: error when closing the socket: " << WSAGetLastError() << std::endl;
    } else {
        std::cerr << "UDPDataController::close: socket closed" << std::endl;
    }
#else
    int rc = close(m_sockFd);

    if (rc < 0) {
        std::cerr << "UDPDataController::close: error when closing the socket: " << strerror(errno) << std::endl;
    } else {
        std::cerr << "UDPDataController::close: socket closed" << std::endl;
    }
#endif
}

void UDPDataController::setSendAddress(std::string& address, int port)
{
    m_sa->sin_family = AF_INET;
    m_sa->sin_port = htons(port);
    m_sa->sin_addr.s_addr = inet_addr(address.c_str());
}

int UDPDataController::timeout_recvfrom(int sock, char *buf, int length, struct sockaddr_in *connection, int timeoutinmicroseconds)
{
    fd_set socks;
    struct timeval t;
    FD_ZERO(&socks);
    FD_SET(sock, &socks);
    t.tv_sec = timeoutinmicroseconds / 1000000;
    t.tv_usec = timeoutinmicroseconds % 1000000;

    int nready = select(sock + 1, &socks, nullptr, nullptr, &t);

    if (nready > 0)
    {
        socklen_t addrLen = sizeof(struct sockaddr_in);
        int nbytes = recvfrom(sock, buf, length, 0, (struct sockaddr *) connection, &addrLen);
        return nbytes;
    }
    else // error or no data after timeout
    {
        return 0;
    }
}

} // namespace SerialDV