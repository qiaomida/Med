#ifndef _SERVO4_H__
#define _SERVO4_H__

#define MAX_ANGLE 90

const int servoPin1 = 11; // 将舵机信号线连接到数字引脚 11
const int servoPin2 = 12; // 将舵机信号线连接到数字引脚 12
const int servoPin3 = 13; // 将舵机信号线连接到数字引脚 13
const int servoPin4 = 14; // 将舵机信号线连接到数字引脚 14



#include <Arduino.h>
#include <Servo.h>//导入舵机库


void servo_init(void);
void servo_test(void);
void servo_open1(void);
void servo_close1(void);
void servo_open2(void);
void servo_close2(void);
void servo_open3(void);
void servo_close3(void);
void servo_open4(void);
void servo_close4(void);

void servo_openAll(void);
void servo_closeAll(void);

#endif // _SERVO4_H__