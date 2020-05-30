/*
 * usi_i2c_master.c
 *
 * Created: 14/01/2018 22:14:30
 *  Author: Matt
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define IDLE 0x00
#define START 0x01
#define BITTOBUS 0x02
#define BITFROMBUS 0x03
#define STOP 0x04

void _usi_i2c_set_bus(char, char);
void _usi_i2c_start();

int _usi_i2c_state;
int _usi_i2c_num_bits = 0;
uint32_t _usi_i2c_buffer = 0;
int count = 0;

char cycle_parity = 0;

void USI_I2C_Initialise(int scl_freq)
{
	USIDR = 0xFF;
	USIPP |= (0 << USIPOS); //Set USI pin position as PB0 and PB2
	USICR |= (0 << USIWM0)|(1 << USIWM1); //Set the USI to two-wire mode
	USICR |= (0 << USICS0)|(0 << USICS1); //Set USI clock source to be soft-ware (in reality, probably want timer0 as clock source)
	USISR |= (1 << USISIF); //Clear USI start condition interrupt flag which pulls SCL low
	
	TCCR0A |= (1 << WGM01) | (0 << WGM00); //Set Timer0 to CTC mode
	OCR0A = 100; //Set output compare value to 80
	TIMSK0 |= (1 << OCIE0A); //Enable output compare interrupt
	
	_usi_i2c_state = IDLE;
}

void USI_I2C_Single_Write(char addr, char val)
{
	//We can't do anything if we are busy
	if(_usi_i2c_state != IDLE)
	{
		return;
	}
	
	_usi_i2c_num_bits = 17;
	_usi_i2c_buffer = ((uint32_t)(addr | 0x01) << 24) | ((uint32_t)val << 15) | 0x00800000;
	USIDR = 0xFF;
	_usi_i2c_start();
}

void USI_I2C_Single_Read(char addr)
{
	
}

char USI_I2C_Single_Read_Blocking(char addr)
{
	return 0;
}

int USI_I2C_Busy()
{
	return _usi_i2c_state != IDLE;
}

ISR(TIMER0_COMPA_vect)
{
	cycle_parity = ~cycle_parity;
	//If it's halfway between clock edges and we are in BITTOBUS mode then we should shift onto the bus
	if(cycle_parity == 0xFF)
	{
		if(_usi_i2c_state == BITTOBUS)
		{
			_usi_i2c_set_bus(0, _usi_i2c_buffer >> 31); 
			_usi_i2c_buffer = (_usi_i2c_buffer << 1);
		}
		return;
	}
	
	switch(_usi_i2c_state)
	{
		case IDLE: //The timer shouldn't be running if we're in IDLE mode
			TCCR0B &= ~((1 << CS00) | (1 << CS01) | (1 << CS02));
			break;
		case START:
			count = 0;
			_usi_i2c_state = BITTOBUS; //Next state is loading a bit onto the bus (either by slave or master)
			_usi_i2c_set_bus(0, 1); //Lower clock
			USISR |= (1 << USISIF); //Clear the start-condition flag
			break;
		case BITTOBUS:
			//If we have sent all our bits
			if(count >= _usi_i2c_num_bits)
			{
				_usi_i2c_state = STOP;
				_usi_i2c_set_bus(1,0); //Raise SCL and lower SDA in preparation for a stop condition
				break;
			}
			count++;
			_usi_i2c_state = BITFROMBUS; //Next state is removing a bit from the bus (either by slave or master)
			USICR |= (1 << USITC); //Toggle clock (low to high)
			break;
		case BITFROMBUS:
			_usi_i2c_state = BITTOBUS;
			USICR |= (1 << USITC); //Toggle clock (high to low)
			break;
		case STOP:
			_usi_i2c_state = IDLE;
			_usi_i2c_set_bus(1,1);
			USIDR = 0xFF;
			break;
		default:	
			_usi_i2c_state = IDLE;
			TCCR0B &= ~((1 << CS00) | (1 << CS01) | (1 << CS02));
			break;
	}
}

void _usi_i2c_start()
{
	if(_usi_i2c_state != IDLE)
	{
		return;
	}
	
	TCCR0B |= (1 << CS00) | (0 << CS01) | (0 << CS02); //Set clock source to be system clock
	
	_usi_i2c_state = START;
	_usi_i2c_set_bus(1, 0);
}

void _usi_i2c_set_bus(char scl, char sda)
{
	PORTB = ((PORTB & 0b11111010) | (scl << 2) | (sda << 0));
}
