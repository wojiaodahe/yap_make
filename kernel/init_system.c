#include "s3c24xx.h"
#include "system.h"
#include "config.h"

extern void enable_irq(void);
extern void disable_irq(void);
extern void umask_int(unsigned int);
extern void usubmask_int(unsigned int offset);



/*
 * �ر�WATCHDOG������CPU�᲻������
 */
void disable_watch_dog(void)
{
    WTCON = 0;  // �ر�WATCHDOG�ܼ򵥣�������Ĵ���д0����
}


void init_memory(void)
{
    int i = 0;
    volatile unsigned long *p = (volatile unsigned long *)MEM_CTL_BASE;

    /* SDRAM 13���Ĵ�����ֵ */
    unsigned long  const    mem_cfg_val[] = {
					      0x22011110,     //BWSCON
					      0x00000700,     //BANKCON0
					      0x00000700,     //BANKCON1
					      0x00000700,     //BANKCON2
					      0x00000700,     //BANKCON3
					      0x00000700,     //BANKCON4
					      0x00000700,     //BANKCON5
					      0x00018005,     //BANKCON6
					      0x00018005,     //BANKCON7
					      0x008C07A3,     //REFRESH
					      0x000000B1,     //BANKSIZE
					      0x00000030,     //MRSRB6
					      0x00000030,     //MRSRB7
					    };


    for (; i < 13; i++)
        p[i] = mem_cfg_val[i];
}


/*
 * ����ҳ��
 */

extern unsigned int MMU_TLB_BASE;
extern unsigned int SYSTEM_TOTAL_MEMORY_START;

 #define VECTOR_BASE SYSTEM_TOTAL_MEMORY_START
//ӳ��1M��ַ�ռ�
void __set_l1_section_descriptor(unsigned long virtuladdr, unsigned long physicaladdr, unsigned int attributes)
{
    volatile unsigned int *mmu_tlb_base = (unsigned int*)MMU_TLB_BASE;

	*(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | attributes;
}
//ӳ������n M��ַ�ռ�
void set_l1_parallel_descriptor(unsigned long viraddr_start, unsigned long viraddr_end, unsigned long phyaddr_start, unsigned int attributes)
{
	int nSec, i;
	volatile unsigned long *mmu_tlb_base;

	mmu_tlb_base = (unsigned long *)MMU_TLB_BASE + (viraddr_start >> 20);
	nSec = (viraddr_end >> 20) - (viraddr_start >> 20);

	for (i = 0; i < nSec; i++)
		*mmu_tlb_base++ = attributes | (((phyaddr_start >> 20) + i) << 20);
}

void create_page_table(void)
{

    unsigned long virtuladdr, physicaladdr;
    unsigned long *mmu_tlb_base = (unsigned long *)MMU_TLB_BASE;

    /*
     * ��0��1M�������ַӳ�䵽0x30000000
     */
    virtuladdr = 0;
    physicaladdr = VECTOR_BASE;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC_WB;
    virtuladdr = 0x51000000;
    physicaladdr = 0x51000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;
    /*
     * 0x56000000��GPIO�Ĵ�������ʼ�����ַ��
     */
    virtuladdr = 0x56000000;
    physicaladdr = 0x56000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;


    virtuladdr = 0x4a000000;
    physicaladdr = 0x4a000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;

    virtuladdr = 0x4e000000;
    physicaladdr = 0x4e000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;

    virtuladdr = 0x50000000;
    physicaladdr = 0x50000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;

    /*
     * SDRAM�������ַ��Χ��0x30000000��0x33FFFFFF��
     * �������ַ0xB0000000��0xB3FFFFFFӳ�䵽�����ַ0x30000000��0x33FFFFFF�ϣ�
     * �ܹ�64M���漰64����������
     */

	set_l1_parallel_descriptor(0x30000000, 0x34000000, 0x30000000, MMU_SECDESC_WB);
#if 0
    virtuladdr = 0x30000000;
    physicaladdr = 0x30000000;
    while (virtuladdr < 0x34000000)
    {
        *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                                MMU_SECDESC_WB;
        virtuladdr += 0x100000;
        physicaladdr += 0x100000;
    }
#endif
}

/*
 * ����MMU
 */

void start_mmu(void)
{
    unsigned long ttb = MMU_TLB_BASE;

// ARM��ϵ�ܹ�����
// Ƕ���ࣺLINUX�ں���ȫע��
__asm__(
    "mov    r0, #0\n"
    "mcr    p15, 0, r0, c7, c7, 0\n"    /* ʹ��ЧICaches��DCaches */

    "mcr    p15, 0, r0, c7, c10, 4\n"   /* drain write buffer on v4 */
    "mcr    p15, 0, r0, c8, c7, 0\n"    /* ʹ��Чָ����TLB */

    "mov    r4, %0\n"                   /* r4 = ҳ���ַ */
    "mcr    p15, 0, r4, c2, c0, 0\n"    /* ����ҳ���ַ�Ĵ��� */

    "mvn    r0, #0\n"
    "mcr    p15, 0, r0, c3, c0, 0\n"    /* ����ʿ��ƼĴ�����Ϊ0xFFFFFFFF��
                                         * ������Ȩ�޼��
                                         */
    /*
     * ���ڿ��ƼĴ������ȶ�����ֵ����������޸ĸ���Ȥ��λ��
     * Ȼ����д��
     */
    "mrc    p15, 0, r0, c1, c0, 0\n"    /* �������ƼĴ�����ֵ */

    /* ���ƼĴ����ĵ�16λ����Ϊ��.RVI ..RS B... .CAM
     * R : ��ʾ����Cache�е���Ŀʱʹ�õ��㷨��
     *     0 = Random replacement��1 = Round robin replacement
     * V : ��ʾ�쳣���������ڵ�λ�ã�
     *     0 = Low addresses = 0x00000000��1 = High addresses = 0xFFFF0000
     * I : 0 = �ر�ICaches��1 = ����ICaches
     * R��S : ������ҳ���е�������һ��ȷ���ڴ�ķ���Ȩ��
     * B : 0 = CPUΪС�ֽ���1 = CPUΪ���ֽ���
     * C : 0 = �ر�DCaches��1 = ����DCaches
     * A : 0 = ��ݷ���ʱ�����е�ַ�����飻1 = ��ݷ���ʱ���е�ַ������
     * M : 0 = �ر�MMU��1 = ����MMU
     */

    /*
     * �������Ҫ��λ����������Ҫ��������������
     */
                                        /* .RVI ..RS B... .CAM */
    "bic    r0, r0, #0x3000\n"          /* ..11 .... .... .... ���V��Iλ */
    "bic    r0, r0, #0x0300\n"          /* .... ..11 .... .... ���R��Sλ */
    "bic    r0, r0, #0x0087\n"          /* .... .... 1... .111 ���B/C/A/M */

    /*
     * ������Ҫ��λ
     */
    "orr    r0, r0, #0x0002\n"          /* .... .... .... ..1. ���������� */
    "orr    r0, r0, #0x0004\n"          /* .... .... .... .1.. ����DCaches */
    "orr    r0, r0, #0x1000\n"          /* ...1 .... .... .... ����ICaches */
    "orr    r0, r0, #0x0001\n"          /* .... .... .... ...1 ʹ��MMU */

    "mcr    p15, 0, r0, c1, c0, 0\n"    /* ���޸ĵ�ֵд����ƼĴ��� */
    : /* ����� */
    : "r" (ttb) );
}



/*
 * ����MPLLCON�Ĵ�����[19:12]ΪMDIV��[9:4]ΪPDIV��[1:0]ΪSDIV
 * �����¼��㹫ʽ��
 *  S3C2410: MPLL(FCLK) = (m * Fin)/(p * 2^s)
 *  S3C2440: MPLL(FCLK) = (2 * m * Fin)/(p * 2^s)
 *  ����: m = MDIV + 8, p = PDIV + 2, s = SDIV
 * ���ڱ������壬Fin = 12MHz
 * ����CLKDIVN�����Ƶ��Ϊ��FCLK:HCLK:PCLK=1:2:4��
 * FCLK=200MHz,HCLK=100MHz,PCLK=50MHz
 */
void init_clock(void)
{
    CLKDIVN  = 0x03;            // FCLK:HCLK:PCLK=1:2:4, HDIVN=1,PDIVN=1
    
__asm__(
    "mrc    p15, 0, r1, c1, c0, 0\n"        /* 读出控制寄存器 */
    "orr    r1, r1, #0xc0000000\n"          /* 设置为“asynchronous bus mode” */
    "mcr    p15, 0, r1, c1, c0, 0\n"        /* 写入控制寄存器 */
    );

    /* �ж���S3C2410����S3C2440 */
    if ((GSTATUS1 == 0x32410000) || (GSTATUS1 == 0x32410002))
    {
        MPLLCON = S3C2410_MPLL_200MHZ;  /* ���ڣ�FCLK=200MHz,HCLK=100MHz,PCLK=50MHz */
    }
    else
    {
        MPLLCON = S3C2440_MPLL_200MHZ;  /* ���ڣ�FCLK=200MHz,HCLK=100MHz,PCLK=50MHz */
    }
}

void timer_init(void)
{
	TCFG0 |= (100 << 8);
    TCFG1 |= (2 << 16);
	TCON &= (~(7 << 20));
	TCON |= (1 << 22);
	TCON |= (1 << 21);

	TCONB4 = 625;
	TCON |= (1 << 20);
	TCON &= ~(1 << 21);

	umask_int(14);
    umask_int(28);
    usubmask_int(0);
	enable_irq();
}

extern unsigned long __bss_start__;
extern unsigned long __bss_end__;

void clear_bss()
{
	volatile unsigned long *start = &__bss_start__;
	volatile unsigned long *end = &__bss_end__;

	while (start < end)
		*start++ = 0;
}

int init_system(void)
{
	//clear_bss();
    disable_watch_dog();
    init_memory();
    create_page_table();
    init_clock();
    start_mmu();
	
	return 0;
}
