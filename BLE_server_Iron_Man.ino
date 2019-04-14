#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>

//SERVICE AND CHARACTERISTICS
#define RING_SERVICE_UUID         "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define RS_POWER_UUID             "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define RS_COLOR_UUID             "132705f4-3385-4e6e-9790-e7bb9201985d"
#define RS_MODE_UUID              "ade5257f-66d2-4f46-818f-4edd60c7b8c8"
#define RS_BATT_UUID              "a3cd7fd1-79bc-4eec-bfcc-3075d0561e97"

const uint16_t PixelCountRing = 31; 
const uint8_t PixelPinRing = 27;  

NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> stripRing(PixelCountRing, PixelPinRing);

RgbColor black(0);
RgbColor ironMan(31,31,255);
RgbColor currentColor = black;

RgbColor red(128,0,0);
RgbColor green(0,128,0);
RgbColor blue(0,0,128);
RgbColor white(128,128,128);
RgbColor purple(128,0,128);


//Set all pixels of the ring with the same color
void setAllColorsRing(RgbColor color) {
  for (uint16_t i=0; i<PixelCountRing; i++) {
    stripRing.SetPixelColor(i, color);
  }
  currentColor = color;
  stripRing.Show();
}

void setAllColorsInnerRing(RgbColor color) {
  for (uint16_t i=24; i<PixelCountRing; i++) {
    stripRing.SetPixelColor(i, color);
  }
  currentColor = color;
  stripRing.Show();
}

void setTwoColorsOuterRing(RgbColor color, uint8_t startPixel) {

  stripRing.SetPixelColor(startPixel,color);
  stripRing.SetPixelColor(startPixel+1,color);
  stripRing.SetPixelColor(startPixel+2,color);
  currentColor = color;
  stripRing.Show();
}


// Fill the dots one after the other with a color
void colorWipeRing(RgbColor color, uint8_t wait) {
  for(uint16_t i=0; i<PixelCountRing; i++) {
    stripRing.SetPixelColor(i, color);
    stripRing.Show();
    delay(wait);
  }
  currentColor = color;
}

void startUpSequence() {

  for (uint8_t i=0; i<3; i++) {
    setAllColorsInnerRing(ironMan);
    delay(200);
    setAllColorsInnerRing(black);
    delay(200);
  }

for (uint8_t j=0; j<8; j++) {
  for (uint8_t i=0; i<1; i++) {
    setAllColorsInnerRing(ironMan);
    delay(60);
    setAllColorsInnerRing(black);
    delay(60);
    setAllColorsInnerRing(ironMan);
    setTwoColorsOuterRing(ironMan,i+3*j);
    delay(100);
    setTwoColorsOuterRing(black,i+3*j);
    delay(100);
    setTwoColorsOuterRing(ironMan,i+3*j);
    delay(30);
    setTwoColorsOuterRing(black,i+3*j);
    delay(30);
    setTwoColorsOuterRing(ironMan,i+3*j);
    delay(30);
    setTwoColorsOuterRing(black,i+3*j);
    delay(20);
    setTwoColorsOuterRing(ironMan,i+3*j);
    delay(20);
    setTwoColorsOuterRing(black,i+3*j);
    delay(20);
    setTwoColorsOuterRing(ironMan,i+3*j);
    delay(20);
    setTwoColorsOuterRing(black,i+3*j);
    setTwoColorsOuterRing(ironMan,i+3*j);
  }
  delay(100-12*j);
}

  
  
  /*for (uint8_t i=0; i<23; i++) {
    setAllColorsInnerRing(ironMan);
    setTwoColorsOuterRing(ironMan,i);
    delay(30);
    setAllColorsInnerRing(black);
    delay(200);
  }*/
  setAllColorsInnerRing(ironMan);  
}

/**Callback to change ring power**/
class RSPowerCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      RgbColor saveColor = currentColor;
      if (value.compare("ON")==0) {
          Serial.println("POWER ON!");
         setAllColorsRing(currentColor);
      } else {
        Serial.println("POWER OFF!");
        setAllColorsRing(black);
      }
      currentColor = saveColor;
    }
};

/**Callback to change ring color**/
class RSColorCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      
     if (value.compare("R")==0) {
      setAllColorsRing(red);
     } else if (value.compare("G")==0) {
      setAllColorsRing(green);
     } else if (value.compare("I")==0) {
      setAllColorsRing(ironMan);
     } else if (value.compare("B")==0) {
      setAllColorsRing(blue);
     } else if (value.compare("W")==0) {
      setAllColorsRing(white);
     } else if (value.compare("P")==0) {
      setAllColorsRing(purple);
     }
    
    }
};

/**Callback to change ring mode**/
class RSModeCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.compare("START")==0) {
        startUpSequence();
      } else if (value.compare("GLOW")==0) {
        
      }
    }
};

/**Callback to read battery level**/
class RSBattCallback: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic *pCharacteristic) {
       uint16_t val = 0;
       val = analogRead(A13);
       val = val*2;
       Serial.println(val);
       std::string batt((char*)&val, 2);

       pCharacteristic->setValue(batt);
    }
};

void setup() {

  stripRing.Begin();
  stripRing.SetBrightness(128);
  stripRing.Show();
  setAllColorsRing(black);
  
  
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  //Initialize BLE server
  BLEDevice::init("ArcReactor");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(RING_SERVICE_UUID);
  BLECharacteristic *pRSPower = pService->createCharacteristic(
                                         RS_POWER_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pRSPower->setCallbacks(new RSPowerCallback);                                     
  BLECharacteristic *pRSColor = pService->createCharacteristic(
                                         RS_COLOR_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pRSColor->setCallbacks(new RSColorCallback);
  BLECharacteristic *pRSMode = pService->createCharacteristic(
                                         RS_MODE_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pRSMode->setCallbacks(new RSModeCallback);
  BLECharacteristic *pRSBatt = pService->createCharacteristic(
                                         RS_BATT_UUID,
                                         BLECharacteristic::PROPERTY_READ 
                                       );
  pRSBatt->setCallbacks(new RSBattCallback);
  Serial.println("Service and callbacks configured");

  pRSPower->setValue("0FF");
  pRSColor->setValue("");
  pRSMode->setValue("");
  pRSBatt->setValue("");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(RING_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
}
