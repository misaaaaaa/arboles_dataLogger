//código para proj arboles ciudadanos

#include <SD.h>
#include <Wire.h>
#include <SPI.h>  // Necesario para la SD card

//conexiones típicas de SD
//CS 10
//MOSI 11
//CLK 13
//MISO 12

//BOTON EN PIN2
//LED EN PIN 6

#include "ADS1X15.h"
ADS1115 ADS(0x48);

uint8_t pair = 01;
int16_t val_01 = 0;
int16_t val_23 = 0;


File logfile;               // Fichero a escribir en la SD
const int chipSelect = 10;  // SD card pin select
const bool eco = true;
int count = 0;  // Controla cada cuanto tiemepo se vuelcan los datos a la SD

const int buttonPin = 2;  // the number of the pushbutton pin
const int ledPin = 6;     // the number of the LED pin
int buttonState = 0;      // variable for reading the pushbutton status

void setup() {

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);

  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("ADS1X15_LIB_VERSION: ");
  Serial.println(ADS1X15_LIB_VERSION);

  Wire.begin();

  ADS.begin();
  ADS.setGain(0);      //  6.144 volt
  ADS.setDataRate(4);  //  0 = slow   4 = medium   7 = fast

  //  single shot mode
  ADS.setMode(1);
  //  start with first pair
  pair = 01;
  //  trigger first read
  ADS.requestADC_Differential_0_1();

  pinMode(chipSelect, OUTPUT);  // SD card pin select



  if (!SD.begin(chipSelect)) {
    error("No hay tarjeta SD.");
    digitalWrite(ledPin,HIGH);
  } else {
    Serial.println("Tarjeta SD inicializada.");
    blinking();
  }

  // Creamos el fichero de registro
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (!SD.exists(filename))  // Si no existe el fichero, lo creamos
    {
      logfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }
  if (!logfile)
    error("No s epudo crear el fichero de registro");

  Serial.print("Registrando en: ");
  Serial.println(filename);

}

void loop() {

  if (handleConversion() == true) {

    Serial.print("COMP:\t");
    Serial.print(val_01);
    Serial.print("\t");
    Serial.print(val_23);
    Serial.print("\t");
    Serial.print(count);
    Serial.println();

    logfile.print(val_01);
    logfile.print(", ");
    logfile.println(val_23);


    buttonState = digitalRead(buttonPin);


    if (count++ > 64) {
      logfile.flush();  // Para forzar la escritura en la SD
      count = 0;        // Cada 64 lecturas
    }

    if (buttonState == HIGH) {
      logfile.flush();  // Para forzar la escritura en la SD
      digitalWrite(ledPin,HIGH);
    }
  }

  //muestras cada x ms
  delay(500);  
}

void error(char *str) {
  Serial.print("error: ");
  digitalWrite(ledPin,HIGH);
  Serial.println(str);

  while (1)
    ;
}


bool handleConversion() {
  if (ADS.isReady()) {
    if (pair == 01) {
      val_01 = ADS.getValue();
      pair = 23;
      ADS.requestADC_Differential_2_3();
      return false;  //  only one done
    }

    //  last of series to check
    if (pair == 23) {
      val_23 = ADS.getValue();
      pair = 01;
      ADS.requestADC_Differential_0_1();
      return true;  //  both are updated
    }
  }
  return false;  //  default not all read
}

void blinking(){
  digitalWrite(ledPin,HIGH);
    delay(500);
    digitalWrite(ledPin,LOW);
    delay(500);
    digitalWrite(ledPin,HIGH);
    delay(500);
    digitalWrite(ledPin,LOW);
    delay(500);
    digitalWrite(ledPin,HIGH);
    delay(500);
    digitalWrite(ledPin,LOW);
    delay(500);

}