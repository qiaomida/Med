#include "button.h"

    int disState = 0;//一共五个状态，0是显示时间，1 2 3是三个时段，4是闹钟触发
    int timePos = 0;// 0是小时，1是分钟
    int Day = 1;
    bool armSet = false;//是否设置闹钟
    ArmTime armT[3];//三个时段
    int CiShuMAX = 2; //记录闹钟触发多少次进入下一天.默认为已经触发2次后再次触发则进入下一天

    int openAllState = 0;

int ArmTime::armCiShu = 0;


OneButton btn0(
BUTTON_PIN0,  // 按钮的输入引脚
true,        // 低电平触发
true         // 启用内部上拉电阻
);

OneButton btn1(
BUTTON_PIN1,  // 按钮的输入引脚
true,        // 低电平触发
true         // 启用内部上拉电阻
);

OneButton btn2(
BUTTON_PIN2,  // 按钮的输入引脚
true,        // 低电平触发
true         // 启用内部上拉电阻
);

OneButton btn3(
BUTTON_PIN3,  // 按钮的输入引脚
true,        // 低电平触发
true         // 启用内部上拉电阻
);

/**
 * @brief 按钮初始化函数
 * 该函数用于初始化三个按钮(btn0, btn1, btn2)的事件绑定
 * 包括点击事件、长按事件等回调函数的设置
 */
void buttoninit()
{
    // 重置所有按钮状态
    btn0.reset();
    btn1.reset();
    btn2.reset();
    btn3.reset();

    
    // 为btn0绑定事件
    btn0.attachClick(switchPos);     // 绑定单击事件回调函数switchPos
    btn0.attachLongPressStart(nextState);  // 绑定长按开始事件回调函数nextState

    
    // 为btn1绑定事件
    btn1.attachClick(down);          // 绑定单击事件回调函数down
    btn1.attachDuringLongPress(downPress); // 绑定长按持续期间事件回调函数downPress

    
    // 为btn2绑定事件
    btn2.attachClick(up);            // 绑定单击事件回调函数up
    btn2.attachDuringLongPress(upPress);   // 绑定长按持续期间事件回调函数upPress

    // 为btn3绑定事件
    btn3.attachClick(lastState);   // 绑定单击事件回调函数lastState
    btn3.attachLongPressStart(openAll);   // 绑定长按开始事件回调函数openAll
}

/**
 * @brief ArmTime类的down函数，用于调整时间
 * 当disState为0或4时直接返回，否则根据timePos的值调整小时或分钟
 */
void ArmTime::down()
{
    // 如果当前状态为0或4，则直接返回，不做任何操作
    if (disState == 0 || disState == 4)
    {
        return;
    }
    else
    {
        // 根据timePos的值选择调整小时还是分钟
        switch (timePos)
            {
            // 当timePos为0时，调整小时
            case 0:
                // 如果当前小时不为0，则小时数减1
                if (hour != 0)
                {
                    hour--;
                }
                // 如果当前小时为0，则循环到23
                else
                {
                    hour = 23;
                }
                // 显示调整后的时间
                displayArm();
                break;
            // 当timePos为1时，调整分钟
            case 1:
                // 如果当前分钟不为0，则分钟数减1
                if (minute != 0)
                {
                    minute--;
                }
                // 如果当前分钟为0，则循环到59
                else
                {
                    minute = 59;
                }
                // 显示调整后的时间
                displayArm();
                break;
            // 其他情况直接返回
            default:
                return;
                break;
            }
    }
}

/**
 * @brief ArmTime类的up函数，用于增加时间
 * 根据当前timePos的值来增加小时或分钟
 */
void ArmTime::up()
{
    // 如果disState为0或4，则直接返回，不做任何操作
    if (disState == 0 || disState == 4)
    {
        return;
    }
    // 根据timePos的值进行不同的时间增加操作
    switch (timePos)
    {
    case 0:  // 处理小时增加的情况
        if (hour != 23)  // 如果当前小时不是23点
        {
            hour++;  // 小时数加1
        }
        else  // 如果是23点
        {
            hour = 0;  // 小时数归0
        }
        displayArm();  // 更新显示
        break;
    case 1:  // 处理分钟增加的情况
        if (minute != 59)  // 如果当前分钟不是59分
        {
            minute++;  // 分钟数加1
        }
        else  // 如果是59分
        {
            minute = 0;  // 分钟数归0
        }
        displayArm();  // 更新显示
        break;
    default:  // 其他情况直接返回
        return;
        break;
    }
}

/**
 * 完成一次操作后的处理函数
 * 根据当前次数和第几天进行相应的状态更新
 */
void ArmTime::finishOne()
{
    // 如果当天已经触发的闹钟次数为CiShuMAX(默认为2)
    if (armCiShu == CiShuMAX)
    {
        // 如果是第四天
        if (Day == 4)
        {
            // 重置为第一天，解除闹钟状态，重置次数
            Day = 1;
            armSet = false;
            armCiShu = 0;
        }
        // 如果不是第四天
        else
        {
            // 进入下一天，重置闹钟次数
            Day++;
            armCiShu = 0;
        }
    }
    // 如果当天已经触发的闹钟次数不为CiShuMAX
    else
    {
        // 增加触发次数
        armCiShu++;
    }
}

/**
 * 获取闹钟次数的成员函数
 * @return 返回当前闹钟次数的值
 */
int ArmTime::getarmCiShu()
{
    return armCiShu;  // 返回armCiShu成员变量的值
}

/**
 * 初始化闹钟计数器的方法
 * 将臂计数器(armCiShu)的值重置为0
 */
void ArmTime::initarmCiShu()
{
    ArmTime::armCiShu = 0;  // 将臂计数器的值设置为0
}

/**
 * 执行下降操作
 * 该函数根据当前显示状态(disState)调用对应的闹钟(armT)的时间下降方法
 */
void down()
{
    // 根据当前显示状态(disState)减1作为索引，调用对应手臂对象的down方法
    armT[disState - 1].down();
}

void downPress()
{
    if (btn1.isLongPressed())
    {
        armT[disState - 1].down();
    }
    vTaskDelay(pdMS_TO_TICKS(75));//稍作延时处
}


void up()
{
    armT[disState - 1].up(); 
}

void upPress()
{
    if (btn2.isLongPressed())
    {
        armT[disState - 1].up();
    }
    vTaskDelay(pdMS_TO_TICKS(75));//稍作延时处
}



/**
 * 切换位置函数
 * 根据显示状态(disState)来决定是否切换位置(timePos)
 * 当disState为1、2或3时执行切换操作
 */
void switchPos()
{
    // 检查显示状态是否为1、2或3
    if (disState == 1 || disState == 2 || disState == 3)
    {
        // 如果当前位置不是1，则位置值加1
        if (timePos != 1)
        {
            timePos++;
        }
        else
        {
            // 如果当前位置已经是1，则重置为0
            timePos = 0;
        }
    }
}

/**
 * @brief 切换到下一个状态函数
 * 该函数用于处理状态转换逻辑，根据当前状态值(disState)决定下一步操作
 */
void nextState()
{
    // 检查当前状态是否不等于3
    if (disState != 3)
    {
        // 重置时间位置为0
        timePos = 0;
        // 设置armSet标志为false，暂时关闭闹钟
        armSet = false;
        // 状态值递增，进入下一个状态
        disState++;
    }
    else
    {
        // 当状态为3时，重置天数为1
        Day = 1;
        if ((armT[0].hour == armT[1].hour && armT[0].minute == armT[1].minute && (armT[1].hour != armT[2].hour || armT[1].minute != armT[2].minute)) ||
            (armT[0].hour == armT[2].hour && armT[0].minute == armT[2].minute && (armT[0].hour != armT[1].hour || armT[0].minute != armT[1].minute)) ||
            (armT[1].hour == armT[2].hour && armT[1].minute == armT[2].minute && (armT[0].hour != armT[1].hour || armT[0].minute != armT[1].minute)))
            {
                // 如果只有两个闹钟时间重复，将CiShuMAX设置为1
                CiShuMAX = 1;
            }
        else if (armT[0].hour == armT[1].hour && armT[0].minute == armT[1].minute && armT[1].hour == armT[2].hour && armT[1].minute == armT[2].minute)
            {
                // 如果三个闹钟时间都重复，将CiShuMAX设置为0
                CiShuMAX = 0;
            }
        else
            {
                // 否则，将CiShuMAX设置为2
                CiShuMAX = 2;
            }
        // 设置armSet标志为true，打开闹钟
        armSet = true;
        // 将状态重置为0，形成循环
        disState = 0;
        // 初始化闹钟已触发次数
        ArmTime::initarmCiShu();
    }
}

/**
 * @brief 上一个状态的切换函数
 * 根据当前显示状态(disState)切换到上一个状态
 * 当状态为2或3时，会重置时间位置并将显示状态减1
 */
void lastState()
{
    // 判断当前显示状态是否为2或3
    if (disState == 2 || disState == 3)
    {
        // 重置时间位置为0
        timePos = 0;
        // 重置手臂设置标志为false
        armSet = false;
        // 显示状态减1，切换到上一个状态
        disState--;
    }
    else
    {
        // 如果当前状态不是2或3，则直接返回，不执行任何操作
        return;
    }
}

/**
 * 打开/关闭所有舵机（盖子）的函数
 * 该函数根据当前状态切换所有设备的开关状态
 */
void openAll()
{
    // 检查当前状态是否为0（关闭状态）
    if (openAllState == 0)
    {
        // 如果当前是关闭状态，则调用打开所有设备的函数
        servo_openAll();
        // 更新状态为1（打开状态）
        openAllState = 1;
    }
    else
    {
        // 如果当前是打开状态，则调用关闭所有设备的函数
        servo_closeAll();
        // 更新状态为0（关闭状态）
        openAllState = 0;
    }
}