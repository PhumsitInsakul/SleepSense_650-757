#include "thingProperties.h"
#include "Arduino.h"
#include "Wire.h"
#include <Adafruit_GFX.h>   
#include <Adafruit_SSD1306.h>
#include <driver/i2s.h>
#include <HTTPClient.h> //ไม่ได้ใช้

#include <esp_task_wdt.h> // เพิ่ม library watchdog


#define I2S_WS 15
#define I2S_SD 32
#define I2S_SCK 14
#define I2S_PORT I2S_NUM_0
#define bufferLen 64
#define WDT_TIMEOUT 30000 // 30 วินาที

#define LED_2 27

#define OLED_RESET 4

Adafruit_SSD1306 display(OLED_RESET);


int status_sw = 0;
int ledPin = 26;
int event = 0;

int16_t sBuffer[bufferLen];
int counter = 0; // Counter to keep track of mean > 155 occurrencesint
int Timer_2 = 0;
int Timer_3 = 0;
int Timer_Qua = 0;
int lux_Qua = 0; // Counter to keep track of lux > 18 occurrencesint
int light = 0;
int sound = 0;
//----------------------------------------------------------------------- ไม่ได้ใช้

String server = "http://maker.ifttt.com";
String eventName = "SleepSense";
String IFTTT_Key = "dc-BGuvIIXuBltqt2EtDkt";
String IFTTTUral = "https://maker.ifttt.com/trigger/SleepSense/with/key/dc-BGuvIIXuBltqt2EtDkt";

int value1;
int value2;
int value3;
//-----------------------------------------------------------------------

////////////////////////////  BH1750 //////////////////////////////////////

class BH1750FVI {
public:
    enum eDeviceAddress {
        k_DevAddress_L = 0x23,
        k_DevAddress_H = 0x5C
    };

    enum eDeviceMode {
        k_DevModeContLowRes = 0x13
    };

    BH1750FVI() : m_DeviceAddress(k_DevAddress_L), m_DeviceMode(k_DevModeContLowRes) {}

    void begin();
    uint16_t getLightIntensity();

private:
    void I2CWrite(uint8_t data);

    eDeviceAddress m_DeviceAddress;
    eDeviceMode m_DeviceMode;
};

void BH1750FVI::begin() {
    Wire.begin();
    I2CWrite(0x01); // Power up
    delay(10);
    I2CWrite(m_DeviceMode);
}

uint16_t BH1750FVI::getLightIntensity() {
    uint16_t value = 0;

    Wire.requestFrom(static_cast<int>(m_DeviceAddress), 2);
    value = Wire.read();
    value <<= 8;
    value |= Wire.read();

    return value / 1.2;
}

void BH1750FVI::I2CWrite(uint8_t data) {
    Wire.beginTransmission(static_cast<int>(m_DeviceAddress));
    Wire.write(data);
    Wire.endTransmission();
}

BH1750FVI lightSensor;

/////////////////////////// library I2S ///////////////////////////////////

void i2s_install() {
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0, // default interrupt priority
    .dma_buf_count = 8,
    .dma_buf_len = bufferLen,
    .use_apll = false
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

void i2s_setpin() {
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
}

/////////////////////////// setup ///////////////////////////////////

void setup() {
  // Initialize serial and wait for port to open:
  esp_task_wdt_init(WDT_TIMEOUT, true);
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(LED_2, OUTPUT); // status_sw
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c); //initialize I2C addr 0x3c
  display.clearDisplay(); // clears the screen and buffer
  
  display.drawPixel(127, 63, WHITE);
  display.drawLine(0, 63, 127, 21, WHITE);
  display.drawCircle(110, 50, 12, WHITE);
  display.fillCircle(45, 50, 8, WHITE);
  display.drawTriangle(70, 60, 90, 60, 80, 46, WHITE);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Sleep Quality:");
  display.setTextSize(2);
  display.println("Welcome!");
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(1);
  display.println("------------------");
  display.setTextColor(WHITE, BLACK);
  display.display();

  lightSensor.begin();

  Serial.println("Setup I2S ...");
  delay(1000);
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);
  delay(500);

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

/////////////////////////// loop ///////////////////////////////////

void loop() {
  ArduinoCloud.update();

  if (status_sw == 1) {
    event = 1;
    esp_task_wdt_reset(); // watchdog

    if ((millis() - Timer_2) >= 1000) { // 1 วิ
      Timer_2 = millis();

      uint16_t lux = lightSensor.getLightIntensity();
      size_t bytesIn = 0;
      esp_err_t result = i2s_read(I2S_PORT, &sBuffer, bufferLen, &bytesIn, portMAX_DELAY);
      
      light_sensor = lux;

      if (lux <= 18) { // 0-18 lux is considered good quality sleep
        digitalWrite(ledPin, LOW); // Turn on the LED
        Serial.printf("Lux: %d\n", lux);
      } else { // More than 18 lux is considered bad quality sleep
        digitalWrite(ledPin, HIGH); // Turn off the LED
        Serial.printf("Lux: %d - Sleep quality is bad\n", lux);
        lux_Qua++;
      }

      if (result == ESP_OK)
      {
        int samples_read = bytesIn / 8;
        float mean = 0;
        float full_scale = 120;
        int DB = 0;
        
        if (samples_read > 0) {
          for (int i = 0; i < samples_read; ++i) {
            mean += (sBuffer[i]);
          }
          mean /= samples_read;
          sound_sensor = mean;
          DB = mean - full_scale; 

          //Serial.printf("dBFS: %d\n", static_cast<int>(mean));
          Serial.printf("dBFS: %d\n", static_cast<int>(mean));
          Serial.print('\n');
          if (mean > 155) {  // dB = dBFS(155) - full scale lavelของระบบ(120) = 35 dB จากงานวิจัย  
            counter++; //counter = counter + 1; รันได้ผลดี (counter++ ดูสวยดี)
            Serial.printf("dBFS เกินที่กำหนดจำนวน: %d ครั้ง\n", counter/* DB*/);
          }
        }
      }
      
    } 
    
    if ((millis() - Timer_3) >= 10000) { // 10 วิให้แสดงข้อความ + เก็บค่า
      Timer_3 = millis();
      
    display.clearDisplay();  //เคลียร์จอ
      
      
      if(lux_Qua >= 2 || counter >= 2){
        Serial.printf("คุณภาพการนอนของคุณย่ำแย่ ;(\n");
        quality = "Bad";
        lux_Qua = 0;
        counter = 0;
        
        display.drawPixel(127, 63, WHITE);
        display.drawLine(0, 63, 127, 21, WHITE);
        display.drawCircle(110, 50, 12, WHITE);
        display.fillCircle(45, 50, 8, WHITE);
        display.drawTriangle(70, 60, 90, 60, 80, 46, WHITE);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("Sleep Quality:");
        display.setTextSize(2);
        display.println(quality);
        display.setTextColor(BLACK, WHITE);
        display.setTextSize(1);
        display.println("------------------");
        display.setTextColor(WHITE, BLACK);
        display.display();
        
        
      }else{
        Serial.printf("คุณภาพการนอนของคุณดีมาก!!!\n");
        quality = "good!";
        
        display.drawPixel(127, 63, WHITE);
        display.drawLine(0, 63, 127, 21, WHITE);
        display.drawCircle(110, 50, 12, WHITE);
        display.fillCircle(45, 50, 8, WHITE);
        display.drawTriangle(70, 60, 90, 60, 80, 46, WHITE);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("Sleep Quality:");
        display.setTextSize(2);
        display.println(quality);
        display.setTextColor(BLACK, WHITE);
        display.setTextSize(1);
        display.println("------------------");
        display.setTextColor(WHITE, BLACK);
        display.display();
      }
    }

  } else {
    event = 0;
  }
}


void onSwitchChange()  {
  if (_switch_ == 1) {
    Serial.println("Sensor: ON");
    digitalWrite(LED_2, HIGH);
    status_sw = 1;

  } else {
    Serial.println("Sensor: OFF");
    digitalWrite(LED_2, LOW);
    status_sw = 0;
  }

}


void onLightSensorChange()  {

}



void onSoundSensorChange()  {

}


void onQualityChange()  {
  // String quality;
}