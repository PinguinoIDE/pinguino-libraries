/*********************************************
 * Author: Guido Socher 
 * Copyright: GPL V2
 *
 * IP/ARP/UDP/TCP functions
 *
 * initially for Arduino environment
 * adapted to Pinguino Project by Andre Gentric - July 2014
 *
 *********************************************/

#ifndef IPARPUDPTCP_H
#define IPARPUDPTCP_H

#include <typedef.h>

// you must call this function once before you use any of the other functions:
void init_ip_arp_udp_tcp(u8 *mymac,u8 *myip,u8 wwwp);
//
void www_server_reply(u8 spi, u8 *buf,u16 dlen);

void init_len_info(u8 *buf);
u16 get_tcp_data_pointer(void);
u16 fill_tcp_data(u8 *buf,u16 pos, const char *s);

u8 eth_type_is_arp_and_my_ip(u8 *buf,u16 len);
u8 eth_type_is_ip_and_my_ip(u8 *buf,u16 len);

void make_arp_answer_from_request(u8 spi, u8 *buf);
void make_echo_reply_from_request(u8 spi, u8 *buf,u16 len);
void make_udp_reply_from_request(u8 spi, u8 *buf,char *data,u8 datalen,u16 port);
void make_tcp_synack_from_syn(u8 spi, u8 *buf);
void make_tcp_ack_with_data_noflags(u8 spi, u8 *buf,u16 dlen);
void make_tcp_ack_from_any(u8 spi, u8 *buf, s16 datlentoack,u8 addflags);
//void make_tcp_ack_with_data(u8 spi, u8 *buf,u16 dlen);
void make_arp_request(u8 spi, u8 *buf, u8 *server_ip);

u8 arp_packet_is_myreply_arp ( u8 *buf );
u16 tcp_get_dlength ( u8 *buf );

void tcp_client_send_packet(u8 spi, u8 *buf,u16 dest_port, u16 src_port, u8 flags, u8 max_segment_size, 
u8 clear_seqck, u16 next_ack_num, u16 dlength, u8 *dest_mac, u8 *dest_ip);


#endif // IPARPUDPTCP_H
