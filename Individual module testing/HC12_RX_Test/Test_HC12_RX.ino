// ELCOche
/* -------------- TEST RECEPCIÓN RADIO HC12 --------------
 * Este test permite comprobar el correcto funcionamiento
 * de las radios HC12.
 * El test recibe los datos en un string que el transmisor
 * ha enviado previamente
 * Se usa la librería SoftwareSerial para no usar directamente
 * el puerto serie del Arduino, ya que esto causa algunos
 * problemas.
 * Asegúrese de fijar el baudrate a 9600 en el monitor serie.
*/

#define BAUDRATE        9600
#define PIN_TX_HC_12    10
#define PIN_RX_HC_12    11

#include <SoftwareSerial.h>

SoftwareSerial HC12(PIN_TX_HC_12, PIN_RX_HC_12);

char incomingByte;
String readBuffer = "";
int data;

void setup() {
  Serial.begin(BAUDRATE); // Serial port to computer
  HC12.begin(BAUDRATE);   // Serial port to HC12
}

void loop() {
  readBuffer = "";
  boolean start = false;
  
  while (HC12.available()) { 
    incomingByte = HC12.read();
    delay(5);
    if (start == true) {
      if (incomingByte != 'e') {
        readBuffer += char(incomingByte);
      } else {
        start = false;
      }
    } else if (incomingByte == 's') {
      start = true;
    }
  }
  data = readBuffer.toInt();
  Serial.println(data);
}
