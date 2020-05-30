/*
 * usi_i2c_master.h
 *
 * Created: 14/01/2018 22:09:00
 *  Author: Matt
 */ 

#ifndef USI_I2C_MASTER_H_
#define USI_I2C_MASTER_H_

void USI_I2C_Initialise(int scl_freq);
void USI_I2C_Single_Write(char addr, char val);
void USI_I2C_Single_Read(char addr);
char USI_I2C_Single_Read_Blocking(char addr);
int USI_I2C_Busy();

#endif /* USI_I2C_MASTER_H_ */