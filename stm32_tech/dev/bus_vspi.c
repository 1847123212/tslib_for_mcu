/**
 * @file            bus_spi.c
 * @brief           SPI���߹���
 * @author          test
 * @date            2019��03��26�� ������
 * @version         ����
 * @par             
 * @par History:
 * 1.��    ��:      
 *   ��    ��:      test
 *   �޸�����:      �����ļ�
		��Ȩ˵����
		1 Դ����ݼ�ȸ���������С�
		2 �������ڵ�������ҵ��;�����׿��������۳��⣩��������Ȩ��
		3 �ݼ�ȸ�����Ҳ��Դ��빦�����κα�֤����ʹ�������в��ԣ�����Ը���
		4 �������޸�Դ�벢�ַ���������ֱ�����۱�������������ұ�����Ȩ˵����
		5 �緢��BUG�����Ż�����ӭ�������¡�����ϵ��code@wujique.com
		6 ʹ�ñ�Դ�����൱����ͬ����Ȩ˵����
		7 ���ַ����Ȩ��������ϵ��code@wujique.com
		8 һ�н���Ȩ���ݼ�ȸ���������С�
*/
#include "stm32f10x.h"
#include "bus_vspi.h"


#define BUS_VSPI_DEBUG

#ifdef BUS_VSPI_DEBUG
#define VSPI_DEBUG	printf 
#else
#define VSPI_DEBUG(a, ...)
#endif

#define VSPI_MOSI_PORT 	GPIOC
#define VSPI_MOSI_PIN 	GPIO_Pin_5

#define VSPI_MISO_PORT 	GPIOC
#define VSPI_MISO_PIN 	GPIO_Pin_4

#define VSPI_CLK_PORT 	GPIOB
#define VSPI_CLK_PIN 	GPIO_Pin_0

#define VSPI_CS_PORT 	GPIOB
#define VSPI_CS_PIN 	GPIO_Pin_1

/**
 *@brief:      mcu_vspi_init
 *@details:    ��ʼ������SPI
 *@param[in]   void  
 *@param[out]  ��
 *@retval:     
 */
s32 mcu_vspi_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	printf( "vspi init\r\n");

	GPIO_SetBits(VSPI_MOSI_PORT, VSPI_MOSI_PIN);
	/* Configure PD0 and PD2 in output pushpull mode */
   	GPIO_InitStructure.GPIO_Pin = VSPI_MOSI_PIN;
   	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   	GPIO_Init(VSPI_MOSI_PORT, &GPIO_InitStructure);

	GPIO_SetBits(VSPI_MISO_PORT, VSPI_MISO_PIN);
	/* Configure PD0 and PD2 in output pushpull mode */
   	GPIO_InitStructure.GPIO_Pin = VSPI_MISO_PIN;
   	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
   	GPIO_Init(VSPI_MISO_PORT, &GPIO_InitStructure);

	GPIO_SetBits(VSPI_CLK_PORT, VSPI_CLK_PIN);
	/* Configure PD0 and PD2 in output pushpull mode */
   	GPIO_InitStructure.GPIO_Pin = VSPI_CLK_PIN;
   	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   	GPIO_Init(VSPI_CLK_PORT, &GPIO_InitStructure);

	GPIO_SetBits(VSPI_CS_PORT, VSPI_CS_PIN);
	/* Configure PD0 and PD2 in output pushpull mode */
   	GPIO_InitStructure.GPIO_Pin = VSPI_CS_PIN;
   	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   	GPIO_Init(VSPI_CS_PORT, &GPIO_InitStructure);
	
	return 0;
}


/**
 *@brief:      vspi_delay
 *@details:    ����SPIʱ����ʱ
 *@param[in]   u32 delay  
 *@param[out]  ��
 *@retval:     
 */
void vspi_delay(u32 delay)
{
	volatile u32 i=delay;

	while(i>0)
	{
		i--;	
	}

}

u32 VspiDelay = 0;


/**
 *@brief:      mcu_vspi_open
 *@details:    ������SPI
 *@param[in]   SPI_DEV dev    
               SPI_MODE mode  
               u16 pre        
 *@param[out]  ��
 *@retval:     
 */
s32 mcu_vspi_open(SPI_MODE mode, u16 khz)
{
	VspiDelay = khz;
	GPIO_ResetBits(VSPI_CS_PORT, VSPI_CS_PIN);
    return 0;
}
/**
 *@brief:      mcu_vspi_close
 *@details:    �ر�����SPI
 *@param[in]   SPI_DEV dev  
 *@param[out]  ��
 *@retval:     
 */
s32 mcu_vspi_close(void)
{
	GPIO_SetBits(VSPI_CS_PORT, VSPI_CS_PIN);
    return 0;
}
/**
 *@brief:      mcu_vspi_transfer
 *@details:       ����SPIͨ��
 *@param[in]   SPI_DEV dev  
               u8 *snd      
               u8 *rsv      
               s32 len      
 *@param[out]  ��
 *@retval:     

 		node->clk = 0, CLKʱ��1.5M 2018.06.02
 */
s32 mcu_vspi_transfer(u8 *snd, u8 *rsv, s32 len)
{
	u8 i;
	u8 data;
	s32 slen;
	u8 misosta;

	volatile u16 delay;
	
    if( ((snd == NULL) && (rsv == NULL)) || (len < 0) )
    {
        return -1;
    }

	slen = 0;

	while(1)
	{
		if(slen >= len)
			break;

		if(snd == NULL)
			data = 0xff;
		else
			data = *(snd+slen);
		
		for(i=0; i<8; i++)
		{
			GPIO_ResetBits(VSPI_CLK_PORT, VSPI_CLK_PIN);
			delay = VspiDelay;
			while(delay>0)
			{
				delay--;	
			}
			
			if(data&0x80)
				GPIO_SetBits(VSPI_MOSI_PORT, VSPI_MOSI_PIN);
			else
				GPIO_ResetBits(VSPI_MOSI_PORT, VSPI_MOSI_PIN);
			
			delay = VspiDelay;
			while(delay>0)
			{
				delay--;	
			}
			
			data<<=1;
			GPIO_SetBits(VSPI_CLK_PORT, VSPI_CLK_PIN);
			
			delay = VspiDelay;
			while(delay>0)
			{
				delay--;	
			}
			
			misosta = GPIO_ReadInputDataBit(VSPI_MISO_PORT, VSPI_MISO_PIN);
			if(misosta == Bit_SET)
			{
				data |=0x01;
			}
			else
			{
				data &=0xfe;
			}
			
			delay = VspiDelay;
			while(delay>0)
			{
				delay--;	
			}
			
		}

		if(rsv != NULL)
			*(rsv+slen) = data;
		
		slen++;
	}

	return slen;
}


