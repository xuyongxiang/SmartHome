#ifndef OPERATION_H
#define OPERATION_H


#define LED1_OP _IOW('l',0,int)
#define BEEP_OP _IOW('b',0,int)
#define FAN_OP _IOW('f',0,int) 
#define Temperature _IOWR('a',0,short)
#define Humidity _IOWR('a',1,short)
#define GET_CMD_SIZE(cmd) ((cmd>>16)&0x3fff)

#endif
