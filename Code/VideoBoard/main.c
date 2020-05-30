/*
 * VideoBoard.c
 *
 * Created: 29/11/2017 21:52:00
 * Author : Matt
 */ 

#define F_CPU 8000000U

#include <avr/io.h>
#include <util/delay.h>
//#include "usi_i2c_master.h"
#include <avr/interrupt.h>
#include "usiTwiMaster.h"

#define TVP_W 0xB8
#define TVP_R 0xB9
#define TVP_CHIP_REV 0x00
#define TVP_HPLL_DIV_MSB 0x01
#define TVP_HPLL_DIV_LSB 0x02
#define TVP_HPLL_CTRL 0x03
#define TVP_HPLL_PHASE 0x04
#define TVP_CLAMP_START 0x05
#define TVP_CLAMP_WIDTH 0x06
#define TVP_GREEN_FINE_OFFSET 0x0C
#define TVP_SYNC_CTRL1 0x0E
#define TVP_HPLL_PRECOAST 0x12
#define TVP_SYNC_DETECT_STATUS 0x14
#define TVP_MISC_CTRL2 0x17
#define TVP_MISC_CTRL3 0x18
#define TVP_MISC_CTRL4 0x22
#define TVP_ALC_EN 0x26
#define TVP_POWER_CTRL 0x2B
#define TVP_SYNC_BYPASS 0x36
#define TVP_LPFl_STATUS 0x37
#define TVP_LPFh_STATUS 0x38
#define TVP_CPLl_STATUS 0x39
#define TVP_CPLh_STATUS 0x3A
#define TVP_HSYNC_WIDTH 0x3B
#define TVP_VSYNC_WIDTH 0x3C
#define TVP_1ST_COEF 0x4A

//800 pixels per line. hsync is 64. FP is 136. BP is 24.
//417 lines per frame. vsync is 3. FP is 25. BP is 11.
uint8_t reg_setting[66] = {	TVP_W, 0x00, 0x00, 0x64, 0x00, 0x20, 0x01, 0x06, 0x10, 0x51, 0x00, 0x90, 0x00, 0x40, 0x78, 0x80, 0x52, 0x2E, 0x5D, 0x20, 0x03, 0x03, 0x00, 0x04, 0x11, 
							0x12, 0x00, 0x00, 0xC2, 0x77, 0x07, 0x00, 0x10, 0x10, 0x10, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x08, 0x07, 0x85, 0x50, 0x00, 
							0x80, 0x8c, 0x04, 0x18, 0x18, 0x60, 0x03, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x0F};
	
//green fine gain
//hsync output width
//h-pll phase select
//green fine offset msbs
//output formatter
//blue and green course gain
//fine offset LSBs
//green course offset
//hsout output start
// ALC enable
//RGB Course clamp control helps remove hum
//vsync alignment

//sync detect status should be 100100XX of 0x90, 0x91, 0x92 or 0x93
//lines per frame status should be 0xA121 YES!
//clocks per line status should be 0x0401 hmmmm 0x111 or 273... variation is 6.5mhz clock?
//hsync width should be 0x14 or 0x15 but is 0x15 or 0x16, variation in clock?
//vsync width should be 0x03

int main(void)
{
	sei();
	_delay_ms(100); //(TVP must be held in reset for 5ms after power up)
	DDRB = 0x45;
	PORTB |= 0x05;
	PORTB |= 0x40; //Bring TVP out of reset
	_delay_ms(100); //No I2C traffic for 1us after reset is released

	usiTwiMasterInitialize();
	/*
	uint8_t msg_w_sync_control1[3] = {TVP_W,TVP_SYNC_CTRL1, 0x52}; //prefer hsync to sog, derive vsync from vsync pin
	uint8_t msg_w_sync_bypass[3] = {TVP_W,TVP_SYNC_BYPASS, 0x02}; //bypass vsync
	uint8_t msg_w_misc_ctrl2[3] = {TVP_W,TVP_MISC_CTRL2, 0x02}; //enable outputs
	uint8_t msg_w_misc_ctrl3[3] = {TVP_W,TVP_MISC_CTRL3, 0x10}; //enable csc convertion outputs
	uint8_t msg_w_misc_ctrl4[3] = {TVP_W,TVP_MISC_CTRL4, 0x02}; //Disable macrovision and choose vs select
	uint8_t msg_w_hpll_divider[4] = {TVP_W,TVP_HPLL_DIV_MSB, 0x69, 0x80}; //set the pll divider value
	uint8_t msg_w_clamp[4] = {TVP_W,TVP_CLAMP_START, 0x06, 0x10}; //set the clamp length and start
	uint8_t msg_w_green_fine_offset[3] = {TVP_W,TVP_GREEN_FINE_OFFSET, 0x83}; //set the green offset to 1LSB
	uint8_t msg_w_hpll_ctrl[3] = {TVP_W,TVP_HPLL_CTRL, 0xA0}; //Set VCO and Charge Pump current to recommended settings for 1280x1024
	uint8_t msg_w_hpll_phase[3] = {TVP_W,TVP_HPLL_PHASE, 0x00}; //Set Hpll phase to 0
	uint8_t msg_w_hpll_precoast[3] = {TVP_W,TVP_HPLL_PRECOAST, 0x01}; //Set precoast to 13
	uint8_t msg_w_pwr_ctrl[3] = {TVP_W,TVP_POWER_CTRL, 0x40}; //power down SOG
	uint8_t msg_w_alc_en[3] = {TVP_W,TVP_ALC_EN, 0x00}; //disable ALC
	uint8_t msg_w_set_scs[4] = {TVP_W,TVP_1ST_COEF, 0x00, 0x00}; //disable ALC
		
	uint8_t msg_r_sync_status[4] = {TVP_W,TVP_SYNC_DETECT_STATUS,TVP_R, 0xFF};
	uint8_t msg_r_per[7] = {TVP_W,TVP_LPFl_STATUS,TVP_R, 0xFF,0xFF, 0xFF, 0xFF};
	uint8_t msg_r_sync_width[5] = {TVP_W,TVP_HSYNC_WIDTH,TVP_R, 0xFF, 0xFF};	
		*/
	uint8_t msg_r_everything[95];
	msg_r_everything[0] = TVP_W;
	msg_r_everything[1] = 0x00;
	msg_r_everything[2] = TVP_R;
	
	for(int i = 3; i < 96; i++)
	{
		msg_r_everything[i] = 0xff;
	}
	
	/*	
	//usiTwiStartTransceiverWithData(msg_w_sync_control1, 3);
	//usiTwiStartTransceiverWithData(msg_w_sync_bypass, 3);
	usiTwiStartTransceiverWithData(msg_w_misc_ctrl2, 3);
	usiTwiStartTransceiverWithData(msg_w_misc_ctrl4, 3);
	usiTwiStartTransceiverWithData(msg_w_hpll_divider, 4);
	usiTwiStartTransceiverWithData(msg_w_clamp, 4);
	//usiTwiStartTransceiverWithData(msg_w_green_fine_offset, 3);
	usiTwiStartTransceiverWithData(msg_w_hpll_ctrl, 3);
	usiTwiStartTransceiverWithData(msg_w_hpll_phase, 3);
	usiTwiStartTransceiverWithData(msg_w_hpll_precoast, 3);
	usiTwiStartTransceiverWithData(msg_w_pwr_ctrl, 3);
	usiTwiStartTransceiverWithData(msg_w_alc_en, 3);
	//usiTwiStartTransceiverWithData(msg_w_misc_ctrl3, 3);
	//usiTwiStartTransceiverWithData(msg_w_set_scs, 4);
	*/
	usiTwiStartTransceiverWithData(reg_setting, 66);
	
    while (1) 
    {
		//usiTwiStartTransceiverWithData(msg_r_vsync_width, 2);
		//usiTwiStartTransceiverWithData(msg_r_vsync_width+2, 2);
		//usiTwiStartTransceiverWithData(msg_r_per, 2);
		//usiTwiStartTransceiverWithData(msg_r_per+2, 5);
		_delay_ms(5000);
		
			usiTwiStartTransceiverWithData(msg_r_everything, 2);
			usiTwiStartTransceiverWithData(msg_r_everything+2, 93);
    }
}