const int channelPinA = 2;
const int channelPinB = 10;
 
const int timeThreshold = 5;
long timeCounter = 0;
 
const int maxSteps = 255;
volatile int ISRCounter = 0;
int counter = 0;
 
bool IsCW = true;
 
void setup()
{
   pinMode(channelPinA, INPUT_PULLUP);
   Serial.begin(9600);
   attachInterrupt(digitalPinToInterrupt(channelPinA), doEncode, CHANGE);
}
 
void loop()
{
   if (counter != ISRCounter)
   {
      counter = ISRCounter;
      Serial.println(counter);
   }
   delay(100);
}
 
void doEncode()
{
   if (millis() > timeCounter + timeThreshold)
   {
      if (digitalRead(channelPinA) == digitalRead(channelPinB))
      {
         IsCW = true;
         if (ISRCounter + 1 <= maxSteps) ISRCounter++;
      }
      else
      {
         IsCW = false;
         if (ISRCounter - 1 > 0) ISRCounter--;
      }
      timeCounter = millis();
   }
}
