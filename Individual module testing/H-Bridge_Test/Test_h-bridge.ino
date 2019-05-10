// ELCOche
/* -------------------- TEST PUENTE-H --------------------
 * Este test permite comprobar el correcto funcionamiento
 * de un puente en h, concretamente ha sido probado en el 
 * L298N.
 * El test permite controlar la velocidad de un motor con
 * un potenciómetro conectado a la entrada analógica A0.
 * El pin de enable del puente se debe conectar a una entrada
 * PWM (D3 lo es).
 * Asegúrese de fijar el baudrate a 9600 en el monitor serie.
*/

#define BAUDRATE    9600
#define TEST_PIN_0  A0
#define TEST_PIN_1  A1
#define PIN_ME      3
#define PIN_M1      4
#define PIN_M2      5
#define PIN_POT     A0

int motor;

void setup() {
  Serial.begin(BAUDRATE); 
  pinMode(PIN_M1, OUTPUT);
  pinMode(PIN_M2, OUTPUT);
  pinMode(PIN_ME, OUTPUT);
}
 
void loop() {
  motor = map(analogRead(PIN_POT), 512, 1023, 0, 255);
  digitalWrite(PIN_M1, HIGH);
  digitalWrite(PIN_M2, LOW);
  analogWrite(PIN_ME, motor);
  delay(10);
  Serial.println(motor);
}
