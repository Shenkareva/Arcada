// Полная реализация HX711 без библиотек
#define HX711_DT 5
#define HX711_SCK 4

long zero_factor = 0;
float calibration_factor = 480.0; // Подберите при калибровке

void setup() {
  Serial.begin(115200);
  while (!Serial); // для Leonardo
  
  pinMode(HX711_SCK, OUTPUT);
  pinMode(HX711_DT, INPUT);
  
  Serial.println("HX711 без библиотек - запуск");
  
  // Калибровка нуля
  zero_factor = readHX711();
  Serial.print("Zero factor: ");
  Serial.println(zero_factor);
  
  Serial.println("Отправьте команды:");
  Serial.println("t - тарирование (сброс на ноль)");
  Serial.println("+ - увеличить калибровочный коэффициент");
  Serial.println("- - уменьшить калибровочный коэффициент");
}

long readHX711() {
  long count = 0;
  
  // Ждем пока DT станет LOW (готовность данных)
  while(digitalRead(HX711_DT) == HIGH) {
    yield();
  }
  
  // Читаем 24 бита данных
  for(int i = 0; i < 24; i++) {
    digitalWrite(HX711_SCK, HIGH);
    delayMicroseconds(1);
    count = count << 1;
    digitalWrite(HX711_SCK, LOW);
    delayMicroseconds(1);
    
    if(digitalRead(HX711_DT) == HIGH) {
      count++;
    }
  }
  
  // Устанавливаем канал и усиление для следующего чтения
  digitalWrite(HX711_SCK, HIGH);
  delayMicroseconds(1);
  digitalWrite(HX711_SCK, LOW);
  delayMicroseconds(1);
  
  // Преобразуем в signed 32-bit
  if(count & 0x800000) {
    count |= 0xFF000000;
  }
  
  return count;
}

void loop() {
  // Обработка команд из Serial
  if(Serial.available()) {
    char cmd = Serial.read();
    
    if(cmd == 't' || cmd == 'T') {
      zero_factor = readHX711();
      Serial.print("Тарирование выполнено. Zero factor: ");
      Serial.println(zero_factor);
    }
    else if(cmd == '+') {
      calibration_factor += 10;
      Serial.print("Калибровочный коэффициент: ");
      Serial.println(calibration_factor);
    }
    else if(cmd == '-') {
      calibration_factor -= 10;
      Serial.print("Калибровочный коэффициент: ");
      Serial.println(calibration_factor);
    }
  }
  
  // Чтение и отображение веса
  long raw_value = readHX711();
  float weight = (raw_value - zero_factor) / calibration_factor;
  
  Serial.print("Raw: ");
  Serial.print(raw_value);
  Serial.print(" | Weight: ");
  Serial.print(weight, 2);
  Serial.println(" g");
  
  delay(1000);
}

//  -- END OF FILE --
