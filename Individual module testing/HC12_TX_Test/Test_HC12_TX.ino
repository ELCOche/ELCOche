// ELCOche
/* -------------- TEST TRANSMISIÓN RADIO HC12 --------------
 * Este test permite comprobar el correcto funcionamiento
 * de las radios HC12.
 * El test envía tres números en un string que el receptor
 * se encargará de decodificar.
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

int a = 123;
int b = 456;
int c = 789;
String data = "";

void setup() {
  Serial.begin(BAUDRATE); // Serial port to computer
  HC12.begin(BAUDRATE);   // Serial port to HC12
}

void loop() {
  // Se crea el string
  data = "";
  data += String(a);
  data += ",";
  data += String(b);
  data += ",";
  data += String(c);
  // Se envían caracteres de control (s - start; e - end)
  HC12.print("s" + data + "e");  // La función print envía los valores al HC12
  Serial.println(data);          // Se imprime el string por el monitor serie
  delay(10);
}
