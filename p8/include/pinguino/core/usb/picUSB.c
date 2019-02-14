  // Firmware framework for USB I/O on PIC 18F2455 (and siblings)
  // Copyright (C) 2005 Alexander Enzmann
  //
  // Adapted to Pinguino by Jean-Pierre Mandon (C) 2010
  // 
  // This library is free software; you can redistribute it and/or
  // modify it under the terms of the GNU Lesser General Public
  // License as published by the Free Software Foundation; either
  // version 2.1 of the License, or (at your option) any later version.
  //
  // This library is distributed in the hope that it will be useful,
  // but WITHOUT ANY WARRANTY; without even the implied warranty of
  // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  // Lesser General Public License for more details.
  //
  // You should have received a copy of the GNU Lesser General Public
  // License along with this library; if not, write to the Free Software
  // Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  //

#ifndef PICUSB_C
#define PICUSB_C

#include <compiler.h>
#include <string.h>

#include <usb/picUSB.h>
#include <usb/usb_config.h>
#include <usb/usb_microsoft.h>

#ifdef USB_USE_CDC
#include <usb/usb_cdc.c>
#endif

#ifdef USB_USE_BULK
#include <usb/usb_bulk.c>
#endif

#ifdef USB_USE_HID
#include <usb/usb_hid.c>
#endif

#define ALLOW_SUSPEND 0
  // It appears that you need at least 6 loops that are replaced by memcpy()
  // before it is an advantage.
#define USE_MEMCPY 0

  // Device and configuration descriptors.  These are used as the
  // host enumerates the device and discovers what class of device
  // it is and what interfaces it supports.
  // TODO: remove below lines and replace with the apropriate device_desc.blength etc.
/*
#define DEVICE_DESCRIPTOR_SIZE 0x12
#define CONFIG_HEADER_SIZE  0x09
#define HID_DESCRIPTOR_SIZE 0x20
#define HID_HEADER_SIZE 0x09
*/

  // Global variables
u8 deviceState;
u8 remoteWakeup;
u8 deviceAddress;
u8 selfPowered;
u8 currentConfiguration;

  // Control Transfer Stages - see USB spec chapter 5
#define SETUP_STAGE    0                // Start of a control transfer (followed by 0 or more data stages)
#define DATA_OUT_STAGE 1                // Data from host to device
#define DATA_IN_STAGE  2                // Data from device to host
#define STATUS_STAGE   3                // Unused - if data I/O went ok, then back to Setup

u8 ctrlTransferStage;                 // Holds the current stage in a control transfer
u8 requestHandled;                    // Set to 1 if request was understood and processed.

u8 *outPtr;                           // Data to send to the host
u8 *inPtr;                            // Data from the host
u16 wCount;                            // Number of bytes of data

  // HID Class variables
#ifdef USB_USE_HID
u8 HIDPostProcess;                    // Set to 1 if HID needs to process after the data stage
u8 hidIdleRate;
u8 hidProtocol;                       // [0] Boot Protocol [1] Report Protocol
u8 hidRxLen;                          // # of bytes put into buffer
#endif

//
//  USB RAM / Buffer Descriptor Table
//  BDT is used specifically for endpoint buffer control
//  NB : If not all endpoints are used the space in RAM is wasted.
//  RB 09/09/2012 : added some other PIC support
//  RB 30/11/2015 : added PIC16F1459 support
//

#ifdef __XC8__
    #define BD_ADDR_TAG @##BD_ADDR
    volatile BufferDescriptorTable ep_bdt[2*USB_MAX_ENDPOINTS] BD_ADDR_TAG;
    #if defined(__16F1459)
        #define PA_ADDR_TAG @##PA_ADDR
        #define TR_ADDR_TAG @##TR_ADDR
        u8 __section("usbram5") dummy; // to prevent a compilation error
        setupPacketStruct SetupPacket PA_ADDR_TAG; //@ 0x2020; //0x2018;
        u8 controlTransferBuffer[EP0_BUFFER_SIZE] TR_ADDR_TAG; //@ 0x2028; //0x2020; //0x2058;
    #else //18F
        setupPacketStruct __section("usbram5") SetupPacket;
        u8 __section("usbram5") controlTransferBuffer[EP0_BUFFER_SIZE];
    #endif
#else // SDCC
    volatile BufferDescriptorTable __at BD_ADDR ep_bdt[2*USB_MAX_ENDPOINTS];
    #pragma udata usbram5 SetupPacket controlTransferBuffer
    setupPacketStruct SetupPacket;
    u8 controlTransferBuffer[EP0_BUFFER_SIZE];
#endif

//
// Start of code to process standard requests (USB chapter 9)
//

// Process GET_DESCRIPTOR
void GetDescriptor(void)
{
    #ifdef USB_USE_DEBUG
    u8 descriptorType  = SetupPacket.wValue1;
    u8 descriptorIndex = SetupPacket.wValue0;
    Serial_printf("> GETDESCRIPTOR\r\n");
    #endif

    if(SetupPacket.bmRequestType == 0x80)
    {
        if (SetupPacket.wValue1 == DEVICE_DESCRIPTOR)
        {
            #ifdef USB_USE_DEBUG
            Serial_printf("  DEVICE (0x%ux)\r\n",(u16)descriptorType);
            #endif
            requestHandled = 1;
            outPtr = (u8*)&libdevice_descriptor;
            wCount = sizeof(USB_Device_Descriptor);
        }

        else if (SetupPacket.wValue1 == CONFIGURATION_DESCRIPTOR)
        {
            #ifdef USB_USE_DEBUG
            Serial_printf("  CONFIGURATION (0x%ux): %d\r\n", (u16)descriptorType, descriptorIndex);
            #endif

            requestHandled = 1;
            outPtr = (u8*)&libconfiguration_descriptor;
            wCount = libconfiguration_descriptor.Header.wTotalLength;

            #ifdef USB_USE_DEBUG
            //Serial_printf("Total config size: %d\r\n", wCount);
            #endif
        }

        else if (SetupPacket.wValue1 == STRING_DESCRIPTOR)
        {
            #ifdef USB_USE_DEBUG
            Serial_printf("  STRING: %d\r\n", (u16)descriptorIndex);
            #endif

            #ifdef MICROSOFT_OS_DESC_VENDOR_CODE
            if (SetupPacket.wValue0 == MICROSOFT_OS_DESC_VENDOR_CODE)
            {
                const MICROSOFT_OS_DESCRIPTOR microsoft_os_descriptor =
                {
                    0x12,                          /* bLength */
                    0x3,                           /* bDescriptorType */
                    {'M','S','F','T','1','0','0'}, /* qwSignature */
                    MICROSOFT_OS_DESC_VENDOR_CODE, /* bMS_VendorCode */
                    0x0,                           /* bPad */
                };

                requestHandled = 1;
                outPtr = (u8*)&microsoft_os_descriptor;
                wCount = sizeof(microsoft_os_descriptor);
            }
            else
            #endif
            {
                requestHandled = 1;
                //outPtr = (u8 *)&libstring_descriptor[SetupPacket.wValue0];
                //outPtr = libstring_descriptor[SetupPacket.wValue0];
                //outPtr = (u8*)&libstring_descriptor+SetupPacket.wValue0;
                //wCount = *outPtr;
                wCount = GetString(SetupPacket.wValue0, (const void *)&outPtr);
            }
        }

        #if !defined(__16F1459)

        else if (SetupPacket.wValue1 == DEVICE_QUALIFIER_DESCRIPTOR)
        {
            #ifdef USB_USE_DEBUG
            Serial_printf("  QUALIFIER\r\n");
            #endif

            requestHandled = 1;
            // TODO: check if this is needed if not requestHandled is not set to 1 the device will
            // stall later when the linux kernel requests this descriptor
            //outPtr = (u8*)&libconfiguration_descriptor;
            //wCount = sizeof();
        }

        else
        {
            #ifdef USB_USE_DEBUG
            Serial_printf("  UNKNOWN: 0x%ux\r\n", (u16)descriptorType);
            #endif
        }
        
        #endif
    }
}


// Process GET_STATUS
#if !defined(__16F1459)
void GetStatus(void)
{
    // Mask off the Recipient bits
    u8 recipient = SetupPacket.bmRequestType & 0x1F;
    
    #ifdef USB_USE_DEBUG
    Serial_printf("> GETSTATUS %d\r\n", recipient);
    #endif

    controlTransferBuffer[0] = 0;
    controlTransferBuffer[1] = 0;

    // See where the request goes
    if (recipient == 0x00)
    {
        // Device
        requestHandled = 1;
        // Set bits for self powered device and remote wakeup.
        if (selfPowered)
            controlTransferBuffer[0] |= 0x01;
        if (remoteWakeup)
            controlTransferBuffer[0] |= 0x02;
    }

    else if (recipient == 0x01)
    {
        // Interface
        requestHandled = 1;
    }

    else if (recipient == 0x02)
    {
        // Endpoint
        u8 endpointNum = SetupPacket.wIndex0 & 0x0F;
        u8 endpointDir = SetupPacket.wIndex0 & 0x80;
        requestHandled = 1;
        // Endpoint descriptors are 8 bytes long, with each in and out taking 4 bytes
        // within the endpoint. (See PIC datasheet.)
        inPtr = (u8 *)&EP_OUT_BD(0) + (endpointNum << 3); // * 8
        if (endpointDir)
            inPtr += 4;
        if (*inPtr & BDS_BSTALL)
            controlTransferBuffer[0] = 0x01;
    }

    if (requestHandled)
    {
        outPtr = (u8 *)&controlTransferBuffer;
        wCount = 2;
    }
}

// Process SET_FEATURE and CLEAR_FEATURE
void SetFeature(void)
{
    u8 recipient = SetupPacket.bmRequestType & 0x1F;
    u8 feature = SetupPacket.wValue0;

    #ifdef USB_USE_DEBUG
    Serial_printf("> SETFEATURE\r\n");
    #endif

    if (recipient == 0x00)
    {
        // Device
        if (feature == DEVICE_REMOTE_WAKEUP)
        {
            requestHandled = 1;
            if (SetupPacket.bRequest == SET_FEATURE)
                remoteWakeup = 1;
            else
                remoteWakeup = 0;
        }
        // TBD: Handle TEST_MODE
    }

    else if (recipient == 0x02)
    {
        // Endpoint
        u8 endpointNum = SetupPacket.wIndex0 & 0x0F;
        u8 endpointDir = SetupPacket.wIndex0 & 0x80;
        if ((feature == ENDPOINT_HALT) && (endpointNum != 0))
        {
            // Halt endpoint (as long as it isn't endpoint 0)
            requestHandled = 1;
            // Endpoint descriptors are 8 bytes long, with each in and out taking 4 bytes
            // within the endpoint. (See PIC datasheet.)
            inPtr = (u8 *)&EP_OUT_BD(0) + (endpointNum << 3); // * 8
            if (endpointDir)
                inPtr += 4;

            if(SetupPacket.bRequest == SET_FEATURE)
                *inPtr = 0x84;
            else
            {
                if(endpointDir == 1)
                    *inPtr = 0x00;
                else
                    *inPtr = 0x88;
            }
        }
    }
}
#endif

void ProcessStandardRequest(void)
{
    u8 request = SetupPacket.bRequest;

    #ifdef USB_USE_DEBUG
    Serial_printf("StandardRequest\r\n");
    #endif

    // Not a standard request - don't process here.  Class or Vendor
    // requests have to be handled seperately.
    if((SetupPacket.bmRequestType & 0x60) != 0x00)
    {
        #ifdef USB_USE_DEBUG
        Serial_printf("> NONE\r\n");
        #endif
        return;
    }
    
    #ifdef AUTOMATIC_WINUSB_SUPPORT
    if (request == MICROSOFT_OS_DESC_VENDOR_CODE)
    {
        requestHandled = 1;

        if (SetupPacket.bmRequestType == 0xC0 && SetupPacket.wIndex1 == 0x00 && SetupPacket.wIndex0 == 0x04)
            wCount = m_stack_winusb_get_microsoft_compat(SetupPacket.wValue0, &outPtr);

        else if (SetupPacket.bmRequestType == 0xC1 && SetupPacket.wIndex1 == 0x00 && SetupPacket.wIndex0 == 0x05)
            wCount = m_stack_winusb_get_microsoft_property(SetupPacket.wValue0, &outPtr);
    }
    #endif

    // Set the address of the device.  All future requests
    // will come to that address.  Can't actually set UADDR
    // to the new address yet because the rest of the SET_ADDRESS
    // transaction uses address 0.
    if (request == SET_ADDRESS)
    {
        #ifdef USB_USE_DEBUG
        Serial_printf("> SET_ADDRESS: %uhx\r\n", SetupPacket.wValue0);
        #endif
        requestHandled = 1;
        deviceState = ADDRESS;
        deviceAddress = SetupPacket.wValue0;
    }

    else if (request == GET_DESCRIPTOR)
    {
        GetDescriptor();
    }

    else if (request == SET_CONFIGURATION)
    {
        #ifdef USB_USE_DEBUG
        Serial_printf("> SET_CONFIGURATION\r\n");
        #endif
        requestHandled = 1;
        currentConfiguration = SetupPacket.wValue0;
        // TBD: ensure the new configuration value is one that
        // exists in the descriptor.

        // If configuration value is zero, device is put in
        // address state (USB 2.0 - 9.4.7)
        if (currentConfiguration == 0)
            deviceState = ADDRESS;
        else
        {
            // Set the configuration.
              deviceState = CONFIGURED;

            // Initialize the endpoints for all interfaces
            #ifdef USB_USE_HID
              HIDInitEndpoint();
            #endif
            #ifdef USB_USE_UART
              UARTInitEndpoint();
            #endif
            #ifdef USB_USE_CDC
              CDCInitEndpoint();
            #endif      
            #ifdef USB_USE_BULK
              BULKInitEndpoint();
            #endif   

            // TBD: Add initialization code here for any additional
            // interfaces beyond the one used for the HID
        }
    }
    
    else if (request == GET_CONFIGURATION)
    {
        #ifdef USB_USE_DEBUG
        Serial_printf("> GET_CONFIGURATION\r\n");
        #endif
        requestHandled = 1;
        outPtr = (u8*)&currentConfiguration;
        wCount = 1;
    }
    
    else if (request == GET_INTERFACE)
    {
        // No support for alternate interfaces.  Send
        // zero back to the host.
        #ifdef USB_USE_DEBUG
        Serial_printf("> GET_INTERFACE\r\n");
        #endif
        requestHandled = 1;
        controlTransferBuffer[0] = 0;
        outPtr = (u8 *)&controlTransferBuffer;
        wCount = 1;
    }

    #if !defined(__16F1459)

    else if (request == GET_STATUS)
    {
        GetStatus();
    }
    
    else if ((request == CLEAR_FEATURE) || (request == SET_FEATURE))
    {
        SetFeature();
    }
    
    else if (request == SET_INTERFACE)
    {
        // No support for alternate interfaces - just ignore.
        #ifdef USB_USE_DEBUG
        Serial_printf("> SET_INTERFACE\r\n");
        #endif
        requestHandled = 1;
    }
    
    else if (request == SET_DESCRIPTOR)
    {
        #ifdef USB_USE_DEBUG
        Serial_printf("> SET_DESCRIPTOR\r\n");
        #endif
        return;
    }
    
    else if (request == SYNCH_FRAME)
    {
        #ifdef USB_USE_DEBUG
        Serial_printf("> SYNCH_FRAME\r\n");
        #endif
        return;
    }
    
    else
    {
        #ifdef USB_USE_DEBUG
        Serial_printf("> Default Std Request\r\n");
        #endif
        return;
    }
    
    #endif
}

/**
    Data stage for a Control Transfer that sends data to the host
**/
void InDataStage(void) //unsigned char ep)
{
    u16 bufferSize;

    #ifdef USB_USE_DEBUG
    Serial_printf("InDataStage\r\n");
    #endif
    
    // Determine how many bytes are going to the host
    if (wCount < EP0_BUFFER_SIZE)
        bufferSize = wCount;
    else
        bufferSize = EP0_BUFFER_SIZE;

    // Load the high two bits of the u8 count into BC8:BC9
    // Clear BC8 and BC9
    EP_IN_BD(0).Stat.uc &= ~(BDS_BC8 | BDS_BC9);
    EP_IN_BD(0).Stat.uc |= (u8)((bufferSize & 0x0300) >> 8);
    EP_IN_BD(0).Cnt = (u8)(bufferSize & 0xFF);
    EP_IN_BD(0).ADDR = PTR16(&controlTransferBuffer);

    // Update the number of bytes that still need to be sent.  Getting
    // all the data back to the host can take multiple transactions, so
    // we need to track how far along we are.
    wCount = wCount - bufferSize;

    // Move data to the USB output buffer from wherever it sits now.
    inPtr = (u8*)&controlTransferBuffer;

    //for (i=0; i<bufferSize; i++)
    while (bufferSize--)
        *inPtr++ = *outPtr++;
}

/**
    Data stage for a Control Transfer that reads data from the host
**/

void OutDataStage(unsigned char ep)
{
    u16 bufferSize;

    #ifdef USB_USE_DEBUG
    Serial_printf("OutDataStage\r\n");
    #endif
    
    bufferSize = ((0x03 & EP_OUT_BD(ep).Stat.uc) << 8) | EP_OUT_BD(ep).Cnt;

    // Accumulate total number of bytes read
    wCount = wCount + bufferSize;

    outPtr = (u8*)&controlTransferBuffer;

    //for (i=0; i<bufferSize; i++)
    while (bufferSize--)
        *inPtr++ = *outPtr++;
}

/**
    Process the Setup stage of a control transfer.  This code initializes the
    flags that let the firmware know what to do during subsequent stages of
    the transfer.
    TODO:
    Only Ep0 is handled here.
**/

void SetupStage(void)
{
    #ifdef USB_USE_DEBUG
    //Serial_printf("SetupStage\r\n");
    #endif

    // Note: Microchip says to turn off the UOWN bit on the IN direction as
    // soon as possible after detecting that a SETUP has been received.
    EP_IN_BD(0).Stat.uc &= ~BDS_UOWN;
    EP_OUT_BD(0).Stat.uc &= ~BDS_UOWN;

    // Initialize the transfer process
    ctrlTransferStage = SETUP_STAGE;
    requestHandled = 0;                   // Default is that request hasn't been handled
    #ifdef USB_USE_HID
    HIDPostProcess = 0;                   // Assume standard request until know otherwise
    #endif
    wCount = 0;                           // No bytes transferred

    // See if this is a standard (as definded in USB chapter 9) request
    ProcessStandardRequest();

    // only Process CDC or HID if recipient is an interface
    // See if the HID class can do something with it.
    #ifdef USB_USE_HID
    if ((SetupPacket.bmRequestType & USB_RECIP_MASK) == USB_RECIP_INTERFACE)
        ProcessHIDRequest();
    #endif 

    #ifdef USB_USE_CDC
    if ((SetupPacket.bmRequestType & USB_RECIP_MASK) == USB_RECIP_INTERFACE)  
        ProcessCDCRequest();
    #endif

    // TBD: Add handlers for any other classes/interfaces in the device
    if (!requestHandled)
    {
        // If this service wasn't handled then stall endpoint 0
        EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
        EP_OUT_BD(0).ADDR = PTR16(&SetupPacket);
        EP_OUT_BD(0).Stat.uc = BDS_UOWN | BDS_BSTALL;
        EP_IN_BD(0).Stat.uc = BDS_UOWN | BDS_BSTALL;
    }

    else if (SetupPacket.bmRequestType & 0x80)
    {
        // Device-to-host
        if(SetupPacket.wLength < wCount)
            wCount = SetupPacket.wLength;
        InDataStage();
        ctrlTransferStage = DATA_IN_STAGE;
        // Reset the out buffer descriptor for endpoint 0
        EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
        EP_OUT_BD(0).ADDR = PTR16(&SetupPacket);
        EP_OUT_BD(0).Stat.uc = BDS_UOWN;

        // Set the in buffer descriptor on endpoint 0 to send data
        EP_IN_BD(0).ADDR = PTR16(&controlTransferBuffer);
        // Give to SIE, DATA1 packet, enable data toggle checks
        EP_IN_BD(0).Stat.uc = BDS_UOWN | BDS_DTS | BDS_DTSEN;
    }

    else
    {
        // Host-to-device
        ctrlTransferStage = DATA_OUT_STAGE;

        // Clear the input buffer descriptor
        EP_IN_BD(0).Cnt = 0;
        EP_IN_BD(0).Stat.uc = BDS_UOWN | BDS_DTS | BDS_DTSEN;

        // Set the out buffer descriptor on endpoint 0 to receive data
        EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
        EP_OUT_BD(0).ADDR = PTR16(&controlTransferBuffer);
        // Give to SIE, DATA1 packet, enable data toggle checks
        EP_OUT_BD(0).Stat.uc = BDS_UOWN | BDS_DTS | BDS_DTSEN;
    }

    // Enable SIE token and packet processing
    UCONbits.PKTDIS = 0;
}


  // Configures the buffer descriptor for endpoint 0 so that it is waiting for
  // the status stage of a control transfer.
void WaitForSetupStage(void)
{
    #ifdef USB_USE_DEBUG
    //Serial_printf("WaitForSetupStage\r\n");
    #endif

    ctrlTransferStage = SETUP_STAGE;
    EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
    EP_OUT_BD(0).ADDR = PTR16(&SetupPacket);
    // Give EP OUT control to SIE, enable data toggle checks
    EP_OUT_BD(0).Stat.uc = BDS_UOWN | BDS_DTSEN;
    // Give EP IN control to CPU
    EP_IN_BD(0).Stat.uc  = BDS_COWN;
}


  // This is the starting point for processing a Control Transfer.  The code directly
  // follows the sequence of transactions described in the USB spec chapter 5.  The
  // only Control Pipe in this firmware is the Default Control Pipe (endpoint 0).
  // Control messages that have a different destination will be discarded.
void ProcessControlTransfer(void)
{
    #ifdef USB_USE_DEBUG
    //Serial_printf("CtrlTransfert\n\r");
    #endif

    if (USTATbits.DIR == OUT)
    {
        // Endpoint 0:out
        // Pull PID from middle of BD0STAT
        u8 PID = (EP_OUT_BD(0).Stat.uc & 0x3C) >> 2;

        if (PID == 0x0D)
        {
            // SETUP PID - a transaction is starting
            SetupStage();
        }
        
        else if (ctrlTransferStage == DATA_OUT_STAGE)
        {
            // Complete the data stage so that all information has
            // passed from host to device before servicing it.
            OutDataStage(0);

            #ifdef USB_USE_HID
            if (HIDPostProcess)
            {
                // Determine which report is being set.
                u8 reportID = SetupPacket.wValue0;

                // Find out if an Output or Feature report has arrived on the control pipe.
                // Get the report type from the Setup packet.
                if (SetupPacket.wValue1 == 0x02)
                {
                    // Output report
                    SetOutputReport(reportID);
                }
                else if (SetupPacket.wValue1 == 0x03)
                {
                    // Feature report
                    SetFeatureReport(reportID);
                }
                else
                {
                    // Unknown report type
                }
            }
            #endif

            // Turn control over to the SIE and toggle the data bit
            if(EP_OUT_BD(0).Stat.DTS)
                EP_OUT_BD(0).Stat.uc = BDS_UOWN | BDS_DTSEN;
            else
                EP_OUT_BD(0).Stat.uc = BDS_UOWN | BDS_DTS | BDS_DTSEN;
        }
        else
        {
            // Prepare for the Setup stage of a control transfer
            WaitForSetupStage();
        }
    }
    else if (USTATbits.DIR == IN)
    {
        // Endpoint 0:in
        if ((UADDR == 0) && (deviceState == ADDRESS))
        {
            // TBD: ensure that the new address matches the value of
            // "deviceAddress" (which came in through a SET_ADDRESS).
            UADDR = SetupPacket.wValue0;
            #ifdef USB_USE_DEBUG
            Serial_printf("UADDR = 0x%x\r\n", (u16)UADDR);
            #endif
            if(UADDR == 0)
                // If we get a reset after a SET_ADDRESS, then we need
                // to drop back to the Default state.
                deviceState = DEFAULT_STATUS;
        }

        if (ctrlTransferStage == DATA_IN_STAGE)
        {
            // Start (or continue) transmitting data
            InDataStage();

            // Turn control over to the SIE and toggle the data bit
            if(EP_IN_BD(0).Stat.DTS)
                EP_IN_BD(0).Stat.uc = BDS_UOWN | BDS_DTSEN;
            else
                EP_IN_BD(0).Stat.uc = BDS_UOWN | BDS_DTSEN | BDS_DTS ;
        }
        else
        {
            // Prepare for the Setup stage of a control transfer
            WaitForSetupStage();
        }
    }
    else
    {
        #ifdef USB_USE_DEBUG
        Serial_printf("IN on EP %d\r\n", (USTAT >> 3) & 0x0f);
        Serial_printf("USTAT = 0x%uhx\r\n", USTAT);
        #endif
    }
}

void EnableUSBModule(void)
{
    if(UCONbits.USBEN == 0)
    {
        #ifdef USB_USE_DEBUG
        //Serial_printf("USB Enable\r\n");
        #endif
        // Reset the USB module
        UCON = 0;
        // Disable all USB interrupt
        UIE = 0;
        // Enable the USB module
        UCONbits.USBEN = 1;
        
        deviceState = ATTACHED;
    }

    // If we are attached and no single-ended zero is detected, then
    // we can move to the Powered state.
    if ((deviceState == ATTACHED) && !UCONbits.SE0)
    {
        UIR = 0;
        UIE = 0;
        // Enable Reset and Idle interrupt
        UIEbits.URSTIE = 1;
        UIEbits.IDLEIE = 1;
        deviceState = POWERED;
        #ifdef USB_USE_DEBUG
        //Serial_printf("Device powered\r\n");
        #endif
    }
}


  // Unsuspend the device
#if 0
void UnSuspend(void)
{
    #ifdef USB_USE_DEBUG
    Serial_printf("UnSuspend\r\n");
    #endif

    UCONbits.SUSPND = 0;
    UIEbits.ACTVIE = 0;
    UIRbits.ACTVIF = 0;
}
#endif

  // Full speed devices get a Start Of Frame (SOF) packet every 1 millisecond.
  // Nothing is currently done with this interrupt (it is simply masked out).
#if 0
void StartOfFrame(void)
{
    #ifdef USB_USE_DEBUG
    Serial_printf("StatOfFrame\r\n");
    #endif
    // TBD: Add a callback routine to do something
    UIRbits.SOFIF = 0;
}
#endif

  // This routine is called in response to the code stalling an endpoint.
void Stall(void)
{
    #ifdef USB_USE_DEBUG
    Serial_printf("Stall\r\n");
    #endif
    if (UEP0bits.EPSTALL == 1)
    {
        // Prepare for the Setup stage of a control transfer
        WaitForSetupStage();
        UEP0bits.EPSTALL = 0;
    }
    UIRbits.STALLIF = 0;
}


// Suspend all processing until we detect activity on the USB bus
#if ALLOW_SUSPEND == 1
void Suspend(void)
{
        #ifdef USB_USE_DEBUG
        Serial_printf("Suspend\r\n");
        #endif

        UIEbits.ACTVIE = 1;
        UIRbits.IDLEIF = 0;
        UCONbits.SUSPND = 1;

        #if defined(__18f25k50) || defined(__18f45k50)
        PIR3bits.USBIF = 0;
        PIE3bits.USBIE = 1;
        #else
        PIR2bits.USBIF = 0;
        PIE2bits.USBIE = 1;
        #endif
        
        // Why ???
        #if 0
        #if defined(__16F1459) || defined(__18f25k50) || defined(__18f45k50)
        INTCONbits.IOCIF = 0;
        INTCONbits.IOCIE = 1;
        #else
        INTCONbits.RBIF = 0;
        INTCONbits.RBIE = 1;
        #endif
        #endif
        
        // disable the USART
        #ifdef USB_USE_DEBUG
            #if defined(__18f25k50) || defined(__18f45k50)
            RCSTA1bits.CREN = 0;
            TXSTA1bits.TXEN = 0;
            #else
            RCSTAbits.CREN = 0;
            TXSTAbits.TXEN = 0;
            #endif
        #endif

        __asm__("SLEEP");

        // enable the USART
        #ifdef USB_USE_DEBUG
            #if defined(__18f25k50) || defined(__18f45k50)
            RCSTA1bits.CREN = 1;
            TXSTA1bits.TXEN = 1;
            #else
            RCSTAbits.CREN = 1;
            TXSTAbits.TXEN = 1;
            #endif
        #endif

        #if defined(__18f25k50) || defined(__18f45k50)
        PIE3bits.USBIE = 0;
        #else
        PIE2bits.USBIE = 0;
        #endif
        
        // Why ???
        #if 0
        #if defined(__16F1459) || defined(__18f25k50) || defined(__18f45k50)
        INTCONbits.IOCIE = 0;
        #else
        INTCONbits.RBIE = 0;
        #endif
        #endif
}
#endif


void BusReset(void)
{
    #ifdef USB_USE_DEBUG
    //Serial_printf("Reset\r\n");
    #endif
    
    UEIR  = 0x00;
    UIR   = 0x00;
    UEIE  = 0x9f;
    UIE   = 0x7b;
    UADDR = 0x00;

    // Set endpoint 0 as a control pipe
    UEP0 = EP_CTRL | HSHK_EN;

    // Flush any pending transactions
    //if (UIRbits.TRNIF == 1)
    while (UIRbits.TRNIF == 1)
        UIRbits.TRNIF = 0;

    // Enable packet processing
    UCONbits.PKTDIS = 0;

    // Prepare for the Setup stage of a control transfer
    WaitForSetupStage();

    remoteWakeup = 0;                     // Remote wakeup is off by default
    selfPowered = 0;                      // Self powered is off by default
    currentConfiguration = 0;             // Clear active configuration
    deviceState = DEFAULT_STATUS;
}


// Main entry point for USB tasks.  Checks interrupts, then checks for transactions.
void ProcessUSBTransactions(void)
{
    // See if the device is connected yet.
    if (deviceState == DETACHED)
        return;

    // If the USB became active then wake up from suspend
    if (UIRbits.ACTVIF && UIEbits.ACTVIE)
    {
        #ifdef USB_USE_DEBUG
        Serial_printf("Unsuspend\r\n");
        #endif
    
        //UnSuspend();
        UCONbits.SUSPND = 0;
        UIEbits.ACTVIE = 0;
        UIRbits.ACTVIF = 0;
    }
    
    // If we are supposed to be suspended, then don't try performing any
    // processing.
    if (UCONbits.SUSPND == 1)
        return;

    // Process a bus reset
    if (UIRbits.URSTIF && UIEbits.URSTIE)
        BusReset();

    // No bus activity for a while - suspend the firmware
    #if ALLOW_SUSPEND == 1    // cf. picUSB.c 
    if (UIRbits.IDLEIF && UIEbits.IDLEIE)
        Suspend();
    #endif
    
    if (UIRbits.SOFIF && UIEbits.SOFIE)
    {
        #ifdef USB_USE_DEBUG
        //Serial_printf("StatOfFrame\r\n");
        #endif
        // StartOfFrame();
        UIRbits.SOFIF = 0;
    }
    
    if (UIRbits.STALLIF && UIEbits.STALLIE)
        Stall();

    // TBD: See where the error came from.
    // Clear errors
    if (UIRbits.UERRIF && UIEbits.UERRIE)
        UIRbits.UERRIF = 0;

    // Unless we have been reset by the host, no need to keep processing
    if (deviceState < DEFAULT_STATUS)
        return;

    // A transaction has finished.  Try default processing on endpoint 0.
    if(UIRbits.TRNIF && UIEbits.TRNIE)
    {
        ProcessControlTransfer();
        UIRbits.TRNIF = 0;
    }
}

#endif // PICUSB_C
