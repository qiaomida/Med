#include "servo4.h"

Servo servo1; // 创建一个舵机对象
Servo servo2; // 创建一个舵机对象
Servo servo3; // 创建一个舵机对象
Servo servo4; // 创建一个舵机对象

void servo_init(void)
{
    servo1.attach(servoPin1); // 将舵机对象与引脚连接起来，指定舵机控制引脚
    servo2.attach(servoPin2); // 将舵机对象与引脚连接起来，指定舵机控制引脚
    servo3.attach(servoPin3); // 将舵机对象与引脚连接起来，指定舵机控制引脚
    servo4.attach(servoPin4); // 将舵机对象与引脚连接起来，指定舵机控制引脚
    servo1.write(0); // 将舵机初始角度设置为0度
    servo2.write(0); // 将舵机初始角度设置为0度
    servo3.write(0); // 将舵机初始角度设置为0度
    servo4.write(0); // 将舵机初始角度设置为0度
}
void servo_test(void)
{
    // 逐渐将舵机从0度转动到180度
  for (int angle = 0; angle <= 20; angle++) {
    servo1.write(angle); // 控制舵机转到特定角度
    servo2.write(angle); // 控制舵机转到特定角度
    servo3.write(angle); // 控制舵机转到特定角度
    servo4.write(angle); // 控制舵机转到特定角度
    delay(15); // 等待一段时间，使舵机有足够时间到达目标角度
  }
   
  delay(1000); // 在转动结束后等待一秒钟
   
  // 逐渐将舵机从180度转回到0度
  for (int angle = 20; angle >= 0; angle--) {
    servo1.write(angle); // 控制舵机转到特定角度
    servo2.write(angle); // 控制舵机转到特定角度
    servo3.write(angle); // 控制舵机转到特定角度
    servo4.write(angle); // 控制舵机转到特定角度
    delay(15); // 等待一段时间，使舵机有足够时间到达目标角度
  }
   
  delay(1000); // 在转动结束后等待一秒钟
}

void servo_open1(void)
{
  for (int angle = 0; angle <= MAX_ANGLE; angle++) {
    servo1.write(angle); // 控制舵机转到特定角度
    vTaskDelay(pdMS_TO_TICKS(15)); // 等待一段时间，使舵机有足够时间到达目标角度
  }
}

void servo_open2(void)
{
  for (int angle = 0; angle <= MAX_ANGLE; angle++) {
    servo2.write(angle); // 控制舵机转到特定角度
    vTaskDelay(pdMS_TO_TICKS(15)); // 等待一段时间，使舵机有足够时间到达目标角度  
  }
}

void servo_open3(void)
{
  for (int angle = 0; angle <= MAX_ANGLE; angle++) {
    servo3.write(angle); // 控制舵机转到特定角度
    vTaskDelay(pdMS_TO_TICKS(15)); // 等待一段时间，使舵机有足够时间到达目标角度  
  }
}

void servo_open4(void)
{
  for (int angle = 0; angle <= MAX_ANGLE; angle++) {
    servo4.write(angle); // 控制舵机转到特定角度
    vTaskDelay(pdMS_TO_TICKS(15)); // 等待一段时间，使舵机有足够时间到达目标角度  
  }
}

void servo_close1(void)
{
  for (int angle = MAX_ANGLE; angle >= 0; angle--) {
    servo1.write(angle); // 控制舵机转到特定角度
    vTaskDelay(pdMS_TO_TICKS(15)); // 等待一段时间，使舵机有足够时间到达目标角度
  }
}

void servo_close2(void)
{
  for (int angle = MAX_ANGLE; angle >= 0; angle--) {
    servo2.write(angle); // 控制舵机转到特定角度
    vTaskDelay(pdMS_TO_TICKS(15)); // 等待一段时间，使舵机有足够时间到达目标角度  
  }
}

void servo_close3(void)
{
  for (int angle = MAX_ANGLE; angle >= 0; angle--) {
    servo3.write(angle); // 控制舵机转到特定角度
    vTaskDelay(pdMS_TO_TICKS(15)); // 等待一段时间，使舵机有足够时间到达目标角度  
  }
}

void servo_close4(void)
{
  for (int angle = MAX_ANGLE; angle >= 0; angle--) {
    servo4.write(angle); // 控制舵机转到特定角度
    vTaskDelay(pdMS_TO_TICKS(15)); // 等待一段时间，使舵机有足够时间到达目标角度  
  }
}

void servo_openAll(void)
{
  for (int angle = 0; angle <= MAX_ANGLE; angle++) {
    servo1.write(angle); // 控制舵机转到特定角度
    servo2.write(angle); // 控制舵机转到特定角度
    servo3.write(angle); // 控制舵机转到特定角度
    servo4.write(angle); // 控制舵机转到特定角度
    vTaskDelay(pdMS_TO_TICKS(15)); // 等待一段时间，使舵机有足够时间到达目标角度
  }
  vTaskDelay(pdMS_TO_TICKS(500));
}

void servo_closeAll(void)
{
  for (int angle = MAX_ANGLE; angle >= 0; angle--) {
    servo1.write(angle); // 控制舵机转到特定角度
    servo2.write(angle); // 控制舵机转到特定角度
    servo3.write(angle); // 控制舵机转到特定角度
    servo4.write(angle); // 控制舵机转到特定角度
    vTaskDelay(pdMS_TO_TICKS(15)); // 等待一段时间，使舵机有足够时间到达目标角度
  }
  vTaskDelay(pdMS_TO_TICKS(500));
}