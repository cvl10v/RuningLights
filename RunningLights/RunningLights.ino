// Running Lights ver. 1.0
// Author: Vladimir Chaly
// 06.17.2024

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ButtonState.h"

// Кнопки с однократным нажатием.  
#define StartBtnPin 2         // Пин подключен к кнопке "Старт".
#define StopBtnPin 5          // Пин подключен к кнопке "Стоп".
#define ReverseBtnPin 6       // Пин подключен к кнопке "Реверс".
// Кнопки с срабатыванением при удержании.
#define RateIncreaseBtnPin 3  // Пин подключен к кнопке "Увеличение скорости".
#define RateDecreaseBtnPin 4  // Пин подключен к кнопке "Уменьшение скорости".

#define ReverseLedPin 7       // Пин подключен к светодиоду "Включен реверс".

// Подключаем 8-ми разрядный сдвиговый регистер 74HC595
#define dataPin 8    // Пин подключен к DS входу. Вход регистра, на который поступают последовательные данные (Data Serial).
#define latchPin 9   // Пин подключен к ST_CP входу. На этот вход подают синхроимпульс для передачи данных из внутренних ячеек, во внешние (синхронизация выходов). ST - storage, CP - clock pin.
#define clockPin 10  // Пин подключен к SH_CP входу. На этот вход подают синхроимпульс для передачи данных из DS во внутренние ячейки. SH - shift, CP - clock pin.

// Скорость переключения
int Rate = 10;     // Скорость переключения установленная по умолчанию. 10*50 = 500 ms.
int RateStep = 50; // Шаг увеличения/уменьшения скорости 50 ms.

int maxRate = 20;  // Максимальная скорость
int minRate = 1;   // Минимальная скорость

int RateInMSec = 0; // Скорость в миллисекундах

int Pos = 0; // Текущая позиция горящего светодиода

bool Start = false;           // Флаг Старт. Отключен.  
bool Reverse = false;         // Флаг Реверс. Отключен.

ButtonState btnState;

byte regNum = 6; // Максимальное количество задействованных разрядов регистра.

void setup() {

  // Инициализируем кнопки.
  // Кнопки с одним нажатием. 
  btnState.Add(StartBtnPin, Normal);
  btnState.Add(StopBtnPin, Normal);
  btnState.Add(ReverseBtnPin, Normal);

  // Кнопки с автоматическим срабатыванием при удержании кнопки нажатой.
  btnState.Add(RateIncreaseBtnPin, Pressed);
  btnState.Add(RateDecreaseBtnPin, Pressed); 
  // 
  pinMode(StartBtnPin, INPUT_PULLUP);
  pinMode(StopBtnPin, INPUT_PULLUP); 
  pinMode(ReverseBtnPin, INPUT_PULLUP);
  pinMode(RateIncreaseBtnPin, INPUT_PULLUP);
  pinMode(RateDecreaseBtnPin, INPUT_PULLUP);

  pinMode(latchPin, OUTPUT); 
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT); 

  // Устанавливаем нули на выходах регистра.
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, 0);
  digitalWrite(latchPin, HIGH);

  pinMode(ReverseLedPin, OUTPUT);
  digitalWrite(ReverseLedPin, LOW);

  // Запускаем прирывание по таймеру для опроса состояния кнопок на клавиатуре.
  // Отключаем прерывания
  cli();
  // Инициализируем регистры TCCR1A, TCCR1B
  TCCR1A = 0;
  TCCR1B = 0;
  // Устанавливанием коэффициент деления основной частоты на 1024.
  // 16 МГц / 1024 = 15 625 Гц. 
  TCCR1B |= (1<<CS10);
  TCCR1B |= (1<<CS12); 
  // Включаем прерывание по переполнению счетчика.
  TCCR1B |= (1<<WGM12); 
  // Делим 15 625 Гц на 1116.
  OCR1A = 1116; // Такт прерывания - примерно 71 ms (14 раз в секунду).
  // Разрешаем прерывание по переполнению счетчика.
  TIMSK1 |= (1<<OCIE1A);
  // Включаем глобальное прерывание.
  sei();

}

void loop() {

    byte byteToSend = 0;
    // Двигаем бит с первого канала до последнего.
    for (int bitPos = 0; bitPos < regNum; bitPos++) {
      // Выключаем светодиоды, если флаг "Старт" устновлен в false.
      if ( Start == false ) { 

          digitalWrite(latchPin, LOW); // Начинаем передачу данных 
          shiftOut(dataPin, clockPin, LSBFIRST, 0); // Передаем нули.
          digitalWrite(latchPin, HIGH); // Заканчиваем передачу данных 
          break;

      }

      bitWrite(byteToSend, bitPos, HIGH); // Поднимаем один бит в слове в заданной позиции.

      digitalWrite(latchPin, LOW);                       
      shiftOut(dataPin, clockPin, LSBFIRST, byteToSend);  
      digitalWrite(latchPin, HIGH);   

      RateInMSec = 1020 - Rate*RateStep; // Рассчитываем скорость переключения.
      delay(RateInMSec);
      // После задержки гасим светодиод.
      bitWrite(byteToSend, bitPos, LOW);

      digitalWrite(latchPin, LOW);                       
      shiftOut(dataPin, clockPin, LSBFIRST, byteToSend);  
      digitalWrite(latchPin, LOW);   

      Pos = bitPos;

    }

    if ( Reverse ) {

    Pos--; //  
    // Начинаем с предпоследнего канала и заканчиваем вторым.
    for (int bitPos = Pos; bitPos > 0; bitPos--) {

        if ( Start == false ) { 

          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, LSBFIRST, 0);
          digitalWrite(latchPin, HIGH);  
          break;

        }

        bitWrite(byteToSend, bitPos, HIGH);

        digitalWrite(latchPin, LOW);                       
        shiftOut(dataPin, clockPin, LSBFIRST, byteToSend);  
        digitalWrite(latchPin, HIGH);   

        RateInMSec = 1020 - Rate*RateStep;
        delay(RateInMSec);

        bitWrite(byteToSend, bitPos, LOW);

        digitalWrite(latchPin, LOW);                       
        shiftOut(dataPin, clockPin, LSBFIRST, byteToSend);  
        digitalWrite(latchPin, LOW);

    }

    }

}

// Прерывание по таймеру.
ISR(TIMER1_COMPA_vect) {

   bool b;
   // Проверяем нажали ли кнопка "Старт". 
   b = btnState.CheckState(StartBtnPin);
   // Если нажата, то устнавливаем флаг "Cтарт" в true  
   if ( b ) {
      Start = true;
   }
   // Проверяем нажали ли кнопка "Стоп".
   b = btnState.CheckState(StopBtnPin);
   // Если нажата, то устнавливаем флаг "Cтарт" в false
   if ( b ) {
      Start = false;
   }
   // Проверяем нажали ли кнопка "Реверс".
   b = btnState.CheckState(ReverseBtnPin);
   // Устанавливаем флаг "Реверс" в противоположное значение, если нажата.  
   if ( b ) {
    Reverse = !Reverse;
    // Включаем/выключаем светодиод "Вкл. реверс".
    if (Reverse) {
        digitalWrite(ReverseLedPin, HIGH);
    } else {
        digitalWrite(ReverseLedPin, LOW);
    }
   }

  // Скорость регулируется, когда горят светодиоды.
  if (Start) {
    // Проверяем нажали ли кнопка "Увеличение скорости".
    b = btnState.CheckState(RateIncreaseBtnPin);  
    // Увеличиваем скорость до максимальной.
    if( b && Rate < maxRate) {
      Rate++;
    }
    // Проверяем нажали ли кнопка "Уменьшение скорости".
    b = btnState.CheckState(RateDecreaseBtnPin); 
    // Уменьшаем скорость до минимальной.
    if( b && Rate > minRate) {
      Rate--;
    }
  }

}