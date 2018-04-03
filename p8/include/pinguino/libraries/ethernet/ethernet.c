/******************************************************************
 * Author: Andre Gentric
 * inspired from Guido Socher's work (GPL V2)
 * adapted to Pinguino Project
 *****************************************************************/
#ifndef ETHERNET_C
#define ETHERNET_C

#include <ethernet/enc28j60p.c>
#include <ethernet/ip_arp_udp_tcp.c>
#include <typedef.h>
#include <string.h>
#if defined(__PIC32MX__)
#include <delay.c>
#else
#include <delayms.c>
#endif

#define BUFFER_SIZE 500
#define STR_BUFFER_SIZE 32

static u8 buf[BUFFER_SIZE+1];
static u8 strbuf[STR_BUFFER_SIZE+1];
static u16 plen;
static u16 _port;

void eth_print(const char*);
void eth_printNumber(int);
void eth_respond(u8);

void eth_init(u8 myspi, u8 *mymac, u8 *myip, u16 myport)
{
    u8 i;
    
    _port = myport;
    
    // Initialize ENC28J60
    ENC28J60Init(myspi, mymac);
    ENC28J60clkout(myspi, 2); // change clkout from 6.25MHz to 12.5MHz
    Delayms(10);

    // LEDs configuration, see ENC28J60 datasheet, page 11
    // LEDA=green LEDB=yellow
    for (i=0; i<10; i++)
    {
        // 0x880 is PHLCON LEDB=on, LEDA=on
        // 0b1000<<8 | 0b1000<<4 = 0x880
        ENC28J60PhyWrite(myspi, PHLCON, 0x880);
        Delayms(100);

        // 0x990 is PHLCON LEDB=off, LEDA=off
        // 0b1001<<8 | 0b1001<<4 = 0x990
        ENC28J60PhyWrite(myspi, PHLCON, 0x990);
        Delayms(100);
    }

    // 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit,
    //ENC28J60PhyWrite(myspi, PHLCON, 0x470);
    // Stretch LED events by TMSTRCH and 
    // Stretchable LED events will cause lengthened LED pulses based on LFRQ1:LFRQ0 configuration
    // 0b0100<<8 | 0b0111<<4 + 6 = 0x476
    ENC28J60PhyWrite(myspi, PHLCON, 0x476);
    Delayms(100);

    //init the ethernet/ip layer:
    //init_ip_arp_udp_tcp(mymac, myip, myport);
}

char* eth_serviceRequest(u8 spi)
{
    u16 dat_p;

    plen = ENC28J60PacketReceive(spi, BUFFER_SIZE, buf);

    // Is there a valid packet (without crc error) ?
    if (plen != 0)
    {
        // arp is broadcast if unknown but a host may also verify
        // the mac address by sending it to a unicast address.
        if (eth_type_is_arp_and_my_ip(buf, plen))
        {
            make_arp_answer_from_request(spi, buf);
            return 0;
        }
        
        // check if ip packets are for us:
        if (eth_type_is_ip_and_my_ip(buf, plen) == 0)
            return 0;
        
        if (buf[IP_PROTO_P] == IP_PROTO_ICMP_V && buf[ICMP_TYPE_P] == ICMP_TYPE_ECHOREQUEST_V)
        {
            make_echo_reply_from_request(spi, buf, plen);
            return 0;
        }
        
        // tcp port www start, compare only the lower byte
        if (buf[IP_PROTO_P]==IP_PROTO_TCP_V&&buf[TCP_DST_PORT_H_P]==0&&buf[TCP_DST_PORT_L_P] == _port)
        {
            if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V)
            {
                // make_tcp_synack_from_syn does already send the syn,ack
                make_tcp_synack_from_syn(spi, buf);
                return 0;     
            }
            
            if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V)
            {
                init_len_info(buf); // init some data structures
                dat_p = get_tcp_data_pointer();
                
                // we can possibly have no data, just ack:
                if (dat_p == 0)
                {
                    if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V)
                        //make_tcp_ack_from_any(spi, buf);
                        make_tcp_ack_from_any(spi, buf, 0 ,0);
                    return 0;
                }
                
                if (strncmp("GET ",(char *)&(buf[dat_p]),4) != 0)
                {
                    // head, post and other methods for possible status codes see:
                    // http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
                    plen = fill_tcp_data(buf, 0, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 OK</h1>");
                    plen = fill_tcp_data(buf, plen, "<h1>A</h1>");
                    eth_respond(spi);
                }
                
                if (strncmp("/",(char *)&(buf[dat_p+4]),1) == 0) // was "/ " and 2
                {
                    // Copy the request action before we overwrite it with the response
                    int i = 0;
                    while (buf[dat_p+5+i] != ' ' && i < STR_BUFFER_SIZE)
                    {
                        strbuf[i] = buf[dat_p+5+i];
                        i++;
                    }
                    strbuf[i] = '\0';
                    plen = fill_tcp_data(buf, 0, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");		        	
                    return (char*)strbuf;
                }
            }
        }
    }
    return 0;
}

void eth_print(const char* text)
{
    int j = 0;
    while (text[j]) 
    {
        buf[TCP_CHECKSUM_L_P + 3 + plen] = text[j++];
        plen++;
    }
}

void eth_printNumber(int number)
{
    char tempString[9]; 
    itoa(number, tempString, 10);
    eth_print(tempString);
}

void eth_respond(u8 spi)
{
    //make_tcp_ack_from_any(spi, buf); // send ack for http get
    make_tcp_ack_from_any(spi, buf, 0, 0); // send ack for http get
    make_tcp_ack_with_data_noflags(spi, buf, plen); // send data
}

#endif // ETHERNET_C
