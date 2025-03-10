#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN     3  // WS2812 data pin
#define LED_COUNT 70  // Number of LEDs in strip
#define VOL_UP 2
#define VOL_DN 4

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Define color constants
const uint32_t TIFFANY_BLUE = strip.Color(0, 255, 100);
const uint32_t RED = strip.Color(255, 0, 0);
const uint32_t GREEN = strip.Color(0, 255, 0);
const uint32_t BLUE = strip.Color(0, 0, 255);
const uint32_t WHITE = strip.Color(255, 255, 255);
const uint32_t OFF = strip.Color(1, 1, 1);

// 定義腳位
const int inputPin = 7;
const int ledPin = 13; // 使用內建LED作為指示燈
const int detectPin = 5;

// 定義時間常數 (單位: 毫秒)
const unsigned long debounceTime = 2000; // 防彈跳時間，小於此時間的變化視為雜訊
const unsigned long stopTime = 3000;     // 超過此時間沒有變化則輸出STOP

// 宣告變數
int lastState = LOW;             // 上一次的腳位狀態
unsigned long lastDebounceTime = 0; // 上一次腳位變化的時間
bool playing = false;            // 播放狀態


// 新增副程式，設定腳位模式和值
void setPinModeAndValue(int pin, int value) {
  if (value == 0) {

    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  } else if (value == 1) {
    pinMode(pin, INPUT);

  } else {
    // 錯誤處理：不支援的值
    Serial.println("Error: Invalid value for pin mode.");
  }
}

// Function to control the WS2812 LED strip
void controlLEDStrip(bool on, uint32_t color) {
  if (on) {
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, color);
    }
  } else {
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, OFF);
    }
  }
  strip.show(); // Send the updated pixel colors to the hardware.
}

void setup() {
  // Turn on LED on pin 13
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  pinMode(inputPin, INPUT); // 設定輸入腳位，使用內部上拉電阻
  pinMode(detectPin, INPUT_PULLUP); // 設定輸入腳位，使用內部上拉電阻

  pinMode(VOL_DN, INPUT_PULLUP);
  pinMode(VOL_UP, INPUT_PULLUP);

  // 預設10, 11, 12為輸入模式
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(12, INPUT);

  Serial.begin(9600);            // 初始化序列埠


  lastState = digitalRead(inputPin);
  lastDebounceTime = millis();
  while((millis() - lastDebounceTime) < stopTime){
    int currentState = digitalRead(inputPin);
    // 開機等待中 若有變化 表示正在play 則STOP
    if (currentState != lastState) {
      digitalWrite(ledPin, LOW);
      setPinModeAndValue(12,0);
      delay(300);
      setPinModeAndValue(12,1);
      break;
    }
  }
  //   // play
  // setPinModeAndValue(12,0);
  // delay(300);
  // setPinModeAndValue(12,1);
  // delay(100);
  // // play
  // setPinModeAndValue(12,0);
  // delay(300);
  // setPinModeAndValue(12,1);
  // delay(100);

  // volume down then up
  // delay(100);
  setPinModeAndValue(11,0);
  delay(4000);
  setPinModeAndValue(11,1);
  // delay(100);
  // setPinModeAndValue(10,0);
  // delay(4000);
  // setPinModeAndValue(10,1);
  Serial.println("init ok");

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  controlLEDStrip(true, TIFFANY_BLUE);
}

int old_detect = digitalRead(detectPin);
unsigned long lastDetectDebounceTime = 0;
#define S_READY 0
#define S_VOL_UP_PLAY 1
#define S_VOL_DN_STOP 2
int state = S_READY;
int old_state = -1;
unsigned long vol_fade_interval_time = 0;
int vol_fade_loop_count = 0;
int max_vol = 5;
unsigned long led_strip_fade_interval_time = 0;
int led_fade_counter = 0;
int led_fade_color_index = 0;
void loop() {
  if(old_state != state){
    Serial.print("state: ");
    Serial.println(state);
    old_state = state;
  }
  // 偵測是否插入
  int new_detect = digitalRead(detectPin);
  if(state == S_READY){
    if(new_detect == 1){
      // PLAY
      if(!playing && (millis() - lastDetectDebounceTime) > 3000){
        Serial.println("not playing, play it");
        // next (will trigger play, but status LED not falsh)
        setPinModeAndValue(11,0);
        delay(300);
        setPinModeAndValue(11,1);
        delay(300);

        // stop for LED status reset
        setPinModeAndValue(12,0);
        delay(300);
        setPinModeAndValue(12,1);
        // play
        state = S_VOL_UP_PLAY;
        vol_fade_loop_count = 0;
        vol_fade_interval_time = millis();
        delay(500);
        setPinModeAndValue(12,0);
        delay(300);
        setPinModeAndValue(12,1);
        delay(100);

        lastDetectDebounceTime = millis();
        // controlLEDStrip(true, RED);
        led_strip_fade_interval_time = millis();
        led_fade_color_index = 1;
        led_fade_counter = 255;
        // reset status detect time
        lastDebounceTime = millis();
      }
    }else{
      // STOP
      if(playing && (millis() - lastDetectDebounceTime) > 4000){
        Serial.println("playing, stop it");
        state = S_VOL_DN_STOP;
        vol_fade_loop_count = 0;
        vol_fade_interval_time = millis();
        // setPinModeAndValue(12,0);
        // delay(300);
        // setPinModeAndValue(12,1);
        // delay(100);
        // lastDetectDebounceTime = millis();
        // controlLEDStrip(true, GREEN);
        led_strip_fade_interval_time = millis();
        led_fade_color_index = 2;
        led_fade_counter = 255;
      }
    }
  }
  // led fade effect
  if((millis() - led_strip_fade_interval_time) > 20){
    if(led_fade_counter > 0){
      led_fade_counter--;
      if(led_fade_color_index == 1){
        // controlLEDStrip(true, strip.Color(255-led_fade_counter, led_fade_counter, 0));
        controlLEDStrip(true, strip.Color(
          map(led_fade_counter, 0, 255, 255, 0),
          map(led_fade_counter, 0, 255, 0, 255),
          map(led_fade_counter, 0, 255, 0, 100)
        ));
      }
      if(led_fade_color_index == 2){
        // controlLEDStrip(true, strip.Color(led_fade_counter, 255-led_fade_counter, 0));
        controlLEDStrip(true, strip.Color(
          map(led_fade_counter, 0, 255, 0, 255),
          map(led_fade_counter, 0, 255, 255, 0),
          map(led_fade_counter, 0, 255, 100, 0)
        ));
      }
    }
    led_strip_fade_interval_time = millis();
  }
  // fade vol effect
  if(state == S_VOL_UP_PLAY){
    if((millis() - vol_fade_interval_time) > 1200){
      setPinModeAndValue(10,0);
      // delay(1200);
      // setPinModeAndValue(10,1);
      vol_fade_interval_time = millis();
      vol_fade_loop_count++;
    }
    if(vol_fade_loop_count > max_vol){
      setPinModeAndValue(10,1);
      state = S_READY;
    }
  }
  if(state == S_VOL_DN_STOP){
    if((millis() - vol_fade_interval_time) > 1200){
      setPinModeAndValue(11,0);
      // delay(1200);
      // setPinModeAndValue(11,1);
      vol_fade_interval_time = millis();
      vol_fade_loop_count++;
    }
    if(vol_fade_loop_count > max_vol){
      // stop
      setPinModeAndValue(11,1);
      delay(300);
      setPinModeAndValue(12,0);
      delay(300);
      setPinModeAndValue(12,1);
      lastDetectDebounceTime = millis();
      state = S_READY;
    }
  }

  // insert detect change 
  if(old_detect != new_detect){
    Serial.print("detect: ");
    Serial.println(new_detect);
    Serial.print("playing: ");
    Serial.println(playing);
    old_detect = new_detect;
  }

  // 讀取目前播放狀態腳位狀態
  int currentState = digitalRead(inputPin);

  // 檢查腳位狀態是否改變
  if (currentState != lastState) {
    // 記錄上一次腳位變化的時間
    lastDebounceTime = millis();
    // 更新上一次的腳位狀態
    lastState = currentState;
  }

  // 檢查是否超過防彈跳時間
  if ((millis() - lastDebounceTime) < debounceTime) {
    // 如果在防彈跳時間內，則持續閃爍LED
    digitalWrite(ledPin, HIGH);
      if (!playing) {
        Serial.println("PLAY");
        playing = true;
      }
  } else {
    // 如果超過防彈跳時間，則停止閃爍LED
    digitalWrite(ledPin, LOW);

    // 檢查是否超過停止時間
    if ((millis() - lastDebounceTime) >= stopTime) {
      // 如果超過停止時間，且目前為播放狀態，則輸出STOP
      if (playing) {
        Serial.println("STOP");
        playing = false;
      }
    } else {
      // 如果沒有超過停止時間，且目前為停止狀態，則輸出PLAY
    }
  }

  if(digitalRead(VOL_DN) == 0){
    // vol up
        Serial.println("W command received");
    setPinModeAndValue(10,0);
    delay(1200);
    setPinModeAndValue(10,1);
    if(max_vol < 8) max_vol++;
  }
  if(digitalRead(VOL_UP) == 0){
    // vol dn
        Serial.println("S command received");
    setPinModeAndValue(11,0);
    delay(1200);
    setPinModeAndValue(11,1);
    if(max_vol > 0) max_vol--;    
  }
  // 檢查序列埠是否有資料
  if (Serial.available() > 0) {
    // 讀取序列埠資料
    char data = Serial.read();
    // 將讀取到的字元轉換為小寫
    data = tolower(data);

    // 判斷讀取到的字元
    switch (data) {
      case 'p':
        Serial.println("P command received");
        // 在這裡加入處理 'p' 指令的程式碼
        setPinModeAndValue(12,0);
        delay(300);
        setPinModeAndValue(12,1);
        break;
      case 'w':
        Serial.println("W command received");
        // 在這裡加入處理 'w' 指令的程式碼
  setPinModeAndValue(10,0);
  delay(1200);
  setPinModeAndValue(10,1);
        break;
      case 's':
        Serial.println("S command received");
        // 在這裡加入處理 's' 指令的程式碼
  setPinModeAndValue(11,0);
  delay(1200);
  setPinModeAndValue(11,1);
        break;
      case 'a':
        Serial.println("A command received");
        // 在這裡加入處理 'a' 指令的程式碼
  setPinModeAndValue(10,0);
  delay(100);
  setPinModeAndValue(10,1);
        break;
      case 'd':
        Serial.println("D command received");
        // 在這裡加入處理 'd' 指令的程式碼
  setPinModeAndValue(11,0);
  delay(100);
  setPinModeAndValue(11,1);
        break;
      default:
        Serial.print("Unknown command: ");
        Serial.println(data);
        break;
    }
  }  
}

