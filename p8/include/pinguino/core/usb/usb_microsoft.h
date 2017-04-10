/*
 *  M-Stack Microsoft-Specific OS Descriptors
 *  Copyright (C) 2013 Alan Ott <alan@signal11.us>
 *  Copyright (C) 2013 Signal 11 Software
 *
 *  2013-08-27
 *
 *  M-Stack is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, version 3; or the Apache License, version 2.0
 *  as published by the Apache Software Foundation.  If you have purchased a
 *  commercial license for this software from Signal 11 Software, your
 *  commerical license superceeds the information in this header.
 *
 *  M-Stack is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this software.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  You should have received a copy of the Apache License, verion 2.0 along
 *  with this software.  If not, see <http://www.apache.org/licenses/>.
 */

#ifndef USB_MICROSOFT_H__
#define USB_MICROSOFT_H__

#include <typedef.h>

/** @defgroup microsoft_items Microsoft-Specific Descriptors
 *  @brief Packet structs from Microsoft's documentation which deal with
 *  the Microsoft OS String Descriptor and the Extended Compat Descriptors.
 *
 *  For more information about these structures, see the Microsoft
 *  documentation at http://msdn.microsoft.com/library/windows/hardware/gg463182
 *  or search for "Microsoft OS Descriptors" on http://msdn.microsoft.com .
 *  Also see docs/winusb.txt in the M-Stack distribution.
 *
 *  @addtogroup microsoft_items
 *  @{
 */

#define MICROSOFT_OS_DESC_VENDOR_CODE 0xEE

// OS String Descriptor
// This is the first descriptor Windows will request, as string number 0xee.

typedef struct
{
    u8 bLength;         // set to 0x12
    u8 bDescriptorType; // set to 0x3
    u16 qwSignature[7]; // set to "MSFT100" (Unicode) (no NULL)
    u8 bMS_VendorCode;  // Set to the bRequest by which the host
                        // should ask for the Compat ID and
                        // Property descriptors. */
    u8 bPad;            // Set to 0x0 */
} MICROSOFT_OS_DESCRIPTOR;

// Extended Compat ID Header
// This is the header for the Extended Compat ID Descriptor

typedef struct
{
    u32 dwLength;       // Total length of descriptor (header + functions)
    u16 bcdVersion;     // Descriptor version number, 0x0100.
    u16 wIndex;         // This OS feature descriptor; set to 0x04.
    u8  bCount;         // Number of custom property sections
    u8  reserved[7];
} microsoft_extended_compat_header;

// Extended Compat ID Function
// This is the function struct for the Extended Compat ID Descriptor

typedef struct
{
    u8 bFirstInterfaceNumber; // The interface or function number
    u8 reserved;
    u8 compatibleID[8];       // Compatible String
    u8 subCompatibleID[8];    // Subcompatible String
    u8 reserved2[6];
} microsoft_extended_compat_function;

// Extended Properties Header
// This is the header for the Extended Properties Descriptor

typedef struct
{
    u32 dwLength;   // Total length of descriptor (header + functions)
    u16 bcdVersion; // Descriptor version number, 0x0100.
    u16 wIndex;     // This OS feature descriptor; set to 0x04.
    u16 bCount;     // Number of custom property sections
} microsoft_extended_properties_header;

// Extended Property Section header
// This is the first part of the Extended Property Section, which is a
// variable-length descriptor. The Variable-length types must be packed
// manually after this section header.

typedef struct
{
    u32 dwSize;     // Size of this section (this struct + data)
    u32 dwPropertyDataType; // Property Data Format

    /* Variable-length fields and lengths:
    u16 wPropertyNameLength;
    u16 bPropertyName[];
    u32 dwPropertyDataLength;
    u8  bPropertyData[];
    */
} microsoft_extended_property_section_header;

#endif /* USB_MICROSOFT_H__ */
