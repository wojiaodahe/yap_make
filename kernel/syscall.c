#include "common.h"
#include "syscall.h"
#include "head.h"
#include "kernel.h"
#include "error.h"
#include "print.h"

extern pcb_t *current;

#if 0
int sys_call_schedule(int swi_num, int argc, int *argv)
{
    int cmd_type;
	printk("System Call Function swi_num: %d argc: %d argv: %x\r\n", swi_num, argc, *argv);

    cmd_type = argv[0];
    switch (cmd_type)
    {
      case OS_SEND_RECV:
            return sys_sendrec(argv[1], argv[2], (MESSAGE *)argv[3], current);
//            break;
    }

	return -1;
}
#else

int system_call_sendrec(int argc, int *argv)
{
	if (argc < 4)
		return -EINVAL;
	if (argv[0] != OS_SEND_RECV)
		return -EINVAL;

	return sys_sendrec(argv[1], argv[2], (MESSAGE *)argv[3], current);
}

int system_call_open(int argc, int *argv)
{
	if (argc < 4)	
		return -EINVAL;
	if (argv[0] != SYSTEM_CALL_OPEN)
		return -EINVAL;

	return sys_open(argv[1], argv[2], argv[3]);
}

int system_call_read(int argc, int *argv)
{
	if (argc < 4)	
		return -EINVAL;
	if (argv[0] != SYSTEM_CALL_READ)
		return -EINVAL;

	return sys_read(argv[1], argv[2], argv[3]);
}

int system_call_printf(int argc, unsigned int *argv)
{
	if (argc < 3)
		return -EINVAL;
	if (argv[0] != SYSTEM_CALL_PRINTF)
		return -EINVAL;

	sys_write(0, argv[1], argv[2]);
}

int system_call_sleep(int argc, unsigned int *argv)
{
	if (argc < 2)
		return -EINVAL;
	if (argv[0] != SYSTEM_CALL_SLEEP)
		return -EINVAL;
	process_sleep(argv[1]);
}

struct system_call_tag system_call[] = 
{
	{OS_SEND_RECV, system_call_sendrec},
	{SYSTEM_CALL_OPEN, system_call_open},
	{SYSTEM_CALL_READ, system_call_read},
	{SYSTEM_CALL_PRINTF, system_call_printf},
	{SYSTEM_CALL_SLEEP, system_call_sleep},
};

int sys_call_schedule(int swi_num, int argc, int *argv)
{
	int i;
#if 0
	printk("System Call Function swi_num: %d argc: %d argv:", swi_num, argc);
	for (i = 0; i < argc; i++)
		printk("%x ", argv[i]);
	printk("\n");
#endif
	for (i = 0; i < sizeof (system_call) /  sizeof (system_call[0]); i++)
	{
		if (system_call[i].id == argv[0])
			return system_call[i].fun(argc, argv);
	}
	return -EINVAL;
}
#endif

unsigned int OS_Get_Ticks()
{
   	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, OS_SYS_PROCESS_PID, &msg);
	return msg.RETVAL;
}

void milli_delay(int sec)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////
int sendrec(int type, int src_dest, MESSAGE *msg)
{
    const int argc = 4;
    unsigned long argv[argc];

    argv[0] = OS_SEND_RECV;
    argv[1] = type;
    argv[2] = src_dest;
    argv[3] = (unsigned long)msg;
	//return sys_call_schedule(0, argc, argv);
    return syscall(argc, argv);
}


extern int user_syscall(int, unsigned int *);
int open(const char *path, unsigned int flag, unsigned int mode)
{
	const int argc = 4;
	unsigned int argv[argc];

	argv[0] = SYSTEM_CALL_OPEN;
	argv[1] = (unsigned int)path;
	argv[2] = (unsigned int)flag;
	argv[3] = (unsigned int)mode;

	return user_syscall(argc, argv);
}

int read(int fd, char *buff, unsigned int count)
{
	const int argc = 4;
	unsigned int argv[argc];

	argv[0] = SYSTEM_CALL_READ;
	argv[1] = (unsigned int)fd;
	argv[2] = (unsigned int)buff;
	argv[3] = (unsigned int)count;

	return user_syscall(argc, argv);

}

void ssleep(unsigned int time)
{
	const char argc = 2;
	unsigned int argv[argc];
	
	argv[0] = SYSTEM_CALL_SLEEP;
	argv[1] = time;
	user_syscall(argc, argv);
}

int myprintf(char *fmt, ...)
{
	const int argc = 3;
	unsigned int argv[argc];
	int ret;
#if 1

	va_list ap;
	char string[256];
	
	memset(string, 0, 256);
	
	va_start(ap, fmt);
	ret = vsnprintf(string, 256, fmt, ap);
	argv[0] = SYSTEM_CALL_PRINTF;
	argv[1] = (unsigned int )string;
	argv[2] = ret;
	ret = user_syscall(argc, argv);	
	va_end(ap);
#endif
	return ret;
}
