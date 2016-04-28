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

class DV3000SerialController {
public:
    DV3000SerialController(const std::string& device, SERIAL_SPEED speed);
    virtual ~DV3000SerialController();

    bool open();

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

    static const unsigned char DV3000_START_BYTE;

    static const unsigned char DV3000_TYPE_CONTROL;
    static const unsigned char DV3000_TYPE_AMBE;
    static const unsigned char DV3000_TYPE_AUDIO;

    static const unsigned char DV3000_CONTROL_RATEP;
    static const unsigned char DV3000_CONTROL_PRODID;
    static const unsigned char DV3000_CONTROL_READY;

    static const unsigned char DV3000_REQ_PRODID[];
    static const unsigned int DV3000_REQ_PRODID_LEN;

    static const unsigned char DV3000_REQ_RATEP[];
    static const unsigned int DV3000_REQ_RATEP_LEN;

    static const unsigned char DV3000_AUDIO_HEADER[];
    static const unsigned char DV3000_AUDIO_HEADER_LEN;

    static const unsigned char DV3000_AMBE_HEADER[];
    static const unsigned char DV3000_AMBE_HEADER_LEN;

    static const unsigned int DV3000_HEADER_LEN;

    static const unsigned int BUFFER_LENGTH;

    static const unsigned int MBE_AUDIO_BLOCK_SIZE;
    static const unsigned int MBE_AUDIO_BLOCK_BYTES;

    static const unsigned int VOICE_FRAME_LENGTH_BITS;
    static const unsigned int VOICE_FRAME_LENGTH_BYTES;
};

} // namespace SerialDV


#endif /* DV3000SERIAL_H_ */
