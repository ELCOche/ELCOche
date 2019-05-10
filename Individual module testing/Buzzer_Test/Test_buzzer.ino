// ELCOche
/* ----------------- TEST PARA EL BUZZER -----------------
 * Este test permite comprobar el correcto funcionamiento
 * de un buzzer conectado a la entrada digital D7.
 * Reproduce una frecuencia de 1 kHz durante 1 segundo
 * y para durante otro segundo.
 * Asegúrese de fijar el baudrate a 9600 en el monitor serie.
*/

#define BAUDRATE    9600
#define PIN_BUZZER  7
#define FREC        1000

void setup(){
  pinMode(PIN_BUZZER, OUTPUT);
}

void loop() {
  tone(PIN_BUZZER, FREC); // Send 1 kHz sound signal
  delay(1000);            // wait 1 sec
  noTone(PIN_BUZZER);     // Stop sound
  delay(1000);            // wait 1 sec
}
