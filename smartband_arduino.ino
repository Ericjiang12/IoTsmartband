#include <Arduino.h>

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "Adafruit_CCS811.h"
#include "SparkFunLSM6DS3.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>
//global vars
static long irValue = 0.0;
static float temp = 0.0;
static uint16_t stepsTaken = 0;
static float carbon = 0;
float beatsPerMinute;
int beatAvg;
float uvIndex;
//particle
MAX30105 particleSensor;
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
//air quality
Adafruit_CCS811 ccs;
//movement
LSM6DS3Core myIMU( I2C_MODE, 0x6B );

char ssid[]        = "UCInet Mobile Access";
char PASSWORD[]    = "NULL";

int WiFiCon() {
    // Check if we have a WiFi connection, if we don't, connect.
  int xCnt = 0;

  if (WiFi.status() != WL_CONNECTED){

        Serial.println();
        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(ssid);

        WiFi.mode(WIFI_STA);
        
        WiFi.begin(ssid, PASSWORD);
        
        while (WiFi.status() != WL_CONNECTED  && xCnt < 50) {
          delay(500);
          Serial.print(".");
          xCnt ++;
        }

        if (WiFi.status() != WL_CONNECTED){
          Serial.println("WiFiCon=0");
          return 0; //never connected
        } else {
          Serial.println("WiFiCon=1");
          Serial.println("");
          Serial.println("WiFi connected");  
          Serial.println("IP address: ");
          Serial.println(WiFi.localIP());
          return 1; //1 is initial connection
        }

  } else {
    Serial.println("WiFiCon=2");
    return 2; //2 is already connected
  
  }
}

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
const char* hostGet = "169.234.38.99"; 
void postData() {

   WiFiClient clientGet;
   const int httpGetPort = 8080;

   //the path and file to send the data to:
   String urlGet = "/smartband/arduinoListener";

 
  // We now create and add parameters:
  String UV = (String)uvIndex;
  String heartbeat = (String)beatAvg;
  String steps = (String)stepsTaken;
  String tmp = (String) temp;
  String crb = (String)carbon;

  urlGet += "?UV=" + UV + "&heartbeat=" + heartbeat + "&step=" + steps + "&temperature=" + tmp + "&carbon=" + crb;
   
      Serial.print(">>> Connecting to host: ");
      Serial.println(hostGet);
      
       if (!clientGet.connect(hostGet, httpGetPort)) {
        Serial.print("Connection failed: ");
        Serial.print(hostGet);
      } else {
          clientGet.println("GET " + urlGet + " HTTP/1.1");
          clientGet.print("Host: ");
          clientGet.println(hostGet);
          clientGet.println("User-Agent: ESP8266/1.0");
          clientGet.println("Connection: close\r\n\r\n");
          
          unsigned long timeoutP = millis();
          while (clientGet.available() == 0) {
            
            if (millis() - timeoutP > 10000) {
              Serial.print(">>> Client Timeout: ");
              Serial.println(hostGet);
              clientGet.stop();
              return;
            }
          }

          //just checks the 1st line of the server response. Could be expanded if needed.
          while(clientGet.available()){
            String retLine = clientGet.readStringUntil('\r');
            Serial.println(retLine);
            break; 
          }

      } //end client connection if else
                        
      Serial.print(">>> Closing host: ");
      Serial.println(hostGet);
          
      clientGet.stop();

}

void setup() {
  Serial.begin(115200);
  WiFiCon();
      Wire.begin();
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
    {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
    }
    particleSensor.setup(); //Configure sensor with default settings
    particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
    particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
    
    //ccs811 setup
    if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
    }
    //calibrate temperature sensor
    while(!ccs.available());
    float temp = ccs.calculateTemperature();
    ccs.setTempOffset(temp - 25.0);

    //LSMD6 setup
    if( myIMU.beginCore() != 0 )
  {
    Serial.print("Error at beginCore().\n");
  }
  else
  {
    Serial.print("\nbeginCore() passed.\n");
  }
  //Error accumulation variable
  uint8_t errorAccumulator = 0;

  uint8_t dataToWrite = 0;  //Temporary variable

  //Setup the accelerometer******************************
  dataToWrite = 0; //Start Fresh!
  //  dataToWrite |= LSM6DS3_ACC_GYRO_BW_XL_200Hz;
  dataToWrite |= LSM6DS3_ACC_GYRO_FS_XL_2g;
  dataToWrite |= LSM6DS3_ACC_GYRO_ODR_XL_26Hz;

  // //Now, write the patched together data
  errorAccumulator += myIMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, dataToWrite);

  //Set the ODR bit
  errorAccumulator += myIMU.readRegister(&dataToWrite, LSM6DS3_ACC_GYRO_CTRL4_C);
  dataToWrite &= ~((uint8_t)LSM6DS3_ACC_GYRO_BW_SCAL_ODR_ENABLED);

  
  // Enable embedded functions -- ALSO clears the pdeo step count
  errorAccumulator += myIMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL10_C, 0x3E);
  // Enable pedometer algorithm
  errorAccumulator += myIMU.writeRegister(LSM6DS3_ACC_GYRO_TAP_CFG1, 0x40);
  // Step Detector interrupt driven to INT1 pin
  errorAccumulator += myIMU.writeRegister( LSM6DS3_ACC_GYRO_INT1_CTRL, 0x10 );
  
  if( errorAccumulator )
  {
    Serial.println("Problem configuring the device.");
  }
  else
  {
    Serial.println("Device O.K.");
  }
  zopt220xSetup();
  enableUVBSensing();
}

void loop() {
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);
    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
  uint8_t readDataByte = 0;
  myIMU.readRegister(&readDataByte, LSM6DS3_ACC_GYRO_STEP_COUNTER_H);
  stepsTaken = ((uint16_t)readDataByte) << 8;
  
  myIMU.readRegister(&readDataByte, LSM6DS3_ACC_GYRO_STEP_COUNTER_L);
  stepsTaken |= readDataByte;
  carbon = ccs.geteCO2();
  if(millis() % 1000 < 25)
  {
    Serial.print("BPM=");
    Serial.print(beatsPerMinute);
    Serial.write(", Avg BPM=");
    Serial.print(beatAvg);

    if (irValue < 50000)
      Serial.print(" No finger?");
    Serial.println();
    if(ccs.available())
    {
      temp = ccs.calculateTemperature();
      if(!ccs.readData())
      {
        Serial.print("CO2: ");
        Serial.print(carbon);
        Serial.print("ppm, TVOC: ");
        Serial.print(ccs.getTVOC());
        Serial.print("ppb   Temp:");
        Serial.println(temp);
      }
      else
      {
        Serial.println("ERROR!");
        while(1);
      }
    }
    Serial.print("Steps taken: ");
    Serial.println(stepsTaken);
    uvIndex = getUVIndex();
    Serial.print("UV Index: ");
    Serial.println(uvIndex);
    Serial.println();
    Serial.println();
    if (WiFiCon() > 0) {
    postData();
    }
  }
}

