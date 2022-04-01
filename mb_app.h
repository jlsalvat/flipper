#ifndef    USER_APP
#define USER_APP
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbutils.h"

/* -----------------------Slave Defines -------------------------------------*/
#define S_DISCRETE_INPUT_START        100
#define S_DISCRETE_INPUT_NDISCRETES   16
#define S_COIL_START                  100
#define S_COIL_NCOILS                 16
#define S_REG_INPUT_START             100
#define S_REG_INPUT_NREGS             1
#define S_REG_HOLDING_START           100
#define S_REG_HOLDING_NREGS           1
/* salve mode: holding register's all address */
#define          S_HD_RESERVE                     0
#define          S_HD_CPU_USAGE_MAJOR             1
#define          S_HD_CPU_USAGE_MINOR             2
/* salve mode: input register's all address */
#define          S_IN_RESERVE                     0
/* salve mode: coil's all address */
#define          S_CO_RESERVE                     0
/* salve mode: discrete's all address */
#define          S_DI_RESERVE                     0

struct Slave_parameters {
Slave_parameters()
    : usSDiscInStart(S_DISCRETE_INPUT_START), usSCoilStart(S_COIL_START),
      usSRegInStart(S_REG_INPUT_START), usSRegHoldStart(S_REG_HOLDING_START){};
/*------------------------Slave mode use these variables----------------------*/
//Slave mode:DiscreteInputs variables
USHORT   usSDiscInStart                              ;
#if S_DISCRETE_INPUT_NDISCRETES%8
UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8+1];
#else
UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8]  ;
#endif
//Slave mode:Coils variables
USHORT   usSCoilStart                              ;
#if S_COIL_NCOILS%8
UCHAR    ucSCoilBuf[S_COIL_NCOILS/8+1]                ;
#else
UCHAR    ucSCoilBuf[S_COIL_NCOILS/8]                  ;
#endif
//Slave mode:InputRegister variables
USHORT   usSRegInStart                              ;
USHORT   usSRegInBuf[S_REG_INPUT_NREGS]               ;
//Slave mode:HoldingRegister variables
USHORT   usSRegHoldStart                           ;
USHORT   usSRegHoldBuf[S_REG_HOLDING_NREGS]           ;


} ;

#endif