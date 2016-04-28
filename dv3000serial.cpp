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

#include <cassert>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "dv3000serial.h"

namespace SerialDV
{

DV3000SerialController::DV3000SerialController()
{
}

DV3000SerialController::~DV3000SerialController()
{
}

bool DV3000SerialController::open(const std::string& device, SERIAL_SPEED speed)
{
    bool res = m_serial.open(device, speed);

    if (!res) {
        return false;
    }

    m_serial.write(DV3000_REQ_PRODID, DV3000_REQ_PRODID_LEN);

    bool found = false;

    for (unsigned int i = 0U; i < 10U; i++)
    {
        unsigned char buffer[BUFFER_LENGTH];
        RESP_TYPE type = getResponse(buffer, BUFFER_LENGTH);

        if (type == RESP_ERROR)
        {
            m_serial.close();
            return false;
        }

        if (type == RESP_NAME)
        {
            std::string name((char *) &buffer[5]);
            fprintf(stderr, "DV3000SerialController::open: DV3000 chip identified as: %s", name.c_str());
            found = true;
            break;
        }

        usleep(10UL);
    }

    if (!found)
    {
        m_serial.close();
        return false;
    }

    m_serial.write(DV3000_REQ_RATEP, DV3000_REQ_RATEP_LEN);

    for (int i = 0; i < 500000; i++)
    {
        unsigned char buffer[BUFFER_LENGTH];
        RESP_TYPE type = getResponse(buffer, BUFFER_LENGTH);

        if (type == RESP_ERROR)
        {
            m_serial.close();
            return false;
        }

        if (type == RESP_RATEP)
            return true;

        usleep(10UL);
    }

    fprintf(stderr, "DV3000SerialController::open: Timeout");
    return false;
}

void DV3000SerialController::encodeIn(const short* audio, unsigned int length)
{
    assert(audio != 0);
    assert(length == MBE_AUDIO_BLOCK_SIZE);

    // TODO: optimization with fixed initialization of the audio header
    unsigned char buffer[DV3000_AUDIO_HEADER_LEN + MBE_AUDIO_BLOCK_BYTES];

    ::memcpy(buffer, DV3000_AUDIO_HEADER, DV3000_AUDIO_HEADER_LEN);

    uint8_t* q = (uint8_t*) (buffer + DV3000_AUDIO_HEADER_LEN);

    for (unsigned int i = 0; i < MBE_AUDIO_BLOCK_SIZE; i++, q += 2U)
    {
        q[0U] = (audio[i] & 0xFF00) >> 8;
        q[1U] = (audio[i] & 0x00FF) >> 0;
    }

    m_serial.write(buffer, DV3000_AUDIO_HEADER_LEN + MBE_AUDIO_BLOCK_BYTES);
}

bool DV3000SerialController::encodeOut(unsigned char* ambe, unsigned int length)
{
    assert(ambe != 0);
    assert(length == VOICE_FRAME_LENGTH_BYTES);

    unsigned char buffer[BUFFER_LENGTH];
    RESP_TYPE type = getResponse(buffer, BUFFER_LENGTH);

    if (type != RESP_AMBE) {
        return false;
    }

    ::memcpy(ambe, buffer + DV3000_AMBE_HEADER_LEN, VOICE_FRAME_LENGTH_BYTES);

    return true;
}

void DV3000SerialController::decodeIn(const unsigned char* ambe, unsigned int length)
{
    assert(ambe != 0);
    assert(length == VOICE_FRAME_LENGTH_BYTES);

    unsigned char buffer[DV3000_AMBE_HEADER_LEN + VOICE_FRAME_LENGTH_BYTES];
    ::memcpy(buffer, DV3000_AMBE_HEADER, DV3000_AMBE_HEADER_LEN);
    ::memcpy(buffer + DV3000_AMBE_HEADER_LEN, ambe, VOICE_FRAME_LENGTH_BYTES);

    m_serial.write(buffer, DV3000_AMBE_HEADER_LEN + VOICE_FRAME_LENGTH_BYTES);
}

bool DV3000SerialController::decodeOut(short* audio, unsigned int length)
{
    assert(audio != 0);
    assert(length == MBE_AUDIO_BLOCK_SIZE);

    unsigned char buffer[BUFFER_LENGTH];
    RESP_TYPE type = getResponse(buffer, BUFFER_LENGTH);

    if (type != RESP_AUDIO) {
        return false;
    }

    uint8_t* q = (uint8_t*) (buffer + DV3000_AUDIO_HEADER_LEN);

    for (unsigned int i = 0U; i < MBE_AUDIO_BLOCK_SIZE; i++, q += 2U)
    {
        short word = (q[0] << 8) | (q[1U] << 0);
        audio[i] = word;
    }

    return true;
}

void DV3000SerialController::close()
{
    m_serial.close();
}

DV3000SerialController::RESP_TYPE DV3000SerialController::getResponse(unsigned char* buffer, unsigned int length)
{
    assert(buffer != 0);
    assert(length >= BUFFER_LENGTH);

    int len1 = m_serial.read(buffer, 1U);

    if (len1 == 0) {
        return RESP_NONE;
    }

    if (len1 < 0) {
        return RESP_ERROR;
    }

    unsigned int offset = 3U;   // The normal case

    if (buffer[0U] != DV3000_START_BYTE)
    {
        int len2 = m_serial.read(buffer + 1U, 2U);

        if (len2 == 0) {
            return RESP_NONE;
        }

        if (len2 < 0) {
            return RESP_ERROR;
        }

        bool found = false;

        for (unsigned int i = 1U; i < 4U && !found; i++)
        {
            int ret1 = ::memcmp(buffer, DV3000_AUDIO_HEADER + i, 3U);
            int ret2 = ::memcmp(buffer, DV3000_AMBE_HEADER + i, 3U);

            if (ret1 == 0)
            {
                ::memcpy(buffer + i, buffer + 0U, 3U);
                ::memcpy(buffer, DV3000_AUDIO_HEADER, i);
                offset = i + 3U;
                found = true;
            }
            else if (ret2 == 0)
            {
                ::memcpy(buffer + i, buffer + 0U, 3U);
                ::memcpy(buffer, DV3000_AMBE_HEADER, i);
                offset = i + 3U;
                found = true;
            }
        }

        if (!found)
        {
            fprintf(stderr, "DV3000SerialController::getResponse: Unknown bytes from the DV3000, %02X %02X %02X", buffer[0U], buffer[1U], buffer[2U]);
            return RESP_UNKNOWN;
        }
    }
    else
    {
        int len2 = m_serial.read(buffer + 1U, 2U);

        if (len2 == 0) {
            return RESP_NONE;
        }

        if (len2 < 0) {
            return RESP_ERROR;
        }

        if (len2 != 2)
        {
            fprintf(stderr, "DV3000SerialController::getResponse: Invalid DV3000 data read, %d != 2", len2);
            return RESP_ERROR;
        }
    }

    unsigned int respLen = (buffer[1U] & 0x0FU) * 256U + buffer[2U] + DV3000_HEADER_LEN;

    int len3 = m_serial.read(buffer + offset, respLen - offset);

    if (len3 == 0) {
        return RESP_NONE;
    }

    if (len3 < 0) {
        return RESP_ERROR;
    }

    if (len3 != int(respLen - offset))
    {
        fprintf(stderr, "DV3000SerialController::getResponse: Invalid DV3000 data, %d != %u", len3, respLen - offset);
        return RESP_ERROR;
    }

    if (buffer[3U] == DV3000_TYPE_AUDIO)
    {
        return RESP_AUDIO;
    }
    else if (buffer[3U] == DV3000_TYPE_AMBE)
    {
        return RESP_AMBE;
    }
    else if (buffer[3U] == DV3000_TYPE_CONTROL)
    {
        if (buffer[4U] == DV3000_CONTROL_PRODID)
        {
            return RESP_NAME;
        }
        else if (buffer[4U] == DV3000_CONTROL_RATEP)
        {
            return RESP_RATEP;
        }
        else if (buffer[4U] == DV3000_CONTROL_READY)
        {
            return RESP_UNKNOWN;
        }
        else
        {
            return RESP_UNKNOWN;
        }
    }
    else
    {
        return RESP_UNKNOWN;
    }
}

} // namespace SerialDV
