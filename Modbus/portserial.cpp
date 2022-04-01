/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include "mbed.h"                   // Cam

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#define INPUT 0
#define OUTPUT 1

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );
static void prvvUARTISR( void );

//#define DEF_RS485_PORT 1
/* ----------------------- System Variables ---------------------------------*/

#if defined(MBED_CONF_APP_DEF_RS485_PORT) && (MBED_CONF_APP_DEF_RS485_PORT)// mbed serial port
    #include "nvt_rs485.h"
    // RS485 TX, RX, RTS pins
    #if defined(TARGET_NUMAKER_PFM_NUC472)  // for NUC472 board
        NvtRS485  pc(PF_13, PF_14, PF_11);
    #elif defined(TARGET_NUMAKER_PFM_M453)  // for M453 board
        NvtRS485  pc(PE_8, PE_9, PE_11);
    #else
        #error "The demo code can't be executed on this board."
    #endif
#else
    //UART TX, RX
    #if defined(TARGET_NUMAKER_PFM_NUC472)      // for NUC472 board
    Serial pc(PG_2, PG_1);
    #elif defined(TARGET_NUMAKER_PFM_M453)  // for M453 board
    Serial pc(PD_1, PD_6);    
    #elif defined(TARGET_NUMAKER_PFM_M487)  // for M478 board
    Serial pc(PC_12, PC_11);
    #else
    static UnbufferedSerial modbus_serial(D14, D15);    
    DigitalOut modbus_dir(PTD6);

    #endif
#endif

int us_time_to_send_bit;
static volatile BOOL RxEnable, TxEnable;     // Cam - keep a static copy of the RxEnable and TxEnable
                                    // status for the simulated ISR (ticker)


/* ----------------------- Start implementation -----------------------------*/
// Cam - This is called every 1mS to simulate Rx character received ISR and
// Tx buffer empty ISR.
static void
prvvUARTISR( void )
{
    if ( TxEnable )
        if(modbus_serial.writeable())
            prvvUARTTxReadyISR();

    if ( RxEnable )
        if(modbus_serial.readable())
            prvvUARTRxISR();
}

void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
    RxEnable = xRxEnable;
    TxEnable = xTxEnable;
    
    //printf("\r\nRx: %d, TX:%d\r\n", RxEnable, TxEnable);
}

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    modbus_serial.baud(ulBaudRate);
    modbus_serial.format(8,(SerialBase::Parity) eParity,1);
    us_time_to_send_bit=1000000/(ulBaudRate);
#if defined(DEF_RS485_PORT) // mbed serial port
    #if defined(TARGET_NUMAKER_PFM_NUC472)      // for NUC472 board
    pc.set_rs485_mode(PF_11);
    #elif defined(TARGET_NUMAKER_PFM_M453)  // for M453 board
    pc.set_rs485_mode(PE_11);
    #endif    
#endif
    return TRUE;
}

void xMBPortSerialPolling( void )
{
    prvvUARTISR( );
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
    printf("[%02x]", ucByte );
    modbus_dir=OUTPUT;
    modbus_serial.write( &ucByte,1);
    wait_us(us_time_to_send_bit*11);//maximum 11 bits to send
    modbus_dir=INPUT;
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
     modbus_dir=INPUT;
    modbus_serial.read(pucByte,1);
    printf("<%02x>", *pucByte );
    return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
    vMBPortTimersDisable();
    pxMBFrameCBByteReceived(  );
}


