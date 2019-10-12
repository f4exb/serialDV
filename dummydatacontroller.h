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

#ifndef DUMMYDATACONTROLLER_H_
#define DUMMYDATACONTROLLER_H_

#include "datacontroller.h"

namespace SerialDV
{

class SERIALDV_API DummyDataController : public DataController {
public:
    DummyDataController();
    virtual ~DummyDataController();

    virtual bool open(const std::string& device, SERIAL_SPEED speed);

    virtual bool initResponse();
    virtual int  read(unsigned char* buffer, unsigned int lengthInBytes);
    virtual int  write(const unsigned char* buffer, unsigned int lengthInBytes);

    virtual void closeIt();
};

} // namespace SerialDV

#endif /* DUMMYDATACONTROLLER_H_ */
