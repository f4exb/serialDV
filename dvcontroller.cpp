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

#include "dvcontroller.h"

namespace SerialDV
{

DVController::DVController(std::string& deviceName, bool halfSpeed)
{
	m_open = m_dvSerialController.open(deviceName, halfSpeed ? SERIAL_230400 : SERIAL_460800);
}

bool DVController::encode(short *audioFrame, unsigned char *mbeFrame)
{
	if (!m_open) {
		return false;
	}

	m_dvSerialController.encodeIn(audioFrame, MBE_AUDIO_BLOCK_SIZE);
	return m_dvSerialController.encodeOut(mbeFrame, VOICE_FRAME_LENGTH_BYTES);
}


bool DVController::decode(short *audioFrame, unsigned char *mbeFrame)
{
	if (!m_open) {
		return false;
	}

	m_dvSerialController.decodeIn(mbeFrame, VOICE_FRAME_LENGTH_BYTES);
	return m_dvSerialController.decodeOut(audioFrame, MBE_AUDIO_BLOCK_SIZE);
}

DVController::~DVController()
{
	m_dvSerialController.close();
	m_open = false;
}

} // namespace SerialDV

