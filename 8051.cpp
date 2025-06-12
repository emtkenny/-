```cpp
#include <reg52.h>

#define DataPort P0
sbit LATCH1 = P2^2;  // 段鎖存
sbit LATCH2 = P2^3;  // 位鎖存
sbit KEY1 = P3^2;
sbit KEY2 = P3^3;
sbit KEY3 = P3^4;
sbit KEY4 = P3^5;
sbit KEY5 = P3^6;
sbit KEY6 = P3^7;

unsigned char code dofly_DuanMa[10] = {
  0x3f, 0x06, 0x5b, 0x4f,
  0x66, 0x6d, 0x7d, 0x07,
  0x7f, 0x6f
};

unsigned char code dofly_WeiMa[8] = {
  0xfe, 0xfd, 0xfb, 0xf7,
  0xef, 0xdf, 0xbf, 0x7f
};

unsigned char display_buf[2] = {0, 0};  // 右二位顯示
unsigned char current_city = 255;

// 延遲
void DelayMs(unsigned int ms) {
  unsigned int i, j;
  for (i = 0; i < ms; i++)
    for (j = 0; j < 120; j++);
}

// 顯示右二位氣溫（需周期掃描）
void DisplayTemperature() {
  static unsigned char i = 0;

  DataPort = 0;     // 防殘影
  LATCH1 = 1; LATCH1 = 0;

  DataPort = dofly_WeiMa[6 + i]; // 第7、8位
  LATCH2 = 1; LATCH2 = 0;

  DataPort = dofly_DuanMa[display_buf[i]];
  LATCH1 = 1; LATCH1 = 0;

  i = (i + 1) % 2;
}

// 傳送一個 byte 給 ESP32
void UART_SendByte(unsigned char dat) {
  SBUF = dat;
  while (!TI);
  TI = 0;
}

// 接收一個 byte（阻塞式）
unsigned char UART_ReceiveByte() {
  while (!RI);
  RI = 0;
  return SBUF;
}

// 初始化 UART
void UART_Init() {
  SCON = 0x50;  // 8-bit UART, enable RX
  TMOD |= 0x20; // Timer1 mode 2
  TH1 = 0xfd;   // 9600 baud @ 11.0592MHz
  TL1 = 0xfd;
  TR1 = 1;
  ES = 0;
}

// 按鍵掃描（P3.2~P3.7），回傳 0~5，否則 255
unsigned char ScanButton(void)
{
    if (!KEY1) {
        DelayMs(10);
        if (!KEY1) {
            while (!KEY1)DisplayTemperature() ; // 顯示不中斷
            return 0;  // 城市代碼 0
        }
    }
    else if (!KEY2) {
        DelayMs(10);
        if (!KEY2) {
            while (!KEY2)DisplayTemperature() ;
            return 1;
        }
    }
    else if (!KEY3) {
        DelayMs(10);
        if (!KEY3) {
            while (!KEY3)DisplayTemperature() ;
            return 2;
        }
    }
    else if (!KEY4) {
        DelayMs(10);
        if (!KEY4) {
            while (!KEY4)DisplayTemperature() ;
            return 3;
        }
    }
    else if (!KEY5) {
        DelayMs(10);
        if (!KEY5) {
            while (!KEY5)DisplayTemperature() ;
            return 4;
        }
    }
    else if (!KEY6) {
        DelayMs(10);
        if (!KEY6) {
            while (!KEY6)DisplayTemperature() ;
            return 5;
        }
    }

    return 255; // 沒有按任何鍵
}

void main() {
  unsigned char temp;

  UART_Init();

  while (1) {
    DisplayTemperature();  // 每圈掃描一位

    // 每次主迴圈掃一次按鈕
    current_city = ScanButton();

    if (current_city <= 5) {
      UART_SendByte(current_city);      // 傳送城市代碼
      DelayMs(300);                     // 等待 ESP32 回傳
      temp = UART_ReceiveByte();        // 讀回氣溫（單 byte）

      if (temp <= 99) {
        display_buf[0] = temp / 10;
        display_buf[1] = temp % 10;
      } else {
        display_buf[0] = 0;
        display_buf[1] = 0;
      }

      DelayMs(300); // 顯示時間後再重掃
    }
    DisplayTemperature();
  }
}
```
