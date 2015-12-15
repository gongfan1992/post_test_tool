/*************************************************************************
	> File Name: checksum.c
	> Author: likeyi
	> Mail: likeyiyy@sina.com 
	> Created Time: Mon 05 May 2014 03:22:26 PM CST
 ************************************************************************/
#include "includes.h"
void checksum_ip(struct iphdr * iph)
{
    //int ihl = iph->ihl;
    uint16_t * ipheader = (uint16_t *)iph;
    int i = 0;
    uint32_t sum = 0;
    printf("iph->ihl:%d\n",iph->ihl);
    iph->check = 0;
    for(i = 0; i < iph->ihl * 2; ++i)
    {
        printf("iph[%d] = %4x\n",i,ntohs(ipheader[i]));
        sum += (ipheader[i]);
    }
    while(sum >> 16)
        sum = (sum >> 16) + (sum & 0xffff);
    iph->check = ~sum;
}
// Internet Checksum calculation
// Internet Checksum calculation
uint16_t ip_xsum (uint16_t *buff, int len, uint32_t xsum)
{
  // calculate IP checksum for a buffer of bytes
  // len is number of 16-bit values
  while (len--)
    xsum += (*buff++);               // accumulate 16-bit sum
  while (xsum >> 16)                // propagate carries for 1's complement
    xsum = (xsum & 0xFFFF) + (xsum >> 16);
  return (uint16_t) xsum;
}
