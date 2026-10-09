/*
 * menu.h
 *
 *  Created on: 2026年6月30日
 *      Author: USER
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_


#include "string.h"
#include "dma_printf.h"
#include "dma_scanf.h"
#include "log.h"
#include "project_info.h"
#include "stdlib.h"



// Menu structure
typedef struct {
    const char *name;
    void (*handler)(void);
} MenuItem;


#define MENU_TIMEOUT	30000

void run_main_menu(uint32_t timeout);
void run_aprs_menu(uint32_t timeout);
void run_dtmf_menu(uint32_t timeout);
void run_command_menu(uint32_t timeout);
//void run_main_menu_nonblocking(uint32_t timeout_ms) ;

int run_command_menu_nonblocking(int incoming, int start_trigger) ;
int run_main_menu_nonblocking(int incoming, int start_trigger) ;
#endif /* INC_MENU_H_ */
