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

#ifndef DV3000SERIAL_H_
#define DV3000SERIAL_H_

#include <string>
#include "serialdatacontroller.h"

namespace SerialDV
{

const unsigned char DV3000_START_BYTE   = 0x61U;

const unsigned char DV3000_TYPE_CONTROL = 0x00U;
const unsigned char DV3000_TYPE_AMBE    = 0x01U;
const unsigned char DV3000_TYPE_AUDIO   = 0x02U;

const unsigned char DV3000_CONTROL_RATEP  = 0x0AU;
const unsigned char DV3000_CONTROL_PRODID = 0x30U;
const unsigned char DV3000_CONTROL_READY  = 0x39U;

const unsigned char DV3000_REQ_PRODID[] = {DV3000_START_BYTE, 0x00U, 0x01U, DV3000_TYPE_CONTROL, DV3000_CONTROL_PRODID};
const unsigned int DV3000_REQ_PRODID_LEN = 5U;

const unsigned char DV3000_REQ_RATEP[] = {DV3000_START_BYTE, 0x00U, 0x0DU, DV3000_TYPE_CONTROL, DV3000_CONTROL_RATEP, 0x01U, 0x30U, 0x07U, 0x63U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x48U};
const unsigned int DV3000_REQ_RATEP_LEN = 17U;

const unsigned char DV3000_AUDIO_HEADER[] = {DV3000_START_BYTE, 0x01U, 0x42U, DV3000_TYPE_AUDIO, 0x00U, 0xA0U};
const unsigned char DV3000_AUDIO_HEADER_LEN = 6U;

const unsigned char DV3000_AMBE_HEADER[] = {DV3000_START_BYTE, 0x00U, 0x0BU, DV3000_TYPE_AMBE, 0x01U, 0x48U};
const unsigned char DV3000_AMBE_HEADER_LEN  = 6U;

const unsigned int DV3000_HEADER_LEN = 4U;

const unsigned int BUFFER_LENGTH = 400U;

const unsigned int MBE_AUDIO_BLOCK_SIZE  = 160U;
const unsigned int MBE_AUDIO_BLOCK_BYTES = MBE_AUDIO_BLOCK_SIZE * 2U;

const unsigned int VOICE_FRAME_LENGTH_BITS = 72U;
const unsigned int VOICE_FRAME_LENGTH_BYTES = VOICE_FRAME_LENGTH_BITS / 8U;


class DV3000SerialController {
public:
    DV3000SerialController();
    virtual ~DV3000SerialController();

    bool open(const std::string& device, SERIAL_SPEED speed);

    void encodeIn(const short* audio, unsigned int length);
    bool encodeOut(unsigned char* ambe, unsigned int length);

    void decodeIn(const unsigned char* ambe, unsigned int length);
    bool decodeOut(short* audio, unsigned int length);

    void close();

private:
    SerialDataController m_serial;

    enum RESP_TYPE {
        RESP_NONE,
        RESP_ERROR,
        RESP_RATEP,
        RESP_NAME,
        RESP_AMBE,
        RESP_AUDIO,
        RESP_UNKNOWN
    };

    RESP_TYPE getResponse(unsigned char* buffer, unsigned int length);

};

} // namespace SerialDV


#endif /* DV3000SERIAL_H_ */
