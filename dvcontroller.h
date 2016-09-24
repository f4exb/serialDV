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
    DVRate3600x2450, //!< DMR, dPMR, YSF V/D type 1
    DVRate7200x4400,
    DVRate7100x4400,
    DVRate2400,
    DVRate2450,      //!< YSF V/D type 2 (does not use FEC in AMBE codec)
    DVRate4400
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
	 * - 9 or 18 bytes (72 or 144 bits) for the AMBE frame.
	 *   - SerialDV::VOICE_FRAME_MAX_LENGTH_BYTES constant is the maximum number of bytes (18)
	 */
	bool encode(const short *audioFrame, unsigned char *mbeFrame, DVRate rate, int gain = 0);

	/** Encoding process of one AMBE frame to one audio frame
	 * Buffers are supposed to be allocated with the correct size. That is
	 * - 320 bytes (160 short samples) for the audio frame.
	 *   - SerialDV::MBE_AUDIO_BLOCK_BYTES constant is the number of bytes (320)
	 *   - SerialDV::MBE_AUDIO_BLOCK_SIZE constant is the number of short samples (160)
	 * - 9 or 18 bytes (72 or 144 bits) for the AMBE frame.
     *   - SerialDV::VOICE_FRAME_MAX_LENGTH_BYTES constant is the maximum number of bytes (18)
	 */
	bool decode(short *audioFrame, const unsigned char *mbeFrame, DVRate rate, int gain = 0);

	/** Returns the number of bytes in a MBE frame given the MBE rate
	 */
	static unsigned short getNbMbeBytes(DVRate mbeRate);

    /** Returns the number of bits in a MBE frame given the MBE rate
     */
    static unsigned char getNbMbeBits(DVRate mbeRate);

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
    unsigned char m_currentNbMbeBits;
    unsigned short m_currentNbMbeBytes;
    bool m_littleEndian;

    bool isLittleEndian()
    {
        short int number = 0x1;
        char *numPtr = (char*)&number;
        return (numPtr[0] == 1);
    }

    void encodeIn(const short* audio, unsigned int length);
    bool encodeOut(unsigned char* ambe, unsigned int length);

    void decodeIn(const unsigned char* ambe, unsigned char nbBits, unsigned short nbBytes);
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
