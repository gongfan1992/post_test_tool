/*************************************************************************
	> File Name: parse_packet.h
	> Author: likeyi
	> Mail: likeyiyy@sina.com 
	> Created Time: Wed 02 Apr 2014 04:07:40 PM CST
 ************************************************************************/

#ifndef PARSE_PACKET_H
#define PARSE_PACKET_H

void parse_iphdr(struct iphdr * iph);
void parse_tcphdr(struct tcphdr * tcph);
void parse_full_packet(void * packet);
void print_packet(unsigned char * packet,int size);    
#endif
