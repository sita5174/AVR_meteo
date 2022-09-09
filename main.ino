// настольная метеостанция с экраном и меню


#include <TFT.h>            // библиотека для дисплея TFT
#include <SPI.h>            // SPI библиотека для дисплея
#include <GyverPower.h>     // библиотека для режима сна
#include "DHT.h"            // библиотека для датчика влажности DHT11
#include "RotaryEncoder.h"  // библиотека для энкодера

// контакты дисплея
#define cs     10 
#define dc      9
#define rst     8

// позиционирование энкодера
#define STEPS   6
#define POSMIN  0
#define POSMAX 12

TFT TFTscreen = TFT(cs, dc, rst);   // контакты дисплея
RotaryEncoder encoder(7, 3);        // контакты энкодера
DHT dht(4, DHT11);                  // контакт датчика



//====== П Е Р Е М Е Н Н Ы Е =======//

volatile boolean awake_flag;        // флаг срабатывания прерывания
volatile unsigned long sleep_timer; // отсчет времени простоя

boolean menu_flag = false;          // флаг для рисования меню
int lastPos, newPos;                // позиция выбранного пункта меню
boolean buttonWasUp = true;         // флаг нажатия на кнопку

boolean sensor_flag = true;         // флаг отсчета времени для снятия показаний с датчика
unsigned long sensor_timer;         // отсчет времени для снятия показаний с датчика

int tempArray[120];                 // массив для хранения показаний температуры
int tcurrent = 0;                   // текущие показания температуры
int humArray[120];                  // массив для хранения показаний влажности
int hcurrent = 0;                   // текущие показания влажности

char currentString[5];              // массив для хранения текущего показания с датчика
char menuString[5];                 // массив для хранения показаний для текста над меню
int y_last = 127;                   // координата для рисования графика



//========= Ф У Н К Ц И И ==========//

// рисование меню
void start_menu(){
    TFTscreen.background(0, 0, 0);
    TFTscreen.setTextSize(1);

    // текст НАД менюшкой
    TFTscreen.stroke(255, 255, 255);    
    String menu_t = String((int)dht.readTemperature());
    menu_t.toCharArray(menuString, 3);
    TFTscreen.text("Current t:   C", 52, 10);
    TFTscreen.text(menuString, 113, 10);    
    String menu_h = String((int)dht.readHumidity());
    menu_h.toCharArray(menuString, 3);
    TFTscreen.text("Current h:   %", 52, 25);
    TFTscreen.text(menuString, 113, 25);

  if (lastPos == 0){
    TFTscreen.fill(255, 0, 255);
    TFTscreen.rect(35, 20+0*4 + 30, 120, 15);
    TFTscreen.fill(0, 255, 0);
    TFTscreen.rect(35, 20+6*4 + 30, 120, 15);
    TFTscreen.rect(35, 20+12*4 + 30, 120, 15);

    TFTscreen.stroke(255, 255, 255);
    TFTscreen.text("TEMPERATURE", 62, 20+0*4 + 34);
    TFTscreen.stroke(0, 0, 255);
    TFTscreen.text("HUMIDITY", 62, 20+6*4 + 34);
    TFTscreen.text("SLEEP MODE", 62, 20+12*4 + 34);
  } else if (lastPos == 6){
    TFTscreen.fill(255, 0, 255);
    TFTscreen.rect(35, 20+6*4 + 30, 120, 15);
    TFTscreen.fill(0, 255, 0);
    TFTscreen.rect(35, 20+0*4 + 30, 120, 15);
    TFTscreen.rect(35, 20+12*4 + 30, 120, 15);

    TFTscreen.stroke(255, 255, 255);
    TFTscreen.text("HUMIDITY", 62, 20+6*4 + 34);
    TFTscreen.stroke(0, 0, 255);
    TFTscreen.text("TEMPERATURE", 62, 20+0*4 + 34);
    TFTscreen.text("SLEEP MODE", 62, 20+12*4 + 34);
  } else {
    TFTscreen.fill(255, 0, 255);
    TFTscreen.rect(35, 20+12*4 + 30, 120, 15);
    TFTscreen.fill(0, 255, 0);
    TFTscreen.rect(35, 20+0*4 + 30, 120, 15);
    TFTscreen.rect(35, 20+6*4 + 30, 120, 15);

    TFTscreen.stroke(255, 255, 255);
    TFTscreen.text("SLEEP MODE", 62, 20+12*4 + 34);
    TFTscreen.stroke(0, 0, 255);
    TFTscreen.text("TEMPERATURE", 62, 20+0*4 + 34);
    TFTscreen.text("HUMIDITY", 62, 20+6*4 + 34);
  }
    menu_flag = true;
}

// выбор пункта меню с помощью энкодера
void encoder_tick(){
  encoder.tick();
  newPos = encoder.getPosition() * STEPS;
  if (newPos != lastPos)
    sleep_timer = millis();
  if (newPos < POSMIN){
    encoder.setPosition(POSMIN / STEPS);
    newPos = POSMIN;
  } else if (newPos > POSMAX){
    encoder.setPosition(POSMAX / STEPS);
    newPos = POSMAX;
  }
}

// отрисовка выбранного пункта меню
void pos_menu(){
  if (lastPos != newPos){
    TFTscreen.fill(0, 255, 0);
    TFTscreen.rect(35, 20+lastPos*4 + 30, 120, 15);
    if (lastPos == 0){
      TFTscreen.stroke(0, 0, 255);
      TFTscreen.text("TEMPERATURE", 62, 20+0*4 + 34);
    } else if (lastPos == 6){
      TFTscreen.stroke(0, 0, 255);
      TFTscreen.text("HUMIDITY", 62, 20+6*4 + 34);
    } else {
      TFTscreen.stroke(0, 0, 255);
      TFTscreen.text("SLEEP MODE", 62, 20+12*4 +34);
    }
    TFTscreen.fill(255, 0, 255);
    TFTscreen.rect(35, 20+newPos*4 + 30, 120, 15);
    if (newPos == 0){
      TFTscreen.stroke(255, 255, 255);
      TFTscreen.text("TEMPERATURE", 62, 20+0*4 + 34);
    } else if (newPos == 6){
      TFTscreen.stroke(255, 255, 255);
      TFTscreen.text("HUMIDITY", 62, 20+6*4 + 34);
    } else {
      TFTscreen.stroke(255, 255, 255);
      TFTscreen.text("SLEEP MODE", 62, 20+12*4 + 34);
    }    
    lastPos = newPos;
  }
}

// переход в выбранный пункт меню нажатием кнопки энкодера
void button_click(){
  boolean buttonIsUp = digitalRead(2);
  if (buttonWasUp && !buttonIsUp){
    delay(10);
    buttonIsUp = digitalRead(2);
    sleep_timer = millis();
    
    if (!buttonIsUp  && newPos == 0){
      drawScreenT();
      delay(3000);
      menu_flag = false;
    }
    if (!buttonIsUp  && newPos == 6){
      drawScreenH();
      delay(3000);
      menu_flag = false;
    }
    if (!buttonIsUp  && newPos == 12){
      TFTscreen.background(0, 0, 0);
      menu_flag = false;
      awake_flag = false;
      sensor_flag = false;
    }
  }
}

// снятие показаний температуры
void getTemp(){
  int i;
  for (i = 119; i >= 0; --i)
    tempArray[i] = tempArray[i - 1];
  tcurrent = (int)dht.readTemperature();
  tempArray[0] = tcurrent;
}

// снятие показаний влажности
void getHumid(){
  int i;
  for (i = 119; i >= 0; --i)
    humArray[i] = humArray[i - 1];
  hcurrent = (int)dht.readHumidity();
  humArray[0] = hcurrent;
}

// вывод на экран графика температуры
void drawScreenT(){
  int q;
  TFTscreen.background(0, 0, 0);
  TFTscreen.stroke(0, 255, 0);
  TFTscreen.setTextSize(1);
  TFTscreen.text("Current: ", 32, 0);
  String tempString = String(tcurrent);
  tempString.toCharArray(currentString, 3);
  TFTscreen.text(currentString, 80, 0);
  TFTscreen.text(" C", 87, 0);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.setTextSize(1);
  TFTscreen.setTextSize(1);
  TFTscreen.text("50", 35, 20);
  TFTscreen.text("45", 35, 30);
  TFTscreen.text("40", 35, 40);
  TFTscreen.text("35", 35, 50);
  TFTscreen.text("30", 35, 60);
  TFTscreen.text("25", 35, 70);
  TFTscreen.text("20", 35, 80);
  TFTscreen.text("15", 35, 90);
  TFTscreen.text("10", 35, 100);
  TFTscreen.text(" 5", 35, 110);
  TFTscreen.text(" 0", 35, 120);
  TFTscreen.line(32, 20, 32, 127);

  // рисуем график
  TFTscreen.stroke(0, 255, 0);
  for (int i = 25; i < 145; ++i){
    q = (123 - tempArray[i - 25]*2);
    TFTscreen.point(i+27, q);
    if (i != 25)
      TFTscreen.line(i+27, q, i+27, y_last);
    y_last = q;
  }
}

// вывод на экран графика влажности
void drawScreenH(){
  int q;
  TFTscreen.background(0, 0, 0);
  TFTscreen.stroke(0, 255, 0);
  TFTscreen.setTextSize(1);
  TFTscreen.text("Current: ", 32, 0);
  String humString = String(hcurrent);
  humString.toCharArray(currentString, 3);
  TFTscreen.text(currentString, 80, 0);
  TFTscreen.text(" %", 87, 0);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.setTextSize(1);
  TFTscreen.text("100", 35, 20);
  TFTscreen.text(" 90", 35, 30);
  TFTscreen.text(" 80", 35, 40);
  TFTscreen.text(" 70", 35, 50);
  TFTscreen.text(" 60", 35, 60);
  TFTscreen.text(" 50", 35, 70);
  TFTscreen.text(" 40", 35, 80);
  TFTscreen.text(" 30", 35, 90);
  TFTscreen.text(" 20", 35, 100);
  TFTscreen.text(" 10", 35, 110);
  TFTscreen.text("  0", 35, 120);
  TFTscreen.line(32, 20, 32, 127);

  // рисуем график
  TFTscreen.stroke(0, 255, 0);
  for (int i = 25; i < 145; ++i){
    q = (123 - humArray[i - 25]*2)/2;
    TFTscreen.point(i+32, q+65);
    if (i != 25)
      TFTscreen.line(i+32, q+65, i+32, y_last);
    y_last = q+65;
  }
}

// работа с меню
void menu(){
  if (!menu_flag)
    start_menu();   // рисование меню
  encoder_tick();   // выбор пункта меню с помощью энкодера
  pos_menu();       // отрисовка выбранного пункта меню
  button_click();   // переход в выбранный пункт меню нажатием кнопки энкодера
}

// обработчик прерывания
void setAwakeFlag(){
  if (power.inSleep())
    power.wakeUp();
  awake_flag = true;
  sleep_timer = millis();
}

void setup() {
  pinMode(2, INPUT_PULLUP);                  // кнопка энкодера
  attachInterrupt(0, setAwakeFlag, FALLING); // прерывание для выхода из спящего режима
  dht.begin();                               // запуск датчика
  TFTscreen.begin();                         // запуск дисплея
  TFTscreen.background(0, 0, 0);
}

void loop() {
  if (awake_flag){                           // режим бодрствования
    menu();                                  // работа с меню

    // регулярный сбор показаний с датчика
    if (sensor_flag){
      sensor_timer = millis();
      sensor_flag = false;
    }
    if(millis() - sensor_timer >= 10000){
      sensor_timer = millis();
      getTemp();
      getHumid();
    }

    // включение спящего режима при простое
    if(millis() - sleep_timer >= 30000){
      menu_flag = false;
      awake_flag = false;
      sensor_flag = false;
      TFTscreen.background(0, 0, 0);
    }
  } else {                                  // спящий режим и регулярный сбор показаний с датчика
    getTemp();                              // снятие показаний температуры
    getHumid();                             // снятие показаний влажности
    power.sleepDelay(10000);
  }
}
