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

#ifndef __SERIALDV_EXPORT_H
#define __SERIALDV_EXPORT_H

#if defined (__GNUC__) && (__GNUC__ >= 4)
#  define __SERIALDV_EXPORT   __attribute__((visibility("default")))
#  define __SERIALDV_IMPORT   __attribute__((visibility("default")))

#elif defined (_MSC_VER)
#  define __SERIALDV_EXPORT   __declspec(dllexport)
#  define __SERIALDV_IMPORT   __declspec(dllimport)

#else
#  define __SERIALDV_EXPORT
#  define __SERIALDV_IMPORT
#endif

/* The 'SERIALDV_API' controls the import/export of 'sdrbase' symbols and classes.
 */
#if !defined(serialdv_STATIC)
#  if defined serialdv_EXPORTS
#    define SERIALDV_API __SERIALDV_EXPORT
#  else
#    define SERIALDV_API __SERIALDV_IMPORT
#  endif
#else
#  define SERIALDV_API
#endif

#endif // __SERIALDV_EXPORT_H
