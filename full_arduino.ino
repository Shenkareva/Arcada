#include <HX711.h>
#include <Servo.h>
#include <Keyboard.h>


HX711 scale;
Servo s1;
Servo s2;

const int photoPin = A2;
const int ledPin = 13;
const int joyX = A0;
const int joyY = A1;
const int joyBtn = 9;  // кнопка на джойстике
const int button = 10;

bool gameAllowed = false;//можно ли играть?

const int JOY_CENTER = 512;
const int JOY_DEADZONE = 25;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(photoPin, INPUT);
  pinMode(button, INPUT_PULLUP);

  digitalWrite(ledPin, HIGH);
  
  Serial.begin(9600);
  while (!Serial); // для Leonardo
  
  Keyboard.begin();
  s1.attach(12);
  s2.attach(11);
  s1.write(90);// блокируем путь
  s2.write(90);
  delay(300);
  s1.detach();// отключаем — чтобы не дёргался при замере
  s2.detach();

  scale.begin(5, 4);    
  scale.set_gain(128);
  
  scale.tare(100);// обнулить при старте
  Serial.println("Ноль установлен");
}
void good(){
  s2.attach(11);
  s2.write(19);      //Открыть путь в контейнер
  delay(500);

  s1.attach(12);
  s1.write(0);  //Отпустить монету
  delay(1000);
  
  // Возврат в исходное положение
  s1.write(90);
  delay(500);
  s1.detach();
  s2.write(90);
  delay(200);
  s2.detach();
}
void bad() {
  //открыть блокировку — монета падает на сдачу
  s1.attach(12);
  s1.write(0);
  delay(1000);
  s1.write(90);
  delay(200);
  s1.detach();
  Serial.println("Монета возвращена");
}

bool coin_tensor() { //проверим давит ли что-то на тензодатчик
  long raw = scale.read(); // читаем сырое значение (быстро!)
   if (raw > 70500){ // что-то давит
       Serial.println("Монета появилась");
       return 1;}
   else return 0;    
}

void stabilize() { //монета не должна волноваться
  delay(500); 
}

float mean(){//замер веса
  float w1 = scale.get_units(30); // 30 замеров для точности
  delay(100); // пауза между замерами

  float w2 = scale.get_units(30); // 30 замеров для точности
  delay(100); // пауза между замерами
  
  float w3 = scale.get_units(30); // 30 замеров для точности
  delay(100); // пауза между замерами

  float w4 = scale.get_units(30); // 30 замеров для точности
  delay(100); // пауза между замерами
  
  float w5 = scale.get_units(30);// 30 замеров для точности

  
  float weight = (w1 + w2+ w3+w4+w5) / 5.0; // мы хотим охренеть какую точность

  /*Serial.print("Замер 1: "); Serial.print(w1, 2); Serial.println(" г");
  Serial.print("Замер 2: "); Serial.print(w2, 2); Serial.println(" г");
  Serial.print("Замер 3: "); Serial.print(w3, 2); Serial.println(" г");
  Serial.print("Замер 4: "); Serial.print(w4, 2); Serial.println(" г");
  Serial.print("Замер 5: "); Serial.print(w5, 2); Serial.println(" г");*/
  
  return weight;
}

void controls() { // управление джойстиком и консолью
  
  if (!gameAllowed) return;

  int x = analogRead(joyX);
  if (x < JOY_CENTER - JOY_DEADZONE) {
    Keyboard.press(KEY_LEFT_ARROW);     
  } else if (x > JOY_CENTER + JOY_DEADZONE) {
    Keyboard.press(KEY_RIGHT_ARROW);     
  } else {
    Keyboard.release(KEY_LEFT_ARROW);
    Keyboard.release(KEY_RIGHT_ARROW);
  }

  int y = analogRead(joyY); 
  if (y < JOY_CENTER - JOY_DEADZONE) {
    Keyboard.press(KEY_UP_ARROW);
  } else if (y > JOY_CENTER + JOY_DEADZONE) {
    Keyboard.press(KEY_DOWN_ARROW);
  } else {
    Keyboard.release(KEY_UP_ARROW);
    Keyboard.release(KEY_DOWN_ARROW);
  }

  static bool lastBtn = HIGH;
  bool btnPressed = !digitalRead(button); 
  if (lastBtn == HIGH && btnPressed == true) {
    Keyboard.write(' '); //пробел для управления игрой
  }
  lastBtn = btnPressed;
}

void loop() {
  /*while(1){
    long raw = scale.read();
    Serial.println(raw);
    delay(100);
  }*/
  
  controls();  // джойстик и кнопка активны
  
  while (!coin_tensor()) { // монета на тензоре?
    continue; // ничего нет — ждём дальше
  }

  Serial.println("Стабилизация");
  stabilize();
  
  int light = analogRead(A2);
  scale.set_scale(1745.00); //самый важный коэффициент вообще нельзя удалять
 
  float weight = mean(); //вес монеты
  Serial.print("Средний вес: "); 
  Serial.print(weight, 2); 
  Serial.println(" г");
  
  // Распознавание
  bool valid = false;  //логическая переменная
  int coin = 0;
  if (weight >= 3.00 && weight <= 3.44) {
    Serial.println("1 рубль");
    valid = true;
    coin = 1;
  } 
  else if (weight >= 4.90 && weight <= 5.30) {
    Serial.println("2 рубля");
    valid = true;
    coin = 2;
  }
  else if (weight >= 5.48 && weight <= 5.85) {
    Serial.println("10 рублей");
    valid = true;
    coin = 10;
  }
  else if (weight >= 5.90 && weight <= 6.60) { 
    Serial.println("5 рублей");
    valid = true;
    coin = 5;
  }
  else {
    Serial.println("Неизвестная монета");
    valid = false;
  }

  if (valid) {  // подходит монетка
    good();
    Serial.println(light);
    if (light>600){  //монета перекрыла свет
      Serial.println("Монета принята");
      gameAllowed = true; // в игрушку можно поиграть
      switch (coin){
        case 1:
          Serial.println("COIN_1");
          break;
        case 2:
          Serial.println("COIN_2");
          break;
        case 5:
          Serial.println("COIN_5");
          break;
        case 10:
          Serial.println("COIN_10");
          break;}
      }
  } 
  else {
    bad();  //не подходит монетка
  }
  delay(500);
  
  while (coin_tensor()) {
    delay(50);
  }
}
