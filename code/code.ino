/*  ----------------------------------------------------------------
    http://www.prometec.net/data-logger-shield
    Prog_146_1
    
    Ejemplo de data loggin con reloj y SD card
--------------------------------------------------------------------  */
// #include <DHT11.h>
#include <SD.h>
#include <Wire.h>
#include <SPI.h>  // Necesario para la SD card
// #include "RTClib.h"

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

// DHT11 dht11(6);     // Definimos el sensor de temperatura
// RTC_DS1307 RTC;     // Definimos el reloj
File logfile;               // Fichero a escribir en la SD
const int chipSelect = 10;  // SD card pin select
const bool eco = true;
int count = 0;  // Controla cada cuanto tiemepo se vuelcan los datos a la SD

const int buttonPin = 2;  // the number of the pushbutton pin
const int ledPin = 6;     // the number of the LED pin
int buttonState = 0;      // variable for reading the pushbutton status

void setup() {

  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
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
  // pinMode(5, OUTPUT)  ;       // DHT11 GMD
  // pinMode(7, OUTPUT) ;        // DHT11 Vcc
  // pinMode(6, INPUT_PULLUP) ;  // DHT11 pin de lectura
  pinMode(chipSelect, OUTPUT);  // SD card pin select

  // digitalWrite(7, HIGH) ;   // Damos tension al DHT11
  // digitalWrite(5, LOW) ;

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

  // connect to RTC
  // Wire.begin();
  // if (!RTC.begin())
  //     logfile.println("No hay RTC.");
  // else
  //     Serial.println("RTC correcto. Iniciando captura de datos");

  // logfile.print("Fecha/Hora") ;
  // logfile.print(", ");
  // //logfile.print(" ID ") ;
  // //logfile.print(", ");
  // logfile.print("Temp") ;
  // logfile.println("Humedad") ;
}

void loop() {
  //DateTime now;
  //int err;
  //float temp, hum;

  ////////////////////////////////
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

  //  do other stuff here
  delay(10);

  //  if((err = dht11.read(hum, temp)) == 0)    // Si devuelve 0 es que ha leido bien
  //     {  //now = RTC.now();

  //       //  logfile.print(now.unixtime()); // seconds since 1/1/1970
  //       //  logfile.print(", ");
  //        //logfile.print(count);
  //        //logfile.print(", ");
  //        logfile.print(temp);
  //        logfile.print(", ");
  //        logfile.println(hum);
  //        if ( count++ > 64 )
  //           {   logfile.flush(); // Para forzar la escritura en la SD
  //               count = 0 ;       // Cada 64 lecturas
  //           }
  //       //  if (eco)
  //       //   {
  //       //      //Serial.print(now.unixtime()); // seconds since 1/1/1970
  //       //      Serial.print(", ");
  //       //      Serial.print(count);
  //       //      Serial.print(", ");
  //       //      Serial.print(temp);
  //       //      Serial.print(", ");
  //       //      Serial.println(hum);
  //       //   }
  //     }
  //  else
  //     {
  //        Serial.println();
  //        Serial.print("Error Num :");
  //        Serial.print(err);
  //        Serial.println();
  //     }
  delay(500);  //Recordad que solo lee una vez por segundo
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