/*
Referred and adapted originally from: http://stackoverflow.com/questions/29553990/print-tcp-packet-data
*/

#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/icmp.h>

#define HTTP_PORT           80
#define SSH_PORT            22
#define INTERNAL_NETWORK    3232235777  /* 192.168.1.1 */
#define OUTSIDE_NETWORK     3232236033  /* 192.168.2.1 */
#define WEB_SERVER_IP       3232235779  /* 192.168.1.3 */
#define ICMP_PROTOCOL       1


static struct nf_hook_ops nfho;

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



static unsigned int firewall_hook_func(const struct nf_hook_ops *ops,
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

    /* SET THE FIREWALL RULES */
    
    /* 1. Block ICMP if source is not a local host,
            and if destination is not the webserver 
            Important to note that we allow the reverse communication, and hence not ICMP_ECHOREPLY */
    
    if (ip_header->protocol == ICMP_PROTOCOL &&
        (source_address < INTERNAL_NETWORK ||
         source_address > INTERNAL_NETWORK+254) &&
         destination_address != WEB_SERVER_IP &&
         icmp_header->type != ICMP_ECHOREPLY)
    {
        printk(KERN_ALERT "Dropped ICMP: saddr: %s, daddr: %s\n", get_ip_string(source_address), get_ip_string(destination_address));
        return NF_DROP;
    }
    
    /* 2. Block SSH (22) if source is not a local host,
            and if destination is a local host */
    if (destination_port == SSH_PORT &&
        (source_address < INTERNAL_NETWORK ||
         source_address > INTERNAL_NETWORK+254) &&
         (destination_address >= INTERNAL_NETWORK &&
          destination_address < INTERNAL_NETWORK+254))
    {
        printk(KERN_ALERT "Dropped SSH: saddr: %s, daddr: %s\n", get_ip_string(source_address), get_ip_string(destination_address));
        return NF_DROP;
    }
    
    /* 3. Block HTTP (80) if source is not a local host,
            and if destination is not the Web Server */
    if (destination_port == HTTP_PORT &&
        (source_address < INTERNAL_NETWORK ||
         source_address > INTERNAL_NETWORK+254) &&
         destination_address != WEB_SERVER_IP &&
         (destination_address >= INTERNAL_NETWORK &&
          destination_address < INTERNAL_NETWORK+254))
    {
        printk(KERN_ALERT "Dropped HTTP: saddr: %s, daddr: %s\n", get_ip_string(source_address), get_ip_string(destination_address));
        return NF_DROP;
    }
    

    return NF_ACCEPT;
}


static int __init firewall_init(void)
{
    int res;

    nfho.hook = (nf_hookfn *)firewall_hook_func;    /* hook function */
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

static void __exit firewall_exit(void)
{
    nf_unregister_hook(&nfho);
    pr_debug("print_tcp: unloaded\n");
}

module_init(firewall_init);
module_exit(firewall_exit);
