#ifndef _CARMTIME_H__
#define _CARMTIME_H__

#include <Arduino.h>

/**
 * @brief ArmTime 类，用于表示时间并记录当天闹钟触发的次数
 */
class ArmTime
{
  public:
  int hour;      // 小时变量
  int minute;    // 分钟变量
  static int armCiShu;  // 静态成员变量，用于记录当天闹钟触发次数

  /**
   * @brief 时间减少的方法
   */
  void down();

  /**
   * @brief 时间增加的方法
   */
  void up();

  /**
   * @brief 完成一次闹钟的记录
   */
  static void finishOne();

  /**
   * @brief 获取当天闹钟次数的方法
   * @return 返回当天闹钟次数
   */
  static int getarmCiShu();

  /**
   * @brief 初始化闹钟次数的方法
   */
  static void initarmCiShu();

  /**
   * @brief 构造函数，初始化时间为0时0分
   */
  ArmTime():hour(0), minute(0){}
};


#endif // _CARMTIME_H__