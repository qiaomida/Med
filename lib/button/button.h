#ifndef _BUTTON_H__
#define _BUTTON_H__


//这是按钮控制、闹钟设置的代码
#include <Arduino.h>
#include <OneButton.h>
#include "CArmTime.h"
#include "disp.h"

#define BUTTON_PIN0 41
#define BUTTON_PIN1 42
#define BUTTON_PIN2 45
#define BUTTON_PIN3 46


extern int disState;
extern int timePos;


extern int Day;
extern bool armSet;
extern ArmTime armT[3];

extern OneButton btn0;
extern OneButton btn1;
extern OneButton btn2;
extern OneButton btn3;
extern int openAllState;
extern void servo_openAll();
extern void servo_closeAll();
extern int CiShuMAX;





void buttoninit();
void down();
void downPress();
void up();
void upPress();
void switchPos();
void nextState();
void lastState();
void openAll();



#endif // _BUTTON_H__