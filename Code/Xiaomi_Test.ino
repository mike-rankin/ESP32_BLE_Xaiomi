//This software was written by @fast_code_r_us

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define LED_ENABLE 23
#define BLUE_LED 9

Adafruit_SSD1306 display(128, 32, &Wire, 21);
static int T, H, iMaxT, iMinT, iMaxH, iMinH;

static BLEUUID serviceUUID("0000181a-0000-1000-8000-00805f9b34fb"); //Service
//std::string VD_BLE_Name = "ATC_E5A275";
std::string service_data;
char Scanned_BLE_Name[32];
String Scanned_BLE_Address;
BLEScanResults foundDevices;
BLEScan *pBLEScan;
static BLEAddress *Server_BLE_Address;


void Write1Byte( uint8_t Addr ,  uint8_t Data )
{
    Wire1.beginTransmission(0x34);
    Wire1.write(Addr);
    Wire1.write(Data);
    Wire1.endTransmission();
}

uint8_t Read8bit( uint8_t Addr )
{
    Wire1.beginTransmission(0x34);
    Wire1.write(Addr);
    Wire1.endTransmission();
    Wire1.requestFrom(0x34, 1);
    return Wire1.read();
}


void lightSleep(uint64_t time_in_us)
{
  esp_sleep_enable_timer_wakeup(time_in_us);
  esp_light_sleep_start();
}

// Called for each device found during a BLE scan by the client
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice) {
//      Serial.printf("Scan Result: %s \n", advertisedDevice.toString().c_str());
      const char *szAddr = advertisedDevice.getAddress().toString().c_str();
      if (advertisedDevice.haveServiceData() && memcmp(szAddr, "a4:c1:38", 8) == 0) {
          const char *s = service_data.c_str();
          int i, iLen = service_data.length();
          uint8_t *p = (uint8_t *)s; // unsigned data
          service_data = advertisedDevice.getServiceData();
          T = (p[6] << 8) + p[7];
          H = p[8];
          if (T > iMaxT) iMaxT = T;
          if (T < iMinT) iMinT = T;
          if (H > iMaxH) iMaxH = H;
          if (H < iMinH) iMinH = H;
//          Serial.printf("Temp = %d.%dC, Humidity = %d%%\n", T/10, T % 10, p[8]);
        }
    }
};

void ShowInfo(void)
{
char szTemp[64];
  sprintf(szTemp, "Temp %d.%01d, Humid %d%%", T/10, T % 10,  H);
  display.setCursor(0,0);  //over,down
  display.print(szTemp);
 
  sprintf(szTemp, "MaxT %d.%01d, MaxH %d%%", iMaxT/10, iMaxT % 10, iMaxH);
  display.setCursor(0,10);  //over,down
  display.print(szTemp);
  
  sprintf(szTemp, "MinT %d.%01d, MinH %d%%", iMinT/10, iMinT % 10, iMinH);
  display.setCursor(0,20);  //over,down
  display.print(szTemp);

  //Serial.print(szTemp);
  //display.setCursor(0,0);  //over,down
  //display.print(szTemp);
  //display.setCursor(0,10);  //over,down
  //display.print(szTemp);
  
  display.display();
  delay(1000);
  display.clearDisplay();
    
}
void setup() 
{
  Wire.begin(13, 14); 
  display.setRotation(2);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  pinMode(LED_ENABLE, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  digitalWrite(LED_ENABLE, LOW); //Enable Leds
  digitalWrite(BLUE_LED, HIGH);  //Keep off
  
  iMaxT = 0;
  iMinT = 1000;
  iMaxH = 0;
  iMinH = 99;
  ShowInfo();
  //spilcdWriteString(&lcd, 0,4,(char *)"Temperature Capture", 0x7e0,0,FONT_12x16, DRAW_TO_LCD);
  Serial.begin(115200);
  Serial.println("About to start BLE");
  BLEDevice::init("ESP32BLE");
  pBLEScan = BLEDevice::getScan(); //create new scan
  Serial.println("getScan returned");
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); //Call the class that is defined above
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
} /* setup() */

void loop() {
   foundDevices = pBLEScan->start(5, false); //Scan for 5 seconds to find the Fitness band
   pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
   pBLEScan->stop();
   ShowInfo();
   lightSleep(10000000); // wait 10 seconds, then start another scan

   digitalWrite(LED_ENABLE, LOW); //Enable Leds
   digitalWrite(BLUE_LED, LOW);
   delay(250);
   digitalWrite(BLUE_LED, HIGH);
   delay(250);
   
} /* loop */
