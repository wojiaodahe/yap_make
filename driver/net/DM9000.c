#include "DM9000.h"
#include "s3c24xx.h"
#include "kernel.h"
#include "common.h"
#include "netdevice.h"
#include "socket.h"
#include "printk.h"
#include "lib.h"
#include "interrupt.h"

void dm9000_tx_done(struct net_device *ndev);

#define DM_ADD (*((volatile unsigned short *) 0x20000300))
#define DM_CMD (*((volatile unsigned short *) 0x20000304))


#define DM9KS_ID	0x90000A46
#define DM9KS_VID_L	0x28
#define DM9KS_VID_H	0x29
#define DM9KS_PID_L	0x2A
#define DM9KS_PID_H	0x2B


void udelay(unsigned int t)
{
	unsigned int i;
	for(;t>0;t--)
	{
		for(i=0;i<100;i++){}
	}
}
void Test_DM9000AE()
{
	unsigned int id_val;

	BWSCON = (BWSCON & ~(0x03 << 16)) | (0x01 << 16);
	BANKCON4 = (1 << 13) | (2 << 11) | (7 << 8) | (2 << 6) | (1 << 4) | (1 << 2) | 0;

	id_val = 0;
	DM_ADD = DM9KS_VID_L;
	id_val = (unsigned char)DM_CMD;
	DM_ADD = DM9KS_VID_H;
	id_val |= (unsigned char)DM_CMD<<8;
	DM_ADD = DM9KS_PID_L;udelay(2);
	id_val |= (unsigned char)DM_CMD<<16;
	DM_ADD = DM9KS_PID_H;udelay(2);
	id_val |= (unsigned char)DM_CMD<<24;
	printk("DM9000AE ChipId is %x\r\n", id_val);
	if(id_val == DM9KS_ID)
	{
		printk("DM9000 ID ��ȷ\r\n");
	}
	else
	{
		printk("DM9000 ID ����\r\n");
	}

}

//��DM9000�Ĵ���д����
void dm9000_reg_write(unsigned short reg, unsigned short data)
{  
	udelay(20);		//֮ǰ�����΢���ʱ������������ʱ20us
	DM_ADD = reg;	//���Ĵ�����ַд��INDEX�˿�
	udelay(20);
	DM_CMD = data;	//������д��DATA�˿ڣ���д���Ĵ���

}

//��DM9000�Ĵ���������
unsigned short dm9000_reg_read(unsigned short reg)
{
	udelay(20);
	DM_ADD = reg;
	udelay(20);
	return DM_CMD;//�����ݴӼĴ����ж���	
}
//��DM9000PHY�Ĵ���д����
void dm9000_reg_writePHY(unsigned short reg, unsigned short data)
{
	//�Ĵ�����ַд��EPAR/PHY_AR��0CH���Ĵ�����,
	//ע�⽫�Ĵ�����ַ�ĵ�6λ��1����ַ��0x40�����㼴�ɣ���
	///�Ա���д����PHY��ַ��������EEPROM��ַ
	dm9000_reg_write(DM9000_EPAR, reg|0x40);
	//�����ݸ��ֽ�д��PHY_DRH��0EH���Ĵ�����
	dm9000_reg_write(DM9000_EPDRH, (data>>8)&0xff);
	//�����ݵ��ֽ�д��PHY_DRL��0DH���Ĵ�����
	dm9000_reg_write(DM9000_EPDRL, data&0xff);
	//����PHYд����(0x0a����EPCR/PHY_CR��0BH���Ĵ�����	
	dm9000_reg_write(DM9000_EPCR, 0x0a);
	//��ʱ20us����������0x08��EPCR/PHY_CR��0BH���Ĵ����У����PHY������
	udelay(20);
	dm9000_reg_write(DM9000_EPCR, 0x08);
}
//��DM9000PHY�Ĵ���������
unsigned short dm9000_reg_readPHY(unsigned short reg)
{
	unsigned short data;
	//�Ĵ�����ַд��EPAR/PHY_AR��0CH���Ĵ�����,
	//ע�⽫�Ĵ�����ַ�ĵ�6λ��1����ַ��0x40�����㼴�ɣ���
	///�Ա���д����PHY��ַ��������EEPROM��ַ
	dm9000_reg_write(DM9000_EPAR, reg|0x40);
	//����PHY������(0x0c����EPCR/PHY_CR��0BH���Ĵ�����	
	dm9000_reg_write(DM9000_EPCR, 0x0c);
	//�����ݸ��ֽڴ�PHY_DRH��0EH���Ĵ����ж���
	data = dm9000_reg_read(DM9000_EPDRH);
	//�����ݵ��ֽڴ�PHY_DRL��0DH���Ĵ����ж���
	data = (data<<8) | dm9000_reg_read(DM9000_EPDRL);
	//��ʱ20us����������0x08��EPCR/PHY_CR��0BH���Ĵ����У����PHY������
	udelay(20);
	dm9000_reg_write(DM9000_EPCR, 0x08);
	return data;
}

static void int_issue(void *priv)
{
	unsigned int len;
    unsigned char status; 

    status = dm9000_reg_read(DM9000_ISR);
    dm9000_reg_write(DM9000_ISR, status);	//��������жϱ�־λ
    
    if (status & 0x02)
        dm9000_tx_done((struct net_device *)priv);
   
    if (status & 0x01)
	{
        do 
        {
            len = receivepacket(priv);
        }
        while (len > 0);
	}
	//ClearPending(BIT_EINT4_7);
	//EINTPEND |= 1 << 7;
}

int DM9000_sendPcket(struct sk_buff *skb, struct net_device *ndev);
struct net_device dm9000_dev =
{
	.ip 			 = 0xc0a8080a,
	.netmask		 = 0xffffff00,
	.gw  			 = 0x01020301,
	.mtu			 = 1514,
	.hard_start_xmit = DM9000_sendPcket,
};

void IOSetInit(void)
{
	//EINT7
	GPFCON  &= ~(3 << 14);
	GPFCON  |= 2 << 14; //Set GPF7 To EINT7 Mode

	EXTINT0 = (EXTINT0 & (~(0x07 << 28))) | (0x01 << 28);//??EINT7??????
	EINTMASK &= ~(1 << 7);//??EINT7
	SRCPND = SRCPND | (0x1 << 4);
	INTPND = INTPND | (0x1 << 4);
	request_irq(35, int_issue, 0, &dm9000_dev);
	INTMSK &= ~(1 << 4);
}

void DM9000_init(char *mac_addr)
{
	unsigned int i;
	Test_DM9000AE();
	IOSetInit();
	//��ʼ�����ò���: 0
	dm9000_reg_write(DM9000_GPCR, 0x01);	//���� GPCR(1EH) bit[0]=1��ʹDM9000��GPIO0Ϊ�����
	dm9000_reg_write(DM9000_GPR,  0x00);	//GPR bit[0]=0 ʹDM9000��GPIO3���Ϊ���Լ����ڲ�PHY��
	udelay(5000);							//��ʱ2ms���ϵȴ�PHY�ϵ硣
	///��ʼ�����ò���: 1
	//	dm9000_reg_write(DM9000_NCR,  0x80);	//����NCR[7]=1,�����ⲿPHY
	//	udelay(5000);							//��ʱ2ms���ϵȴ�PHY�ϵ�
	//��ʼ�����ò���: 2
	dm9000_reg_write(DM9000_NCR,  0x03);	//������λ
	udelay(30);								//��ʱ20us���ϵȴ�������λ���
	dm9000_reg_write(DM9000_NCR,  0x00);	//��λ��ɣ�������������ģʽ��
	dm9000_reg_write(DM9000_NCR,  0x03);	//�ڶ���������λ��Ϊ��ȷ��������λ��ȫ�ɹ����˲����Ǳ�Ҫ�ġ�
	udelay(30);
	dm9000_reg_write(DM9000_NCR,  0x00);
	//��ʼ�����ò���: 3
	dm9000_reg_write(DM9000_NSR,  0x2c);	//�������״̬��־λ
	dm9000_reg_write(DM9000_ISR,  0x3f);	//��������жϱ�־λ
	//��ʼ�����ò���: 4
	dm9000_reg_write(DM9000_RCR,  0x39);	//���տ���
	dm9000_reg_write(DM9000_TCR,  0x00);	//���Ϳ���
	dm9000_reg_write(DM9000_BPTR, 0x3f);	//��ѹ����
	dm9000_reg_write(DM9000_FCTR, 0x3a);	//����FIFO����3k 8k
	dm9000_reg_write(DM9000_FCR,  0xff);	//����/�������
	dm9000_reg_write(DM9000_SMCR, 0x00);	//����ģʽ
	//��ʼ�����ò���: 5
	for(i = 0; i < 6; i++)
		dm9000_reg_write(DM9000_PAR + i, mac_addr[i]);//mac_addr[]�Լ�����һ�°ɣ�6���ֽڵ�MAC��ַ

	//��ʼ�����ò���: 6
	dm9000_reg_write(DM9000_NSR,  0x2c);	//�������״̬��־λ
	dm9000_reg_write(DM9000_ISR,  0x3f);	//��������жϱ�־λ
	//��ʼ�����ò���: 7
	dm9000_reg_write(DM9000_IMR, 0x83);		//�ж�ʹ��	

}

unsigned int queue_pkt_len = 0;
unsigned int tx_pkt_cnt    = 0; 

void dm9000_tx_done(struct net_device *ndev)
{
    int tx_status;

    tx_status = dm9000_reg_read(DM9000_NSR);

    if (tx_status & ((1 << 2) | (1 << 3)))
    {
        tx_pkt_cnt--;
        if (tx_pkt_cnt > 0)
        {
            dm9000_reg_write(DM9000_TXPLH, (queue_pkt_len >> 8) & 0x0ff);
            dm9000_reg_write(DM9000_TXPLL, queue_pkt_len & 0x0ff);
            dm9000_reg_write(DM9000_TCR, 0x01);		/* �������ݵ���̫���� */
        }
        
        netif_wake_queue(ndev);
    }
    else
    {
        tx_status = dm9000_reg_read(DM9000_TSR1);
        printk("tx_status tsr1: %x\n", tx_status);
        tx_status = dm9000_reg_read(DM9000_TSR2);
        printk("tx_status tsr2: %x\n", tx_status);
    }
}

int DM9000_sendPcket(struct sk_buff *skb, struct net_device *ndev)
{
	unsigned int len,i;
	unsigned char tmp;
    char *datas;

	datas = skb->data_buf;
	len = skb->data_len;							//�ѷ��ͳ���д��

    if (tx_pkt_cnt > 1)
    {
	    tmp = dm9000_reg_read(DM9000_IMR);		//�Ƚ�ֹ�����жϣ���ֹ�ڷ�������ʱ���жϸ���	
        printk("imr: %x\n", tmp);
        return NETDEV_TX_BUSY;
    }

    enter_critical();
    
    tx_pkt_cnt++;
	DM_ADD = DM9000_MWCMD;					//�洢������ַ�Զ����ӵĶ���������
	for(i=0; i<len; i+=2)					//16 bit mode
	{
		udelay(2);
		DM_CMD = datas[i] | (datas[i+1]<<8);
	}
    if (tx_pkt_cnt == 1)
    {
        dm9000_reg_write(DM9000_TXPLH, (len>>8) & 0x0ff);
        dm9000_reg_write(DM9000_TXPLL, len & 0x0ff);
	    dm9000_reg_write(DM9000_TCR, 0x01);		//�������ݵ���̫����
    }
    else
    {
        queue_pkt_len = len;
        netif_stop_queue(ndev);
    }
    
    exit_critical();

	free_skbuff(skb);

	return 0;
}

unsigned int receivepacket(void *priv)
{
    struct sk_buff *skb;
    char *data;
	unsigned short i, tmp, status, len;
    unsigned char GoodPacket;
	unsigned char ready;
	ready = 0;								//ϣ����ȡ��"01H"
	status = 0;								//���ݰ�״̬
	len = 0; 								//���ݰ�����
    
        ready = dm9000_reg_read(DM9000_MRCMDX); // ��һ�ζ�ȡ��һ���ȡ������ 00H
        if((ready & 0x0ff) != 0x01)
        {
            ready = dm9000_reg_read(DM9000_MRCMDX); // �ڶ��ζ�ȡ�����ܻ�ȡ������
            if((ready & 0x01) != 0x01)
            {
                /*
                   if((ready & 0x01) != 0x00) 		//���ڶ��ζ�ȡ���Ĳ��� 01H �� 00H �����ʾû�г�ʼ���ɹ�
                   {
                   dm9000_reg_write(DM9000_IMR, 0x80);//���������ж�
                   DM9000_init();				//���³�ʼ��
                   dm9000_reg_write(DM9000_IMR, 0x81);//�������ж�
                   }
                   return 0;
                   */
                return 0;
            }
        }

        GoodPacket = 1;
        status = dm9000_reg_read(DM9000_MRCMD);
        len =  DM_CMD;

        status = status >> 8;
        if (status & 0xbf)
        {
            GoodPacket = 0;
            if (status & 0x01) 
                printk("<RX FIFO error>\n");
            if (status & 0x02) 
                printk("<RX CRC error>\n");
            if (status & 0x80) 
                printk("<RX Length error>\n");
            if (status & 0x08)
                printk("<Physical Layer error>\n");
            for (i = 0; i < len; i += 2)
                len = DM_CMD;
            return len;
        }

        if((len <= 1522) && GoodPacket)//!(status & 0xbf) &&
        {
            skb = alloc_skbuff(len);
            if (!skb)
            {
                printk("No More SK_Buff %s\n", __func__);
                panic();
            }
            skb->ndev = priv;
            skb->data_len = len;
            data = skb->data_buf;

            for(i = 0; i < len; i += 2)// 16 bit mode
            {
                //udelay(20);
                tmp = DM_CMD;
                data[i] = tmp & 0x0ff;
                data[i + 1] = (tmp >> 8) & 0x0ff;
            }
            
            netif_rx(skb);
        }
        else
        {
            printk("Recv A Big Packet Length: %d\n", len);
            panic();
            return 0;
        }
#if 0
        printk("recv len: %d\n", len);
        for (i = 0; i < len; i++)
            printk("%x ", data[i]);
        printk("\n");	
#endif
        return len;
}



void printkDM9000Reg(void)
{
	/*	unsigned short data,i;
		for(i=0x0;i<0x10;i++)
		{
		data = dm9000_reg_read(i);
		printk("RET_0x%02x = %x\r\n",i,data);
		}
		printk("\r\n");
		for(i=0x10;i<0x16;i++)
		{
		data = dm9000_reg_read(i);
		printk("RET_0x%02x = %02x\r\n",i,data);
		}

		i=0x16;
		data = dm9000_reg_read(i);
		printk("RET_0x%2x = %x\r\n",i,data);

		for(i=0x1e;i<0x26;i++)
		{
		data = dm9000_reg_read(i);
		printk("RET_0x%2x = %x\r\n",i,data);
		}

		for(i=0x28;i<0x2c;i++)
		{
		data = dm9000_reg_read(i);
		printk("RET_0x%2x = 0x%x\r\n",i,data);
		}

		i=0x2f;
		data = dm9000_reg_read(i);
		printk("RET_0x%2x = %x\r\n",i,data);

		for(i=0xf0;i<0xff;i++)
		{
		data = dm9000_reg_read(i);
		printk("RET_0x%2x = %x\r\n",i,data);
		}
		data = dm9000_reg_read(i);
		printk("RET_0x%2x = %x\r\n",i,data);

		data = dm9000_reg_read(0xfe);
		printk("������IOģʽ:%x\r\n",((unsigned char)data)>>6);

		data = dm9000_reg_read(0x01);
		if(((unsigned char)data)>>1)
		printk("10M��̫��\r\n");
		else
		printk("100M��̫��\r\n");

		data = dm9000_reg_read(0x02);
		printk("TCR��02H�������Ϳ��ƼĴ���:%x\r\n",((unsigned char)data));

		TestDm9000();
		*/
	unsigned char val, i;
	unsigned short data;
	printk("Registers:\n");
	val = dm9000_reg_read(DM9000_NCR);
	printk("NCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_NSR);
	printk("NSR = %02x(%03d)    ", val, val);
	val = dm9000_reg_read(DM9000_TCR);
	printk("TCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_RCR);
	printk("RCR = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_RSR);
	printk("RSR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_ROCR);
	printk("ROCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_BPTR);
	printk("BPTR = %02x(%03d)  ", val, val);
	val = dm9000_reg_read(DM9000_FCTR);
	printk("FCTR = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_FCR);
	printk("FCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_GPCR);
	printk("GPCR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_GPR);
	printk("GPR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_WCR);
	printk("WCR = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_TRPAL);
	printk("TRPAL = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_TRPAH);
	printk("TRPAH = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_RWPAL);
	printk("RWPAL = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_RWPAH);
	printk("RWPAH = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_VIDL);
	printk("VIDL  = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_VIDH);
	printk("VIDH  = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_PIDL);
	printk("PIDL  = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_PIDH);
	printk("PIDH  = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_CHIPR);
	printk("CHIPR = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_SMCR);
	printk("SMCR  = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_TXPLL);
	printk("TXPLL = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_TXPLH);
	printk("TXPLH = %02x(%03d) \n", val, val);

	val = dm9000_reg_read(DM9000_ISR);
	printk("ISR   = %02x(%03d)   ", val, val);
	val = dm9000_reg_read(DM9000_IMR);
	printk("IMR   = %02x(%03d) \n", val, val);

	printk("MAC = ");
	for(i = 0; i < 5; i++)
	{
		val = dm9000_reg_read(DM9000_PAR + i);
		printk(" %02x(%03d): ", val, val);
	}
	val = dm9000_reg_read(DM9000_PAR + i);
	printk(" %02x(%03d)\n ", val, val);
	///////////////////////////////

	printk("PHY Registers:\n");
	data = dm9000_reg_readPHY(DM9000_BMCR);
	printk(" BMCR   = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_BMSR);
	printk("BMSR   = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_PHYID1);
	printk("PHYID1 = %04x(%05d)\n ", data, data);

	data = dm9000_reg_readPHY(DM9000_PHYID2);
	printk("PHYID2 = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_ANAR);
	printk("ANAR   = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_DSCR);
	printk("DSCR   = %04x(%05d)\n ", data, data);

	data = dm9000_reg_readPHY(DM9000_ANLPAR);
	printk("ANLPAR = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_ANER);
	printk("ANER   = %04x(%05d) ", data, data);
	data = dm9000_reg_readPHY(DM9000_DSCSR);
	printk("DSCSR  = %04x(%05d)\n ", data, data);

	data = dm9000_reg_readPHY(DM9000_10BTCSR);
	printk("BTCSR  = %04x(%05d)\n ", data, data);

	//TestDm9000();
}

void TestDm9000(void)
{
	//DM9000_sendPcket(arpsendbuf,60);
	//arp_request();
}

void testNetwork(void)
{
	unsigned char dat;
	while(1)
	{
		dat =  dm9000_reg_read(DM9000_NSR);
		if(dat&0x40)
			break;
	}
}

int dm9000_module_init(void)
{
	char mac[6] = {0x00, 0x1c, 0x82, 0x00, 0x33, 0x1f};
	DM9000_init(mac);
	memcpy(dm9000_dev.macaddr, mac, 6);
    INIT_LIST_HEAD(&dm9000_dev.tx_queue);
	register_netdev(&dm9000_dev);
	return 0;
}

void dm9000_mdule_exit()
{

}






