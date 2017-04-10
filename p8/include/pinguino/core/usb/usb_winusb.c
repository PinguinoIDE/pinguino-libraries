/*
 *  M-Stack Automatic WinUSB Support
 *  Copyright (C) 2013 Alan Ott <alan@signal11.us>
 *  Copyright (C) 2013 Signal 11 Software
 *
 *  2013-10-12
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

#include <typedef.h>
//#include <stdint.h>
#include <usb/usb_config.h>
#include <usb/usb_microsoft.h>
#include <usb/usb_winusb.h>

#ifdef AUTOMATIC_WINUSB_SUPPORT

/* Microsoft-specific descriptors for automatic binding of the WinUSB driver.
 * See docs/winusb.txt for details. */
struct extended_compat_descriptor_packet
{
    struct microsoft_extended_compat_header header;
    struct microsoft_extended_compat_function function;
};

static struct extended_compat_descriptor_packet this_extended_compat_descriptor =
{
    /* Header */
    {
    sizeof(struct extended_compat_descriptor_packet), /* dwLength */
    0x0100, /* dwVersion*/
    0x0004, /* wIndex: 0x0004 = Extended Compat ID */
    1,      /* bCount, number of custom property sections */
    {0},    /* reserved[7] */
    },

    /* Function */
    {
    0x0,      /* bFirstInterfaceNumber */
    0x1,      /* reserved. Set to 1 in the Microsoft example */
    "WINUSB", /* compatibleID[8] */
    "",       /* subCompatibleID[8] */
    {0},      /* reserved2[6] */
    },
};

static struct microsoft_extended_properties_header interface_0_property_descriptor =
{
    sizeof(interface_0_property_descriptor), /* dwLength */
    0x0100, /* bcdVersion */
    0x0005, /* wIndex, Extended Properties descriptor */
    0x0,    /* bCount, Number of custom property sections */
};

#ifdef MICROSOFT_COMPAT_ID_DESCRIPTOR_FUNC
/** @brief Callback for the GET_MS_DESCRIPTOR/CompatID request
 *
 * MICROSOFT_COMPAT_ID_DESCRIPTOR_FUNC() is called when a
 * GET_MS_DESCRIPTOR request is received from the host with a wIndex of 0x0004.
 * The value of MS_GET_DESCRIPTOR request is defined by
 * MICROSOFT_OS_DESC_VENDOR_CODE which is set in usb_config.h, and reported to
 * the host as part of the microsoft_os_descriptor. See the MSDN
 * documentation on "Microsoft OS Descriptors" for more information.
 *
 * @param interface      The interface for which the descriptor is queried
 * @param descriptor     a pointer to a pointer which should be set to the
 *                       descriptor data.
 * @returns
 *   Return the length of the descriptor pointed to by *descriptor, or -1
 *   if the descriptor does not exist.
 */

u16 m_stack_winusb_get_microsoft_compat(u8 interface, const void **descriptor)
{
    /* Check the interface here for composite devices. */
    *descriptor = &this_extended_compat_descriptor;
    return sizeof(this_extended_compat_descriptor);
}

#endif

#ifdef MICROSOFT_CUSTOM_PROPERTY_DESCRIPTOR_FUNC
/** @brief Callback for the GET_MS_DESCRIPTOR/Custom_Property request
 *
 * MICROSOFT_CUSTOM_PROPERTY_DESCRIPTOR_FUNC() is called when a
 * GET_MS_DESCRIPTOR request with a wIndex of 0x0005 is received from the host.
 * The value of the MS_GET_DESCRIPTOR request is defined by
 * MICROSOFT_OS_DESC_VENDOR_CODE which is set in usb_config.h, and reported to
 * the host as part of the microsoft_os_descriptor. See the MSDN
 * documentation on "Microsoft OS Descriptors" for more information.
 *
 * @param interface      The interface for which the descriptor is queried
 * @param descriptor     a pointer to a pointer which should be set to the
 *                       descriptor data.
 * @returns
 *   Return the length of the descriptor pointed to by *descriptor, or -1
 *   if the descriptor does not exist.
 */

u16 m_stack_winusb_get_microsoft_property(u8 interface, const void **descriptor)
{
    /* Check the interface here for composite devices. */
    *descriptor = &interface_0_property_descriptor;
    return sizeof(interface_0_property_descriptor);
}

#endif

#endif /* AUTOMATIC_WINUSB_SUPPORT */
