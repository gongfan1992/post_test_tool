/*************************************************************************
	> File Name: parse_packet.c
	> Author: likeyi
	> Mail: likeyiyy@sina.com 
	> Created Time: Wed 02 Apr 2014 04:07:31 PM CST
 ************************************************************************/
#include "includes.h"
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
/* IP flags. */
#define IP_CE       0x8000      /* Flag: "Congestion"       */
#define IP_DF       0x4000      /* Flag: "Don't Fragment"   */
#define IP_MF       0x2000      /* Flag: "More Fragments"   */
#define IP_OFFSET   0x1FFF      /* "Fragment Offset" part   */
/* byte no mean bigedian or litiledian */
static inline void parse_tos(unsigned char tos)
{
    switch(IPTOS_TOS(tos))
    {
        case IPTOS_LOWDELAY:
            printf("IPTOS_LOWDELAY\n");
            break;
        case IPTOS_THROUGHPUT:
            printf("IPTOS_THROUGHPUT\n");
            break;
        case IPTOS_RELIABILITY:
            printf("IPTOS_RELIABILITY\n");
            break;
        case IPTOS_MINCOST:
            printf("IPTOS_MINCOST\n");
            break;
    }
}
static inline void parse_frag_off(__be16 frag_off)
{
    if(frag_off & htons(IP_MF | IP_OFFSET))
    {
        printf("have fragment  ");
        if(frag_off & htons(IP_MF))
        {
            printf("and not end\n");
        }
        else
        {
            printf("and it end\n");
        }
        printf("ip offset:%d\n",ntohs(frag_off & htons(IP_OFFSET)));
        return;
    }
    printf("not yet fragment\n");
}
static inline void parse_ip_protocol(__u8 protocol)
{
    switch(protocol)
    {
        case IPPROTO_IP:       printf("Dummy protocol for TCP\n"); break;
        case IPPROTO_ICMP :    printf(" Internet Control Message Protocol \n");break;
        case IPPROTO_IGMP :    printf(" Internet Group Management Protocol \n");break;
        case IPPROTO_IPIP :    printf(" IPIP tunnels (older KA9Q tunnels use 94) \n");break;
        case IPPROTO_TCP :     printf(" Transmission Control Protocol \n");break;
        case IPPROTO_EGP :     printf(" Exterior Gateway Protocol \n");break;
        case IPPROTO_PUP :     printf(" PUP protocol \n");break;
        case IPPROTO_UDP :     printf(" User Datagram Protocol \n");break;
        case IPPROTO_IDP :     printf(" XNS IDP protocol \n");break;
        case IPPROTO_DCCP :    printf(" Datagram Congestion Control Protocol \n");break;
        case IPPROTO_RSVP :    printf(" RSVP protocol \n");break;
        case IPPROTO_GRE :     printf(" Cisco GRE tunnels (rfc 1701,1702) \n");break;
        case IPPROTO_IPV6:     printf(" IPv6-in-IPv4 tunnelling \n");break;
        case IPPROTO_ESP :     printf(" Encapsulation Security Payload protocol \n");break;
        case IPPROTO_AH :      printf(" Authentication Header protocol \n");break;
        case IPPROTO_PIM    :  printf(" Protocol Independent Multicast \n");break;
        case IPPROTO_COMP   :  printf(" Compression Header protocol \n");break;
        case IPPROTO_SCTP   :  printf(" Stream Control Transport Protocol \n");break;
        case IPPROTO_UDPLITE : printf(" UDP-Lite (RFC 3828) \n");break;
        case IPPROTO_RAW    :  printf(" Raw IP packets \n");break;
        default:printf("unknow protocol\n");break;
    }
}
void parse_iphdr(struct iphdr * iph)
{
    pthread_mutex_lock(&print_lock);
    struct in_addr in;
    printf("ip_version:%d\n",iph->version);
    printf("header_len:%dB\n",iph->ihl * 4);
    parse_tos(iph->tos);
    printf("total_len:%dB\n",ntohs(iph->tot_len));
    printf("id:%d\n",iph->id);
    parse_frag_off(iph->frag_off);
    printf("ttl:%d\n",iph->ttl);
    parse_ip_protocol(iph->protocol);
    in.s_addr = iph->saddr;
    printf("saddr:%s \t",inet_ntoa(in));
    in.s_addr = iph->daddr;
    printf("daddr:%s\n",inet_ntoa(in));
    pthread_mutex_unlock(&print_lock);
}
void parse_tcphdr(struct tcphdr * tcph)
{
    printf("Source port:%u    ",ntohs(tcph->source));        
    printf("dest   port:%u\n",ntohs(tcph->dest));
    printf("seq : %u    ",ntohs(tcph->seq));
    printf("ack seq : %u\n",ntohs(tcph->ack_seq));
    printf("tcp header len:%u\n",tcph->doff * 4);
    printf("URG:%u ACK:%u PSH:%u RST:%u SYN:%u FIN:%u\n",tcph->urg,tcph->ack,tcph->psh,tcph->rst,tcph->syn,tcph->fin);
    printf("window:%u    ",tcph->window);
    printf("Checksum:%x \n",tcph->check);
    printf("Urgent Pointer:%u\n",tcph->urg_ptr);
}
/*
static inline unsigned char * get_dst_mac(struct ethhdr * ether)
{
    return (unsigned char *)ether;
}
static inline unsigned char * get_src_mac(struct ethhdr * ether)
{
    return (unsigned char *)(ether + 6); 
}
static void print_mac(unsigned char * mac)
{
    int i = 0;
    for(i = 0; i < 6; i++)
    {
        printf("%02x ",mac[i]);
    }
    printf("\n");
}
void parse_full_packet(void * packet)
{
    struct ethhdr * ether = (struct ethhdr *)packet;
    unsigned char * dst_mac = get_dst_mac(ether);
    unsigned char * src_mac = get_src_mac(ether);
    printf("dst mac:");
    print_mac(dst_mac);
    printf("src mac:");
    print_mac(src_mac);
}
*/
static void parse_mac_type(__be16 h_proto)
{
	switch(h_proto)
	{
		case ETH_P_LOOP  :      printf(" Ethernet Loopback packet \n");break;
		case ETH_P_PUP   :      printf(" Xerox PUP packet     \n");break;
		case ETH_P_PUPAT :      printf(" Xerox PUP Addr Trans packet  \n");break;
		case ETH_P_IP    :      printf(" Internet Protocol packet \n");break;
		case ETH_P_X25   :      printf(" CCITT X.25           \n");break;
		case ETH_P_ARP   :      printf(" Address Resolution packet    \n");break;
		case ETH_P_BPQ   :      printf(" G8BPQ AX.25 Ethernet Packet  [ NOT AN OFFICIALLY REGISTERED ID ] \n");break;
		case ETH_P_IEEEPUP   :      printf(" Xerox IEEE802.3 PUP packet \n");break;
		case ETH_P_IEEEPUPAT :      printf(" Xerox IEEE802.3 PUP Addr Trans packet \n");break;
		case ETH_P_DEC       :          printf(" DEC Assigned proto           \n");break;
		case ETH_P_DNA_DL    :          printf(" DEC DNA Dump/Load            \n");break;
		case ETH_P_DNA_RC    :          printf(" DEC DNA Remote Console       \n");break;
		case ETH_P_DNA_RT    :          printf(" DEC DNA Routing              \n");break;
		case ETH_P_LAT       :          printf(" DEC LAT                      \n");break;
		case ETH_P_DIAG      :          printf(" DEC Diagnostics              \n");break;
		case ETH_P_CUST      :          printf(" DEC Customer use             \n");break;
		case ETH_P_SCA       :          printf(" DEC Systems Comms Arch       \n");break;
		case ETH_P_TEB   :      printf(" Trans Ether Bridging     \n");break;
		case ETH_P_RARP      :      printf(" Reverse Addr Res packet  \n");break;
		case ETH_P_ATALK :      printf(" Appletalk DDP        \n");break;
		case ETH_P_AARP  :      printf(" Appletalk AARP       \n");break;
		case ETH_P_8021Q :          printf(" 802.1Q VLAN Extended Header  \n");break;
		case ETH_P_IPX   :      printf(" IPX over DIX         \n");break;
		case ETH_P_IPV6  :      printf(" IPv6 over bluebook       \n");break;
		case ETH_P_PAUSE :      printf(" IEEE Pause frames. See 802.3 31B \n");break;
		case ETH_P_SLOW  :      printf(" Slow Protocol. See 802.3ad 43B \n");break;
		case ETH_P_WCCP  :      printf(" Web-cache coordination protocol defined in draft-wilson-wrec-wccp-v2-00.txt \n");break;
		case ETH_P_PPP_DISC  :      printf(" PPPoE discovery messages     \n");break;
		case ETH_P_MPLS_UC   :      printf(" MPLS Unicast traffic     \n");break;
		case ETH_P_MPLS_MC   :      printf(" MPLS Multicast traffic   \n");break;
		case ETH_P_ATMMPOA   :      printf(" MultiProtocol Over ATM   \n");break;
		case ETH_P_ATMFATE   :      printf(" Frame-based ATM Transportover Ethernet \n");break;
		case ETH_P_PAE   :      printf(" Port Access Entity (IEEE 802.1X) \n");break;
		case ETH_P_AOE   :      printf(" ATA over Ethernet        \n");break;
		case ETH_P_TIPC  :      printf(" TIPC             \n");break;
		case ETH_P_1588  :      printf(" IEEE 1588 Timesync \n");break;
		case ETH_P_FCOE  :      printf(" Fibre Channel over Ethernet  \n");break;
		//case ETH_P_TDLS  :      printf(" TDLS \n");break;
		case ETH_P_FIP   :      printf(" FCoE Initialization Protocol \n");break;
		case ETH_P_EDSA  :      printf(" Ethertype DSA [ NOT AN OFFICIALLY REGISTERED ID ] \n");break;
		//case ETH_P_AF_IUCV   :      printf(" IBM af_iucv [ NOT AN OFFICIALLY REGISTERED ID ] \n");break;
		case ETH_P_802_3 :      printf(" Dummy type for 802.3 frames  \n");break;
		case ETH_P_AX25  :      printf(" Dummy protocol id for AX.25  \n");break;
		case ETH_P_ALL   :      printf(" Every packet (be careful!!!) \n");break;
		case ETH_P_802_2 :      printf(" 802.2 frames         \n");break;
		case ETH_P_SNAP  :      printf(" Internal only        \n");break;
		case ETH_P_DDCMP  :             printf(" DEC DDCMP: Internal only     \n");break;
		case ETH_P_WAN_PPP  :           printf(" Dummy type for WAN PPP frames\n");break;
		case ETH_P_PPP_MP   :           printf(" Dummy type for PPP MP frames \n");break;
		case ETH_P_LOCALTALK:       printf(" Localtalk pseudo type    \n");break;
		case ETH_P_CAN     :    printf(" Controller Area Network      \n");break;
		case ETH_P_PPPTALK   :      printf(" Dummy type for Atalk over PPP\n");break;
		case ETH_P_TR_802_2 :       printf(" 802.2 frames         \n");break;
		case ETH_P_MOBITEX  :       printf(" Mobitex (kaz@cafe.net)   \n");break;
		case ETH_P_CONTROL :        printf(" Card specific control frames \n");break;
		case ETH_P_IRDA    :    printf(" Linux-IrDA           \n");break;
		case ETH_P_ECONET  :        printf(" Acorn Econet         \n");break;
		case ETH_P_HDLC     :   printf(" HDLC frames          \n");break;
		case ETH_P_ARCNET  :        printf(" 1A for ArcNet :-)            \n");break;
		case ETH_P_DSA    :     printf(" Distributed Switch Arch. \n");break;
		case ETH_P_TRAILER :        printf(" Trailer switch tagging   \n");break;
		case ETH_P_PHONET   :       printf(" Nokia Phonet frames          \n");break;
		case ETH_P_IEEE802154  :    printf(" IEEE802.15.4 frame       \n");break;
		default:printf("unknow type\n");break;
	}
}
static void printf_mac_info(struct ethhdr * ether_header)
{
        printf("DMAC:%02X:%02X:%02X:%02X:%02X:%02X\n",
                ether_header->h_dest[0],
                ether_header->h_dest[1],
                ether_header->h_dest[2],
                ether_header->h_dest[3],
                ether_header->h_dest[4],
                ether_header->h_dest[5]
                );
        printf("SMAC:%02X:%02X:%02X:%02X:%02X:%02X\n",
                ether_header->h_source[0],
                ether_header->h_source[1],
                ether_header->h_source[2],
                ether_header->h_source[3],
                ether_header->h_source[4],
                ether_header->h_source[5]
                );
        parse_mac_type(ntohs(ether_header->h_proto));
}
void parse_full_packet(void * buffer)
{
    struct ethhdr * ether_header;
    struct iphdr  * ip_header;
    struct tcphdr * tcp_header;
    ether_header = (struct ethhdr *)buffer;
    printf_mac_info(ether_header);
    if(ether_header->h_proto == htons(ETH_P_IP))
    {
        ip_header = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        parse_iphdr(ip_header);
        switch(ip_header->protocol)
        {
            case IPPROTO_TCP:
                tcp_header  = (struct tcphdr *)(buffer + sizeof(struct ethhdr) + ip_header->ihl *4 );          
                parse_tcphdr(tcp_header);
                break;
            default:break;
        }
    }
}
void print_packet(unsigned char * packet,int size)    
{    
    pthread_mutex_lock(&print_lock);
    int i = 0;  
    for(i = 0; i < size; i++)                   
    {
                if((i == 0)||(i % 16))                  
                {       
                            printf("%02x ",packet[i]);          
                        }       
                else    
                {       
                            printf("\n%02x ",packet[i]);        
                        }       
    }
    printf("\n");                               
   pthread_mutex_unlock(&print_lock);
}   
