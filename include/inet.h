#ifndef __INET_H__
#define __INET_H__

#include "inet_socket.h"

struct inet_pseudo_hdr
{
	int sip;
	int dip;
	char mbz;
	char proto;
	unsigned short len;
}__attribute__((packed));




/* 
 *  对以下两个SB结构体的说明：
 *  ARM V4 架构不支持地址非对齐访问
 *  在keil中使用_packed指令强制1字节对齐: 类似 __packed unsigned int *ptr 这样的定义
 *  然而在gcc下 对于 unsigned int *ptr __attribute__((packed)) 后面的packed
 *  在编译的时候提示被ignore，懒的查原因，改成结构体的形式之后可行
 *  以下两个结构体声明用在inet_check 和 tcp_build_options等函数之中，这种
 *  函数可能会访问非对齐地址从而导致data abort,要使用packed属性强制1字节对齐
 */ 
struct packed_ptr_short
{
    volatile unsigned short ptr;
}__attribute__((packed));

struct packed_ptr_int
{
    volatile unsigned int ptr;
} __attribute__((packed));




extern unsigned short htons(unsigned short h);
extern unsigned int htonl(unsigned int h);
extern unsigned int ntohl(unsigned long int n);
extern unsigned short ntohs(unsigned short int n);
extern unsigned short inet_chksum(void *buffer, unsigned short size);
extern int net_core_init(void);
extern short int inet_check(struct ip_addr *sip, struct ip_addr *dip, unsigned char *buffer, unsigned short size, int proto);
extern unsigned short __inet_chksum(void *buffer, unsigned short size);


extern unsigned short htons(unsigned short h);
extern unsigned int htonl(unsigned int h);
extern unsigned int ntohl(unsigned long int n);
extern unsigned short ntohs(unsigned short int n);

#endif


