/**
 * @file            dev_ILI9341.c
 * @brief           TFT LCD ����оƬILI6341��������
 * @author          wujique
 * @date            2017��11��8�� ������
 * @version         ����
 * @par             ��Ȩ���� (C), 2013-2023
 * @par History:
 * 1.��    ��:        2017��11��8�� ������
 *   ��    ��:         wujique
 *   �޸�����:   �����ļ�
       	1 Դ����ݼ�ȸ���������С�
		2 �������ڵ�������ҵ��;�����׿��������۳��⣩��������Ȩ��
		3 �ݼ�ȸ�����Ҳ��Դ��빦�����κα�֤����ʹ�������в��ԣ�����Ը���
		4 �������޸�Դ�벢�ַ���������ֱ�����۱���������������뱣��WUJIQUE��Ȩ˵����
		5 �緢��BUG�����Ż�����ӭ�������¡�����ϵ��code@wujique.com
		6 ʹ�ñ�Դ�����൱����ͬ����Ȩ˵����
		7 ���ַ����Ȩ��������ϵ��code@wujique.com
		8 һ�н���Ȩ���ݼ�ȸ���������С�
*/
#include "stm32f10x.h"
#include "dev_ILI9341.h"

#define DEV_ILI9341_DEBUG

#ifdef DEV_ILI9341_DEBUG
#define ILI9341_DEBUG	printf 
#else
#define ILI9341_DEBUG(a, ...)
#endif

//ɨ�跽����
//BIT 0 ��ʶLR��1 R-L��0 L-R
//BIT 1 ��ʶUD��1 D-U��0 U-D
//BIT 2 ��ʶLR/UD��1 DU-LR��0 LR-DU
#define LR_BIT_MASK 0X01
#define UD_BIT_MASK 0X02
#define LRUD_BIT_MASK 0X04

#define L2R_U2D  (0) //������,���ϵ���
#define L2R_D2U  (0 + UD_BIT_MASK)//������,���µ���
#define R2L_U2D  (0 + LR_BIT_MASK) //���ҵ���,���ϵ���
#define R2L_D2U  (0 + UD_BIT_MASK + LR_BIT_MASK) //���ҵ���,���µ���

#define U2D_L2R  (LRUD_BIT_MASK)//���ϵ���,������
#define U2D_R2L  (LRUD_BIT_MASK + LR_BIT_MASK) //���ϵ���,���ҵ���
#define D2U_L2R  (LRUD_BIT_MASK + UD_BIT_MASK) //���µ���,������
#define D2U_R2L  (LRUD_BIT_MASK + UD_BIT_MASK+ LR_BIT_MASK) //���µ���,���ҵ���	 
/*
	���������壬
	�������LCD�ڲ��Դ��page��colum���������
	�����û��Ƕȵ�XY������
*/
#define TFTLCD_FRAME_W 320
#define TFTLCD_FRAME_H 240

struct _strDevLcd
{
	/*������Ҫ�ı���*/
	u8  dir;	//���������������ƣ�0��������1��������	
	u8  scandir;//ɨ�跽��
	u16 width;	//LCD ��� 
	u16 height;	//LCD �߶�
};
/*
	����һ��LCD����͸߶�Ӧ����XY���ꡣ
*/
struct _strDevLcd DevLcd={
			#if 0
			.dir = W_LCD,
			.width = TFTLCD_FRAME_W,
			.height = TFTLCD_FRAME_H,
			#else
			.dir = H_LCD,
			.width = TFTLCD_FRAME_H,
			.height = TFTLCD_FRAME_W,
			#endif
	};
/* ��һ��ת�������ṹ��ʵ��ת��Ϊָ�� */			
struct _strDevLcd *lcd = &DevLcd;

/*---------------------------------------------------------------------------*/
extern volatile u16 *LcdReg;
extern volatile u16 *LcdData;

#define COG_RST_PIN                    GPIO_Pin_15                  
#define COG_RST_GPIO_PORT              GPIOA 

#define COG_BL_PIN                    GPIO_Pin_13                  
#define COG_BL_GPIO_PORT              GPIOC 


void drv_tftlcd_io_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;


	GPIO_SetBits(COG_RST_GPIO_PORT, COG_RST_PIN);
	/* Configure PD0 and PD2 in output pushpull mode */
   	GPIO_InitStructure.GPIO_Pin = COG_RST_PIN;
   	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   	GPIO_Init(COG_RST_GPIO_PORT, &GPIO_InitStructure);

	GPIO_SetBits(COG_BL_GPIO_PORT, COG_BL_PIN);
	/* Configure PD0 and PD2 in output pushpull mode */
   	GPIO_InitStructure.GPIO_Pin = COG_BL_PIN;
   	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   	GPIO_Init(COG_BL_GPIO_PORT, &GPIO_InitStructure);
}


s32 bus_lcd_rst(u8 sta)
{
	switch(sta)
	{
		case 1:
			GPIO_SetBits(COG_RST_GPIO_PORT, COG_RST_PIN);
			break;
			
		case 0:
			GPIO_ResetBits(COG_RST_GPIO_PORT, COG_RST_PIN);
			break;
			
		default:
			return -1;

	}

	return 0;
}

/*
	bus_lcd_write_data
	bus_lcd_write_cmd
	������������ͨ��I2C���������ݻ�����͵�LCD��
	OLED��Ļֻ��I2C�͵�Դ�أ�û�����������źš�
	
*/
s32 bus_lcd_write_data(u8 *data, u32 len)
{
	u32 i;
	u16 *p;
	p = (u16 *)data;
	for(i=0; i<len; i++)
	{
		*LcdData = *(p+i);	
	}
	return 0;
}
s32 bus_lcd_w_data(u16 color, u32 len)
{
	u32 i;

	for(i=len; i>0; i--)
	{
		*LcdData = color;	
	}

	return 0;
}
s32 bus_lcd_read_data(u8 *data, u32 len)
{
	u32 i;
	
	u16 *p;
	p = (u16 *)data;
	
	for(i=0; i<len; i++)
	{
		*(p+i) = *LcdData;	
	}

	return 0;	
}

s32 bus_lcd_write_cmd(u8 cmd)
{
	*LcdReg = cmd;
	return 0;
}

s32 bus_lcd_bl(u8 sta)
{
	if(sta ==1)
	{
		GPIO_SetBits(COG_BL_GPIO_PORT, COG_BL_PIN);
	}
	else
	{
		GPIO_ResetBits(COG_BL_GPIO_PORT, COG_BL_PIN);	
	}
	return 0;
}

/*

	9341����

*/

/*9341�����*/
#define ILI9341_CMD_WRAM 0x2c
#define ILI9341_CMD_SETX 0x2a
#define ILI9341_CMD_SETY 0x2b


void drv_ILI9341_lcd_bl(u8 sta)
{
	bus_lcd_bl(sta);
}
	
/**
 *@brief:      drv_ILI9341_scan_dir
 *@details:    �����Դ�ɨ�跽�� ������Ϊ�����Ƕ�
 *@param[in]   u8 dir  
 *@param[out]  ��
 *@retval:     static
 */
static void drv_ILI9341_scan_dir(u8 dir)
{
	u16 regval=0;

	/*���ô���ߵ��ұ߻����ұߵ����*/
	switch(dir)
	{
		case R2L_U2D:
		case R2L_D2U:
		case U2D_R2L:
		case D2U_R2L:
			regval|=(1<<6); 
			break;	 
	}

	/*���ô��ϵ��»��Ǵ��µ���*/
	switch(dir)
	{
		case L2R_D2U:
		case R2L_D2U:
		case D2U_L2R:
		case D2U_R2L:
			regval|=(1<<7); 
			break;	 
	}

	/*
		���������һ��������� Reverse Mode
		�������Ϊ1��LCD�������Ѿ����и��жԵ��ˣ�
		�����Ҫ����ʾ�н��е���
	*/
	switch(dir)
	{
		case U2D_L2R:
		case D2U_L2R:
		case U2D_R2L:
		case D2U_R2L:
			regval|=(1<<5);
			break;	 
	}
	/*
		����������RGB����GBR
		���������õ�ת����
	*/	
	regval|=(1<<3);//0:GBR,1:RGB  ��R61408�෴

	bus_lcd_write_cmd(0x36);
	u16 tmp[2];
	tmp[0] = regval;
	bus_lcd_write_data((u8*)tmp, 1);


}

/**
 *@brief:      drv_ILI9341_set_cp_addr
 *@details:    ���ÿ����������е�ַ��Χ
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  ��
 *@retval:     
 */
s32 drv_ILI9341_set_cp_addr(u16 sc, u16 ec, u16 sp, u16 ep)
{
	u16 tmp[4];

	bus_lcd_write_cmd(ILI9341_CMD_SETX);
	tmp[0] = (sc>>8);
	tmp[1] = (sc&0XFF);
	tmp[2] = (ec>>8);
	tmp[3] = (ec&0XFF);
	bus_lcd_write_data((u8*)tmp, 4);

	bus_lcd_write_cmd((ILI9341_CMD_SETY));
	tmp[0] = (sp>>8);
	tmp[1] = (sp&0XFF);
	tmp[2] = (ep>>8);
	tmp[3] = (ep&0XFF);
	bus_lcd_write_data((u8*)tmp, 4);

	bus_lcd_write_cmd((ILI9341_CMD_WRAM));
	
	return 0;
}

/**
 *@brief:      drv_ILI9341_display_onoff
 *@details:    ��ʾ��ر�
 *@param[in]   u8 sta  
 *@param[out]  ��
 *@retval:     static
 */
static s32 drv_ILI9341_display_onoff(u8 sta)
{
	if(sta == 1)
		bus_lcd_write_cmd((0x29));
	else
		bus_lcd_write_cmd((0x28));

	return 0;
}

/**
 *@brief:      drv_ILI9341_init
 *@details:    ��ʼ��FSMC�����Ҷ�ȡILI9341���豸ID
 *@param[in]   void  
 *@param[out]  ��
 *@retval:     
 */
s32 drv_ILI9341_init(void)
{
	u16 data;

	u16 tmp[16];

	drv_tftlcd_io_init();
	
	bus_lcd_rst(1);
	delay(50000);
	bus_lcd_rst(0);
	delay(50000);
	bus_lcd_rst(1);
	delay(50000);

	bus_lcd_write_cmd((0x00d3));
	/*��4���ֽڣ���һ���ֽ���dummy read�� �ڶ��ֽ���0x00�� �����ֽ���93�������ֽ���41*/
	bus_lcd_read_data((u8*)tmp, 4);
	
	data = tmp[2]; 
	data<<=8;
	data |= tmp[3];

	ILI9341_DEBUG("read reg:%04x\r\n", data);

	if(data != 0x9341)
	{
		ILI9341_DEBUG("lcd drive no 9341\r\n");	

		return -1;
	}

	bus_lcd_write_cmd((0xCF));//Power control B
	tmp[0] = 0x00;
	tmp[1] = 0xC1;
	tmp[2] = 0x30;
	bus_lcd_write_data((u8*)tmp, 3);
	

	bus_lcd_write_cmd((0xED));//Power on sequence control 
	tmp[0] = 0x64;
	tmp[1] = 0x03;
	tmp[2] = 0x12;
	tmp[3] = 0x81;
	bus_lcd_write_data((u8*)tmp, 4);

	bus_lcd_write_cmd((0xE8));//Driver timing control A
	tmp[0] = 0x85;
	//tmp[1] = 0x01;
	tmp[1] = 0x10;
	tmp[2] = 0x7A;
	bus_lcd_write_data((u8*)tmp, 3);

	bus_lcd_write_cmd((0xCB));//Power control 
	tmp[0] = 0x39;
	tmp[1] = 0x2C;
	tmp[2] = 0x00;
	tmp[3] = 0x34;
	tmp[4] = 0x02;
	bus_lcd_write_data((u8*)tmp, 5);

	bus_lcd_write_cmd((0xF7));//Pump ratio control
	tmp[0] = 0x20;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0xEA));//Driver timing control
	tmp[0] = 0x00;
	tmp[1] = 0x00;
	bus_lcd_write_data((u8*)tmp, 2);

	bus_lcd_write_cmd((0xC0));
	tmp[0] = 0x1B;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0xC1));
	tmp[0] = 0x01;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0xC5));
	tmp[0] = 0x30;
	tmp[1] = 0x30;
	bus_lcd_write_data((u8*)tmp, 2);

	bus_lcd_write_cmd((0xC7));
	tmp[0] = 0xB7;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0x36));
	tmp[0] = 0x48;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0x3A));
	tmp[0] = 0x55;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0xB1));
	tmp[0] = 0x00;
	tmp[1] = 0x1A;
	bus_lcd_write_data((u8*)tmp, 2);

	bus_lcd_write_cmd((0xB6));
	tmp[0] = 0x0A;
	tmp[1] = 0xA2;
	bus_lcd_write_data((u8*)tmp, 2);

	bus_lcd_write_cmd((0xF2));
	tmp[0] = 0x00;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0x26));
	tmp[0] = 0x01;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0xE0));
	tmp[0] = 0x0F; tmp[1] = 0x2A; tmp[2] = 0x28; tmp[3] = 0x08;
	tmp[4] = 0x0E; tmp[5] = 0x08; tmp[6] = 0x54; tmp[7] = 0xa9;
	tmp[8] = 0x43; tmp[9] = 0x0a; tmp[10] = 0x0F; tmp[11] = 0x00;
	tmp[12] = 0x00; tmp[13] = 0x00; tmp[14] = 0x00;
	bus_lcd_write_data((u8*)tmp, 15);

	bus_lcd_write_cmd((0XE1));
	tmp[0] = 0x00; tmp[1] = 0x15; tmp[2] = 0x17; tmp[3] = 0x07;
	tmp[4] = 0x11; tmp[5] = 0x06; tmp[6] = 0x2B; tmp[7] = 0x56;
	tmp[8] = 0x3C; tmp[9] = 0x05; tmp[10] = 0x10; tmp[11] = 0x0F;
	tmp[12] = 0x3F; tmp[13] = 0x3F; tmp[14] = 0x0F;
	bus_lcd_write_data((u8*)tmp, 15);	

	bus_lcd_write_cmd((0x2B));
	tmp[0] = 0x00; tmp[1] = 0x00; tmp[2] = 0x01; tmp[3] = 0x3f;
	bus_lcd_write_data((u8*)tmp, 4);

	bus_lcd_write_cmd((0x2A));
	tmp[0] = 0x00; tmp[1] = 0x00; tmp[2] = 0x00; tmp[3] = 0xef;
	bus_lcd_write_data((u8*)tmp, 4);

	bus_lcd_write_cmd((0x11));
	delay(12000);
	bus_lcd_write_cmd((0x29));

	delay(5000);
	
	return 0;
}
/**
 *@brief:      drv_ILI9341_xy2cp
 *@details:    ��xy����ת��ΪCP����
 *@param[in]   ��
 *@param[out]  ��
 *@retval:     
 */
s32 drv_ILI9341_xy2cp(u16 sx, u16 ex, u16 sy, u16 ey, u16 *sc, u16 *ec, u16 *sp, u16 *ep)
{
	/*
		��ʾXY�᷶Χ
	*/
	if(sx >= lcd->width)
		sx = lcd->width-1;
	
	if(ex >= lcd->width)
		ex = lcd->width-1;
	
	if(sy >= lcd->height)
		sy = lcd->height-1;
	
	if(ey >= lcd->height)
		ey = lcd->height-1;
	/*
		XY�ᣬʵ��Ƕ�����������ȡ���ں�����������
		CP�ᣬ�ǿ������Դ�Ƕȣ�
		XY���ӳ���ϵȡ����ɨ�跽��
	*/
	if(
		(((lcd->scandir&LRUD_BIT_MASK) == LRUD_BIT_MASK)
		&&(lcd->dir == H_LCD))
		||
		(((lcd->scandir&LRUD_BIT_MASK) == 0)
		&&(lcd->dir == W_LCD))
		)
		{
			*sc = sy;
			*ec = ey;
			*sp = sx;
			*ep = ex;
		}
	else
	{
		*sc = sx;
		*ec = ex;
		*sp = sy;
		*ep = ey;
	}
	
	return 0;
}
/**
 *@brief:      drv_ILI9341_drawpoint
 *@details:    ����
 *@param[in]   u16 x      
               u16 y      
               u16 color  
 *@param[out]  ��
 *@retval:     static
 */
s32 drv_ILI9341_drawpoint( u16 x, u16 y, u16 color)
{
	u16 sc,ec,sp,ep;

	drv_ILI9341_xy2cp(x, x, y, y, &sc,&ec,&sp,&ep);
	drv_ILI9341_set_cp_addr(sc, ec, sp, ep);

	u16 tmp[2];
	tmp[0] = color;
	bus_lcd_write_data((u8*)tmp, 1);

	return 0;
}
/**
 *@brief:      drv_ILI9341_color_fill
 *@details:    ��һ�������趨Ϊĳ����ɫ
 *@param[in]   u16 sx     
               u16 sy     
               u16 ex     
               u16 ey     
               u16 color  
 *@param[out]  ��
 *@retval:     
 */
s32 drv_ILI9341_color_fill(u16 sx,u16 ex,u16 sy,u16 ey,u16 color)
{

	u16 height,width;
	u16 i,j;
	u16 hsa,hea,vsa,vea;

	drv_ILI9341_xy2cp(sx, ex, sy, ey, &hsa,&hea,&vsa,&vea);
	drv_ILI9341_set_cp_addr(hsa, hea, vsa, vea);

	width = hea - hsa + 1;//�õ����Ŀ��
	height = vea - vsa + 1;//�߶�
	
	//uart_printf("ili9325 width:%d, height:%d\r\n", width, height);
	
	u32 cnt;
	
	cnt = height*width;
	
	bus_lcd_w_data(color, cnt);

	return 0;

}

/**
 *@brief:      drv_ILI9341_color_fill
 *@details:    ����������
 *@param[in]   u16 sx      
               u16 sy      
               u16 ex      
               u16 ey      
               u16 *color  ÿһ�������ɫ����
 *@param[out]  ��
 *@retval:     
 */
s32 drv_ILI9341_fill(u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{

	u16 height,width;
	u32 i,j;
	u16 sc,ec,sp,ep;

	drv_ILI9341_xy2cp(sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	drv_ILI9341_set_cp_addr(sc, ec, sp, ep);

	width=(ec+1)-sc;
	height=(ep+1)-sp;

	printf("fill width:%d, height:%d\r\n", width, height);
	
	bus_lcd_write_data((u8 *)color, height*width);	
 
	return 0;

} 

s32 drv_ILI9341_prepare_display(u16 sx, u16 ex, u16 sy, u16 ey)
{
	u16 sc,ec,sp,ep;
	
	printf("XY:-%d-%d-%d-%d-\r\n", sx, ex, sy, ey);
	drv_ILI9341_xy2cp(sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	
	printf("cp:-%d-%d-%d-%d-\r\n", sc, ec, sp, ep);
	drv_ILI9341_set_cp_addr(sc, ec, sp, ep);	
	return 0;
}

s32 drv_ILI9341_flush(u16 *color, u32 len)
{
	bus_lcd_write_data((u8 *)color,  len);	

	return 0;
} 
s32 drv_ILI9341_update(void)
{
	return 0;	
}

/*---------------------------------------------------------------------------*/
#include "font.h"
/*

	��ʾ�ӿ�

*/
/**
 *@brief:	   line
 *@details:    ��һ����
 *@param[in]   int x1			
			   int y1			
			   int x2			
			   int y2			
			   unsigned colidx	
 *@param[out]  ��
 *@retval:	   
 */
void line (int x1, int y1, int x2, int y2, unsigned colidx)
{
	int tmp;
	int dx = x2 - x1;
	int dy = y2 - y1;

	if (abs (dx) < abs (dy)) 
	{
		if (y1 > y2) 
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		x1 <<= 16;
		/* dy is apriori >0 */
		dx = (dx << 16) / dy;
		while (y1 <= y2)
		{
			drv_ILI9341_drawpoint( x1 >> 16, y1, colidx);
			x1 += dx;
			y1++;
		}
	} 
	else 
	{
		if (x1 > x2) 
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		
		y1 <<= 16;
		dy = dx ? (dy << 16) / dx : 0;
		while (x1 <= x2) 
		{
			drv_ILI9341_drawpoint( x1, y1 >> 16, colidx);
			y1 += dy;
			x1++;
		}
	}
}

/**
 *@brief:	  put_cross
 *@details:   ��ʮ��
 *@param[in]  int x 		   
			  int y 		   
			  unsigned colidx  
 *@param[out]  ��
 *@retval:	   
 */
void put_cross(int x, int y, unsigned colidx)
{
	
	line ( x - 10, y, x - 2, y, colidx);
	line ( x + 2, y, x + 10, y, colidx);
	line ( x, y - 10, x, y - 2, colidx);
	line ( x, y + 2, x, y + 10, colidx);

	line ( x - 6, y - 9, x - 9, y - 9, colidx + 1);
	line ( x - 9, y - 8, x - 9, y - 6, colidx + 1);
	line ( x - 9, y + 6, x - 9, y + 9, colidx + 1);
	line ( x - 8, y + 9, x - 6, y + 9, colidx + 1);
	line ( x + 6, y + 9, x + 9, y + 9, colidx + 1);
	line ( x + 9, y + 8, x + 9, y + 6, colidx + 1);
	line ( x + 9, y - 6, x + 9, y - 9, colidx + 1);
	line ( x + 8, y - 9, x + 6, y - 9, colidx + 1);

}

/**
 *@brief:	   put_char
 *@details:    ��ʾһ��Ӣ��
 *@param[in]   int x	   
			   int y	   
			   int c	   
			   int colidx  
 *@param[out]  ��
 *@retval:	   
 */
void put_char(int x, int y, int c, int colidx)
{
	int i,j,bits;
	u8* p;
	

	p = (u8*)font_vga_8x8.path;//need fix
	for (i = 0; i < font_vga_8x8.height; i++) 
	{
		bits =	p[font_vga_8x8.height * c + i];
		for (j = 0; j < font_vga_8x8.width; j++, bits <<= 1)
		{
			if (bits & 0x80)
			{
				drv_ILI9341_drawpoint(x + j, y + i, colidx);
			}
		}
	}
}
/**
 *@brief:	   put_string
 *@details:    ��ʾһ���ַ���
 *@param[in]   int x			
			   int y			
			   char *s			
			   unsigned colidx	
 *@param[out]  ��
 *@retval:	   
 */
void put_string(int x, int y, char *s, unsigned colidx)
{
	int i;
	
	for (i = 0; *s; i++, x += font_vga_8x8.width, s++)
		put_char(x, y, *s, colidx);
}


void dev_lcd_init(void)
{
	drv_ILI9341_scan_dir(L2R_U2D);
	drv_ILI9341_display_onoff(1);
	drv_ILI9341_color_fill(0, lcd->width, 0, lcd->height, WHITE);
	drv_ILI9341_update();
	drv_ILI9341_color_fill(0, lcd->width, 0, lcd->height, BLUE);
	drv_ILI9341_update();	
}

/*
	���Գ���
*/
void dev_tftlcd_test(void)
{
	/* ��Ҫ�����������ɨ�跽�� */
	drv_ILI9341_scan_dir(L2R_U2D);
	drv_ILI9341_display_onoff(1);
	drv_ILI9341_color_fill(0, lcd->width, 0, lcd->height, RED);
	drv_ILI9341_update();
	drv_ILI9341_color_fill(0, lcd->width, 0, lcd->height, GREEN);
	drv_ILI9341_update();
	drv_ILI9341_color_fill(0, lcd->width, 0, lcd->height, BLUE);
	drv_ILI9341_update();
	put_string(0,0, "Hello word!", BLACK);
}



