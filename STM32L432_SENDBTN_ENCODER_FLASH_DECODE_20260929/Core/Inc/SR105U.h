/*
 * SR105U.h
 *
 *  Created on: 2026年6月30日
 *      Author: USER
 */

#ifndef INC_SR105U_H_
#define INC_SR105U_H_

// Defines the PD and PTT Pin
// PD: SR105U Power Control, 0=Off, 1=On
// PTT: SR105U Transmit/Receive Control, 1=Receive, 0=Transmit
#define PTT_PIN 		17
#define PD_PIN  		22

// PD control
#define SR105U_ON		1
#define SR105U_OFF		0

// PTT control
#define SR105U_TX		0
#define SR105U_RX		1


#define TX_FREQ  		434.9
#define RX_FREQ  		435.9

#define SR105U_RESPONSE_TIMEOUT 10000
#define SR105U_TRANSMIT_TIMEOUT 1000

//COMMAND: AT+<CMD> <CR><LF>
//RESPONSE: +<CMD>:<FLAG><CR><LF>
//              FLAG: 0 Success
//                    1 Failed
//#define SR105_CONFIG   "AT+DMOSETGROUP=0,435.9000,434.9000,0,0,0,0\r\n"
#define SR105_CONFIG_FREQ "AT+DMOSETGROUP=0,%.4f,%.4f,0,0,0,0\r\n"
//                                      Rx   Tx
#define SR105_CONFIG_VOL  "AT+DMOVOL=%ld\r\n"
#define SR105_CONFIG_FUN   "AT+DMOFUN=%ld,5,0,0,0\r\n"


extern ERROR_Code global_error_code;


void ConfigSR105U_Freq(float, float);
void ConfigSR105U_SQ(uint32_t sq_level);
void ConfigSR105U_Vol(uint32_t afout_level) ;
#endif /* INC_SR105U_H_ */
