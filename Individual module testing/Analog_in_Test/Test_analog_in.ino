// ELCOche
/* ----------- TEST PARA LEER ENTRADAS ANALÓGICAS -----------
 * Este test permite leer el valor de las entradas analógicas
 * A0 y A1 (puede cambiar estos valor en TEST_PIN_X).
 * Sirve para establecer los valores mínimo y máximo del pedal
 * y el volante.
 * Asegúrese de fijar el baudrate a 9600 en el monitor serie.
*/

#define BAUDRATE    9600
#define TEST_PIN_0  A0
#define TEST_PIN_1  A1

void setup() { 
  Serial.begin(BAUDRATE);
  pinMode(TEST_PIN_0, INPUT);
  pinMode(TEST_PIN_1, INPUT);
}
 
void loop() { 
  Serial.print("A0: ");
  Serial.print(analogRead(TEST_PIN_0));
  Serial.print(", ");
  Serial.print("A1: ");
  Serial.print(analogRead(TEST_PIN_1));
  Serial.println();
  delay(100);
}
