#include "bmp280.h"
#include "i2c.h"
#include "stdio.h"

static int mTempCalib[3];

/**
  * 连续读bmp280的温度
  * 输入：addr (寄存器地址)
  * 输入：*date
  * 输入：size (读值的数量)
  * 返回值：0
  */
static int bmp280_read_bytes(uint8_t addr, uint8_t *date, uint8_t size)
{
	HAL_StatusTypeDef status;
	status = HAL_I2C_Mem_Read(&hi2c1, BMP280_SLAVE_ADDR,
		addr, I2C_MEMADD_SIZE_8BIT, date,
		size, 100);
	if(status != HAL_OK)
	{
		/*没有读到bmp280寄存器地址的值*/
		printf("bmp280_init: failed to read registor\r\n");
		return -1;
	}
	return 0;
}

/**
  * 读bmp280Compensation
  * 输入：addr (Compensation寄存器地址)
  * 返回值：byte_data的高八位和低八位
  */
static int bmp280_read_word(uint8_t addr)
{
	HAL_StatusTypeDef status;
	uint8_t byte_data[2];
	status = HAL_I2C_Mem_Read(&hi2c1, BMP280_SLAVE_ADDR,
		addr, I2C_MEMADD_SIZE_8BIT, byte_data,
		2, 100);
	if(status != HAL_OK)
	{
		printf("bmp280_init: failed to read registor\r\n");
		return -1;
	}
	/*返回byte_data的高八位和低八位*/
	return (byte_data[1] << 8 | byte_data[0]);
}

/**
  * 读bmp280寄存器
  * 输入：addr (bmp280寄存器地址)
  * 返回值：-1 (没有读到bmp280寄存器地址的值)
  * 返回值：byte_data (bmp280寄存器的值)
  */
static int bmp280_read_byte(uint8_t addr)
{
	HAL_StatusTypeDef status;
	uint8_t byte_data;
	status = HAL_I2C_Mem_Read(&hi2c1, BMP280_SLAVE_ADDR,
		addr, I2C_MEMADD_SIZE_8BIT, &byte_data,
		1, 100);
	if(status != HAL_OK)
	{
		/*没有读到bmp280寄存器地址的值*/
		printf("bmp280_init: failed to read registor\r\n");
		return -1;
	}
	return byte_data;
}

/**
  * 写bmp280寄存器
  * 输入：addr (bmp280寄存器地址)
  * 输入：data (bmp280寄存器的值)
  * 返回值：-1 (没有写bmp280寄存器)
  * 返回值：0 (写bmp280寄存器)
  */
static int bmp280_write_byte(uint8_t addr, uint8_t data)
{
	HAL_StatusTypeDef status;

	status = HAL_I2C_Mem_Write(&hi2c1, BMP280_SLAVE_ADDR,
		addr, I2C_MEMADD_SIZE_8BIT, &data,
		1, 100);
	if(status != HAL_OK)
	{
		printf("bmp280_init: failed to write registor\r\n");
		return -1;
	}
	return 0;
}

/**
  * bmp280初始化
  * 输入：无
  * 返回值：无
  */
void bmp280_init(void)
{
	HAL_StatusTypeDef status;
	uint8_t chipid = 0;
	status = HAL_I2C_Mem_Read(&hi2c1, BMP280_SLAVE_ADDR,
		BMP280_CHIPID_REG, I2C_MEMADD_SIZE_8BIT, &chipid,
		1, 100);
	if(status != HAL_OK)
	{
		/*如果没有读出chip id则返回出来*/
		printf("bmp280_init: failed to read chip id\r\n");
		return;
	}
	/*如果读出了chip id则输出其值*/
	printf("bmp280_init: chip id = 0x%02x\r\n", chipid);
	if(chipid != BMP280_CHIPID)
	{
		/***判断读到的chip id是不是正确的***/
		printf("bmp280_init: chip id not math bmp280\r\n");
	}
	//读bmp280Compensation
	mTempCalib[0] = bmp280_read_word(0x88) & 0xffff;
	mTempCalib[1] = bmp280_read_word(0x8A) & 0xffff;
	mTempCalib[2] = bmp280_read_word(0x8C) & 0xffff;
	bmp280_write_byte(0xf4, 0x23); //0010 0011
}

/**
  * 计算bmp280采集的温度数据
  * 输入：byte_data (高八位和低八位)
  * *calib (bmp280的Compensation)
  * 返回值：温度
  */
static float bmp280_cal_temp(int raw, int *calib)
{
	int dig_T1 = calib[0];
	int dig_T2 = calib[1];
	int dig_T3 = calib[2];

	float adc_T = (float) raw;
	float var1 = (adc_T / 16384.0f - ((float) dig_T1) / 1024.0f) * ((float) dig_T2);
	float var2 = ((adc_T / 131072.0f - ((float) dig_T1) / 8192.0f) * (adc_T / 131072.0f
				 - ((float) dig_T1 / 8192.0f))) * ((float) dig_T3);
    float fineTemp = var1 + var2;
    return (fineTemp / 5120.0f);
}

/**
  * 取bmp280的温度数据
  * 输入：无
  * 返回值：温度
  */
float bmp280_get_temp(void)
{
	uint8_t buffer[3];
	bmp280_read_bytes(0xfa, buffer, 3);
	int msb = buffer[0] & 0xff;
	int lsb = buffer[1] & 0xff;
	int xlsb = buffer[2] & 0xff;
	int raw = (msb << 16 | lsb << 8 | xlsb) >> 4;
	//return 0;
	return bmp280_cal_temp(raw, mTempCalib);
}
