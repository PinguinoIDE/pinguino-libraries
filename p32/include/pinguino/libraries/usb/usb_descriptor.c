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
    sizeof(usb_device),     // Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,  // DEVICE descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
    CDC_DEVICE,             // Class Code CDC_DEVICE
    0x00,                   // Subclass code
    0x00,                   // Protocol code
    USB_EP0_BUFF_SIZE,      // Max packet size for EP0
    0x04D8,                 // Vendor ID
    0xFEAB,                 // Product ID: PINGUINO USB CDC
    0x0000,                 // Device release number in BCD format
    1,                      // Manufacturer string index
    2,                      // Product string index
    0,                      // Device serial number string index
    1,                      // Number of possible configurations
};

/*
 * Configuration 1 descriptor
 */

// Total length in chars of data returned
#define CONFIGURATION_TOTAL_LENGTH (sizeof(USB_CONFIGURATION_DESCRIPTOR) + \
                                    sizeof(USB_INTERFACE_DESCRIPTOR) + \
                                    sizeof(USB_CDC_HEADER_FN_DSC) + \
                                    sizeof(USB_CDC_ACM_FN_DSC) + \
                                    sizeof(USB_CDC_UNION_FN_DSC) + \
                                    sizeof(USB_CDC_CALL_MGT_FN_DSC) + \
                                    sizeof(USB_ENDPOINT_DESCRIPTOR) + \
                                    sizeof(USB_INTERFACE_DESCRIPTOR) + \
                                    sizeof(USB_ENDPOINT_DESCRIPTOR) + \
                                    sizeof(USB_ENDPOINT_DESCRIPTOR) )

//const USB_Configuration_Descriptor usb_config1_descriptor = {
const unsigned char usb_config1_descriptor[] = {

    // Configuration descriptor
    sizeof(USB_CONFIGURATION_DESCRIPTOR),
    USB_DESCRIPTOR_CONFIGURATION,
    CONFIGURATION_TOTAL_LENGTH,         // Total length of data for this cfg
    2,                                  // Number of interfaces in this cfg     // 1
    1,                                  // Index value of this configuration
    0,                                  // Configuration string index
    USB_CFG_DSC_SELF_PWR,               //(_DEFAULT | _SELF),     // Attributes                           // USB_CFG_DSC_REQUIRED
    20,                                 // Max power consumption (100 mA) in 2mA units

    // Interface Descriptor
    sizeof(USB_INTERFACE_DESCRIPTOR),   // Size of this descriptor in bytes // 0x09
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
    0x10, 0x01,                         // bcdCDC (RB20150129 : was 0x0120)

    // Abstract Control Management Functional Descriptor
    sizeof(USB_CDC_ACM_FN_DSC),         // Size of this descriptor in bytes (4)
    CS_INTERFACE,                       // bDescriptorType
    DSC_FN_ACM,                         // bDescriptorSubtype
    0x00, //USB_CDC_ACM_FN_DSC_VAL,             // bmCapabilities: (see PSTN120.pdf Table 4)

    // Union Functional Descriptor
    sizeof(USB_CDC_UNION_FN_DSC),       // Size of this descriptor in bytes (5)
    CS_INTERFACE,                       // bDescriptorType
    DSC_FN_UNION,                       // bDescriptorSubtype
    CDC_COMM_INTF_ID,                   // bControlInterface        // 01 ?
    CDC_DATA_INTF_ID,                   // bSubordinateInterface0   // 02 ?

    // Call Management Functional Descriptor
    sizeof(USB_CDC_CALL_MGT_FN_DSC),    // Size of this descriptor in bytes (5)
    CS_INTERFACE,                       // bDescriptorType
    DSC_FN_CALL_MGT,                    // bDescriptorSubtype
    0x00,                               // bmCapabilities           // 00
    CDC_DATA_INTF_ID,                   // bDataInterface           // 02

    // Endpoint Descriptor
    sizeof(USB_ENDPOINT_DESCRIPTOR),    // 0x07
    USB_DESCRIPTOR_ENDPOINT,            // Endpoint descriptor
    _EP02_IN,                           // Endpoint address         // _EP01_IN
    _INTERRUPT,                         // Attributes
    16,//CDC_COMM_IN_EP_SIZE,                // Size
    0x02,                               // Interval 2ms

    // CDC Data Interface
    sizeof(USB_INTERFACE_DESCRIPTOR),   // 0x09
    USB_DESCRIPTOR_INTERFACE,           // Interface descriptor type
    CDC_DATA_INTF_ID,                   // Interface number         // 0x02
    0x00,                               // Alternate setting number
    0x02,                               // Number of endpoints in this interface
    DATA_INTF,                          // Class code
    0x00,                               // Subclass code
    NO_PROTOCOL,                        // Protocol code
    0x00,                               // Interface string index

    // Endpoint Descriptor
    sizeof(USB_ENDPOINT_DESCRIPTOR),    // 0x07
    USB_DESCRIPTOR_ENDPOINT,            // Endpoint descriptor
    _EP03_OUT,                          // Endpoint address         // _EP02_OUT
    _BULK,                              // Attributes
    0x0040,                             // Size
    0x00,                               // Interval

    // Endpoint Descriptor
    sizeof(USB_ENDPOINT_DESCRIPTOR),    // 0x07
    USB_DESCRIPTOR_ENDPOINT,            // Endpoint descriptor
    _EP03_IN,                           // Endpoint address         // _EP02_IN
    _BULK,                              // Attributes
    0x0040,                             // Size
    0x00                                // Interval
};

/*
 * USB Strings
 */

#if defined(USB_NUM_STRING_DESCRIPTORS)

static const USB_STRING_INIT(1) string0_descriptor = {
    sizeof(string0_descriptor),
    USB_DESCRIPTOR_STRING,              /* Language code */
    { 0x0409 }
};

static const USB_STRING_INIT(10) string1_descriptor = {
    sizeof(string1_descriptor),
    USB_DESCRIPTOR_STRING,              /* Manufacturer */
    {'R','.','B','l','a','n','c','h','o','t'}
};

static const USB_STRING_INIT(8) string2_descriptor = {
    sizeof(string2_descriptor),
    USB_DESCRIPTOR_STRING,              /* Product */
    { 'P','I','N','G','U','I','N','O' }
};
/*
static const USB_STRING_INIT(10) string3_descriptor = {
    sizeof(string3_descriptor),
    USB_DESCRIPTOR_STRING,              // Serial Number
    {}
};
*/
// Array of configuration descriptors
const unsigned char *const usb_config[] = {
    (const unsigned char *const) &usb_config1_descriptor,
};

// Array of string descriptors
const unsigned char *const usb_string[USB_NUM_STRING_DESCRIPTORS] = {
    (const unsigned char *const) &string0_descriptor,
    (const unsigned char *const) &string1_descriptor,
    (const unsigned char *const) &string2_descriptor,
    //(const unsigned char *const) &string3_descriptor,
};

#endif

#endif	/* USBDESCRIPTORS_C */
