#ifndef _DISP_H__
#define _DISP_H__

//这是屏幕显示相关代码

#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>
#include "SSD1306Wire.h"//导入0.96寸屏幕显示库
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "CArmTime.h"
#include <OneButton.h>
#include "button.h"






extern int currentHour;    // 小时（全局变量前缀g_便于区分）
extern int currentMinute;  // 分钟
extern int currentSecond;  // 秒

// 获取“应用内认为”的当前本地时间（Unix epoch 秒，带时区偏移）
// 若尚未成功校时返回 0。
uint64_t getNowEpochSec();
// 调试用：在不改动系统时间的情况下给 now 加偏移（秒），可为负数。
void setDebugTimeOffsetSec(int32_t offsetSec);

void drawLines();
void drawRect(void);
void fillRect(void);
void drawCircle(void);
void printBuffer(void);
void display_test(void);
void display_initt();
void updateTime();   
void displayTime();
void updateNtpTime();
void displayInitializing();
void displayArm();
void displayNoHour();
void displayNoMinute();
void displayTimeAlarm();
void displayDAY();

#endif // _DISP_H__