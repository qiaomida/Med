#ifndef _BLINKERR_H__
#define _BLINKERR_H__

#include <Arduino.h>
#include <Blinker.h>//导入Blinker库


char auth[] = "5e6143510577";
char ssid[] = "5019";
char pswd[] = "5019yyds";
// 新建组件对象
BlinkerButton Button1("btn-abc");
BlinkerNumber Number1("num-abc");

int counter = 0;

void blinker_init();
void dataRead(const String & data);
void button1_callback(const String & state);
void dataRead(const String & data);
void blinker_run();



#endif // _DISP_H__