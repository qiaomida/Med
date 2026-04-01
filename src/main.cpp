#include <Arduino.h>
#include <OneButton.h>


void servo_test(void);

#define BLINKER_WIFI

#include <disp.h>
#include <Blinker.h>//导入Blinker库
#include <servo4.h>//导入舵机库
#include "HX711.h"
#include <OneButton.h>
#include "button.h"


#define LOADCELL_DOUT_PIN1  1
#define LOADCELL_SCK_PIN1   2
#define LOADCELL_DOUT_PIN2  3
#define LOADCELL_SCK_PIN2   4
#define LOADCELL_DOUT_PIN3  5
#define LOADCELL_SCK_PIN3   6
#define LOADCELL_DOUT_PIN4  7
#define LOADCELL_SCK_PIN4   8
 
#define GapValue 208.05  // 重量差值
#define MedWeight 5.0    // 药物重量
HX711 scale1;
HX711 scale2;
HX711 scale3;
HX711 scale4;


char auth[] = "6820a0a174cb"; //blinker的密钥
char ssid[] = "Xiaomi 13";         // wifi名称
char pswd[] = "qiaomi667";     // wifi密码
// 新建组件对象
BlinkerButton Button1("btn-abc"); 
BlinkerNumber Number1("num-abc");
BlinkerButton Button_up("btn-up");
BlinkerButton Button_down("btn-down");
BlinkerButton Button_left("btn-left");
BlinkerButton Button_right("btn-right");
BlinkerSlider Slider1("ran-abc"); // 新增：用于调整吃药间隔的滑动条



int counter = 0;
int loopTime = 0;

// 吃药触发间隔（秒）
// 0 表示不启用“间隔模式”，仍使用你原先 armT[0..2] 的固定时刻触发。
static uint32_t g_doseIntervalSec = 0;
static uint64_t g_nextDoseEpochSec = 0;
static bool g_lastArmSet = false;
static uint64_t g_lastTriggeredEpochSec = 0;

// 串口调试：支持
// 1) offset <秒>           例：offset 60 / offset -30
// 2) interval <小时>      例：interval 8
// 3) interval_sec <秒>    例：interval_sec 28800
// 4) interval 0            禁用间隔模式
void handleSerialDebug()
{
  static String serialBuffer = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      // 收到换行，处理缓冲区中的命令
      String line = serialBuffer;
      line.trim();
      serialBuffer = ""; // 清空缓冲区
      if (line.length() == 0) return;

      int sp = line.indexOf(' ');
      String cmd = (sp < 0) ? line : line.substring(0, sp);
      String arg = (sp < 0) ? String("") : line.substring(sp + 1);
      cmd.toLowerCase();

      if (cmd == "offset") {
        int32_t v = (int32_t)arg.toInt();
        setDebugTimeOffsetSec(v);
        Serial.printf("[debug] time offset sec = %ld\r\n", (long)v);
        return;
      }

      if (cmd == "interval") {
        int32_t hours = (int32_t)arg.toInt();
        if (hours <= 0) g_doseIntervalSec = 0;
        else g_doseIntervalSec = (uint32_t)hours * 3600UL;
        g_nextDoseEpochSec = 0;
        g_lastTriggeredEpochSec = 0;
        Serial.printf("[debug] interval sec = %lu\r\n", (unsigned long)g_doseIntervalSec);
        return;
      }

      if (cmd == "interval_sec") {
        int32_t sec = (int32_t)arg.toInt();
        if (sec <= 0) g_doseIntervalSec = 0;
        else g_doseIntervalSec = (uint32_t)sec;
        g_nextDoseEpochSec = 0;
        g_lastTriggeredEpochSec = 0;
        Serial.printf("[debug] interval sec = %lu\r\n", (unsigned long)g_doseIntervalSec);
        return;
      }

      if (cmd == "sync_ntp") {
        updateNtpTime();
        Serial.println("[debug] ntp sync requested");
        return;
      }
    } else if (c != '\r') {
      serialBuffer += c;
    }
  }
}

/**
 * @brief 警告函数，用于通过控制两个LED引脚闪烁产生警告效果
 * 该函数会使引脚21和39交替闪烁，闪烁周期为500ms，闪烁两次
 */
void WARNING()
{
  // 触发引脚21和39对应的LED和蜂鸣器
  digitalWrite(21,HIGH);
  digitalWrite(39,HIGH);
  // 延时500ms
  vTaskDelay(pdMS_TO_TICKS(500));
  // 熄灭引脚21和39对应的LED和蜂鸣器
  digitalWrite(21,LOW);
  digitalWrite(39,LOW);
  // 再次延时500ms
  vTaskDelay(pdMS_TO_TICKS(500));
  // 再次触发引脚21和39对应的LED和蜂鸣器
  digitalWrite(21,HIGH);
  digitalWrite(39,HIGH);
  // 再次延时500ms
  vTaskDelay(pdMS_TO_TICKS(500));
  // 再次熄灭引脚21和39对应的LED和蜂鸣器
  digitalWrite(21,LOW);
  digitalWrite(39,LOW);
  // 最后延时500ms，完成一次完整的警告闪烁周期
  vTaskDelay(pdMS_TO_TICKS(500));
}

/**
 * @brief takeMed - 服药检测功能函数
 * 根据不同的天数，使用不同的称重传感器和舵机来控制服药过程
 * 当检测到药被取走（重量减少超过MedWeight）后，关闭舵机并重置显示状态
 */
void takeMed()
{
  bool alreadytakeMed = false;  // 标记是否已经服药的标志位

  float oriweight, nowweight;// 原始重量与当前重量

  // 根据服药次数进行判断
  switch (Day)
  {
    // 第一天服药的情况
    case 1:

        oriweight = scale1.get_units(5); // 获取原始重量


        // 设置显示状态为4（闹钟显示模式）
        disState = 4;
        servo_open1();  // 打开第一天的舵机
        while (!alreadytakeMed)
        {
          WARNING();  // 发出警告提示
          WARNING();
          nowweight = scale1.get_units(5); // 获取当前重量
          if (oriweight - nowweight > MedWeight)  // 判断重量差是否大于药物重量
          {
            alreadytakeMed = true;  // 标记已服药
            WARNING();  // 再次发出警告提示
            servo_close1();  // 关闭第一天的舵机
            updateNtpTime();
            disState = 0;  // 重置显示状态
            ArmTime::finishOne();
            break;
          }
        }
        break;
    // 第二天服药的情况
    case 2:
        oriweight = scale2.get_units(5); // 获取原始重量
        disState = 4;  // 设置显示状态为4（闹钟显示模式）
        servo_open2();  // 打开第二天的舵机
        while (!alreadytakeMed)
        {
          WARNING();  // 发出警告提示
          WARNING();
          nowweight = scale2.get_units(5); // 获取当前重量
          if (oriweight - nowweight > MedWeight)  // 判断重量差是否大于药物重量
          {
            alreadytakeMed = true;  // 标记已服药
            WARNING();  // 再次发出警告提示
            servo_close2();  // 关闭第二天的舵机
            updateNtpTime();
            disState = 0;  // 重置显示状态
            ArmTime::finishOne();
            break;
          }
        }
        break;

    // 第三天服药的情况
    case 3:
        oriweight = scale3.get_units(5); // 获取原始重量
        disState = 4;  // 设置显示状态为4（闹钟显示模式）
        servo_open3();  // 打开第三天的舵机
        while (!alreadytakeMed)
        {
          WARNING();  // 发出警告提示
          WARNING();

          nowweight = scale3.get_units(5); // 获取当前重量
          if (oriweight - nowweight > MedWeight)  // 判断重量差是否大于药物重量
          {
            alreadytakeMed = true;  // 标记已服药
            WARNING();  // 再次发出警告提示
            servo_close3();  // 关闭第三天的舵机
            updateNtpTime();
            disState = 0;  // 重置显示状态
            ArmTime::finishOne();
            break;
          }
        }
        break;

          //第四天服药的情况
    case 4:
        oriweight = scale4.get_units(5); // 获取原始重量
        disState = 4;  // 设置显示状态为4（闹钟显示模式）
        servo_open4();  // 打开第四天的舵机
        while (!alreadytakeMed)
        {
          WARNING();  // 发出警告提示
          WARNING();

          nowweight = scale4.get_units(5); // 获取当前重量
          if (oriweight - nowweight > MedWeight)  // 判断重量差是否大于药物重量
          {
            alreadytakeMed = true;  // 标记已服药
            WARNING();  // 再次发出警告提示
            servo_close4();  // 关闭第四天的舵机
            updateNtpTime();
            disState = 0;  // 重置显示状态
            ArmTime::finishOne();
            break;
          }
        }
        break;
    default:
        break;
  }
}

//这个按钮没用，只是用来测试
void button1_callback(const String & state)
{
  BLINKER_LOG("get button state: ", state);
  // digitalWrite(48, !digitalRead(48));
  counter++;
  Number1.print(counter);
}


void button_up_callback(const String & state)
{
    if (state == BLINKER_CMD_BUTTON_TAP) {
      BLINKER_LOG("get button state: ", state);  // 打印按钮状态到日志
      digitalWrite(48, counter % 2);            // 切换48号引脚的电平状态
      counter++;                                 // 计数器递增
      Number1.print(counter);                    // 将计数值发送到Number1组件
      switchPos();
    }
    else if (state == BLINKER_CMD_BUTTON_PRESSED) {
      BLINKER_LOG("get button state: ", state);  // 打印按钮状态到日志
      digitalWrite(48, counter % 2);            // 切换48号引脚的电平状态
      counter++;                                 // 计数器递增
      counter++;                                 // 计数器递增
      Number1.print(counter);                    // 将计数值发送到Number1组件
      nextState();
    }
}
void button_down_callback(const String & state)
{
    if (state == BLINKER_CMD_BUTTON_TAP) {
      BLINKER_LOG("get button state: ", state);  // 打印按钮状态到日志
      digitalWrite(48, counter % 2);            // 切换48号引脚的电平状态
      counter++;                                 // 计数器递增
      Number1.print(counter);                    // 将计数值发送到Number1组件
      lastState();
    }
    else if (state == BLINKER_CMD_BUTTON_PRESSED) {
      BLINKER_LOG("get button state: ", state);  // 打印按钮状态到日志
      digitalWrite(48, counter % 2);            // 切换48号引脚的电平状态
      counter++;                                 // 计数器递增
      counter++;                                 // 计数器递增
      Number1.print(counter);                    // 将计数值发送到Number1组件
      openAll();
    }
}
void button_left_callback(const String & state)
{
    if (state == BLINKER_CMD_BUTTON_TAP) {
      BLINKER_LOG("get button state: ", state);  // 打印按钮状态到日志
      digitalWrite(48, counter % 2);            // 切换48号引脚的电平状态
      counter++;                                 // 计数器递增
      Number1.print(counter);                    // 将计数值发送到Number1组件
      down();
    }
    else if (state == BLINKER_CMD_BUTTON_PRESSED) {
      BLINKER_LOG("get button state: ", state);  // 打印按钮状态到日志
      digitalWrite(48, counter % 2);            // 切换48号引脚的电平状态
      counter++;                                 // 计数器递增
      counter++;                                 // 计数器递增
      Number1.print(counter);                    // 将计数值发送到Number1组件
      downPress();
    }
}
void button_right_callback(const String & state)
{
    if (state == BLINKER_CMD_BUTTON_TAP) {
      BLINKER_LOG("get button state: ", state);  // 打印按钮状态到日志
      digitalWrite(48, counter % 2);            // 切换48号引脚的电平状态
      counter++;                                 // 计数器递增
      Number1.print(counter);                    // 将计数值发送到Number1组件
      up();
    }
    else if (state == BLINKER_CMD_BUTTON_PRESSED) {
      BLINKER_LOG("get button state: ", state);  // 打印按钮状态到日志
      digitalWrite(48, counter % 2);            // 切换48号引脚的电平状态
      counter++;                                 // 计数器递增
      counter++;                                 // 计数器递增
      Number1.print(counter);                    // 将计数值发送到Number1组件
      upPress();
    }
}

// 滑动条回调函数，用于设置吃药间隔（单位：小时）
void slider1_callback(int32_t value)
{
    BLINKER_LOG("get slider value: ", value);
    if (value <= 0) {
        g_doseIntervalSec = 0; // 禁用间隔模式
        Serial.println("[Blinker] Interval mode disabled (set to fixed times).");
    } else {
        g_doseIntervalSec = (uint32_t)value * 60UL; // 将分钟转换为秒
        armSet = true;        // 强制开启闹钟，使间隔模式生效
        Day = 1;              // 从第一天开始重新计数
        ArmTime::initarmCiShu(); // 重置服药次数
        g_nextDoseEpochSec = 0; // 重置下次触发时间，使其在 loop 中重新计算
        g_lastTriggeredEpochSec = 0;
        Serial.printf("[Blinker] Dose interval set to %d minutes (%lu seconds).\r\n", value, (unsigned long)g_doseIntervalSec);
        Serial.println("[Blinker] Alarm ENABLED, Day RESET to 1.");
    }
}




// 如果未绑定的组件被触发，则会执行其中内容
void dataRead(const String & data)
{
  BLINKER_LOG("Blinker readString: ", data);
  counter++;
  Number1.print(counter);
}

void blinker_init()
{
    
  // BLINKER_DEBUG.stream(Serial);
  // BLINKER_DEBUG.debugAll(); // 注释掉，减少串口输出压力，防止缓冲区溢出导致重启
    
  // 初始化有LED的IO
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  // 初始化blinker
  Blinker.begin(auth, ssid, pswd);
  Blinker.attachData(dataRead);

  Button1.attach(button1_callback);
  Button_up.attach(button_up_callback);
  Button_down.attach(button_down_callback);
  Button_left.attach(button_left_callback);
  Button_right.attach(button_right_callback);
  Slider1.attach(slider1_callback); // 绑定滑动条回调
}



// blinker联网远程控制运行
void xTaskOne(void *xTask1)
{
  while (1)
  {
      // 断网时 Blinker 内部的网络/SSL 读写可能会抛错并触发 abort，
      // 所以 WiFi 未连接时先暂停运行，避免影响看门狗和系统稳定性。
      if (WiFi.status() == WL_CONNECTED) {
        Blinker.run(); // 运行blinker功能
      }
      vTaskDelay(pdMS_TO_TICKS(10));
  }
}

//定期对时+按钮检测
void xTaskTwo(void *xTask2)
{
  int tickcnt = 0;  // 计时器计数变量，用于时间更新控制



  // 系统启动后延迟2秒并更新NTP时间，重复4次
  vTaskDelay(pdMS_TO_TICKS(2000));  // FreeRTOS延时函数，将毫秒转换为tick数
  updateNtpTime();                  // 更新NTP时间
  vTaskDelay(pdMS_TO_TICKS(2000));
  updateNtpTime();
  vTaskDelay(pdMS_TO_TICKS(2000));
  updateNtpTime();
  vTaskDelay(pdMS_TO_TICKS(2000));
  updateNtpTime();
  // 进入主循环
  while (1)
  {
    btn0.tick();  // 处理按钮0的检测
    btn1.tick();  // 处理按钮1的检测
    btn2.tick();  // 处理按钮2的检测
    btn3.tick();  // 处理按钮3的检测
    tickcnt++;    // 计时器计数变量自增
    vTaskDelay(pdMS_TO_TICKS(25)); 
    if (tickcnt == 12000)  // 每隔5分钟执行一次
    {
      tickcnt = 0;  // 计时器计数变量清零
      // 仅在 WiFi 已连接时尝试 NTP 校时，避免断网/重连时阻塞影响看门狗
      if (WiFi.status() == WL_CONNECTED) {
        updateNtpTime();  // 更新NTP时间
      }
    }
  }
}

//根据状态显示时间
/**
 * @brief 任务三的函数实现，负责显示界面和时间
 * @param xTask3 任务参数，在此函数中未使用
 */
void xTaskThree(void *xTask3)
{
  drawCircle();
  // 显示初始化界面，并延时
  displayInitializing();
  vTaskDelay(pdMS_TO_TICKS(1000));

  // 无限循环，持续处理显示逻辑
  while (1)
  {
    // 根据disState的不同值执行不同的显示逻辑
    switch (disState)
    {
      // 状态0：显示时间，每秒刷新一次
      case 0:
        if (armSet)
        {
          displayTimeAlarm();
        }
        else
        {
          displayTime();
        }
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1秒刷新一次
        break;
      // 状态1：显示闹钟时间，根据timePos决定闪烁小时还是分钟
      case 1:
        displayArm();
        vTaskDelay(pdMS_TO_TICKS(500));
        if (timePos == 0)
        {
          displayNoHour();
          vTaskDelay(pdMS_TO_TICKS(500));
        }
        else
        {
          displayNoMinute();
          vTaskDelay(pdMS_TO_TICKS(500));
        }
        break;
      // 状态2：逻辑与状态1相同
      case 2:
        displayArm();
        vTaskDelay(pdMS_TO_TICKS(500));
        if (timePos == 0)
        {
          displayNoHour();
          vTaskDelay(pdMS_TO_TICKS(500));
        }
        else
        {
          displayNoMinute();
          vTaskDelay(pdMS_TO_TICKS(500));
        }
        break;
      // 状态3：逻辑与状态1相同
      case 3:
        displayArm();
        vTaskDelay(pdMS_TO_TICKS(500));
        if (timePos == 0)
        {
          displayNoHour();
          vTaskDelay(pdMS_TO_TICKS(500));
        }
        else
        {
          displayNoMinute();
          vTaskDelay(pdMS_TO_TICKS(500));
        }
        break;
      // 状态4：闹钟触发的显示，每秒刷新一次
      case 4:
        displayDAY();
        vTaskDelay(pdMS_TO_TICKS(1000));
        break;
    }
  }
}



/**
 * @brief 初始化函数，在程序开始时执行一次
 * 该函数用于初始化各种外设和硬件组件，包括串口通信、显示屏、舵机、
 * 按钮以及称重传感器等，并创建多任务运行环境
 */
void setup()
{
  // 初始化串口通信，波特率设置为115200
  Serial.begin(115200);

  //初始化函数，在程序开始时执行一次
  display_initt(); //屏幕初始化，配置显示参数
  servo_init(); // 初始化舵机功能
  blinker_init(); // 初始化blinker功能
  buttoninit();  // 初始化按钮
  // 设置 GPIO48 引脚 为输出模式
  pinMode(21,OUTPUT);
  pinMode(39,OUTPUT);
  // 设置48引脚 输出低电平
  digitalWrite(21,LOW);
  digitalWrite(39,LOW);

  

  Serial.println("HX711 scale demo");

  scale1.begin(LOADCELL_DOUT_PIN1, LOADCELL_SCK_PIN1);
  scale2.begin(LOADCELL_DOUT_PIN2, LOADCELL_SCK_PIN2);
  scale3.begin(LOADCELL_DOUT_PIN3, LOADCELL_SCK_PIN3);
  scale4.begin(LOADCELL_DOUT_PIN4, LOADCELL_SCK_PIN4);
  

  // 这里需要调整以匹配你的具体硬件设置
  scale1.set_scale(GapValue); // 这个校准因子是一个例子，你需要调整它
  scale2.set_scale(GapValue); // 这个校准因子是一个例子，你需要调整它
  scale3.set_scale(GapValue); // 这个校准因子是一个例子，你需要调整它
  scale4.set_scale(GapValue); // 这个校准因子是一个例子，你需要调整它

  scale1.tare();  // 重置秤，忽略已有的重量，仅测量后加上去的重量
  scale2.tare();  // 重置秤，忽略已有的重量，仅测量后加上去的重量
  scale3.tare();  // 重置秤，忽略已有的重量，仅测量后加上去的重量
  scale4.tare();  // 重置秤，忽略已有的重量，仅测量后加上去的重量

  xTaskCreatePinnedToCore(xTaskOne, "TaskOne", 8192, NULL, 1, NULL, 0);//Blinker任务：核心0，增加栈空间防止溢出
  xTaskCreatePinnedToCore(xTaskTwo, "TaskTwo", 4096, NULL, 1, NULL, 1);//按键任务：核心1
  xTaskCreatePinnedToCore(xTaskThree, "TaskThree", 4096, NULL, 1, NULL, 0);//显示任务：核心0，降低优先级减少冲突

}
void loop() 
{

  // 串口打印时间（调试用）
  // Serial.printf("当前时间：%02d:%02d:%02d\n", currentHour, currentMinute, currentSecond);
  /*if (scale1.is_ready()) {
    float weight1 = scale1.get_units(3); //连续读取5次 或者可以用scale.get_units()获取单个测量值
    Serial.print("Weight1: ");
    Serial.print(weight1, 2); // 减小到两位小数
    Serial.println(" g");
  }
  else {
    Serial.println("HX711 not ready");
  }*/
  handleSerialDebug();

  updateTime(); // 更新时间

  // 检测 armSet 从 false -> true 的边沿，用于初始化间隔模式的首次触发时间
  if (armSet && !g_lastArmSet) {
    g_nextDoseEpochSec = 0;
    g_lastTriggeredEpochSec = 0;
  }
  g_lastArmSet = armSet;

  if (armSet == true)
  {
    // 间隔模式：从第一次触发开始每隔 g_doseIntervalSec 秒触发一次
    if (g_doseIntervalSec > 0) {
      uint64_t nowEpoch = getNowEpochSec();

      // nowEpoch 尚未可用（未完成 NTP 校时）时暂不触发，打印提示
      if (nowEpoch == 0) {
        static uint32_t lastPrint = 0;
        if (millis() - lastPrint > 5000) {
          Serial.println("[debug] Waiting for NTP sync before interval alarm starts...");
          lastPrint = millis();
        }
      }

      if (nowEpoch > 0 && g_nextDoseEpochSec == 0) {
        // 间隔模式启用时，从当前时间开始计算下一次提醒
        g_nextDoseEpochSec = nowEpoch + g_doseIntervalSec;
        Serial.printf("[debug] Interval mode started. Next dose epoch = %llu\r\n", (unsigned long long)g_nextDoseEpochSec);
      }

      // 允许延迟触发：只要当前时间已到达（或超过）next，就触发一次并重排后续。
      if (g_nextDoseEpochSec > 0 && nowEpoch > 0 && nowEpoch >= g_nextDoseEpochSec) {
        // 避免同一个 scheduled epoch 因为卡顿重复触发
        if (g_lastTriggeredEpochSec != g_nextDoseEpochSec) {
          g_lastTriggeredEpochSec = g_nextDoseEpochSec;
          takeMed(); // 触发闹钟，执行对应函数

          // takeMed 里会更新 Day/armSet；如果已关闭则停止调度
          if (armSet) {
            uint64_t nowAfter = getNowEpochSec();
            uint64_t next = g_nextDoseEpochSec + g_doseIntervalSec;
            while (nowAfter > 0 && next <= nowAfter) next += (uint64_t)g_doseIntervalSec;
            g_nextDoseEpochSec = next;
          }
        }
      }
    }
    else {
      // 固定时刻模式：保持你原有逻辑
      if ((currentHour == armT[0].hour && currentMinute == armT[0].minute && currentSecond == 0) ||
          (currentHour == armT[1].hour && currentMinute == armT[1].minute && currentSecond == 0) ||
          (currentHour == armT[2].hour && currentMinute == armT[2].minute && currentSecond == 0))
      {
          takeMed();
      }
    }
  }
  vTaskDelay(pdMS_TO_TICKS(1000)); // 延时1秒
  
}