/***********************************************************************
* 
* Title         : Microchip ENC28J60 Ethernet Interface Driver
* Author        : Pascal Stang (c)2005
* Modified by   : Guido Socher
*                 nuelectronics.com (Ethershield for Arduino)
*                 Jean-Pierre Mandon (Pinguino32 - 2010)
*                 André Gentric (Pinguino8 - May 2014)

* Copyright: GPL V2
*
* This driver provides initialization and transmit/receive functions
* for the Microchip ENC28J60 10Mb Ethernet Controller and PHY.
* This chip is novel in that it is a full MAC+PHY interface all in a
* 28-pin chip, using an SPI interface to the host processor.
***********************************************************************/
#ifndef ENC28J60_C
#define ENC28J60_C

#include <typedef.h>
#include <macro.h>
#include <ethernet/enc28j60p.h>
#include <spi.c>
#if defined(__PIC32MX__)
#include <delay.c>
#else
#include <delayms.c>
#include <delayus.c>
#endif

static u8  gENC28J60CurrentBank;
static u16 gENC28J60NextPacketPtr;

void ENC28J60Init(u8 bSpi, u8* macaddr)
{
    SPI_deselect(bSpi);
    // init SPI module
    SPI_setMode(bSpi, SPI_MASTER);
    // ENC28J60 supports SPI mode 0,0 only (datasheet p25)
    SPI_setDataMode(bSpi, SPI_MODE0);
    // ENC28J60 supports SPI clock speeds up to 20 MHz
    // but not under 8 MHz (Rev. B4 Silicon Errata).
    // P8 : maximum baud rate possible = FPB = FOSC/4 = 12 MHz
    // P32: maximum baud rate possible = FPB/2 = 40 MHz
    #if defined(__PIC32MX__)
    SPI_setClockDivider(bSpi, SPI_PBCLOCK_DIV4);
    #else
    SPI_setClockDivider(bSpi, SPI_CLOCK_DIV4);
    #endif
    SPI_begin(bSpi, NULL);
    //SPI_begin(bSpi);

    // Perform a system reset
    ENC28J60WriteOp(bSpi, ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
    //ENC28J60WriteOp(bSpi, ENC28J60_SOFT_RESET, ENC28J60_SOFT_RESET, 0);
    //SPI_select(bSpi);
    //SPI_write(bSpi, ENC28J60_SOFT_RESET);
    //SPI_deselect(bSpi);
    
    // Check CLKRDY bit to see if reset is complete
    // The CLKRDY does not work. See Rev. B4 Silicon Errata point.
    // Work around : wait for at least 1 ms for the device to be ready
    //while(!(ENC28J60Read(ESTAT) & ESTAT_CLKRDY));
    Delayms(50);

    // Do bank 0 stuff
    // - initialize receive buffer
    // - 16-bit transfers, must write low byte first
    // - set receive buffer start address
    gENC28J60NextPacketPtr = RXSTART_INIT;

    // Rx start
    ENC28J60Write(bSpi, ERXSTL, low8(RXSTART_INIT));
    ENC28J60Write(bSpi, ERXSTH, high8(RXSTART_INIT));

    // set receive pointer address
    ENC28J60Write(bSpi, ERXRDPTL, low8(RXSTART_INIT));
    ENC28J60Write(bSpi, ERXRDPTH, high8(RXSTART_INIT));

    // RX end
    ENC28J60Write(bSpi, ERXNDL, low8(RXSTOP_INIT));
    ENC28J60Write(bSpi, ERXNDH, high8(RXSTOP_INIT));

    // TX start
    ENC28J60Write(bSpi, ETXSTL, low8(TXSTART_INIT));
    ENC28J60Write(bSpi, ETXSTH, high8(TXSTART_INIT));

    // TX end
    ENC28J60Write(bSpi, ETXNDL, low8(TXSTOP_INIT));
    ENC28J60Write(bSpi, ETXNDH, high8(TXSTOP_INIT));

    // Do bank 1 stuff
    // For broadcast packets we allow only ARP packtets
    // All other packets should be unicast only for our mac (MAADR)
    //
    // The pattern to match on is therefore
    // Type     ETH.DST
    // ARP      BROADCAST
    // 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
    // in binary these poitions are:11 0000 0011 1111
    // This is hex 303F->EPMM0=0x3f,EPMM1=0x30
    //ENC28J60Write(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
    ENC28J60Write(bSpi, ERXFCON, ERXFCON_CRCEN);
    ENC28J60Write(bSpi, EPMM0,   0x3f);
    ENC28J60Write(bSpi, EPMM1,   0x30);
    ENC28J60Write(bSpi, EPMCSL,  0xf9);
    ENC28J60Write(bSpi, EPMCSH,  0xf7);

    // Do bank 2 stuff
    // enable MAC receive
    ENC28J60Write(bSpi, MACON1,  MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
    // bring MAC out of reset
    ENC28J60Write(bSpi, MACON2,  0x00);
    // enable automatic padding to 60bytes and CRC operations
    ENC28J60WriteOp(bSpi, ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
    // set inter-frame gap (non-back-to-back)
    ENC28J60Write(bSpi, MAIPGL,  0x12);
    ENC28J60Write(bSpi, MAIPGH,  0x0C);
    // set inter-frame gap (back-to-back)
    ENC28J60Write(bSpi, MABBIPG, 0x12);
    // Set the maximum packet size which the controller will accept
    // Do not send packets longer than MAX_FRAMELEN:
    ENC28J60Write(bSpi, MAMXFLL, low8(MAX_FRAMELEN));
    ENC28J60Write(bSpi, MAMXFLH, high8(MAX_FRAMELEN));

    // Do bank 3 stuff
    // write MAC address
    // NOTE: MAC address in ENC28J60 is byte-backward
    ENC28J60Write(bSpi, MAADR5,  macaddr[0]);
    ENC28J60Write(bSpi, MAADR4,  macaddr[1]);
    ENC28J60Write(bSpi, MAADR3,  macaddr[2]);
    ENC28J60Write(bSpi, MAADR2,  macaddr[3]);
    ENC28J60Write(bSpi, MAADR1,  macaddr[4]);
    ENC28J60Write(bSpi, MAADR0,  macaddr[5]);

    // no loopback of transmitted frames
    ENC28J60PhyWrite(bSpi, PHCON2, PHCON2_HDLDIS);
    // switch to bank 0
    ENC28J60SetBank(bSpi, ECON1);
    // enable interrutps
    ENC28J60WriteOp(bSpi, ENC28J60_BIT_FIELD_SET, EIE, 0);
    //ENC28J60WriteOp(bSpi, ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
    // enable packet reception
    ENC28J60WriteOp(bSpi, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

u8 ENC28J60ReadOp(u8 bSpi, u8 bOp, u8 bAddr)
{
    u8 received_byte;

    SPI_select(bSpi);
    // issue read command
    SPI_write(bSpi, bOp | (bAddr & ADDR_MASK));
    // read data
    received_byte = SPI_read(bSpi);
    // do dummy read if needed (for mac and mii, see datasheet page 29)
    if (bAddr & 0x80)
        received_byte = SPI_read(bSpi);
    SPI_deselect(bSpi);

    return (received_byte);
}

void ENC28J60WriteOp(u8 bSpi, u8 bOp, u8 bAddr, u8 bData)
{
    SPI_select(bSpi);
    SPI_write(bSpi, bOp | (bAddr & ADDR_MASK));
    SPI_write(bSpi, bData);
    SPI_deselect(bSpi);
}

void ENC28J60ReadBuffer(u8 bSpi, u16 wLen, u8* buffer)
{
    u8 received_byte;
    
    SPI_select(bSpi);
    // issue read command
    SPI_write(bSpi, ENC28J60_READ_BUF_MEM);
    while(wLen)
    {
        wLen--;
        // read data
        received_byte = SPI_read(bSpi);
        *buffer = received_byte;
        buffer++;
    }
    *buffer='\0';
    SPI_deselect(bSpi);
}

void ENC28J60WriteBuffer(u8 bSpi, u16 wLen, u8* buffer)
{
    SPI_select(bSpi);
    // issue write command
    SPI_write(bSpi, ENC28J60_WRITE_BUF_MEM);
    while(wLen)
    {
        wLen--;
        // write data
        SPI_write(bSpi, *buffer);
        buffer++;
    }
    SPI_deselect(bSpi);
}

void ENC28J60SetBank(u8 bSpi, u8 bAddr)
{
    // set the bank only if needed
    if ((bAddr & BANK_MASK) != gENC28J60CurrentBank)
    {
        // set the bank
        ENC28J60WriteOp(bSpi, ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
        ENC28J60WriteOp(bSpi, ENC28J60_BIT_FIELD_SET, ECON1, (bAddr & BANK_MASK)>>5);
        gENC28J60CurrentBank = (bAddr & BANK_MASK);
    }
}

u8 ENC28J60Read(u8 bSpi, u8 bAddr)
{
    // set the bank
    ENC28J60SetBank(bSpi, bAddr);
    // do the read
    return ENC28J60ReadOp(bSpi, ENC28J60_READ_CTRL_REG, bAddr);
}

void ENC28J60Write(u8 bSpi, u8 bAddr, u8 bData)
{
    // set the bank
    ENC28J60SetBank(bSpi, bAddr);
    // do the write
    ENC28J60WriteOp(bSpi, ENC28J60_WRITE_CTRL_REG, bAddr, bData);
}

void ENC28J60PhyWrite(u8 bSpi, u8 bAddr, u16 wData)
{
    // set the PHY register address
    ENC28J60Write(bSpi, MIREGADR, bAddr);
    // write the PHY data
    ENC28J60Write(bSpi, MIWRL,  low8(wData));
    ENC28J60Write(bSpi, MIWRH, high8(wData));
    // wait until the PHY write completes
    while(ENC28J60Read(bSpi, MISTAT) & MISTAT_BUSY)
        Delayus(15);
}

// read upper 8 bits
u8 ENC28J60PhyReadH(u8 bSpi, u8 bAddr)
{
    // Set the right address and start the register read operation
    ENC28J60Write(bSpi, MIREGADR, bAddr);
    ENC28J60Write(bSpi, MICMD, MICMD_MIIRD);
    //_delay_loop_1(40); // 10us
    Delayus(15);

    // wait until the PHY read completes
    while(ENC28J60Read(bSpi, MISTAT) & MISTAT_BUSY);

    // reset reading bit
    ENC28J60Write(bSpi, MICMD, 0x00);
    
    return (ENC28J60Read(bSpi, MIRDH));
}

// Change CLKOUT
// ECOCON: CLOCK OUTPUT CONTROL REGISTER
// 101 = CLKOUT outputs main clock divided by 8 (3.125 MHz)
// 100 = CLKOUT outputs main clock divided by 4 (6.25 MHz) = DEFAULT
// 011 = CLKOUT outputs main clock divided by 3 (8.333333 MHz)
// 010 = CLKOUT outputs main clock divided by 2 (12.5 MHz)
// 001 = CLKOUT outputs main clock divided by 1 (25 MHz)
// 000 = CLKOUT is disabled. The pin is driven low.
void ENC28J60clkout(u8 bSpi, u8 bDiv)
{
    ENC28J60Write(bSpi, ECOCON, bDiv & 0x7);
}

// read the revision of the chip:
u8 ENC28J60getrev(u8 bSpi)
{
    return (ENC28J60Read(bSpi, EREVID));
}

// link status
u8 ENC28J60linkup(u8 bSpi)
{
    // bit 10 (= bit 3 in upper reg)
    return (ENC28J60PhyReadH(bSpi, PHSTAT2) && 4);
}

// just probe if there might be a packet
u8 ENC28J60hasRxPkt(u8 bSpi)
{
    if (ENC28J60Read(bSpi, EPKTCNT)==0)
        return (0);
    else
        return (1);
}

void ENC28J60PacketSend(u8 bSpi, u16 wLen, u8* packet)
{
    // Check no transmit in progress
    while (ENC28J60ReadOp(bSpi, ENC28J60_READ_CTRL_REG, ECON1) & ECON1_TXRTS)
    {
        // Reset the transmit logic problem.
        // See Rev. B4 Silicon Errata point 12.
        //if( (ENC28J60Read(EIR) & EIR_TXERIF) ) 
        {
            ENC28J60WriteOp(bSpi, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
            ENC28J60WriteOp(bSpi, ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
        }
    }

    // Set the write pointer to start of transmit buffer area
    ENC28J60Write(bSpi, EWRPTL,  low8(TXSTART_INIT));
    ENC28J60Write(bSpi, EWRPTH, high8(TXSTART_INIT));
    // Set the TXND pointer to correspond to the packet size given
    ENC28J60Write(bSpi, ETXNDL,  low8(TXSTART_INIT + wLen));
    ENC28J60Write(bSpi, ETXNDH, high8(TXSTART_INIT + wLen));
    // write per-packet control byte (0x00 means use macon3 settings)
    ENC28J60WriteOp(bSpi, ENC28J60_WRITE_BUF_MEM, 0, 0x00);
    // copy the packet into the transmit buffer
    ENC28J60WriteBuffer(bSpi, wLen, packet);
    // send the contents of the transmit buffer onto the network
    ENC28J60WriteOp(bSpi, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

// Gets a packet from the network receive buffer, if one is available.
// The packet will by headed by an ethernet header.
// maxlen : The maximum acceptable length of a retrieved packet.
// packet : Pointer where packet data should be stored.
// Returns: Packet length in bytes if a packet was retrieved, zero otherwise.
u16 ENC28J60PacketReceive(u8 bSpi, u16 maxlen, u8* packet)
{
    u16 rxstat;
    u16 wLen;

    // check if a packet has been received and buffered
    //if( !(ENC28J60Read(EIR) & EIR_PKTIF) ){
    // The above does not work. See Rev. B4 Silicon Errata point 6.
    if (ENC28J60Read(bSpi, EPKTCNT) == 0)
        return(0);

    // Set the read pointer to the start of the received packet
    ENC28J60Write(bSpi, ERDPTL,  low8(gENC28J60NextPacketPtr));
    ENC28J60Write(bSpi, ERDPTH, high8(gENC28J60NextPacketPtr));

    // read the next packet pointer
    gENC28J60NextPacketPtr  = ENC28J60ReadOp(bSpi, ENC28J60_READ_BUF_MEM, 0);
    gENC28J60NextPacketPtr |= ENC28J60ReadOp(bSpi, ENC28J60_READ_BUF_MEM, 0)<<8;

    // read the packet length (see datasheet page 43)
    wLen  = ENC28J60ReadOp(bSpi, ENC28J60_READ_BUF_MEM, 0);
    wLen |= ENC28J60ReadOp(bSpi, ENC28J60_READ_BUF_MEM, 0)<<8;
    wLen-=4; //remove the CRC count

    // read the receive status (see datasheet page 43)
    rxstat  = ENC28J60ReadOp(bSpi, ENC28J60_READ_BUF_MEM, 0);
    rxstat |= ((u16)ENC28J60ReadOp(bSpi, ENC28J60_READ_BUF_MEM, 0))<<8;

    // limit retrieve length
    if (wLen > maxlen-1)
        wLen = maxlen-1;

    // check CRC and symbol errors (see datasheet page 44, table 7-3):
    // The ERXFCON.CRCEN is set by default. Normally we should not
    // need to check this.
    // invalid
    if ((rxstat & 0x80)==0)
        wLen = 0;

    // copy the packet from the receive buffer
    else
        ENC28J60ReadBuffer(bSpi, wLen, packet);

    // Move the RX read pointer to the start of the next received packet
    // This frees the memory we just read out
    ENC28J60Write(bSpi, ERXRDPTL,  low8(gENC28J60NextPacketPtr));
    ENC28J60Write(bSpi, ERXRDPTH, high8(gENC28J60NextPacketPtr));

    // Move the RX read pointer to the start of the next received packet
    // This frees the memory we just read out.
    // However, compensate for the errata point 13, rev B4: enver write an even address!
    if ((gENC28J60NextPacketPtr - 1 < RXSTART_INIT) || (gENC28J60NextPacketPtr -1 > RXSTOP_INIT))
    {
        ENC28J60Write(bSpi, ERXRDPTL,  low8(RXSTOP_INIT));
        ENC28J60Write(bSpi, ERXRDPTH, high8(RXSTOP_INIT));
    }
    else
    {
        ENC28J60Write(bSpi, ERXRDPTL,  low8(gENC28J60NextPacketPtr-1));
        ENC28J60Write(bSpi, ERXRDPTH, high8(gENC28J60NextPacketPtr-1));
    }

    // decrement the packet counter indicate we are done with this packet
    ENC28J60WriteOp(bSpi, ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);

    return(wLen);
}

#endif // ENC28J60_C
