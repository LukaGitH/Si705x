#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static const int OLED_DC = 6;
static const int OLED_CS = 7;
static const int OLED_RESET = 8;
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

static const uint8_t I2C_ID = 0x40;

static const int NUM_SAMPLES = 50;
float samples[NUM_SAMPLES];
int sample_index = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  display.print("Si70");
  display.println(getModel());
  uint8_t version = getFirmwareVersion();
  if (version == 0xff) display.print("ver 1.0");
  if (version == 0x20) display.print("ver 2.0");
  display.display();

  for (int i = 0; i < NUM_SAMPLES; i++) {
    samples[i] = read_temperature();
    delay(100);
  }
}

void loop() {
  float temp_c = read_temperature();
  samples[sample_index++] = temp_c;
  if (sample_index == NUM_SAMPLES) sample_index = 0;

  float temp_c_avg = get_average();
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0, 10);
  display.print(temp_c_avg);
  display.print(" C");
  display.drawCircle(100, 13, 3, WHITE);

  float temp_f_avg = temp_c_avg * 1.8 + 32;
  display.setTextSize(2);
  display.setCursor(25, 45);
  display.print(temp_f_avg);
  display.print(" F");
  display.drawCircle(90, 47, 2, WHITE);

  if (Serial.available()) {
    uint8_t command = Serial.read();

    display.setTextSize(1);
    display.setCursor(0, 56);

    switch (command) {
      case 'c':
        Serial.println(temp_c);
        display.print("c");
        break;
      case 'f':
        Serial.println(temp_c * 1.8 + 32);
        display.print("f");
        break;
      case 'C':
        Serial.println(temp_c_avg);
        display.print("C");
        break;
      case 'F':
        Serial.println(temp_f_avg);
        display.print("F");
        break;
      default:
        display.print(command, HEX);
    }
  }

  display.display();
  delay(100);
}

float read_temperature() {
  Wire.beginTransmission(I2C_ID);
  Wire.write(0xe3);
  Wire.endTransmission();
  Wire.requestFrom(I2C_ID, 2);
  uint8_t byte1 = Wire.read();
  uint8_t byte2 = Wire.read();
  uint16_t value = (byte1 << 8) + byte2;
  return 175.72 * (float) value / 65536 - 46.85;
}

float get_average() {
  float sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += samples[i];
  }
  return sum / NUM_SAMPLES;
}

uint8_t getModel() {
  Wire.beginTransmission(I2C_ID);
  Wire.write(0xfc);
  Wire.write(0xc9);
  Wire.endTransmission();
  Wire.requestFrom(I2C_ID, 4);
  uint8_t byte1 = Wire.read();
  uint8_t byte2 = Wire.read();
  uint8_t byte3 = Wire.read();
  uint8_t byte4 = Wire.read();
  return byte1;
}

uint8_t getFirmwareVersion() {
  Wire.beginTransmission(I2C_ID);
  Wire.write(0x84);
  Wire.write(0xB8);
  Wire.endTransmission();
  Wire.requestFrom(I2C_ID, 1);
  return Wire.read();
}
