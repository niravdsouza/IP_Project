/*
Kernel module to edit DSCP fields of the IP header
Refrences:
1. http://stackoverflow.com/questions/26774761/ip-and-tcp-header-checksum-calculate-in-c

 */

#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/icmp.h>
#include <linux/string.h>

#define THERMAL_PORT_RANGE1     6000    /* port number range for thermal sensor 1 TCP server */
#define THERMAL_PORT_RANGE2     7000    /* port number range for thermal sensor 2 TCP server */
#define MOTION_PORT_RANGE       8000    /* port number range for motion sensor TCP server */
#define TEST_PORT               2000    /* test port for test iperf traffic */

/* globals */
static struct nf_hook_ops nfho;

static int average_thermal_reading = 0;
static int motion_reading = 0;
static int thermal_mean_value = 0;
static int thermal_reading1 = 0;
static int thermal_reading2 = 0;

/* utility function */
char* get_ip_string(unsigned ip)
{
    char* buffer;
    buffer = kmalloc(16*sizeof(char), GFP_KERNEL);
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;	
    snprintf(buffer, 16, "%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]);
    return buffer;    
}

int isdigit(char c) {
    int i = c;
    if (i >= '0' && i <= '9') {
        return 1;
    } else {
        return 0;
    }
}

char getCharDigit(int num, int pos) {
    char c;
    if (pos == 0) {
        num /= 10;
        
    } else {
        num %= 10;
    }
    c = num + '0';
    return c;
}

/* calculate IP header checksum */
static int get_checksum(u_short *address, int length)
{
    register int nleft = length;
    register u_short *words = address;
    register int sum = 0;
    u_short answer = 0;

    /*
     using a 32 bit accumulator (sum), add sequential
     16 bit words and at the end, fold back all carry bits
     from the top 16 bits into the lower 16 bits.
    */
    while (nleft > 1)  {
        sum += *words++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if (nleft == 1) {
        *(u_char *)(&answer) = *(u_char *) words ;
        sum += answer;
    }

    /* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);         /* add carry */
    answer = ~sum;              /* truncate to 16 bits */
    
    return(answer);
}


// thanx to http://seclists.org/lists/bugtraq/1999/Mar/0057.html
struct tcp_pseudo /*the tcp pseudo header*/
{
    __u32 src_addr;
    __u32 dst_addr;
    __u8 zero;
    __u8 proto;
    __u16 length;
} pseudohead;

long checksum(unsigned short *addr, unsigned int count) {
    /* Compute Internet Checksum for "count" bytes
    *         beginning at location "addr".
    */
    register long sum = 0;
    
    
    while( count > 1 )  {
       /*  This is the inner loop */
           sum += * addr++;
           count -= 2;
    }
       /*  Add left-over byte, if any */
    if( count > 0 )
           sum += * (unsigned char *) addr;
    
       /*  Fold 32-bit sum to 16 bits */
    while (sum>>16)
       sum = (sum & 0xffff) + (sum >> 16);
    
    return ~sum;
}

long get_tcp_checksum(struct iphdr * myip, struct tcphdr * mytcp) {

    u16 total_len = ntohs(myip->tot_len);
    
    int tcpopt_len = mytcp->doff*4 - 20;
    int tcpdatalen = total_len - (mytcp->doff*4) - (myip->ihl*4);
    
    pseudohead.src_addr=myip->saddr;
    pseudohead.dst_addr=myip->daddr;
    pseudohead.zero=0;
    pseudohead.proto=IPPROTO_TCP;
    pseudohead.length=htons(sizeof(struct tcphdr) + tcpopt_len + tcpdatalen);
    
    int totaltcp_len = sizeof(struct tcp_pseudo) + sizeof(struct tcphdr) + tcpopt_len + tcpdatalen;
    unsigned short * tcp = kmalloc(totaltcp_len*sizeof(unsigned short), GFP_KERNEL);
    
    memcpy((unsigned char *)tcp,&pseudohead,sizeof(struct tcp_pseudo));
    memcpy((unsigned char *)tcp+sizeof(struct tcp_pseudo),(unsigned char *)mytcp,sizeof(struct tcphdr));
    memcpy((unsigned char *)tcp+sizeof(struct tcp_pseudo)+sizeof(struct tcphdr), (unsigned char *)myip+(myip->ihl*4)+(sizeof(struct tcphdr)), tcpopt_len);
    memcpy((unsigned char *)tcp+sizeof(struct tcp_pseudo)+sizeof(struct tcphdr)+tcpopt_len, (unsigned char *)mytcp+(mytcp->doff*4), tcpdatalen);
    
    /*      printf("pseud length: %d\n",pseudohead.length);
    printf("tcp hdr length: %d\n",mytcp->doff*4);
    printf("tcp hdr struct length: %d\n",sizeof(struct tcphdr));
    printf("tcp opt length: %d\n",tcpopt_len);
    printf("tcp total+psuedo length: %d\n",totaltcp_len);
    
    fflush(stdout);
    
    printf("tcp data len: %d, data start %u\n", tcpdatalen,mytcp + (mytcp->doff*4));
    */
    
    
    return checksum(tcp,totaltcp_len);

}


static unsigned int tos_hook(const struct nf_hook_ops *ops,
                                       struct sk_buff *skb,
                                       const struct net_device *in,
                                       const struct net_device *out,
                                       int (*okfn)(struct sk_buff *))
{
    struct iphdr *ip_header;        /* IPv4 header */
    struct tcphdr *tcp_header;      /* TCP header */
    struct icmphdr *icmp_header;    /* ICMP header */
    u16 source_port, destination_port;           /* Source and destination ports */
    u32 source_address, destination_address;    /* Source and destination addresses */
    unsigned char *it;          /* TCP data iterator */
    unsigned char *user_data;   /* TCP data begin pointer */
    unsigned char *tail;        /* TCP data end pointer */
    
    /* Network packet is empty, seems like some problem occurred. Skip it */
    if (!skb)
        return NF_ACCEPT;
    
    ip_header = ip_hdr(skb);            /* get IP header */
    tcp_header = tcp_hdr(skb);          /* get TCP header */
    icmp_header = icmp_hdr(skb);        /* get ICMP header */
    
    /* Convert network endianness to host endiannes */
    source_address = ntohl(ip_header->saddr);
    destination_address = ntohl(ip_header->daddr);
    source_port = ntohs(tcp_header->source);
    destination_port = ntohs(tcp_header->dest);
    
    /* Core TOS setting operation
        1. Set TOS field
        2. Update checksum
    */
    if (source_port >= MOTION_PORT_RANGE && source_port < MOTION_PORT_RANGE+999) {
        
        user_data = (unsigned char *)((unsigned char *)tcp_header + (tcp_header->doff * 4));
        tail = skb_tail_pointer(skb);
        
        int c = *user_data;
        motion_reading = c - '0';
        printk("Motion reading: %d\n", motion_reading);
        
         // Context aware QoS
        if (motion_reading == 1) {
            ip_header->tos = 2;
            
        } else {
            ip_header->tos = 1;
        }
        
        ip_header->check = 0;
        ip_header->check = get_checksum((unsigned short*)ip_header, 4*ip_header->ihl);
    
    
    } else if ((source_port >= THERMAL_PORT_RANGE1 && source_port < THERMAL_PORT_RANGE1+999)||\
    (source_port >= THERMAL_PORT_RANGE2 && source_port < THERMAL_PORT_RANGE2+999)){
        
        if (motion_reading == 1) {
            ip_header->tos = 1;
            
        } else {
            ip_header->tos = 2;
        }
        
        ip_header->check = 0;
        ip_header->check = get_checksum((unsigned short*)ip_header, 4*ip_header->ihl);
        
        /* Calculate pointers for begin and end of TCP packet data */
        user_data = (unsigned char *)((unsigned char *)tcp_header + (tcp_header->doff * 4));
        tail = skb_tail_pointer(skb);
        
        int i = 0, j = 0, k =0;
        int thermalIndex = 0;
        int c1, c2, c3, c4;
        int space_count = 0;
        for (it = user_data; it != tail; ++it) {
            char c = *(char *)it;
            k++;
            
            if (space_count == 3) {
                if (c == ' ') {
                    break;
                }
                if (i == 0) {
                    c1 = c;
                    
                } else if (i == 1) {
                    c2 = c;
                    break;
                }
                i++;
                
            } else if (space_count == 2) {
                if (c == ' ') {
                    space_count++;
                    continue;
                }
                if (j == 0) {
                    c3 = c;
                    thermalIndex = k-1;
                    
                } else if (j == 1) {
                    c4 = c;
                }
                j++;
                
            } else {
                if (c == ' ') {
                    space_count++;
                }
            }
            
            if (c == '\0')
                break;
        }
        
        int thermal_reading = 0;
        
        /* Operations pertaining to CPU usage */
        if (i > 0 && isdigit(c1) == 1 && (isdigit(c2) == 1 || c2 == '.')) {
        
            if (c2 == '.') {
                printk("CPU usage: %c\%\n", c1);
            } else {
                printk("CPU usage: %c%c\%\n", c1, c2);
            }
            /* Regulation 1: Discard sensor reading when CPU usage exceeds a threshold: 75% */
            if (c2 != '.' && (c1 > '7' || (c1=='7' && c2 > '5'))) {
                printk("Modifying the packet with average thermal reading %d. CPU: %c%c\%\n",\
                average_thermal_reading, c1, c2);
                printk("UserData: %s\n", user_data);
                *(user_data + thermalIndex) =  getCharDigit(average_thermal_reading, 0);
                *(user_data + thermalIndex + 1) =  getCharDigit(average_thermal_reading, 1);
                printk("UserData: %s\n", user_data);

                //printk("Original checksum: %u\n", tcp_header->check);
                //unsigned backup = tcp_header->check;
                tcp_header->check = 0;
                tcp_header->check = get_tcp_checksum(ip_header, tcp_header);
                //printk("Modified checksum: %u\n", tcp_header->check);
                // 	tcp_header->check = backup;
            }
            
            if (c4 != '.') {
                thermal_reading = 10 * (c3 - '0') + c4 - '0';
                
            } else {
                thermal_reading = c3 - '0';
            }
            printk("TR: %d\tTA: %d\n", thermal_reading, average_thermal_reading);
                
            average_thermal_reading = (int) ((average_thermal_reading + thermal_reading)/2);
        }
        
        /* Operations pertaining to reading difference */
        if (i > 0 && isdigit(c3) == 1 && (isdigit(c4) == 1 || c4 == '.')) {
            if (source_port >= THERMAL_PORT_RANGE1 && source_port < THERMAL_PORT_RANGE1+999) {
                thermal_reading1 = thermal_reading;
            } else {
                thermal_reading2 = thermal_reading;
            }
        
            int diff = thermal_reading2 - thermal_reading1;
            printk("T1: %d T2: %d diff: %d", thermal_reading1, thermal_reading2, diff);
            thermal_mean_value = (thermal_reading2 + thermal_reading1)/2;
            /* Regulation2: modify the thermal reading to the mean value of the two readings */
            if ((diff > 10 || diff < -10) && thermal_reading1 != 0 && thermal_reading2 != 0) {
                printk("Modifying the packet with mean thermal reading %d. Difference is: %d\n",\
                thermal_mean_value, diff);
                printk("UserData: %s\n", user_data);
                *(user_data + thermalIndex) =  getCharDigit(thermal_mean_value, 0);
                *(user_data + thermalIndex + 1) =  getCharDigit(thermal_mean_value, 1);
                printk("UserData: %s\n", user_data);
                
                // Update checksum
                tcp_header->check = 0;
                tcp_header->check = get_tcp_checksum(ip_header, tcp_header);
            }
        }
        
    } else if (destination_port == 22 || (destination_port > 51000 && destination_port < 51007)) {
        // We need to prioritize SSH packets as well to retain connections in load
        ip_header->tos = 3;
        ip_header->check = 0;
        ip_header->check = get_checksum((unsigned short*)ip_header, 4*ip_header->ihl);
        
    } else if (source_port == 2000 || destination_port == 2000) {
        // test traffic to hog the network
        ip_header->tos = 7;
        ip_header->check = 0;
        ip_header->check = get_checksum((unsigned short*)ip_header, 4*ip_header->ihl);
        
    }
    
    printk(KERN_ALERT "SRC_IP: %s\t SPort: %u\tDST_IP: %s\tDPort: %u\tTOS: %u\n", \
    get_ip_string(source_address),source_port, get_ip_string(destination_address),destination_port, ip_header->tos);

    
    


    return NF_ACCEPT;
}


static int __init tos_init(void)
{
    int res;
    
    nfho.hook = (nf_hookfn *)tos_hook;          /* hook function */
    nfho.hooknum = NF_INET_PRE_ROUTING;         /* received packets */
    nfho.pf = PF_INET;                          /* IPv4 */
    nfho.priority = NF_IP_PRI_FIRST;            /* max hook priority */
    
    res = nf_register_hook(&nfho);
    if (res < 0) {
        pr_err("print_tcp: error in nf_register_hook()\n");
        return res;
    }
    
    pr_debug("print_tcp: loaded\n");
    return 0;
}

static void __exit tos_exit(void)
{
    nf_unregister_hook(&nfho);
    pr_debug("print_tcp: unloaded\n");
}

module_init(tos_init);
module_exit(tos_exit);
