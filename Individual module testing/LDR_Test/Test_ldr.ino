const int LDRPin = A0;
const int threshold = 100;
 
void setup() {
  Serial.begin(9600);
  pinMode(LDRPin, INPUT);
}
 
void loop() {
  int input = analogRead(LDRPin);
  Serial.println(input);
}
