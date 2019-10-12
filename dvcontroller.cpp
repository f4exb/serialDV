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

#include <chrono>
#include <thread>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdint.h>

#ifdef __APPLE__
#include "dummydatacontroller.h"
#else
#include "udpdatacontroller.h"
#include "serialdatacontroller.h"
#endif
#include "dvcontroller.h"

namespace SerialDV
{

DVController::DVController() :
        m_serial(nullptr),
        m_open(false),
        m_currentRate(DVRateNone),
        m_currentGainIn(0),
        m_currentGainOut(0),
        m_currentNbMbeBits(72),
        m_currentNbMbeBytes(9)
{
    m_littleEndian = isLittleEndian();
}

DVController::~DVController()
{
    if (m_serial) {
        delete m_serial;
    }
}

bool DVController::open(const std::string& device, bool halfSpeed)
{
    m_open = false;

#if __APPLE__
    m_serial = new DummyDataController();
#else
    if (device.find(':') != std::string::npos) {
        m_serial = new UDPDataController();
    } else {
        m_serial = new SerialDataController();
    }
#endif

    bool res = m_serial->open(device, halfSpeed ? SERIAL_230400 : SERIAL_460800);

    if (!res) {
        return false;
    }

    m_serial->write(DV3000_REQ_PRODID, DV3000_REQ_PRODID_LEN);

    unsigned char buffer[BUFFER_LENGTH];
    RESP_TYPE type = getResponse(buffer, BUFFER_LENGTH);

    if (type == RESP_ERROR)
    {
        fprintf(stderr, "DVController::open: serial device error\n");
        m_serial->closeIt();
        return false;
    }
    else if (type == RESP_NAME)
    {
        std::string name((char *) &buffer[5]);
        fprintf(stderr, "DVController::open: DV3000 chip identified as: %s\n", name.c_str());
        m_open = true;
        return true;
    }
    else
    {
        fprintf(stderr, "DVController::open: response mismatch\n");
        m_serial->closeIt();
        return false;
    }
}

void DVController::close()
{
    m_serial->closeIt();
    m_open = false;
}

bool DVController::encode(const short *audioFrame, unsigned char *mbeFrame, DVRate rate, int gain)
{
	if (!m_open) {
		return false;
	}

	if (rate != m_currentRate)
	{
	    setRate(rate);
	    m_currentRate = rate;
	}

	if (gain != m_currentGainIn)
	{
	    setGain(gain, m_currentGainOut);
	    m_currentGainIn = gain;
	}

	encodeIn(audioFrame, MBE_AUDIO_BLOCK_SIZE_INTERNAL);
	return encodeOut(mbeFrame, m_currentNbMbeBytes);
}


bool DVController::decode(short *audioFrame, const unsigned char *mbeFrame, DVRate rate, int gain)
{
	if (!m_open) {
		return false;
	}

    if (rate != m_currentRate)
    {
        setRate(rate);
        m_currentRate = rate;
    }

    if (gain != m_currentGainOut)
    {
        setGain(m_currentGainIn, gain);
        m_currentGainOut = gain;
    }

	decodeIn(mbeFrame, m_currentNbMbeBits, m_currentNbMbeBytes);
	return decodeOut(audioFrame, MBE_AUDIO_BLOCK_SIZE_INTERNAL);
}

unsigned short DVController::getNbMbeBytes(DVRate mbeRate)
{
    switch (mbeRate)
    {
    case DVRateNone:
        return 0;
    case DVRate3600x2400:
        return 9;
        break;
    case DVRate3600x2450:
        return 9;
        break;
    case DVRate7200x4400:
        return 18;
        break;
    case DVRate2450:
        return 7;
        break;
    case DVRate4400:
        return 11;
        break;
    default:
        return 0;
    }
}

unsigned char DVController::getNbMbeBits(DVRate mbeRate)
{
    switch (mbeRate)
    {
    case DVRateNone:
        return 0;
    case DVRate3600x2400:
        return 72;
        break;
    case DVRate3600x2450:
        return 72;
        break;
    case DVRate7200x4400:
        return 144;
        break;
    case DVRate2450:
        return 49;
        break;
    case DVRate4400:
        return 88;
        break;
    default:
        return 0;
    }
}

bool DVController::setGain(signed char dBGainIn, signed char dBGainOut)
{
    if (!m_open) {
        return false;
    }

    if (dBGainIn < -90) {
        dBGainIn = -90;
    } else if (dBGainIn > 90) {
        dBGainIn = 90;
    }

    if (dBGainOut < -90) {
        dBGainOut = -90;
    } else if (dBGainOut > 90) {
        dBGainOut = 90;
    }

    unsigned char buffer[DV3000_REQ_GAIN_LEN + 2];
    ::memcpy(buffer, DV3000_REQ_GAIN, DV3000_REQ_GAIN_LEN);

    buffer[DV3000_REQ_GAIN_LEN]   = dBGainIn;
    buffer[DV3000_REQ_GAIN_LEN+1] = dBGainOut;

    m_serial->write(buffer, DV3000_REQ_GAIN_LEN + 2);
    RESP_TYPE type = getResponse(buffer, BUFFER_LENGTH);

    if (type == RESP_ERROR)
    {
        fprintf(stderr, "DVController::setGain: serial device error\n");
        return false;
    }
    else if (type == RESP_GAIN)
    {
        fprintf(stderr, "DVController::setGain: in: %d dB out: %d dB: OK\n", (int) dBGainIn, (int) dBGainOut);
        return true;
    }
    else
    {
        fprintf(stderr, "DVController::setGain: response mismatch\n");
        return false;
    }
}

void DVController::encodeIn(const short* audio, unsigned int length)
{
    (void) length;
    assert(audio != 0);
    assert(length == MBE_AUDIO_BLOCK_SIZE_INTERNAL);

    // TODO: optimization with fixed initialization of the audio header
    unsigned char buffer[DV3000_AUDIO_HEADER_LEN + MBE_AUDIO_BLOCK_BYTES_INTERNAL];

    ::memcpy(buffer, DV3000_AUDIO_HEADER, DV3000_AUDIO_HEADER_LEN);

    uint8_t* q = (uint8_t*) (buffer + DV3000_AUDIO_HEADER_LEN);

    for (unsigned int i = 0; i < MBE_AUDIO_BLOCK_SIZE_INTERNAL; i++, q += 2U)
    {
        q[0U] = (audio[i] & 0xFF00) >> 8;
        q[1U] = (audio[i] & 0x00FF) >> 0;
    }

    m_serial->write(buffer, DV3000_AUDIO_HEADER_LEN + MBE_AUDIO_BLOCK_BYTES_INTERNAL);
}

bool DVController::encodeOut(unsigned char* ambe, unsigned int length)
{
    assert(ambe != 0);
    assert(length == m_currentNbMbeBytes);

    unsigned char buffer[BUFFER_LENGTH];
    RESP_TYPE type = getResponse(buffer, BUFFER_LENGTH);

    if (type != RESP_AMBE)
    {
        fprintf(stderr, "DVController::encodeOut: error\n");
        return false;
    }

    ::memcpy(ambe, buffer + DV3000_AMBE_HEADER_LEN, length);

    return true;
}

void DVController::decodeIn(const unsigned char* ambe, unsigned char nbBits, unsigned short nbBytes)
{
    assert(ambe != 0);
    assert(nbBytes == m_currentNbMbeBytes);
    unsigned short length = nbBytes + 2;
    unsigned char *lengthPtr = (unsigned char *) &length;

    unsigned char buffer[DV3000_AMBE_HEADER_LEN + MBE_FRAME_MAX_LENGTH_BYTES_INTERNAL];
    ::memcpy(buffer, DV3000_AMBE_HEADER, DV3000_AMBE_HEADER_LEN);
    ::memcpy(buffer + DV3000_AMBE_HEADER_LEN, ambe, nbBytes);

    if (m_littleEndian)
    {
        ::memcpy(&buffer[1], &lengthPtr[1], 1); // set header length field with little endian byte order
        ::memcpy(&buffer[2], &lengthPtr[0], 1); // set header length field with little endian byte order
    }
    else
    {
        ::memcpy(&buffer[1], &lengthPtr[0], 1); // set header length field with big endian byte order
        ::memcpy(&buffer[2], &lengthPtr[1], 1); // set header length field with big endian byte order
    }

    ::memcpy(&buffer[5], &nbBits, 1); // set CHAND number of bits

    m_serial->write(buffer, DV3000_AMBE_HEADER_LEN + nbBytes);
}

bool DVController::decodeOut(short* audio, unsigned int length)
{
    (void) length;
    assert(audio != 0);
    assert(length == MBE_AUDIO_BLOCK_SIZE_INTERNAL);

    unsigned char buffer[BUFFER_LENGTH];
    RESP_TYPE type = getResponse(buffer, BUFFER_LENGTH);

    if (type != RESP_AUDIO)
    {
        fprintf(stderr, "DVController::decodeOut: error\n");
        return false;
    }

    uint8_t* q = (uint8_t*) (buffer + DV3000_AUDIO_HEADER_LEN);

    for (unsigned int i = 0U; i < MBE_AUDIO_BLOCK_SIZE_INTERNAL; i++, q += 2U)
    {
        short word = (q[0] << 8) | (q[1U] << 0);
        audio[i] = word;
    }

    return true;
}

bool DVController::setRate(DVRate rate)
{
    if (!m_open) {
        return false;
    }

    if (rate == DVRateNone) {
        return true;
    }

    const unsigned char *ratepStr;

    switch(rate)
    {
    case DVRateNone:
        return true;
    case DVRate3600x2400:
        ratepStr = DV3000_REQ_3600X2400_RATEP;
        m_currentNbMbeBits = 72;
        m_currentNbMbeBytes = 9;
        break;
    case DVRate3600x2450:
        ratepStr = DV3000_REQ_3600X2450_RATEP;
        m_currentNbMbeBits = 72;
        m_currentNbMbeBytes = 9;
        break;
    case DVRate7200x4400:
        ratepStr = DV3000_REQ_7200X4400_3_RATEP; // AMBE 3000 version
        m_currentNbMbeBits = 144;
        m_currentNbMbeBytes = 18;
        break;
    case DVRate2450:
        ratepStr = DV3000_REQ_2450_RATEP;
        m_currentNbMbeBits = 49;
        m_currentNbMbeBytes = 7;
        break;
    case DVRate4400:
        ratepStr = DV3000_REQ_4400_RATEP;
        m_currentNbMbeBits = 88;
        m_currentNbMbeBytes = 11;
        break;
    default:
        return true;
    }

    m_serial->write(ratepStr, DV3000_REQ_RATEP_LEN);

    unsigned char buffer[BUFFER_LENGTH];
    RESP_TYPE type = getResponse(buffer, BUFFER_LENGTH);

    if (type == RESP_ERROR)
    {
        fprintf(stderr, "DVController::setRate: serial device error\n");
        return false;
    }
    else if (type == RESP_RATEP)
    {
        fprintf(stderr, "DVController::setRate (%d): OK\n", (int) rate);
        return true;
    }
    else
    {
        fprintf(stderr, "DVController::setRate: response mismatch\n");
        return false;
    }

}

DVController::RESP_TYPE DVController::getResponse(unsigned char* buffer, unsigned int length)
{
    (void) length;
    assert(buffer != 0);
    assert(length >= BUFFER_LENGTH);

    if (!m_serial->initResponse())
    {
        fprintf(stderr, "DVController::getResponse: cannot get response\n");
        return RESP_ERROR;
    }

    bool found = false;
    int packetLength, offset;
    unsigned char packetType;

    for (int i = 0; i < 2000; i++)
    {
        int len1 = m_serial->read(buffer, 1U);

        if (len1 < 0)
        {
            fprintf(stderr, "DVController::getResponse: Error (start byte)\n");
            return RESP_ERROR;
        }
        else if ((len1 == 1) && (buffer[0U] == DV3000_START_BYTE))
        {
            found = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    if (!found)
    {
        fprintf(stderr, "DVController::getResponse: Timeout (start byte)\n");
        return RESP_ERROR;
    }

    packetLength = 3;
    offset = 0;
    found = false;

    for (int i = 0; i < 2000; i++)
    {
        int len1 = m_serial->read(&buffer[1 + offset], packetLength - offset);

        if (len1 < 0)
        {
            fprintf(stderr, "DVController::getResponse: Error (packet header at %d)\n", offset);
            return RESP_ERROR;
        }
        else if (offset + len1 == packetLength)
        {
            found = true;
            break;
        }
        else
        {
            offset += len1;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    if (!found)
    {
        fprintf(stderr, "DVController::getResponse: Timeout (packet header)\n");
        return RESP_ERROR;
    }

    packetLength = buffer[1] * 256 + buffer[2];
    packetType = buffer[3];
    offset = 0;
    found = false;

    for (int i = 0; i < 2000; i++)
    {
        int len1 = m_serial->read(&buffer[4 + offset], packetLength - offset);

        if (len1 < 0)
        {
            fprintf(stderr, "DVController::getResponse: Error (packet payload at %d)\n", offset);
            return RESP_ERROR;
        }
        else if (offset + len1 == packetLength)
        {
            found = true;
            break;
        }
        else
        {
            offset += len1;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    if (!found) {
        fprintf(stderr, "DVController::getResponse: Timeout (packet payload)\n");
        return RESP_ERROR;
    }

    //fprintf(stderr, "DVController::getResponse: packet type %02x\n", packetType);

    if (packetType == DV3000_TYPE_AUDIO)
    {
        return RESP_AUDIO;
    }
    else if (packetType == DV3000_TYPE_AMBE)
    {
        return RESP_AMBE;
    }
    else if (packetType == DV3000_TYPE_CONTROL) // check the field type buffer[4]
    {
        //fprintf(stderr, "DVController::getResponse: field type %02x\n", buffer[4]);

        if (buffer[4] == DV3000_CONTROL_PRODID)
        {
            return RESP_NAME;
        }
        else if (buffer[4] == DV3000_CONTROL_RATEP)
        {
            return RESP_RATEP;
        }
        else if (buffer[4] == DV3000_CONTROL_GAIN)
        {
            return RESP_GAIN;
        }
        else if (buffer[4] == DV3000_CONTROL_READY)
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

