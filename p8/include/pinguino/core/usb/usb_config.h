#ifndef USBCONFIG_H_
#define USBCONFIG_H_

/**
This file need to be filled up with the desired data for the usb bus
See also: http://www.linuxjournal.com/article/6573
**/

/** 
How many Endpoints do we have, should be at least one for the configuration endpoint 
not used ATM
TODO: use it 
**/
#define USB_MAX_ENDPOINTS 4

/** how many different configuration should be available. At least configuration 1 must exist.
TODO: implement ;)
**/
#define USB_MAX_CONFIGURATION 1

#include <compiler.h>
#include <typedef.h>
#include <usb/picUSB.h>
#ifdef USB_USE_UART
    #include <usb/usb_uart.h>
#endif
#ifdef USB_USE_CDC
    #include <usb/usb_cdc.h>
#endif

/**
 Example Composite of Configuration Header and Interface Descriptors for an ACM Header
 See USB CDC 1.1 Page 15
 Define your own Configuration Descriptor here.
**/

#ifdef USB_USE_CDC
typedef struct
{
    USB_Configuration_Descriptor_Header  Header;
        USB_Interface_Descriptor CMInterface;
            USB_Header_Functional_Descriptor Header_descriptor;
            USB_Abstract_Control_Management_Functional_Descriptor ACM_descriptor;
            USB_Union_Functional_Descriptor UF_descriptor;
            USB_Call_Management_Functional_Descriptor CMF_descriptor;
            USB_Endpoint_Descriptor ep_config; // not used, to remove ?
        USB_Interface_Descriptor DataInterface;
            USB_Endpoint_Descriptor ep_out;
            USB_Endpoint_Descriptor ep_in;
} USB_Configuration_Descriptor;
#endif

#ifdef USB_USE_BULK
typedef struct
{   
    USB_Configuration_Descriptor_Header  Header;
        USB_Interface_Descriptor Interface;
            USB_Endpoint_Descriptor ep_out;
            USB_Endpoint_Descriptor ep_in;
} USB_Configuration_Descriptor;
#endif

// String Descriptor
typedef struct
{
    u8  bLength;
    u8  bDescriptorType;
    u16 string[9];
} USB_String_Descriptor;

#ifdef USB_USE_CDC
    /* Definitions for CDC : 115200 8N1 */
    #define USB_CDC_COMM_INTERFACE  0       /* in which interface are the communication EPs */
    #define USB_CDC_DATA_INTERFACE  1       /* in which interface are the data EPs */
    #define USB_COMM_EP_NUM         2       /* Numm for Comm EP */
    #define USB_COMM_EP_UEP         UEP2    /* corresponding UEP for Data EP */
    #define USB_CDC_DATA_EP_NUM     3       /* Num of Data EP */
    #define USB_CDC_DATA_EP_UEP     UEP3    /* corresponding UEP for Data EP */
    #define USB_CDC_BAUDRATE_DEFAULT        115200
    #define USB_CDC_DATA_BITS       0x08    /* 8-bit      */
    #define USB_CDC_PARITY          0x00    /* No parity  */
    #define USB_CDC_STOP_BITS       0x00    /* 1 Stop Bit */
#endif

#ifdef USB_USE_BULK
    #define USB_BULK_DATA_EP_NUM    1
    #define USB_BULK_DATA_EP_UEP    UEP1
#endif

extern const USB_Device_Descriptor libdevice_descriptor;
extern const USB_Configuration_Descriptor libconfiguration_descriptor;
extern const u8 * const libstring_descriptor[]; // rb 25-01-2013

#endif /* USBCONFIG_H_ */
