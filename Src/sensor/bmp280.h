#ifndef __BMP280_H
#define __BMP280_H
#include "main.h"

#define BMP280_SLAVE_ADDR (0x76 << 1)	//写地址作为slave address
#define BMP280_CHIPID_REG 0xD0	//读CHIPID
#define BMP280_CHIPID     0x58	//CHIPID
void bmp280_init(void);
float bmp280_get_temp(void);

#endif
