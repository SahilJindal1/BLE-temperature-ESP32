#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer){
    deviceConnected = true; 
  };
  void onDisconnect(BLEServer* pServer){
    deviceConnected = false;  
  }
};

Adafruit_BME280 bme;

float temperature, humidity, pressure, altitude;

#define SEALEVELPRESSURE_HPA (1050)

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
  bme.begin(0x76);

  BLEDevice::init("MyESP32");
  // BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic= pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_NOTIFY 
                                       );
  pCharacteristic->addDescriptor(new BLE2902());
  
  pService->start();

  pServer->getAdvertising()->start();

  Serial.println("Waiting for a client connection to notify");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (deviceConnected){
    temperature = bme.readTemperature();

    char txString[8];
    dtostrf(temperature,1,2,txString);
    
    pCharacteristic->setValue(txString);

    pCharacteristic->notify();
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");
    
   delay(500);
  } 
}
