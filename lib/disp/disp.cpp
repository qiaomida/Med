#include <disp.h>
#include <esp_timer.h>

SSD1306Wire display(0x3c, 10, 9);   // 三个参数分别是 器件地址, SDA引脚, SCL引脚

// 时间变量
int currentHour = 0;
int currentMinute = 0;
int currentSecond = 0;

// 本地单调时钟：使用最近一次 NTP 成功校时的 epoch 作为基准，
// 再叠加 esp_timer 的单调递增时间，保证断网时不会“回到 00:00:00”。
static bool s_timeValid = false;
static uint64_t s_baseEpochSec = 0; // 校时成功时的 epoch（秒，已包含时区偏移）
static int64_t s_baseUs = 0;        // 校时成功时的单调时间（us，esp_timer_get_time）
static int32_t s_debugOffsetSec = 0;

uint64_t getNowEpochSec()
{
  if (!s_timeValid)
    return 0;

  int64_t nowUs = esp_timer_get_time();
  int64_t deltaUs = nowUs - s_baseUs;
  int64_t nowEpoch = (int64_t)s_baseEpochSec + (deltaUs / 1000000LL) + (int64_t)s_debugOffsetSec;
  if (nowEpoch < 0)
    return 0;
  return (uint64_t)nowEpoch;
}

void setDebugTimeOffsetSec(int32_t offsetSec)
{
  s_debugOffsetSec = offsetSec;
}

// NTP配置（东八区UTC+8）
#define NTP_OFFSET    8 * 3600
#define NTP_INTERVAL  60000
const char* NTP_SERVER = "cn.pool.ntp.org";  // 国内NTP服务器
WiFiUDP ntpUDP;


NTPClient timeClient(ntpUDP, NTP_SERVER, NTP_OFFSET, NTP_INTERVAL);


void updateNtpTime() {
  if (WiFi.status() == WL_CONNECTED) {
    // NTPClient::update() 返回值很关键：失败时 getEpochTime() 可能为 0，
    // 你现在遇到的“断网初始化”大概率就是这里把 0 写进了时间变量。
    if (timeClient.update()) {
      unsigned long epochTime = timeClient.getEpochTime();

      // 粗略校验 epoch 是否有效（避免 0/异常值污染本地时钟）
      if (epochTime > 1000000UL) {
        s_baseEpochSec = (uint64_t)epochTime;
        s_baseUs = esp_timer_get_time();
        s_timeValid = true;

        // 将NTP时间赋值给全局变量（所有文件都能访问）
        currentHour = (epochTime % 86400L) / 3600;
        currentMinute = (epochTime % 3600) / 60;
        currentSecond = epochTime % 60;
      }
    }
  }
}
// 更新系统时间（示例：可替换为RTC/网络时间同步逻辑）
void updateTime() {
  uint64_t nowEpoch = getNowEpochSec();
  if (nowEpoch > 0) {
    currentHour = (nowEpoch % 86400ULL) / 3600ULL;
    currentMinute = (nowEpoch % 3600ULL) / 60ULL;
    currentSecond = nowEpoch % 60ULL;
    return;
  }

  // 还没校时成功：保底继续用“递增模拟”，确保断网也不会把时间归零
  currentSecond++;
  if (currentSecond >= 60) {
    currentSecond = 0;
    currentMinute++;
    if (currentMinute >= 60) {
      currentMinute = 0;
      currentHour++;
      if (currentHour >= 24) {
        currentHour = 0;
      }
    }
  }
}

// 格式化并显示时间到OLED
void displayTime() 
{
  display.clear();  // 清空屏幕缓存
  
  // 设置字体（可选：ArialMT_Plain_10/16/24，数字显示推荐大字体）
  display.setFont(ArialMT_Plain_24);
  
  // 格式化时间字符串（补零，如 9:5:3 → 09:05:03）
  char timeStr[9];  // 存储 "HH:MM:SS"
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", 
           currentHour, currentMinute, currentSecond);
  
  // 居中显示时间（128x64屏幕，居中坐标x=64, y=26）
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 26, timeStr);
  
  // 刷新显示（将缓存内容写入屏幕）
  display.display();
}

void displayArm() 
{
  display.clear();  // 清空屏幕缓存
  
  // 设置字体（可选：ArialMT_Plain_10/16/24，数字显示推荐大字体）
  display.setFont(ArialMT_Plain_24);
  
  // 关键修改1：调整字符数组长度（原"HH:MM:SS"占8位+结束符=9，新格式"HH:MM:[N]"需更大空间）
  // 新格式示例："09:30:[12]"，预留足够长度避免缓冲区溢出，设为12足够容纳多位数disState
  char timeStr[12];  // 存储 "HH:MM:[N]"，替换原"HH:MM:SS"
  
  // 关键修改2：修改snprintf格式化字符串和参数
  // 格式串改为"%02d:%02d:[%d]"，对应：两位小时:两位分钟:[全局变量disState]
  // 时间来源替换为armT[]的成员（此处以armT[0]为例，可修改索引0/1/2切换不同闹钟）
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d [%d]", 
           armT[disState - 1].hour,  // 替换原currentHour
           armT[disState - 1].minute, // 替换原currentMinute
           disState);     // 替换原currentSecond，显示[N]格式
  
  // 居中显示时间（128x64屏幕，居中坐标x=64, y=26）—— 保留原逻辑不变
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 26, timeStr);
  
  // 刷新显示（将缓存内容写入屏幕）—— 保留原逻辑不变
  display.display();
}


void displayNoHour() 
{
  display.clear();  // 清空屏幕缓存
  
  // 设置字体（可选：ArialMT_Plain_10/16/24，数字显示推荐大字体）
  display.setFont(ArialMT_Plain_24);
  
  // 关键修改1：调整字符数组长度（原"HH:MM:SS"占8位+结束符=9，新格式"HH:MM:[N]"需更大空间）
  // 新格式示例："09:30:[12]"，预留足够长度避免缓冲区溢出，设为12足够容纳多位数disState
  char timeStr[12];  // 存储 "HH:MM:[N]"，替换原"HH:MM:SS"
  
  // 关键修改2：修改snprintf格式化字符串和参数
  // 格式串改为"%02d:%02d:[%d]"，对应：两位小时:两位分钟:[全局变量disState]
  // 时间来源替换为armT[]的成员（此处以armT[0]为例，可修改索引0/1/2切换不同闹钟）
  snprintf(timeStr, sizeof(timeStr), "    :%02d [%d]", 
           armT[disState - 1].minute, // 替换原currentMinute
           disState);     // 替换原currentSecond，显示[N]格式
  
  // 居中显示时间（128x64屏幕，居中坐标x=64, y=26）—— 保留原逻辑不变
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 26, timeStr);
  
  // 刷新显示（将缓存内容写入屏幕）—— 保留原逻辑不变
  display.display();
}

void displayNoMinute() 
{
  display.clear();  // 清空屏幕缓存
  
  // 设置字体（可选：ArialMT_Plain_10/16/24，数字显示推荐大字体）
  display.setFont(ArialMT_Plain_24);
  
  // 关键修改1：调整字符数组长度（原"HH:MM:SS"占8位+结束符=9，新格式"HH:MM:[N]"需更大空间）
  // 新格式示例："09:30:[12]"，预留足够长度避免缓冲区溢出，设为12足够容纳多位数disState
  char timeStr[12];  // 存储 "HH:MM:[N]"，替换原"HH:MM:SS"
  
  // 关键修改2：修改snprintf格式化字符串和参数
  // 格式串改为"%02d:%02d:[%d]"，对应：两位小时:两位分钟:[全局变量disState]
  // 时间来源替换为armT[]的成员（此处以armT[0]为例，可修改索引0/1/2切换不同闹钟）
  snprintf(timeStr, sizeof(timeStr), "%02d:     [%d]", 
           armT[disState - 1].hour, // 替换原currentMinute
           disState);     // 替换原currentSecond，显示[N]格式
  
  // 居中显示时间（128x64屏幕，居中坐标x=64, y=26）—— 保留原逻辑不变
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 26, timeStr);
  
  // 刷新显示（将缓存内容写入屏幕）—— 保留原逻辑不变
  display.display();
}

void displayTimeAlarm() 
{
  display.clear();  // 清空屏幕缓存
  
  // 设置小字体显示"DAY N"
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  char dayStr[16];  // 存储"DAY N"格式
  snprintf(dayStr, sizeof(dayStr), "DAY %d", Day);  // 格式化字符串
  display.drawString(0, 0, dayStr);  // 左上角显示"DAY N"
  
  // 设置小字体显示"ALARM"
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 0, "ALARM");  // 右上角显示"ALARM"
  
  // 设置字体（可选：ArialMT_Plain_10/16/24，数字显示推荐大字体）
  display.setFont(ArialMT_Plain_24);
  
  // 格式化时间字符串（补零，如 9:5:3 → 09:05:03）
  char timeStr[9];  // 存储 "HH:MM:SS"
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", 
           currentHour, currentMinute, currentSecond);
  
  // 居中显示时间（128x64屏幕，居中坐标x=64, y=26）
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 26, timeStr);
  
  // 刷新显示（将缓存内容写入屏幕）
  display.display();
}


// void displayDAY() 
// {
//   display.clear();  // 清空屏幕缓存，保留原逻辑
  
//   // 设置字体（可选：ArialMT_Plain_10/16/24，保留原字体设置，如需适配可调整）
//   display.setFont(ArialMT_Plain_24);
  
//   // 关键修改1：调整字符数组长度，容纳"QAQ  DayN"格式
//   // 示例："QAQ  Day123"，预留足够长度（16）避免缓冲区溢出，兼容多位数Day
//   char showStr[16];  // 替换原timeStr，存储最终显示字符串"QAQ  DayN"
  
//   // 关键修改2：修改snprintf格式化字符串和参数，生成"QAQ  DayN"
//   // 格式串"%s  Day%d"：固定字符串"QAQ" + 空格 + 固定字符串"Day" + 全局变量Day
//   snprintf(showStr, sizeof(showStr), "QAQ  Day%d", Day);
  
//   // 居中显示字符串，保留原屏幕坐标（128x64屏幕居中），逻辑不变
//   display.setTextAlignment(TEXT_ALIGN_CENTER);
//   display.drawString(64, 26, showStr);
  
//   // 刷新显示，将缓存内容写入屏幕，保留原逻辑
//   display.display();
// }

void displayDAY() 
{
  display.clear();  // 清空屏幕缓存，保留原逻辑
  
  // 设置字体（可选：ArialMT_Plain_10/16/24，保留原字体设置，如需适配可调整）
  display.setFont(ArialMT_Plain_24);
  
  // 修改字符数组长度，容纳"DayN [M]"格式
  // 示例："Day123 [45]"，预留足够长度（16）避免缓冲区溢出
  char showStr[16];  // 存储"DayN [M]"格式
  
  // 修改snprintf格式化字符串和参数
  // 格式串"Day%d [%d]"：固定字符串"Day" + 全局变量Day + 固定字符串" [" + ArmTime::getarmCiShu() + 1 + "]"
  snprintf(showStr, sizeof(showStr), "Day%d [%d]", 
           Day,  // 第一个参数：全局变量Day
           ArmTime::getarmCiShu() + 1);  // 第二个参数：获取闹钟次数并加1
  
  // 居中显示字符串，保留原屏幕坐标（128x64屏幕居中），逻辑不变
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 26, showStr);
  
  // 刷新显示，将缓存内容写入屏幕，保留原逻辑
  display.display();
}


//显示联网对时初始化中
void displayInitializing() 
{
  display.clear();  // 清空屏幕缓存
  
  // 设置字体（可选：ArialMT_Plain_10/16/24，英文显示适配原西文字体，推荐大字体）
  display.setFont(ArialMT_Plain_16);
  
  // 要显示的固定字符串："initializing..."
  const char* initializingStr = "initializing...";
  
  // 居中显示初始化提示文字（128x64屏幕，居中坐标x=64, y=26，与原显示保持一致）
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 26, initializingStr);
  
  // 刷新显示（将缓存内容写入屏幕）
  display.display();
}

//画线
void drawLines()
{
  for (int16_t i = 0; i < display.getWidth(); i += 4)
  {
    display.drawLine(0, 0, i, display.getHeight() - 1);
    display.display();
    delay(10);
  }
  for (int16_t i = 0; i < display.getHeight(); i += 4)
  {
    display.drawLine(0, 0, display.getWidth() - 1, i);
    display.display();
    delay(10);
  }
  delay(250);

  display.clear();
  for (int16_t i = 0; i < display.getWidth(); i += 4)
  {
    display.drawLine(0, display.getHeight() - 1, i, 0);
    display.display();
    delay(10);
  }
  for (int16_t i = display.getHeight() - 1; i >= 0; i -= 4)
  {
    display.drawLine(0, display.getHeight() - 1, display.getWidth() - 1, i);
    display.display();
    delay(10);
  }
  delay(250);

  display.clear();
  for (int16_t i = display.getWidth() - 1; i >= 0; i -= 4)
  {
    display.drawLine(display.getWidth() - 1, display.getHeight() - 1, i, 0);
    display.display();
    delay(10);
  }
  for (int16_t i = display.getHeight() - 1; i >= 0; i -= 4)
  {
    display.drawLine(display.getWidth() - 1, display.getHeight() - 1, 0, i);
    display.display();
    delay(10);
  }
  delay(250);
  display.clear();
  for (int16_t i = 0; i < display.getHeight(); i += 4)
  {
    display.drawLine(display.getWidth() - 1, 0, 0, i);
    display.display();
    delay(10);
  }
  for (int16_t i = 0; i < display.getWidth(); i += 4)
  {
    display.drawLine(display.getWidth() - 1, 0, i, display.getHeight() - 1);
    display.display();
    delay(10);
  }
  delay(250);
}

//画矩形
void drawRect(void)
{
  for (int16_t i = 0; i < display.getHeight() / 2; i += 2)
  {
    display.drawRect(i, i, display.getWidth() - 2 * i, display.getHeight() - 2 * i);
    display.display();
    delay(10);
  }
}

//画填充矩形
void fillRect(void)
{
  uint8_t color = 1;
  for (int16_t i = 0; i < display.getHeight() / 2; i += 3)
  {
    display.setColor((color % 2 == 0) ? BLACK : WHITE); // alternate colors
    display.fillRect(i, i, display.getWidth() - i * 2, display.getHeight() - i * 2);
    display.display();
    delay(10);
    color++;
  }
  // Reset back to WHITE
  display.setColor(WHITE);
}

// 画圆
void drawCircle(void)
{
  
  display.clear();

  display.drawCircleQuads(display.getWidth() / 2, display.getHeight() / 2, display.getHeight() / 4, 0b00000001);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(200));
  display.drawCircleQuads(display.getWidth() / 2, display.getHeight() / 2, display.getHeight() / 4, 0b00000011);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(200));
  display.drawCircleQuads(display.getWidth() / 2, display.getHeight() / 2, display.getHeight() / 4, 0b00000111);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(200));
  display.drawCircleQuads(display.getWidth() / 2, display.getHeight() / 2, display.getHeight() / 4, 0b00001111);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(500));
  for (int16_t i = 0; i < display.getHeight(); i += 2)
  {
    display.drawCircle(display.getWidth() / 2, display.getHeight() / 2, i);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  vTaskDelay(pdMS_TO_TICKS(500));
}

void printBuffer(void)
{
  // 初始化日志缓冲区
  // 分配内存来存储8行文本，每行30个字符。
  display.setLogBuffer(5, 30);

  // 测试内容
  const char* test[] =
  {
    "Hello",
    "World" ,
    "----",
    "Show off",
    "how",
    "the log buffer",
    "is",
    "working.",
    "Even",
    "scrolling is",
    "working"
  };

  for (uint8_t i = 0; i < 11; i++)
  {
    //清屏
    display.clear();
    //打印到屏幕并且换行
    display.println(test[i]);
    // 将其绘制到内部屏幕缓冲区
    display.drawLogBuffer(0, 0);
    // 把它显示在屏幕上
    display.display();
    delay(500);
  }
}

void display_test(void)
{
  //设置对比度
  display.setContrast(255);

  //画线测试
  drawLines();
  delay(1000);
  //清屏
  display.clear();
  //画矩形测试
  drawRect();
  delay(1000);
  //清屏
  display.clear();
  //画填充矩形测试
  fillRect();
  delay(1000);
  //清屏
  display.clear();
  //画圆测试
  drawCircle();
  delay(1000);
  display.clear();
  //显示文本测试
  printBuffer();
  delay(1000);
  display.clear();
}

void display_initt()
{
  display.init();//屏幕初始化，配置显示参数
}