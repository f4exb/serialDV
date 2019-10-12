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

#include "dummydatacontroller.h"

namespace SerialDV
{

DummyDataController::DummyDataController()
{
}

DummyDataController::~DummyDataController()
{
}

bool DummyDataController::open(const std::string& device, SERIAL_SPEED speed)
{
    (void) device;
    (void) speed;
    return false;
}

int DummyDataController::read(unsigned char* buffer, unsigned int lengthInBytes)
{
    (void) buffer;
    (void) lengthInBytes;
    return 0;
}

int DummyDataController::write(const unsigned char* buffer, unsigned int lengthInBytes)
{
    (void) buffer;
    (void) lengthInBytes;
    return 0;
}

void DummyDataController::closeIt()
{
}

bool DummyDataController::initResponse()
{
    return true; // Do nothing for dummmy
}


} // namespace SerialDV
