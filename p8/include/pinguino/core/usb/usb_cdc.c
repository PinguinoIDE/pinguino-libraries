/**
    All CDC functions should go here
**/

#ifndef USB_CDC_C_
#define USB_CDC_C_

//#include <string.h>
#include <typedef.h>
#include <usb/usb_cdc.h>
#include <usb/picUSB.c>
#include <usb/usb_config.c>

//#ifdef USB_USE_CDC

// Put USB CDC specific I/O buffers into dual port RAM.
// sizeof(CDCRxBuffer) = CDC_BULK_OUT_SIZE = 64
// sizeof(CDCTxBuffer) = CDC_BULK_IN_SIZE  = 64

//extern setupPacketStruct SetupPacket;
//extern u8 requestHandled;
extern u32 cdc_baudrate;

#ifdef __XC8__
    //extern volatile BufferDescriptorTable ep_bdt[2*USB_MAX_ENDPOINTS];
    #if defined(__16F1459)
        #define RX_ADDR_TAG @##RX_ADDR
        #define TX_ADDR_TAG @##TX_ADDR
        u8 __section("usbram5") dummy; // to prevent a compilation error
        u8 CDCRxBuffer[USB_CDC_OUT_EP_SIZE] RX_ADDR_TAG; //@ 0x2028; //0x2098;
        u8 CDCTxBuffer[USB_CDC_IN_EP_SIZE]  TX_ADDR_TAG; //@ 0x2030; //0x20D8;
    #else
        u8 __section("usbram5") CDCRxBuffer[USB_CDC_OUT_EP_SIZE];
        u8 __section("usbram5") CDCTxBuffer[USB_CDC_IN_EP_SIZE];
    #endif
#else // SDCC
    //extern volatile BufferDescriptorTable __at BD_ADDR ep_bdt[2*USB_MAX_ENDPOINTS];
    #pragma udata usbram5 CDCRxBuffer CDCTxBuffer
    volatile u8 CDCRxBuffer[USB_CDC_OUT_EP_SIZE];
    volatile u8 CDCTxBuffer[USB_CDC_IN_EP_SIZE];
#endif

u8 CDCControlBuffer[USB_CDC_CTRL_EP_SIZE];
USB_CDC_Line_Coding line_config;
Zero_Packet_Length zlp;

/**
    Initialize all enpoints
**/

void CDCInitEndpoint(void)
{
    #ifdef DEBUG_PRINT_CDC
    printf("CDCInitEndpoint\r\n");
    #endif

    line_config.dwDTERate =   cdc_baudrate;
    line_config.bDataBits =   USB_CDC_DATA_BITS;    // 8
    line_config.bParityType = USB_CDC_PARITY;       // N
    line_config.bCharFormat = USB_CDC_STOP_BITS;    // 1
    
    zlp.wValue0=0;
    zlp.wValue1=0;
    zlp.wValue2=0;
    zlp.wValue3=0;
    zlp.wValue4=0;
    zlp.wValue5=0;
    zlp.wValue6=0;
    zlp.wValue7=0;
    
    // set global state variable

    // Configure USB_COMM_EP_UEP as IN and Communication PIPE
    USB_COMM_EP_UEP = EP_IN | HSHK_EN;

    // CDC Data EP is IN and OUT EP
    USB_CDC_DATA_EP_UEP = EP_OUT_IN | HSHK_EN;

    // Communication EP (not used, to remove ?)
    EP_IN_BD(USB_COMM_EP_NUM).ADDR = PTR16(&CDCControlBuffer);
    EP_IN_BD(USB_COMM_EP_NUM).Stat.uc = BDS_DAT1 | BDS_COWN;

    // Data OUT EP
    EP_OUT_BD(USB_CDC_DATA_EP_NUM).Cnt = sizeof(CDCRxBuffer);
    EP_OUT_BD(USB_CDC_DATA_EP_NUM).ADDR = PTR16(&CDCRxBuffer);
    EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.uc = BDS_DAT0 | BDS_UOWN | BDS_DTSEN;

    // Data IN EP
    EP_IN_BD(USB_CDC_DATA_EP_NUM).ADDR = PTR16(&CDCTxBuffer); // +1 
    EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.uc = BDS_DAT1 | BDS_COWN; 

    deviceState=CONFIGURED; 

    #ifdef DEBUG_PRINT_CDC
    printf("end CDCInitEndpoint\r\n");
    #endif         
}

/**
    Special Setup function for CDC Class to handle Setup requests.
**/

void ProcessCDCRequest(void)
{
    #ifdef DEBUG_PRINT_CDC
    printf("requete %x\r\n",SetupPacket.bmRequestType);
    #endif  

    // If its not a Class request return
    if ((SetupPacket.bmRequestType & USB_TYPE_MASK) != USB_TYPE_CLASS)
        return;

    #ifdef DEBUG_PRINT_CDC
    printf("%x\r\n",SetupPacket.bRequest);
    #endif

    switch(SetupPacket.bRequest)
    {
        //****** These commands are required ******//
        case USB_CDC_SEND_ENCAPSULATED_COMMAND:
        case USB_CDC_GET_ENCAPSULATED_RESPONSE:
            break;

        case USB_CDC_REQ_SET_LINE_CODING:
        case USB_CDC_REQ_GET_LINE_CODING:
            outPtr = (u8*)&line_config;
            wCount = sizeof(USB_CDC_Line_Coding) ;
            requestHandled = 1;
            break;

        case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
            // Populate dummy_encapsulated_cmd_response first.
            #ifdef DEBUG_PRINT_CDC
            printf("set control line\r\n");
            printf("%x\r\n",SetupPacket.wValue0);
            printf("%x\r\n",SetupPacket.wValue1);
            #endif
            if (SetupPacket.wValue0==3)
                CONTROL_LINE=1;
            else
                CONTROL_LINE=0;
            outPtr = (u8*)&zlp;
            wCount = sizeof(Zero_Packet_Length) ;
            requestHandled = 1;
            break;
        //****** End of required commands ******//    
    }
}

/**
    Function to read a string from USB
    @param buffer Buffer for reading data
    @param lenght Number of u8s to be read
    @return number of u8s acutally read
**/
/*
u8 CDCgets(u8 *buffer)
{
    u8 i=0;
    u8 length=64;

    if (deviceState != CONFIGURED)
        return 0;

    // Only Process if we own the buffer aka not own by SIE
    if (!CONTROL_LINE)
        return 0;

    // Only process if a serial device is connected
    if (!EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.UOWN)
    {
        #ifdef DEBUG_PRINT_CDC
        printf("Rx on EP %d, Size %d\r\n", USB_CDC_DATA_EP_NUM, EP_OUT_BD(USB_CDC_DATA_EP_NUM).Cnt);
        #endif

        // check how much bytes came
        if (length > EP_OUT_BD(USB_CDC_DATA_EP_NUM).Cnt)
            length = EP_OUT_BD(USB_CDC_DATA_EP_NUM).Cnt;
            
        for (i=0; i < EP_OUT_BD(USB_CDC_DATA_EP_NUM).Cnt; i++)
        {
            buffer[i] = CDCRxBuffer[i];

            #ifdef DEBUG_PRINT_CDC
            printf("%c",CDCRxBuffer[i]);
            #endif
        }
        
        #ifdef DEBUG_PRINT_CDC
        printf("->");
        #endif

        // clear BDT Stat bits beside DTS and then togle DTS
        EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.uc &= BDS_DTS;
        EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.DTS ^= 1;
        //!EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.DTS;
        // reset buffer count and handle controll of buffer to USB
        EP_OUT_BD(USB_CDC_DATA_EP_NUM).Cnt = sizeof(CDCRxBuffer);
        EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.uc |= (BDS_UOWN | BDS_DTSEN);
    }
    // return number of bytes read
    return i;
}
*/
/*
u8 CDCputs(const u8 *buffer, u8 length)
{
    u8 i=0;

    if (deviceState != CONFIGURED) return 0;
    
    if (!CONTROL_LINE) return 0;
    
    if (!EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.UOWN)
    {
        if (length > CDC_BULK_IN_SIZE)
            length = CDC_BULK_IN_SIZE;
        
        for (i=0; i < length; i++)
        {
            CDCTxBuffer[i] = buffer[i];

            #ifdef DEBUG_PRINT_CDC
            printf("%c",CDCTxBuffer[i]);
            #endif
        }

        #ifdef DEBUG_PRINT_CDC
        printf("->");
        #endif

        // Set counter to num bytes ready for send
        EP_IN_BD(USB_CDC_DATA_EP_NUM).Cnt = i;
        // clear BDT Stat bits beside DTS and then togle DTS
        EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.uc &= BDS_DTS;
        EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.DTS = !EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.DTS;
        // reset Buffer to original state
        EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.uc |= BDS_UOWN | BDS_DTSEN;
    }
    return i;
}
*/

/*
void CDCputc(u8 c)
{
    if (deviceState != CONFIGURED) return;
    
    if (!CONTROL_LINE) return;
    
    if (!EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.UOWN)
    {
        CDCTxBuffer[0] = c;

        // Set counter to num bytes ready for send
        EP_IN_BD(USB_CDC_DATA_EP_NUM).Cnt = 1;
        // clear BDT Stat bits beside DTS and then togle DTS
        EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.uc &= 0x40;
        EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.DTS = !EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.DTS;
        // reset Buffer to original state
        EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.uc |= BDS_UOWN | BDS_DTSEN;
    }
}
*/

//#endif // USB_USE_CDC
#endif // USB_CDC_C_
