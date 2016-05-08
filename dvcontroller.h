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

#ifndef DVCONTROLLER_H_
#define DVCONTROLLER_H_

#include <string>

#include "serialdatacontroller.h"

namespace SerialDV
{

typedef enum
{
    DVRateNone,
    DVRate3600x2400, //!< D-Star
    DVRate3600x2450  //!< DMR and the likes
} DVRate;

class DVController
{
public:
	DVController();
	~DVController();

    bool open(const std::string& device, bool halfSpeed=false);
    void close();
    bool isOpen() const { return m_open; }

	/** Encoding process of one audio frame to one AMBE frame
	 * Buffers are supposed to be allocated with the correct size. That is
	 * - 320 bytes (160 short samples) for the audio frame.
	 *   - SerialDV::MBE_AUDIO_BLOCK_BYTES constant is the number of bytes (320)
	 *   - SerialDV::MBE_AUDIO_BLOCK_SIZE constant is the number of short samples (160)
	 * - 9 bytes (72 bits) for the AMBE frame.
	 *   - SerialDV::VOICE_FRAME_LENGTH_BYTES constant is the number of bytes (9)
	 *   - SerialDV::VOICE_FRAME_LENGTH_BITS constant is the number of bits (72)
	 */
	bool encode(const short *audioFrame, unsigned char *mbeFrame, DVRate rate, int gain = 0);

	/** Encoding process of one AMBE frame to one audio frame
	 * Buffers are supposed to be allocated with the correct size. That is
	 * - 320 bytes (160 short samples) for the audio frame.
	 *   - SerialDV::MBE_AUDIO_BLOCK_BYTES constant is the number of bytes (320)
	 *   - SerialDV::MBE_AUDIO_BLOCK_SIZE constant is the number of short samples (160)
	 * - 9 bytes (72 bits) for the AMBE frame.
	 *   - SerialDV::VOICE_FRAME_LENGTH_BYTES constant is the number of bytes (9)
	 *   - SerialDV::VOICE_FRAME_LENGTH_BITS constant is the number of bits (72)
	 */
	bool decode(short *audioFrame, const unsigned char *mbeFrame, DVRate rate, int gain = 0);

private:

    enum RESP_TYPE {
        RESP_NONE,
        RESP_ERROR,
        RESP_RATEP,
        RESP_NAME,
        RESP_AMBE,
        RESP_AUDIO,
        RESP_GAIN,
        RESP_UNKNOWN
    };

    SerialDataController m_serial;
    bool m_open; //!< True if the serial DV device has been correctly opened
    DVRate m_currentRate;
    int m_currentGainIn;
    int m_currentGainOut;

    void encodeIn(const short* audio, unsigned int length);
    bool encodeOut(unsigned char* ambe, unsigned int length);

    void decodeIn(const unsigned char* ambe, unsigned int length);
    bool decodeOut(short* audio, unsigned int length);

    bool setRate(DVRate rate);

    /** Set input and output gain in dB (-90 to +90 dB)
     * If the input gain is < 0 dB then the input speech samples are attenuated prior to encoding.
     * If the input gain is > 0 dB then the input speech samples are amplified prior to encoding.
     * If the output gain is < 0 dB then the output speech samples are attenuated after decoding.
     * If the output gain is > 0 dB then the output speech samples are amplified after decoding.
     */
    bool setGain(char dBGainIn, char dBGainOut);

    RESP_TYPE getResponse(unsigned char* buffer, unsigned int length);
};

} // namespace SerialDV

#endif /* DVCONTROLLER_H_ */
