#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <MPU6050.h>
#include <SD.h>
#include <Adafruit_NeoPixel.h>


#define NEOPIXEL_PIN D4   
#define BUTTON_PIN D3    
#define CS_PIN_SD D8    

Adafruit_BMP085 bmp;
MPU6050 mpu;

Adafruit_NeoPixel strip(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

File flightDataFile;
bool isSDInitialized = false;
bool isRecording = false;
unsigned long startTime = 0;
unsigned long lastRecordTime = 0;
unsigned long fileStartTime = 0;
int fileCounter = 0;
float initAlt;

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show(); // Initialize NeoPixel to 'off'

  pinMode(BUTTON_PIN, INPUT_PULLUP);
Wire.begin();

mpu.initialize();
  mpu.CalibrateGyro();
  mpu.CalibrateAccel();
setStatusColor(0, 100, 100);
delay(200); 

  // Initialize BMP085
  if (!bmp.begin()) {
    Serial.println("Failed to initialize BMP180!");
  }

  // Initialize SD card
  if (SD.begin(CS_PIN_SD)) {
    isSDInitialized = true;
    Serial.println("SD card initialized successfully.");
    setStatusColor(0, 0, 255); // Blue
    delay(1000);
  } else {
    Serial.println("SD card initialization failed!");
    setStatusColor(255, 0, 0); // Red
    delay(1000);
  }
   initAlt = (bmp.readAltitude() * 3.28084);
  setStatusColor(0, 255, 0); // Green
  
}

void loop() {
  Serial.println(bmp.readAltitude());
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (!isRecording) {
      startRecording();
    } else {
      stopRecording();
    }
    delay(1000); // Debounce delay to avoid rapid button presses
  }

  if (isRecording) {
    recordData();
  }
}

void startRecording() {
  fileStartTime = millis(); // Store the time when file recording starts
  startTime = 0; // Reset startTime for each new file
  lastRecordTime = 0;
  
  fileCounter++;
  String fileName = "FlightData_" + String(fileCounter) + ".csv";
  flightDataFile = SD.open(fileName, FILE_WRITE);
  if (flightDataFile) {
    Serial.println("Recording started.");
    flightDataFile.println("Time,gx,gy,gz,ax,ay,az,alt");
    isRecording = true;
    setStatusColor(0, 255, 0); // Green
    delay(100);
    setStatusColor(0, 0, 255);
    delay(100);
    setStatusColor(0, 255, 0);
     

  } else {
    Serial.println("Error opening file!");
    setStatusColor(255, 0, 0); // Red
  }
}

void recordData() {
  unsigned long currentTime = millis() - fileStartTime; // Calculate time elapsed since file started
  
  // If startTime is 0, it means it's the first record, set startTime
  if (startTime == 0) {
    startTime = currentTime;
    lastRecordTime = 0;
  }
  
  unsigned long elapsedTime = currentTime - startTime;


    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure();
    float altitude = bmp.readAltitude(); 
float caltitude1 = ((altitude * 3.28084) - initAlt);
float caltitude2 = ((altitude * 3.28084) - initAlt);
float Faltitude = (caltitude1 + caltitude2) / 2;
    // Format and write data to the file
    flightDataFile.print(elapsedTime);
    flightDataFile.print(",");
    flightDataFile.print(gx);
    flightDataFile.print(",");
    flightDataFile.print(gy);
    flightDataFile.print(",");
    flightDataFile.print(gz);
    flightDataFile.print(",");
    flightDataFile.print(ax);
    flightDataFile.print(",");
    flightDataFile.print(ay);
    flightDataFile.print(",");
    flightDataFile.print(az);
    flightDataFile.print(",");
    flightDataFile.println(Faltitude);
   
    
}

void stopRecording() {
  if (isRecording) {
    flightDataFile.close();
    isRecording = false;
    Serial.println("Recording stopped.");
    setStatusColor(0, 0, 255); // Blue
  }
}

void setStatusColor(uint8_t red, uint8_t green, uint8_t blue) {
  strip.setPixelColor(0, strip.Color(red, green, blue));
  strip.show();
}
