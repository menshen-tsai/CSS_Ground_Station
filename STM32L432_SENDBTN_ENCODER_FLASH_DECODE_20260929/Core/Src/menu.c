/*
 * menu.c
 *
 *  Created on: 2026年6月29日
 *      Author: USER
 */


#include "main.h"
#include "global.h"

#include "string.h"
#include "dma_printf.h"
#include "dma_scanf.h"
#include "log.h"
#include "project_info.h"
#include "stdlib.h"

#include "menu.h"

#include "dtmf.h"
#include "SR105U.h"

#include "BitFIFO.h"
#include "AFSK.h"
#include "APRS.h"


void menu_command_option_1(void);
void menu_command_option_2(void);
void menu_command_option_3(void);
void menu_command_option_4(void);
void menu_command_option_5(void);
void menu_command_option_6(void);
void menu_command_option_7(void);
void menu_command_option_8(void);
void menu_exit(void);


#if 0

void menu_dtmf_option_1(void);
void menu_dtmf_option_2(void);
void menu_dtmf_option_3(void);
void menu_exit(void);

#endif



void menu_main_option_1(void);
void menu_main_option_2(void);
void menu_main_option_3(void);
void menu_exit(void);




// Menu items table
// Menu items table
MenuItem menu_main[] = {
    {"Mode Selection (APRS/DTMF)", menu_main_option_1},
    {"Specify Frequency",          menu_main_option_2},
	{"SR105U SQ",                  menu_main_option_3},
	{"Exit",                       menu_exit}

};



// Aprs items table
MenuItem menu_command[] = {
    {"Change to APRS Mode",                		 menu_command_option_1},
    {"Change to FSK Mode",                		 menu_command_option_2},
	{"Change to BPSK Mode",                		 menu_command_option_3},
    {"Change to SSTV Mode",                		 menu_command_option_4},
    {"Change to CW Mode",                		 menu_command_option_5},
	{"Change to Repeater Mode",                	 menu_command_option_6},
    {"Change to FUNcube Mode",                	 menu_command_option_7},
    {"Turn Transmission of telemetry on or off", menu_command_option_8},
	{"Exit",                                     menu_exit}

};

#if 0
// Aprs items table
MenuItem menu_dtmf[] = {
    {"DTMF Mode 1",                menu_dtmf_option_1},
    {"DTMF Mode 2",                menu_dtmf_option_2},
	{"DTMF Mode 3",                menu_dtmf_option_3},
	{"Exit",                       menu_exit}

};

#endif



#define MENU_MAIN_COUNT (sizeof(menu_main) / sizeof(MenuItem))
#define MENU_COMMAND_COUNT (sizeof(menu_command) / sizeof(MenuItem))
//#define MENU_APRS_COUNT (sizeof(menu_aprs) / sizeof(MenuItem))
//#define MENU_DTMF_COUNT (sizeof(menu_dtmf) / sizeof(MenuItem))


// Menu implementations


void menu_main_option_1(void) {
	int mode;
	uint8_t flag;


	flag = 0;
	while(flag == 0) {
	  printf("Mode Selection: ");
	  printf("APRS/DTMF (A[1]/D[2]) (Timeout %d Seconds) \n\r", 10);
	  mode = get_char_with_timeout(10000) ;
	  if (mode == -2) {
		printf("\n\rTimeout, Jumper connection selected\n\r");

		flag = 1;
		break;
	  } else if (mode == 'A' || mode == 'a' || mode == 'D'|| mode == 'd' || mode == '1' || mode == '2')
		  break;
	  else {
		  printf("Invalid character entered\n\r");
	  }
	}
	if (mode == 'A' || mode =='a' || mode == '1') {
		printf("\n\rAPRS Mode selected\n\r");
		ModeState = MODE_APRS;
	} else if (mode == 'D' || mode == 'd' || mode == '2') {
		printf("\n\rDTMF Mode selected\n\r");
		ModeState = MODE_DTMF;
	}
}

void menu_main_option_2(void) {

//    ConfigSR105U(rxFreq, txFreq);


    float rx_freq;
    float tx_freq;

    Load_Config_From_Flash(&sysConfig);

	printf("Main Menu 2 Selected\n\r");
	  // Config SR105U Tx and Rx frequency

	  printf("Current Rx/Tx Frequencies: RX %.4f, TX %.4f\n\r", sysConfig.rxFreq, sysConfig.txFreq);

	  printf("Rx Freq: ");
	  scanf("%f", &rx_freq);

	  printf("\n\rTx Freq: ");
	  scanf("%f", &tx_freq);

	  printf("\n\rNew Rx/Tx Frequencies: RX %.4f, TX %.4f\n\r", rx_freq, tx_freq);

	  rxFreq = rx_freq;
	  txFreq = tx_freq;

	  ConfigSR105U_Freq(rx_freq, tx_freq);


	  //
	  printf("Write Frequencies to Flash\n\r");
      // Load default values if uninitialized
      sysConfig.version = 1;
//      sysConfig.device_id = 101;
//      sysConfig.calib_factor = 1.05f;

      strcpy(sysConfig.device_name, "GroundStation-01");
      sysConfig.txFreq = tx_freq;
      sysConfig.rxFreq = rx_freq;
      sysConfig.checksum = 0x5A5A5A5A;

      // Save default settings to flash
      Save_Config_To_Flash(&sysConfig);

}


void menu_main_option_3(void) {
  uint32_t sq_level;
  printf("Main Menu 3 Selected\n\r");

  Load_Config_From_Flash(&sysConfig);

  // Config SR105U SQ

  printf("Current SQ Level: %ld\n\r", sysConfig.sq_level);

  printf("SQ Level (0-8): ");
  scanf("%ld", &sq_level);

  printf("\n\rNew SQ Level: %ld\n\r", sq_level);

  ConfigSR105U_SQ(sq_level);
  printf("Write SQ Level to Flash\n\r");
  sysConfig.sq_level = sq_level;

  // Save settings to flash
  Save_Config_To_Flash(&sysConfig);

}

void menu_exit(void) {
    printf("Exiting menu...\r\n");

}



// APRS option implementations
void menu_command_option_1(void) {

  if (ModeState == MODE_APRS) {
    printf("APRS Mode 1 Selected\n\r");

    // N0CALL>APCSS,WIDE1-1:>MODE=a
    strcpy(tx_pack.callsign, "NOCALL");
    strcpy(tx_pack.dest, "APCSS");
    strcpy(tx_pack.digi, "WIDE1-1");
    strcpy(tx_pack.info, ">MODE=a");
    APRS_print(&tx_pack);

    EnablePTT();
    StartAPRS();
    APRS_encode(&tx_fifo, &tx_pack);
    AFSK_send(&tx_fifo);
  }
  else if (ModeState == MODE_DTMF) {
    printf("DTMF Item 1 Selected\n\r");

    EnablePTT();
    StartDTMF();
    play_dtmf_tone_Delay('1', TONE_DURATION_MS);
    play_dtmf_tone_Delay('#', TONE_DURATION_MS);
    play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
    StopDTMF();
    DisablePTT();
  }
}

void menu_command_option_2(void) {
  if (ModeState == MODE_APRS) {
	  printf("APRS Mode 2 Selected\n\r");

	  // N0CALL>APCSS,WIDE1-1:>MODE=f
	  strcpy(tx_pack.callsign, "NOCALL");
	  strcpy(tx_pack.dest, "APCSS");
	  strcpy(tx_pack.digi, "WIDE1-1");
	  strcpy(tx_pack.info, ">MODE=f");
	  APRS_print(&tx_pack);

      EnablePTT();
	  StartAPRS();
	  APRS_encode(&tx_fifo, &tx_pack);
	  AFSK_send(&tx_fifo);
  }
  else if (ModeState == MODE_DTMF) {
    printf("DTMF Item 2 Selected\n\r");

    EnablePTT();
    StartDTMF();
    play_dtmf_tone_Delay('2', TONE_DURATION_MS);
    play_dtmf_tone_Delay('#', TONE_DURATION_MS);
    play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
    StopDTMF();
    DisablePTT();
  }

}
void menu_command_option_3(void) {
  if (ModeState == MODE_APRS) {
    printf("APRS Mode 3 Selected\n\r");

    // N0CALL>APCSS,WIDE1-1:>MODE=b
    strcpy(tx_pack.callsign, "NOCALL");
    strcpy(tx_pack.dest, "APCSS");
    strcpy(tx_pack.digi, "WIDE1-1");
    strcpy(tx_pack.info, ">MODE=b");
    APRS_print(&tx_pack);

    EnablePTT();
    StartAPRS();
    APRS_encode(&tx_fifo, &tx_pack);
    AFSK_send(&tx_fifo);
  }
  else if (ModeState == MODE_DTMF) {
    printf("DTMF Item 3 Selected\n\r");

    EnablePTT();
    StartDTMF();
    play_dtmf_tone_Delay('3', TONE_DURATION_MS);
    play_dtmf_tone_Delay('#', TONE_DURATION_MS);
    play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
    StopDTMF();
    DisablePTT();
  }

}


void menu_command_option_4(void) {
  if (ModeState == MODE_APRS) {
    printf("APRS Mode 4 Selected\n\r");

    // N0CALL>APCSS,WIDE1-1:>MODE=s
    strcpy(tx_pack.callsign, "NOCALL");
    strcpy(tx_pack.dest, "APCSS");
    strcpy(tx_pack.digi, "WIDE1-1");
    strcpy(tx_pack.info, ">MODE=s");
    APRS_print(&tx_pack);

    EnablePTT();
    StartAPRS();
    APRS_encode(&tx_fifo, &tx_pack);
    AFSK_send(&tx_fifo);
  }
  else if (ModeState == MODE_DTMF) {
    printf("DTMF Item 4 Selected\n\r");

    EnablePTT();
    StartDTMF();
    play_dtmf_tone_Delay('4', TONE_DURATION_MS);
    play_dtmf_tone_Delay('#', TONE_DURATION_MS);
    play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
    StopDTMF();
    DisablePTT();
  }

}

void menu_command_option_5(void) {
  if (ModeState == MODE_APRS) {
    printf("APRS Mode 5 Selected\n\r");

    // N0CALL>APCSS,WIDE1-1:>MODE=m
    strcpy(tx_pack.callsign, "NOCALL");
    strcpy(tx_pack.dest, "APCSS");
    strcpy(tx_pack.digi, "WIDE1-1");
    strcpy(tx_pack.info, ">MODE=m");
    APRS_print(&tx_pack);

    EnablePTT();
    StartAPRS();
    APRS_encode(&tx_fifo, &tx_pack);
    AFSK_send(&tx_fifo);
  }
  else if (ModeState == MODE_DTMF) {
    printf("DTMF Item 5 Selected\n\r");

    EnablePTT();
    StartDTMF();
    play_dtmf_tone_Delay('5', TONE_DURATION_MS);
    play_dtmf_tone_Delay('#', TONE_DURATION_MS);
    play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
    StopDTMF();
    DisablePTT();
  }

}


void menu_command_option_6(void) {
  if (ModeState == MODE_APRS) {
    printf("APRS Mode 6 Selected\n\r");

    // N0CALL>APCSS,WIDE1-1:>MODE=e
    strcpy(tx_pack.callsign, "NOCALL");
    strcpy(tx_pack.dest, "APCSS");
    strcpy(tx_pack.digi, "WIDE1-1");
    strcpy(tx_pack.info, ">MODE=e");
    APRS_print(&tx_pack);

    EnablePTT();
    StartAPRS();
    APRS_encode(&tx_fifo, &tx_pack);
    AFSK_send(&tx_fifo);
  }
  else if (ModeState == MODE_DTMF) {
    printf("DTMF Item 6 Selected\n\r");

    EnablePTT();
    StartDTMF();
    play_dtmf_tone_Delay('6', TONE_DURATION_MS);
    play_dtmf_tone_Delay('#', TONE_DURATION_MS);
    play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
    StopDTMF();
    DisablePTT();
  }

}


void menu_command_option_7(void) {
  if (ModeState == MODE_APRS)	{
    printf("APRS Mode 7 Selected\n\r");

    // N0CALL>APCSS,WIDE1-1:>MODE=j
    strcpy(tx_pack.callsign, "NOCALL");
    strcpy(tx_pack.dest, "APCSS");
    strcpy(tx_pack.digi, "WIDE1-1");
    strcpy(tx_pack.info, ">MODE=j");
    APRS_print(&tx_pack);

    EnablePTT();
    StartAPRS();
    APRS_encode(&tx_fifo, &tx_pack);
    AFSK_send(&tx_fifo);
  }
  else if (ModeState == MODE_DTMF) {
    printf("DTMF Item 7 Selected\n\r");

    EnablePTT();
    StartDTMF();
    play_dtmf_tone_Delay('7', TONE_DURATION_MS);
    play_dtmf_tone_Delay('#', TONE_DURATION_MS);
    play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
    StopDTMF();
    DisablePTT();
  }

}

void menu_command_option_8(void) {
  if (ModeState == MODE_APRS) {
    printf("APRS Mode 8 Selected\n\r");

    // N0CALL>APCSS,WIDE1-1:>MODE=o
    strcpy(tx_pack.callsign, "NOCALL");
    strcpy(tx_pack.dest, "APCSS");
    strcpy(tx_pack.digi, "WIDE1-1");
    strcpy(tx_pack.info, ">MODE=o");
    APRS_print(&tx_pack);

    EnablePTT();
    StartAPRS();
    APRS_encode(&tx_fifo, &tx_pack);
    AFSK_send(&tx_fifo);
  }
  else if (ModeState == MODE_DTMF) {
    printf("DTMF Item 8 Selected\n\r");

    EnablePTT();
    StartDTMF();
    play_dtmf_tone_Delay('1', TONE_DURATION_MS);
    play_dtmf_tone_Delay('0', TONE_DURATION_MS);
    play_dtmf_tone_Delay('#', TONE_DURATION_MS);
    play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
    StopDTMF();
    DisablePTT();
  }

}


typedef enum {
    MENU_STATE_OFF = 0,
    MENU_STATE_PRINT,
    MENU_STATE_WAIT
} SubMenuState_t;

/**
 * @brief Non-blocking Main Menu processor
 * @param incoming Character from DMA buffer (-1 if none)
 * @param start_trigger Set to 1 to force open/reset the menu
 * @return 1 if menu is active/running, 0 if closed/finished
 */
int run_main_menu_nonblocking(int incoming, int start_trigger) {
    static SubMenuState_t state = MENU_STATE_OFF;
    static uint32_t start_time = 0;

    if (start_trigger) {
        state = MENU_STATE_PRINT;
    }

    if (state == MENU_STATE_OFF) return 0;

    switch (state) {
        case MENU_STATE_PRINT:
            HAL_GPIO_WritePin(TX_PTT_LED_GPIO_Port, TX_PTT_LED_Pin, GPIO_PIN_RESET);
            if (ModeState == MODE_DTMF)
                printf("\r\n=== Main Menu (DTMF) ===\r\n");
            else if (ModeState == MODE_APRS)
                printf("\r\n=== Main Menu (APRS) ===\r\n");

            for (size_t i = 0; i < MENU_MAIN_COUNT; i++) {
                printf("%d) %s\r\n", (int)(i + 1), menu_main[i].name);
            }
            printf("Select option (timeout %d Secs) : ", MENU_TIMEOUT / 1000);

            start_time = HAL_GetTick();
            state = MENU_STATE_WAIT;
            break;

        case MENU_STATE_WAIT:
            if (incoming >= 0) {
                char input[2] = {(char)incoming, '\0'};
                printf("%s\r\n", input);

                int choice = atoi(input);
                if (choice >= 1 && choice <= MENU_MAIN_COUNT) {
                    menu_main[choice - 1].handler();
                    if (choice == MENU_MAIN_COUNT) {
                        state = MENU_STATE_OFF; // Exit Main Menu
                        return 0;
                    }
                } else {
                    printf("Invalid choice. Try again.\r\n");
                }
                state = MENU_STATE_PRINT; // Refresh
            }
            else if (MENU_TIMEOUT > 0 && (HAL_GetTick() - start_time >= MENU_TIMEOUT)) {
                printf("\r\nMain Menu timeout, quitting...\r\n");
                state = MENU_STATE_OFF;
                return 0;
            }
            break;

        default:
            state = MENU_STATE_OFF;
            return 0;
    }

    return 1; // Still active
}

/**
 * @brief Non-blocking Command Menu processor
 * @param incoming Character from DMA buffer (-1 if none)
 * @param start_trigger Set to 1 to force open/reset the menu
 * @return 1 if menu is active/running, 0 if closed/finished
 */
int run_command_menu_nonblocking(int incoming, int start_trigger) {
    static SubMenuState_t state = MENU_STATE_OFF;
    static uint32_t start_time = 0;

    if (start_trigger) {
        DisablePTT();
        state = MENU_STATE_PRINT;
    }

    if (state == MENU_STATE_OFF) return 0;

    switch (state) {
        case MENU_STATE_PRINT:
            if (ModeState == MODE_APRS)
                printf("\r\n=== APRS Menu Rx:%.2f Tx:%.2f ===\r\n", rxFreq, txFreq);
            else if (ModeState == MODE_DTMF)
                printf("\r\n=== DTMF Menu Rx:%.2f Tx:%.2f ===\r\n", rxFreq, txFreq);

            for (size_t i = 0; i < MENU_COMMAND_COUNT; i++) {
                printf("%d) %s\r\n", (int)(i + 1), menu_command[i].name);
            }
            printf("Select option (timeout %d Secs) : ", MENU_TIMEOUT / 1000);

            start_time = HAL_GetTick();
            state = MENU_STATE_WAIT;
            break;

        case MENU_STATE_WAIT:
            if (incoming >= 0) {
                char input[2] = {(char)incoming, '\0'};
                printf("%s\r\n", input);

                int choice = atoi(input);
                if (choice >= 1 && choice <= MENU_COMMAND_COUNT) {
                    menu_command[choice - 1].handler();
                    if (choice == MENU_COMMAND_COUNT) {
////                        printf("Stop Command transmitting\r\n");
                        StopAPRS();
                        state = MENU_STATE_OFF; // Exit Command Menu
                        return 0;
                    }
                } else {
                    printf("Invalid choice. Try again.\r\n");
                }
                state = MENU_STATE_PRINT; // Refresh
            }
            else if (MENU_TIMEOUT > 0 && (HAL_GetTick() - start_time >= MENU_TIMEOUT)) {
                printf("\r\nCommand Menu timeout, quitting...\r\n");
                StopAPRS();
                state = MENU_STATE_OFF;
                return 0;
            }
            break;

        default:
            state = MENU_STATE_OFF;
            return 0;
    }

    return 1; // Still active
}




