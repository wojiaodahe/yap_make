#include "fs.h"
#include "syscall.h"
#include "vfs.h"
#include "kmalloc.h"
#include "lib.h"
#include "printk.h"
#include "list.h"

#include "unistd.h"
#include "fcntl.h"
#include "wait.h"
//#include "socket.h"
//#include "inet.h"
//#include "inet_socket.h"
#include "timer.h"
#include "completion.h"

int  test_get_ticks(void *p)
{
	int t;
	while (1)
    {	
		printk("Process1 GetTicks\r\n");
		t = OS_Get_Ticks();
        printk("Process1 GetTicks: %d\r\n", t);
        OS_Sched();
	}

	//return 0;
}

int test_wait_queue(void *p)
{
	int fd;
	char buf[64];

	fd = sys_open("/key", 0, 0);
	while (1)
	{
		memset(buf, 0, 64);
		sys_read(fd, buf, 64);
		printk("++++++++++++++++wait_queue read++++++++++++++++++\n");
	}
}
int  test_open_led0(void *p)
{
	int fd;

	fd = sys_open("led0", 0, 0);
	while (1)
    {
		sys_ioctl(fd, 0, 0);
		ssleep(1);
		sys_ioctl(fd, 1, 1);
		ssleep(1);
	}

}

int  test_open_led1(void *p)
{
	int fd;

	fd = sys_open("led1", 0, 0);
	while (1)
    {
		sys_ioctl(fd, 0, 0);
		msleep(500);
		sys_ioctl(fd, 1, 1);
		msleep(500);
	}
}

int  test_open_led2(void *p)
{
	int fd;

	fd = sys_open("led2", 0, 0);
	while (1)
    {
		sys_ioctl(fd, 0, 0);
		msleep(250);
		sys_ioctl(fd, 1, 1);
		msleep(250);
	}
}

int  test_open_led3(void *p)
{
	int fd;

	fd = sys_open("led3", 0, 0);
	while (1)
    {
		sys_ioctl(fd, 0, 0);
		msleep(125);
		sys_ioctl(fd, 1, 1);
		msleep(125);
	}
}

char buff[3096];
int  test_nand(void *p)
{
	int i;
	int fd;
	int ret;

	if ((fd = sys_open("/nand/abc/test_ofs.c", 0, 0)) < 0)
	{
		printk("open error");
	}

	while (1)
	{
		ret = sys_read(fd, buff, 2555);
		if (ret <= 0)
			break;
		for (i = 0; i < ret; i++)
		{
			printk("%c", buff[i]);
		}
	}
	while (1)
		OS_Sched();

	//return 0;
}

int test_user_syscall_open(void *argc)
{
	int fd;
	int ret, i;
	fd = open("/nand/abc/test_ofs.c", 0, 0);
	
	while (1)
	{
		ret = read(fd, buff, 2555);
		if (ret <= 0)
			break;
		for (i = 0; i < ret; i++)
		{
			printk("%c", buff[i]);
		}
	}

	while (1)
		OS_Sched();
}

int test_user_syscall_printf(void *argc)
{
	while (1)
	{
//		myprintf("Process Test Printf %d %x %c %s", 10, 0xaa, 'p', "test string\n");
		ssleep(1);
	}
}

int test_exit(void *arg)
{
    printk("thread exit!\n");
    while (1)
        ;
}

void test_completion_func(void *x)
{
    complete(x);
}

struct completion test_done;
struct timer_list test_completion_timer = 
{
    .expires = 100,
    .data = &test_done,
    .function = test_completion_func
};

int test_completion(void *arg)
{
    init_completion(&test_done);
    add_timer(&test_completion_timer);
    printk("&test_completion_timer: %x\n", &test_completion_timer);
    printk("&test_done: %x\n", &test_done);
    while (1)
    {
        mod_timer(&test_completion_timer, 100);
//        printk("Test completion test_done.done %d\n", test_done.done);
        wait_for_completion(&test_done);
    }
}

extern int s3c24xx_init_tty(void);
extern void init_user_program_space(void);
extern int test_platform(void);
extern void init_key_irq(void);
int kernel_main()
{
    init_key_irq();
	
	init_user_program_space();
    
	OS_Init();
	test_platform();
	OS_Start();

	while (1)
	{
		
	}
//    return 0;
}
