/**
    Definitions for the USB BULK
**/

#ifndef USB_BULK_H_
#define USB_BULK_H_

#include <typedef.h>

/*******************************
 * BULK Specific Configuration *
 *******************************/

// Configurations
#define BULK_CONFIG_NUM                 1

// Interfaces
#define BULK_INT_NUM                    1

// Endpoint
#define BULK_EP_NUM                     2
#define BULK_IN_EP_SIZE                 8
#define BULK_BULK_IN_SIZE               64
#define BULK_BULK_OUT_SIZE              64

/*
 * USB directions
 *
 * This bit flag is used in endpoint descriptors' bEndpointAddress field.
 * It's also one of three fields in control requests bRequestType.
 */
#define USB_DIR_OUT                     0               /* to device */
#define USB_DIR_IN                      0x80            /* to host */

/*
 * USB types, the second of three bRequestType fields
 */
#define USB_TYPE_MASK                   (0x03 << 5)
#define USB_TYPE_STANDARD               (0x00 << 5)
#define USB_TYPE_CLASS                  (0x01 << 5)
#define USB_TYPE_VENDOR                 (0x02 << 5)
#define USB_TYPE_RESERVED               (0x03 << 5)

/*
 * USB recipients, the third of three bRequestType fields
 */
#define USB_RECIP_MASK                  0x1f
#define USB_RECIP_DEVICE                0x00
#define USB_RECIP_INTERFACE             0x01
#define USB_RECIP_ENDPOINT              0x02
#define USB_RECIP_OTHER                 0x03

/*
 * Functions for BULK classes
 */

//#ifdef USB_USE_BULK
// BULK specific buffers
extern volatile byte bulk_data_rx[BULK_BULK_OUT_SIZE];
extern volatile byte bulk_data_tx[BULK_BULK_IN_SIZE];

//void usb_bulk_check_request(void);
void usb_bulk_init_endpoint(void);
//#endif

#endif /* USB_BULK_H_ */
