#include "ads1292r.h"
#include "spi.h"
#include "usart.h"

#define SPI1_Handler hspi2

void delay_us(uint8_t us) 
{	
	unsigned char a;
	for ( int i  = 0; i < us; i++)
	{
		for(a=20;a>0;a--);
	}	
}

#define delay_ms(x) HAL_Delay(x)

uint8_t ads1292r_data_buff[9] = {0};

void ADS1292R_Init(void)
{
	ADS1292R_CS_H;
	ADS1292R_START_L;
	ADS1292R_PWDN_H;
	delay_ms(1000);
	printf("Init success!\r\n");
}

uint8_t ADS1292R_SPI_RW(uint8_t data)
{
	uint8_t rx_data = 0;
	HAL_SPI_TransmitReceive(&SPI1_Handler, &data, &rx_data, 1, 10);
	return rx_data;
}

/**ADS1292R上电复位 **/
void ADS1292R_PowerOnInit(void)
{
	uint8_t device_id ;

	ADS1292R_START_H;
	ADS1292R_CS_L;
//	ADS1292R_PWDN_L;//进入掉电模式
//	delay_ms(10);
//	ADS1292R_PWDN_H;//退出掉电模式
	delay_ms(10);//等待稳定
	ADS1292R_PWDN_L;//发出复位脉冲
	delay_us(10);
	ADS1292R_PWDN_H;
	delay_ms(1000);//等待稳定，可以开始使用ADS1292R
	ADS1292R_START_L;
	ADS1292R_CMD(ADS1292R_SDATAC);//发送停止连续读取数据命令	
	delay_ms(1);
	ADS1292R_START_H;
	while(device_id!=0x73)       //识别芯片型号，1292r为0x73
	{
		device_id = ADS1292R_REG(ADS1292R_RREG|ADS1292R_ID,0X00);
		delay_ms(200);
		HAL_UART_Transmit(&huart1,&device_id,2,10);
	}
	printf("detected");
	
	ADS1292R_REG(ADS1292R_WREG|ADS1292R_CONFIG2,    0X80);	//使用内部参考电压
	delay_ms(10);//等待内部参考电压稳定
	ADS1292R_REG(ADS1292R_WREG|ADS1292R_CONFIG1,    0X02);	//设置转换速率为500SPS
	//ADS1292R_REG(ADS1292R_WREG|ADS1292R_CONFIG1,    0X00);	//设置转换速率为125SPS
	ADS1292R_REG(ADS1292R_WREG|ADS1292R_LOFF,       0X10);
	ADS1292R_REG(ADS1292R_WREG|ADS1292R_CH1SET,     0X00);
	ADS1292R_REG(ADS1292R_WREG|ADS1292R_CH2SET,     0x00);
	ADS1292R_REG(ADS1292R_WREG|ADS1292R_RLD_SENS,   0x00);
	//	ADS1292R_REG(ADS1292R_WREG|ADS1292R_RLD_SENS,   0x3C);	//使用通道2提取共模电压
	ADS1292R_REG(ADS1292R_WREG|ADS1292R_LOFF_SENS,  0x00);
	//  ADS1292R_REG(ADS1292R_WREG|LOFF_STAT,  0X00);
	ADS1292R_REG(ADS1292R_WREG|ADS1292R_RESP1,      0x02);
	ADS1292R_REG(ADS1292R_WREG|ADS1292R_RESP2,      0x07);
	ADS1292R_REG(ADS1292R_WREG|ADS1292R_GPIO,       0x0C);

}
void ADS1292R_Work(void)
{
	printf("启动连续转换");
	ADS1292R_START_H;//启动转换
	ADS1292R_CMD(ADS1292R_RDATAC);//回到连续读取数据模式，检测噪声数据

}
void ADS1292R_Halt(void)
{
	ADS1292R_START_L;//启动转换
	ADS1292R_CMD(ADS1292R_SDATAC);//发送停止连续读取数据命令
}
//对ADS1292R写入指令
void ADS1292R_CMD(uint8_t cmd)
{
	ADS1292R_CS_L;
	delay_us(1);
	ADS1292R_SPI_RW(cmd);
	delay_us(1);
	ADS1292R_CS_H;
}
//对ADS1292R内部寄存器进行操作
uint8_t ADS1292R_REG(uint8_t cmd, uint8_t data)	//只读一个数据
{
	uint8_t rx_data = 0;
	ADS1292R_CS_L;
	delay_us(1);
	ADS1292R_SPI_RW(cmd);	//读写指令
	ADS1292R_SPI_RW(0X00);	//需要写几个数据（n+1个）
	if((cmd&0x20)==0x20)	//判断是否为读寄存器指令
		rx_data = ADS1292R_SPI_RW(0X00);	//返回寄存器值
	else
		rx_data = ADS1292R_SPI_RW(data);	//写入数值
	delay_us(1);
	ADS1292R_CS_H;
	return rx_data;
}

static uint8_t temp1292r[9] = {0};	//并没有什么卵用
void ADS1292R_ReadData(void)
{
	ADS1292R_CS_L;
	delay_us(1);
	HAL_SPI_TransmitReceive(&SPI1_Handler, temp1292r, ads1292r_data_buff, 9, 10);
	delay_us(1);
	ADS1292R_CS_H;
}



