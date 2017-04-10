/**
    All BULK functions should go here
**/

#ifndef USB_BULK_C_
#define USB_BULK_C_

#include <typedef.h>
#include <usb/usb_device.h>
#include <usb/usb_function_bulk.h>

volatile u8 bulk_data_rx[BULK_BULK_OUT_SIZE];
volatile u8 bulk_data_tx[BULK_BULK_IN_SIZE];

#define BULKavailable() (!EP_OUT_BD(USB_BULK_DATA_EP_NUM).Stat.UOWN) && (EP_OUT_BD(USB_BULK_DATA_EP_NUM).Cnt > 0)

/**
    Initialize
**/

void bulk_init_endpoint(void)
{
    // BULK Data EP is IN and OUT EP
    USB_BULK_DATA_EP_UEP = EP_OUT_IN | HSHK_EN;

    // now build EP
    EP_OUT_BD(USB_BULK_DATA_EP_NUM).Cnt = sizeof(bulk_data_rx);
    EP_OUT_BD(USB_BULK_DATA_EP_NUM).ADDR = &bulk_data_rx;
    // USB owns buffer
    EP_OUT_BD(USB_BULK_DATA_EP_NUM).Stat.uc = BDS_UOWN | BDS_DTSEN;

    EP_IN_BD(USB_BULK_DATA_EP_NUM).ADDR = &bulk_data_tx;
    // CPU owns buffer
    EP_IN_BD(USB_BULK_DATA_EP_NUM).Stat.uc = BDS_DTS ;
}

/**
    Function to read a string from USB
    @param buffer Buffer for reading data
    @param lenght Number of bytes to be read
    @return number of bytes acutally read
**/

u8 BULKgets(char *buffer)
{
    u8 i=0;
    u8 length=64;

    if (deviceState != CONFIGURED)
        return 0;

    // Only Process if we own the buffer aka not own by SIE
    if (!EP_OUT_BD(USB_BULK_DATA_EP_NUM).Stat.UOWN)
    {
        // check how much bytes came
        if (length > EP_OUT_BD(USB_BULK_DATA_EP_NUM).Cnt)
            length = EP_OUT_BD(USB_BULK_DATA_EP_NUM).Cnt;
            
        for (i=0; i < EP_OUT_BD(USB_BULK_DATA_EP_NUM).Cnt; i++)
            buffer[i] = bulk_data_rx[i];

        // clear BDT Stat bits beside DTS and then togle DTS
        EP_OUT_BD(USB_BULK_DATA_EP_NUM).Stat.uc &= 0x40;
        EP_OUT_BD(USB_BULK_DATA_EP_NUM).Stat.DTS = !EP_OUT_BD(USB_BULK_DATA_EP_NUM).Stat.DTS;
        // reset buffer count and handle controll of buffer to USB
        EP_OUT_BD(USB_BULK_DATA_EP_NUM).Cnt = sizeof(bulk_data_rx);
        EP_OUT_BD(USB_BULK_DATA_EP_NUM).Stat.uc |= BDS_UOWN | BDS_DTSEN;
    }
    // return number of bytes read
    return i;
}

/**
    Function writes string to USB
    atm not more than MAX_SIZE is allowed
    if more is needed transfer must be split up
**/

u8 BULKputs(char *buffer, u8 length)
{
    u8 i=0;

    if (deviceState != CONFIGURED) return 0;

    if (!EP_IN_BD(USB_BULK_DATA_EP_NUM).Stat.UOWN)
    {
        if (length > BULK_BULK_IN_SIZE)
            length = BULK_BULK_IN_SIZE;
        for (i=0; i < length; i++)
            bulk_data_tx[i] = buffer[i];

        // Set counter to num bytes ready for send
        EP_IN_BD(USB_BULK_DATA_EP_NUM).Cnt = i;
        // clear BDT Stat bits beside DTS and then togle DTS
        EP_IN_BD(USB_BULK_DATA_EP_NUM).Stat.uc &= 0x40;
        EP_IN_BD(USB_BULK_DATA_EP_NUM).Stat.DTS = !EP_IN_BD(USB_BULK_DATA_EP_NUM).Stat.DTS;
        // reset Buffer to original state
        EP_IN_BD(USB_BULK_DATA_EP_NUM).Stat.uc |= BDS_UOWN | BDS_DTSEN;
    }
    return i;
}

//#endif /* USB_USE_BULK */

#endif /* USB_BULK_C_  */
