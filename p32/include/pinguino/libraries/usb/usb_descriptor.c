/*
 * This file contains functions, macros, definitions, variables,
 * datatypes, etc. that are required for usage with the MCHPFSUSB device
 * stack. This file should be included in projects that use the device stack.
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the 'Company') for its PIC® Microcontroller is intended and
 * supplied to you, the Company's customer, for use solely and
 * exclusively on Microchip PIC Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN 'AS IS' CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 */
 
#ifndef USBDESCRIPTORS_C
#define USBDESCRIPTORS_C

//#include "usb/usb.h"
//#include "usb/usb_function_hid.h"
#include "usb/usb_function_cdc.h"
//#include <usb/usb_device.c>
//#include <usb/usb_function_cdc.c>

/***********************************************************************
 * Device descriptor.
 **********************************************************************/
 
const USB_DEVICE_DESCRIPTOR usb_device = {
    sizeof (usb_device),    // Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,  // DEVICE descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
    0x00,                   // Class Code
    0x00,                   // Subclass code
    0x00,                   // Protocol code
    USB_EP0_BUFF_SIZE,      // Max packet size for EP0
    0x04D8,                 // Vendor ID
    0xFEAB,                 // Product ID: PINGUINO USB CDC
    0x0002,                 // Device release number in BCD format
    1,                      // Manufacturer string index
    2,                      // Product string index
    0,                      // Device serial number string index
    1,                      // Number of possible configurations
};

/*
 * Configuration 1 descriptor
 */

const unsigned char usb_config1_descriptor[] = {

    // Configuration descriptor
    sizeof (USB_CONFIGURATION_DESCRIPTOR),
    USB_DESCRIPTOR_CONFIGURATION,
    0x29, 0x00,             // Total length of data for this cfg
    1,                      // Number of interfaces in this cfg
    1,                      // Index value of this configuration
    0,                      // Configuration string index
    _DEFAULT | _SELF,       // Attributes
    50,                     // Max power consumption (2X mA)

    /// HID ************************************************************
/*
    // Interface descriptor
    sizeof (USB_INTERFACE_DESCRIPTOR),
    USB_DESCRIPTOR_INTERFACE,
    0,                      // Interface Number
    0,                      // Alternate Setting Number
    2,                      // Number of endpoints in this intf
    HID_INTF,               // Class code
    0,                      // Subclass code
    0,                      // Protocol code
    0,                      // Interface string index

    // HID Class-Specific descriptor
    sizeof (USB_HID_DSC) + 3,
    DSC_HID,
    0x11, 0x01,             // HID Spec Release Number in BCD format (1.11)
    0x00,                   // Country Code (0x00 for Not supported)
    HID_NUM_OF_DSC,         // Number of class descriptors
    DSC_RPT,                // Report descriptor type
    HID_RPT01_SIZE, 0x00,   // Size of the report descriptor

    // Endpoint descriptor
    sizeof (USB_ENDPOINT_DESCRIPTOR),
    USB_DESCRIPTOR_ENDPOINT,
    HID_EP | _EP_IN,        // EndpointAddress
    _INTERRUPT,             // Attributes
    PACKET_SIZE, 0,         // Size
    1,                      // Interval

    // Endpoint descriptor
    sizeof (USB_ENDPOINT_DESCRIPTOR),
    USB_DESCRIPTOR_ENDPOINT,
    HID_EP | _EP_OUT,       // EndpointAddress
    _INTERRUPT,             // Attributes
    PACKET_SIZE, 0,         // Size
    1,                      // Interval
*/

    /// CDC ************************************************************

    // Interface Descriptor
    0x09,                               // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,           // Interface descriptor type
    CDC_COMM_INTF_ID,                   // Interface number
    0x00,                               // Alternate setting number
    0x01,                               // Number of endpoints in this interface
    COMM_INTF,                          // Class code
    ABSTRACT_CONTROL_MODEL,             // Subclass code
    V25TER,                             // Protocol code
    0x00,                               // Interface string index

    // CDC Class Specific
    sizeof(USB_CDC_HEADER_FN_DSC),      // Size of this descriptor in bytes (5)
    CS_INTERFACE,                       // bDescriptorType
    DSC_FN_HEADER,                      // bDescriptorSubtype
    0x20, 0x01,                         // bcdCDC

    // Abstract Control Management Functional Descriptor
    sizeof(USB_CDC_ACM_FN_DSC),         // Size of this descriptor in bytes (4)
    CS_INTERFACE,                       // bDescriptorType
    DSC_FN_ACM,                         // bDescriptorSubtype
    USB_CDC_ACM_FN_DSC_VAL,             // bmCapabilities: (see PSTN120.pdf Table 4)

    // Union Functional Descriptor
    sizeof(USB_CDC_UNION_FN_DSC),       // Size of this descriptor in bytes (5)
    CS_INTERFACE,                       // bDescriptorType
    DSC_FN_UNION,                       // bDescriptorSubtype
    0x01,                               // bControlInterface
    0x02,                               // bSubordinateInterface0

    // Call Management Functional Descriptor
    sizeof(USB_CDC_CALL_MGT_FN_DSC),    // Size of this descriptor in bytes (5)
    CS_INTERFACE,                       // bDescriptorType
    DSC_FN_CALL_MGT,                    // bDescriptorSubtype
    0x00,                               // bmCapabilities
    0x02,                               // bDataInterface

    // Endpoint Descriptor
    0x07,
    USB_DESCRIPTOR_ENDPOINT,            // Endpoint descriptor
    _EP01_IN,                           // Endpoint address
    _INTERRUPT,                         // Attributes
    CDC_COMM_IN_EP_SIZE, 0x00,          // Size
    0x02,                               // Interval

    // CDC Data Interface
    0x09,                               // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,           // Interface descriptor type
    0x02,                               // Interface number
    0x00,                               // Alternate setting number
    0x02,                               // Number of endpoints in this interface
    DATA_INTF,                          // Class code
    0x00,                               // Subclass code
    NO_PROTOCOL,                        // Protocol code
    0x00,                               // Interface string index

    // Endpoint Descriptor
    0x07,                               // Size of this descriptor in bytes
    USB_DESCRIPTOR_ENDPOINT,            // Endpoint descriptor
    _EP02_OUT,                          // Endpoint address
    _BULK,                              // Attributes
    DESC_CONFIG_WORD(0x40),             // Size
    0x00,                               // Interval

    // Endpoint Descriptor
    0x07,                               // Size of this descriptor in bytes
    USB_DESCRIPTOR_ENDPOINT,            // Endpoint descriptor
    _EP02_IN,                           // Endpoint address
    _BULK,                              // Attributes
    DESC_CONFIG_WORD(0x40),             // Size
    0x00                                // Interval
};

/*
 * Class specific descriptor - HID
 */
 
/*
const unsigned char hid_rpt01 [HID_RPT01_SIZE] = {
    0x06, 0x00, 0xFF,       // Usage Page = 0xFF00 (Vendor Defined Page 1)
    0x09, 0x01,             // Usage (Vendor Usage 1)
    0xA1, 0x01,             // Collection (Application)

    0x19, 0x01,             // Usage Minimum
    0x29, 0x40,             // Usage Maximum 64 input usages total (0x01 to 0x40)
    0x15, 0x00,             // Logical Minimum (data bytes in the report may have minimum value = 0x00)
    0x26, 0xFF, 0x00,       // Logical Maximum (data bytes in the report may have maximum value = 0x00FF = unsigned 255)
    0x75, 0x08,             // Report Size: 8-bit field size
    0x95, 0x40,             // Report Count: Make sixty-four 8-bit fields (the next time the parser hits an "Input", "Output", or "Feature" item)
    0x81, 0x00,             // Input (Data, Array, Abs): Instantiates input packet fields based on the above report size, count, logical min/max, and usage.

    0x19, 0x01,             // Usage Minimum
    0x29, 0x40,             // Usage Maximum 64 output usages total (0x01 to 0x40)
    0x91, 0x00,             // Output (Data, Array, Abs): Instantiates output packet fields.  Uses same report size and count as "Input" fields, since nothing new/different was specified to the parser since the "Input" item.

    0xC0,                   // End Collection
};
*/

/*
 * USB Strings
 */

#if defined(USB_NUM_STRING_DESCRIPTORS)

static const USB_STRING_INIT(1) string0_descriptor = {
    sizeof(string0_descriptor),
    USB_DESCRIPTOR_STRING,              /* Language code */
    { 0x0409 },
};

static const USB_STRING_INIT(25) string1_descriptor = {
    sizeof(string1_descriptor),
    USB_DESCRIPTOR_STRING,              /* Manufacturer */
    { 'M','i','c','r','o','c','h','i','p',' ',
      'T','e','c','h','n','o','l','o','g','y',
      ' ','I','n','c','.' },
};

static const USB_STRING_INIT(19) string2_descriptor = {
    sizeof(string2_descriptor),
    USB_DESCRIPTOR_STRING,              /* Product */
    { 'P','I','N','G','U','I','N','O',' ','C','D','C',' ','D','e','v','i','c','e' },
};

// Array of configuration descriptors
const unsigned char *const usb_config[] = {
    (const unsigned char *const) &usb_config1_descriptor,
};

// Array of string descriptors
const unsigned char *const usb_string[USB_NUM_STRING_DESCRIPTORS] = {
    (const unsigned char *const) &string0_descriptor,
    (const unsigned char *const) &string1_descriptor,
    (const unsigned char *const) &string2_descriptor,
};

#endif

#endif	/* USBDESCRIPTORS_C */
