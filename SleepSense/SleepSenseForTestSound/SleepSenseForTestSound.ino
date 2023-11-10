#include "Arduino.h"
#include "Wire.h"
#include <driver/i2s.h>
#define I2S_WS 15
#define I2S_SD 32
#define I2S_SCK 14
#define I2S_PORT I2S_NUM_0
#define bufferLen 64

int16_t sBuffer[bufferLen];
int counter = 0; // Counter to keep track of mean > 100 occurrences
int lux_Qua = 0;

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
int ledPin = 26;

void i2s_install(){
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

void i2s_setpin(){
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  lightSensor.begin();

  Serial.println("Setup I2S ...");
  delay(1000);
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);
  delay(500);
    
}

void loop() {
  uint16_t lux = lightSensor.getLightIntensity();
  //float mean = 0; //unsigned long ค่าไม่ติดลบแต่ค่าเป็นล้าน
  size_t bytesIn = 0;
  esp_err_t result = i2s_read(I2S_PORT, &sBuffer, bufferLen, &bytesIn, portMAX_DELAY);

  if (lux <= 18) { // 0-20 lux is considered good quality sleep
      digitalWrite(ledPin, LOW); // Turn on the LED
      Serial.printf("Lux: %d\n", lux);
  } else { // More than 20 lux is considered bad quality sleep
      digitalWrite(ledPin, HIGH); // Turn off the LED
      Serial.printf("Lux: %d - Sleep quality is bad\n", lux);
      lux_Qua++;
  }
  //delay(250);

  if (result == ESP_OK)
  {
    int samples_read = bytesIn / 8;
    float mean = 0; //unsigned long ค่าไม่ติดลบแต่ค่าเป็นล้าน
    if (samples_read > 0) {
      for (int i = 0; i < samples_read; ++i) {
        mean += (sBuffer[i]);
      }
      mean /= samples_read;
      //Serial.printf("dBFS: %d\n", static_cast<int>(mean));
      Serial.print(static_cast<int>(mean));
      Serial.print('\n');
      if (mean > 155) {
        counter++; //counter = counter + 1; รันได้ผลดี (counter++ ดูสวยดี)
        Serial.printf("dBFS เกินที่กำหนดจำนวน: %d ครั้ง\n", counter); 
      }
    }
    //delay(100); 
  }
  
  if(lux_Qua >= 2 || counter >= 2){
    Serial.printf("คุณภาพการนอนของคุณย่ำแย่ ;( \n"); 
  }else{
    Serial.printf("คุณภาพการนอนของคุณดีมากกกก!!!\n"); 
  }
  delay(500); //delay มีผลอย่างมากต่อการรับค่าของเสียง (175 โอเค, >250 พัง)
}


