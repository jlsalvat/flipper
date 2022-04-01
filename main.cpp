/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Grove_LCD_RGB_Backlight.h"
#include "grove_two_rgb_led_matrix.h"
#include "max7219.h"
#include "mbed.h"
#include "mb.h"
#include "mbport.h"
#include "mb_app.h"
#include "mbproto.h"

#define WAIT_TIME_MS 2000
const int NB_IN = 20;

// modbus
#define SLAVE_ID 1
// Sharing buffer index
eMBErrorCode    eStatus;
extern Slave_parameters slave;

DigitalOut led1(LED1);
DigitalIn uc_in1(D2);
DigitalIn uc_in2(D3);
DigitalIn uc_in3(D4);
DigitalIn uc_in4(D5);
DigitalIn uc_in5(D6);
DigitalIn uc_in6(D7);
DigitalIn uc_in7(D8);
DigitalIn uc_in8(D9);
DigitalIn uc_in9(PTE21);
DigitalIn uc_in10(PTE20);
DigitalIn uc_in11(PTE30);
DigitalIn uc_in12(PTE29);
DigitalIn uc_in13(A0);
DigitalIn uc_in14(A1);
DigitalIn uc_in15(A2);
DigitalIn uc_in16(A3);
DigitalIn uc_in17(PTC11);
DigitalIn uc_in18(PTC10);
DigitalIn uc_in19(PTC6);
DigitalIn uc_in20(PTC5);
DigitalIn tab_uc_in[NB_IN] = {uc_in1,  uc_in2,  uc_in3,  uc_in4,  uc_in5,
                              uc_in6,  uc_in7,  uc_in8,  uc_in9,  uc_in10,
                              uc_in11, uc_in12, uc_in13, uc_in14, uc_in15,
                              uc_in16, uc_in17, uc_in18, uc_in19, uc_in20};
DigitalOut out1(PTC12);
DigitalOut out2(PTC13);
DigitalOut out3(PTC16);
DigitalOut out4(PTC17);
DigitalOut tab_out[4] = {out1, out2, out3, out4};
DigitalOut cmd_24V(PTC3);
DigitalOut eject(PTC0);
DigitalOut up(PTC7);
BufferedSerial grove_serial(PTE22, PTE23);
I2C grove_I2C(PTC2, PTC1);
GroveTwoRGBLedMatrixClass grove_I2C_matrix8_8(PTC2, PTC1);
SPI max7219(D11, D12, D13);
DigitalOut max7219_cs(D10);
Max7219 max_7219(D11, D12, D13, D10);
Grove_LCD_RGB_Backlight lcd(PTC2, PTC1);
Ticker tick_modbus;

const int colorR = 255;
const int colorG = 0;
const int colorB = 0;



void ISR_poll_modbus(){
         //launch modbus poll
      if(eStatus==MB_ENOERR){
        xMBPortSerialPolling();
        eStatus=eMBPoll( );    
      }
}

int main() {
  bool tab_in[NB_IN];

  max7219_configuration_t cfg = {.device_number = 1,
                                 .decode_mode = 0,
                                 .intensity = Max7219::MAX7219_INTENSITY_8,
                                 .scan_limit = Max7219::MAX7219_SCAN_8};
// on lance l'init Modbus en mode RTU, adresse slave 1, 115200 baud, parité paire
 printf("modbus slave ID-%d(0x%x) for the device.\r\n", SLAVE_ID, SLAVE_ID ); 
    /* Enable the Modbus Protocol Stack. */
    if ( (eStatus = eMBInit( MB_RTU, SLAVE_ID, 0, 600, MB_PAR_NONE )) !=  MB_ENOERR ){
        eMBClose();  
        printf("ERROR modbus : eMBClose()\n\r")  ; 
    }
    else if ( (eStatus = eMBEnable(  ) ) != MB_ENOERR ){
        printf("ERROR modbus : eMBEnable() eStatus=%d\n\r",eStatus)  ; 
    }
    else{
        printf("MODBUS OK : polling...\n\r")  ; 
        tick_modbus.attach(ISR_poll_modbus,5ms);
    }
 
  max_7219.init_device(cfg);
  max_7219.enable_device(1);

  uint16_t VID = 0;
  VID = grove_I2C_matrix8_8.getDeviceVID();
  if (VID != 0x2886) {
    printf("Can not detect led matrix!!!");
  } else
    printf("Matrix init success!!!");
  grove_I2C_matrix8_8.displayClockwise(true, true, 5000, true);
  // ok
  // lcd.begin(16, 2);
  // lcd.setRGB(colorR, colorG, colorB);
  // Print a message to the LCD.
  // char buf_hello[] = "hello, world!";
  // lcd.print(buf_hello);
  printf("This is the bare metal blinky example running on Mbed OS %d.%d.%d.\n",
         MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
  for (int i = 0; i < NB_IN; i++) {
    tab_uc_in[i].mode(PullUp);
  }
  while (true) {
    for (int i = 0; i < NB_IN; i++) {
      tab_in[i] = tab_uc_in[i].read();
     // printf("%d ", tab_in[i]);
    }
    printf("\n\r");
    for (int i = 0; i < 4; i++) {
      tab_out[i].write(!tab_out[i].read());
    }
    // ca fait du bruit
    // cmd_24V = !cmd_24V;
    // eject = !eject;
    // up = !up;
    // grove_I2C.write(0x70);
    char buf[20];
    int cpt = 0;
    sprintf(buf, "test %d\n\r", cpt);
    grove_serial.write(buf, sizeof(buf));

    led1 = !led1;
    if(slave.ucSCoilBuf[0]==1)
        max_7219.set_display_test();
    else
        max_7219.clear_display_test();
    thread_sleep_for(WAIT_TIME_MS);
// print modbus
    printf("eStatus=%d \n\r",eStatus);
    printf("DISCRETE:%d add reg:%d|",MB_FUNC_READ_DISCRETE_INPUTS,S_DISCRETE_INPUT_START);
    for(int i=0;i<S_DISCRETE_INPUT_NDISCRETES/8;i++)
        printf("%2x ",slave.ucSDiscInBuf[i]);
    printf(" COIL:%d add reg:%d|",MB_FUNC_READ_COILS,S_COIL_START);
    for(int i=0;i<S_COIL_NCOILS/8;i++)
        printf("%2x ",slave.ucSCoilBuf[i]);
    printf("\n\r");
 /*   printf("REG:%d add:%d|",MB_FUNC_READ_INPUT_REGISTER,S_REG_INPUT_START);
    for(int i=0;i<S_REG_INPUT_NREGS;i++)
        printf("%d",slave.usSRegInBuf[i]);
    printf("REG:%d func:%d  add:%d|",MB_FUNC_READ_HOLDING_REGISTER,S_REG_HOLDING_START);
    for(int i=0;i<S_REG_HOLDING_NREGS;i++)
        printf("%d",slave.usSRegHoldBuf[i]);
    printf("\n\r");*/

  }

}

