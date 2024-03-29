#ifndef __PRINTk_H__
#define __PRINTK_H__

typedef char * va_list;
#define _INTSIZEOF(n)   ((sizeof(n) + sizeof(n) - 1) & ~(sizeof(n) - 1))
#define va_start(ap, v) (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap, t)   (*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap)      (ap = (va_list)0 )

extern int vsprintk(char *buf, char *fmt, va_list vp);
extern int sprintk(char *buf, char *fmt, ...);
extern void printk(char *fmt, ...);
#endif
