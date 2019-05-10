/*
 * ELCOche
 * 
 * This project has been created by:
 * - Mª del Pilar Aguilera Manzanera
 * - Enrique Calatayud Candelas
 * - Guillermo González Martín
 * - María Tejedor Rami
 * for the subject Electronica de Comunicaciones (ELCO) at Escuela Técnica Superior
 * de Ingenieros de Telecomunicación (ETSIT) in the universidad Politécnica de Madrid (UPM).
 *  
 * In this file you can find the code for the System Receptor (car).
 * The refered components are:
 * - Arduino Nano
 * - Receptor (o transceptor) of radio with serial communication
 * - Bridge in H (L298N)
 * - Motor DC (12V)
 * - 3 servomotors
 * - LEDs
 * - Buzzer
 * - Proximity sensor (HC-SR04)
 * - Photoresistor (LDR)
 */

// Libraries
#include <SoftwareSerial.h>
#include <Servo.h>
#include <NewPing.h>
/* tone() has conflicts NewPing.h. There are two solutions:
 *  1. Use another library to use the buzzer.
 *  2. Set TIMER_ENABLED = false in NewPing.h
 *  Uncomment the following line if 1.
 */
//#include <TimerFreeTone.h>

// Configuration
/*
 * You can change all the parameters of the RX in this section.
 */
#define SERIAL_BAUDRATE 9600
#define RADIO_BAUDRATE  57600
#define DATA_LENGTH     12
#define MIN_MOTOR       -255
#define MAX_MOTOR       255
#define MIN_DIR         60
#define MAX_DIR         120
#define MIN_YAW         -1000
#define MAX_YAW         1000
#define MIN_PITCH       -1000
#define MAX_PITCH       1000
#define MAX_DISTANCE    200
#define MIN_DIST_BRAKE  5
#define MAX_DIST_BRAKE  20
#define LDR_THRESHOLD   500
#define FREC_CLAXON     2000
#define DATA_HIGH       3
#define DATA_LOW        2
#define INTERVAL_WARN   500
#define INTERVAL_NO_SIG 1000

#define PIN_RADIO_TX    10
#define PIN_RADIO_RX    11
#define PIN_ME          3
#define PIN_M1          4
#define PIN_M2          5
#define PIN_DIR         8
#define PIN_CAM_YAW     6
#define PIN_CAM_PITCH   7
#define PIN_CLAXON      13
#define PIN_ULTRASONIC  9
#define PIN_LIGHT_F1    A0
#define PIN_LIGHT_F2    A1
#define PIN_LIGHT_B1    A2
#define PIN_LIGHT_R1    A3
#define PIN_LIGHT_W1    A4
#define PIN_LIGHT_W2    A5
#define PIN_LDR         A6

unsigned long intervalWarn =      INTERVAL_WARN;
unsigned long intervalNoSignal =  INTERVAL_NO_SIG;
unsigned long prevMillisBlink;
unsigned long prevMillisSignal;

int motor, dir, c_yaw, c_pitch;
bool lever_d, lever_r, sw_lights_on, sw_lights_auto, sw_warnings, sw_ultrasonic,  btn_stop, btn_claxon;
bool state_warning = LOW;
int dist, ldr;
int old_s_dir = 0;

bool stringComplete = false;
int data[DATA_LENGTH];
const char separator = ',';
char incomingByte;
String readBuffer = "";

SoftwareSerial radioSerial(PIN_RADIO_TX, PIN_RADIO_RX);
NewPing sonar(PIN_ULTRASONIC, PIN_ULTRASONIC, MAX_DISTANCE);
Servo servo_dir;

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  radioSerial.begin(RADIO_BAUDRATE);
  
  pinMode(PIN_CAM_YAW, OUTPUT);
  digitalWrite(PIN_CAM_YAW, LOW);
  pinMode(PIN_CAM_PITCH, OUTPUT);
  digitalWrite(PIN_CAM_PITCH, LOW);

  pinMode(PIN_ME,           OUTPUT);
  pinMode(PIN_M1,           OUTPUT);
  pinMode(PIN_M2,           OUTPUT);
  pinMode(PIN_LIGHT_F1,     OUTPUT);
  pinMode(PIN_LIGHT_F2,     OUTPUT);
  pinMode(PIN_LIGHT_B1,     OUTPUT);
  pinMode(PIN_LIGHT_R1,     OUTPUT);
  pinMode(PIN_LIGHT_W1,     OUTPUT);
  pinMode(PIN_LIGHT_W2,     OUTPUT);
  pinMode(PIN_LDR,          INPUT);

  prevMillisBlink = millis();
  prevMillisSignal = millis();
}
 
void loop() {
  bool start = false;
  readBuffer = "";
  servo_dir.detach();
  servo_dir.attach(PIN_DIR);
  dist = sonar.ping_cm();
  ldr = analogRead(PIN_LDR);

  /*
   * Checking if a switch or button has been pressed on the TX.
   */
  checkEmergency();
  checkObstacle();
  checkLever();
  checkControls();

  /*
   * Checking if the connection has been lost. In this case, warning lights blink and the motor stops. 
   */
  unsigned long currentMillis = millis();
  if ((unsigned long)(currentMillis - prevMillisSignal) >= intervalNoSignal) {
    while (!radioSerial.available()) {
      analogWrite(PIN_ME, 0);
      blink_lights();
    }
  }

  /*
   * Receiving data from the TX.
   */
  while (radioSerial.available()) {
    prevMillisSignal = millis();
    incomingByte = radioSerial.read(); 
    delay(1);
    if (start == true) {
      if (incomingByte != 'e') {
        readBuffer += char(incomingByte);
      } else {
        start = false;
      }
    } else if (incomingByte == 's') {
      start = true;
      stringComplete = true;
    }
  }

  /*
   * If the data string has arrived, we proceed and read it. 
   */
  if (stringComplete) {
    
    /*Debug
     * Serial.print("Data received: ");
     * Serial.println(readBuffer);
     */
    
    string_data(readBuffer);

    /*
     * Checking if the string is valid.
     */
    if (check_values(data)) {
      motor =           data[0];
      dir =             data[1];
      c_yaw =           data[2];
      c_pitch =         data[3]; 
      lever_d =         readValue(data[4]);
      lever_r =         readValue(data[5]);
      sw_lights_on =    readValue(data[6]);
      sw_lights_auto =  readValue(data[7]);
      sw_warnings =     readValue(data[8]);
      sw_ultrasonic =   readValue(data[9]);
      btn_stop =        readValue(data[10]);
      btn_claxon =      readValue(data[11]);

      /*
       * Writting in servo only if the current value is different of the old one.// writting in servo? no es wrriting a servo
       */
      if (old_s_dir != dir) {
        servo_dir.write(dir);
        old_s_dir = dir;
      }
      
      analogWrite(PIN_ME, motor);
      move_servo(PIN_CAM_YAW, c_yaw);
      move_servo(PIN_CAM_PITCH, c_pitch);
      printLogs();
    } else {
      Serial.print("Reception ERROR!: ");
      for (int i = 0; i < DATA_LENGTH-1; i++) {
        Serial.print(data[i]);
        Serial.print(", ");
      }
      Serial.println();
    }
  }
  stringComplete = false;
}

/*
 * Function that converts the number that means 'high' to '1' and
 * the number that means 'low' to '0'.
 */
bool readValue(int data) {
  return data == DATA_HIGH ? HIGH : LOW;
}

/*
 * Function that checks if the button of emergency has been pressed.
 * In this case, the warning lights turn on and Arduino stops a second
 * (The action that follows the emergency button can be changed as you
 * wish, set a bigger stop or even sound the buzzer).
 */
void checkEmergency() {
  if (btn_stop) {
    Serial.println("!!!-----------------------!!!");
    Serial.println("!!!----EMERGENCY STOP!----!!!");
    Serial.println("!!!-----------------------!!!");
    digitalWrite(PIN_LIGHT_W1, HIGH);
    digitalWrite(PIN_LIGHT_W2, HIGH);
    /*
     * while(1){
     *   blink_lights();
     * }
    }*/
    delay(1000);
  }
}

/*
 * Function that checks if there is an obstacle near the car.
 * In this case, the car stops.
 */
void checkObstacle() {
  if (lever_d && dist < MAX_DIST_BRAKE && dist > MIN_DIST_BRAKE) {   //Allows backward
    Serial.println("!!!------------------------!!!");
    Serial.println("!!!----EMERGENCY BRAKE!----!!!");
    Serial.println("!!!------------------------!!!");
    analogWrite(PIN_ME, 0);
  }
}

/*
 * Function that checks the gear lever (D, N or R).
 */
void checkLever() {
  if (lever_d) {
    digitalWrite(PIN_M1, HIGH);
    digitalWrite(PIN_M2, LOW);
    digitalWrite(PIN_LIGHT_R1, LOW);
  } else if (lever_r) {
    digitalWrite(PIN_M1, LOW);
    digitalWrite(PIN_M2, HIGH);
    digitalWrite(PIN_LIGHT_R1, HIGH);
  } else {
    digitalWrite(PIN_M1, LOW);
    digitalWrite(PIN_M2, LOW);
    digitalWrite(PIN_LIGHT_R1, LOW);
  }
}

/*
 * Function that checks if a switch of lights or claxon have changed.
 */
void checkControls() {
  if ((sw_lights_auto && checkLightIntensity()) || sw_lights_on) {
    digitalWrite(PIN_LIGHT_F1, HIGH); // Front left headlights
    digitalWrite(PIN_LIGHT_F2, HIGH); // Front right headlights
    digitalWrite(PIN_LIGHT_B1, HIGH); // Back headlights
  } else {
    digitalWrite(PIN_LIGHT_F1, LOW);  // Front left headlights
    digitalWrite(PIN_LIGHT_F2, LOW);  // Front right headlights
    digitalWrite(PIN_LIGHT_B1, LOW);  // Back headlights
  }
  
  if (sw_warnings) {
    blink_lights();                   // Function that blink lights
  } else {
    digitalWrite(PIN_LIGHT_W1, LOW);  // Front left warning lights
    digitalWrite(PIN_LIGHT_W2, LOW);  // Front right warning lights
  }
  
  if (btn_claxon) {
    tone(PIN_CLAXON, FREC_CLAXON);    // Plays a frecuency in a pin
  } else {
    noTone(PIN_CLAXON);               // Stop the sound
  }
}

/*
 * Function that returns '1' if the light is lower than a threshold and '0' if the
 * light is higher than a threshold 
 */
bool checkLightIntensity() {
  return ldr > LDR_THRESHOLD ? LOW : HIGH;
}

/*
 * Function that turns on and off the warning lights with a certain frequency 
 */
void blink_lights() {
  unsigned long currentMillis = millis();
  if ((unsigned long)(currentMillis - prevMillisBlink) >= intervalWarn) {
    state_warning = !state_warning;
    digitalWrite(PIN_LIGHT_W1, state_warning);
    digitalWrite(PIN_LIGHT_W2, state_warning);
    prevMillisBlink = millis();
  }
}

/*
 * Function that checks if the data sent from the TX is in the correct set of ranges.
 */
bool check_values(int data[]) {
  if (data[0] >= MIN_MOTOR && data[0] <= MAX_MOTOR &&
      data[1] >= MIN_DIR && data[1] <= MAX_DIR &&
      data[2] >= MIN_YAW && data[2] <= MAX_YAW &&
      data[3] >= MIN_PITCH && data[3] <= MAX_PITCH) {
    // checking flags
    for (int i = 4; i < DATA_LENGTH-1; i++) {
      if (data[i] != DATA_LOW && data[i] != DATA_HIGH) {
        return false;
      }
    }
    return true;
  }
  return false;
}

/*
 * Function that converts the string sent from the TX to integers values 
 * and saves it in an array.
 */
void string_data(String sdata){
  for (int i = 0; i < DATA_LENGTH; i++) {
    int index = sdata.indexOf(separator);
    data[i] = sdata.substring(0, index).toInt();
    sdata = sdata.substring(index + 1);
  }
}

/*
 * Function that moves an  certain angle the servomotor.
 * This allows to not use the servo library and avoid interrups.
 */
void move_servo(int pin, int angle) {
   float pause;
   pause = angle * 2000.0/180.0 + 700;
   digitalWrite(pin, HIGH); 
   delayMicroseconds(pause);
   digitalWrite(pin, LOW);
   delayMicroseconds(25000-pause);
}

/*
 * Function that prints in the console all the necesary information for debug. 
 */
void printLogs() {
  Serial.print("motor: ");
  Serial.print(motor);
  Serial.print(", ");
  Serial.print("dir: ");
  Serial.print(dir);
  Serial.print(", ");
  Serial.print("yaw: ");
  Serial.print(c_yaw);
  Serial.print(", ");
  Serial.print("pitch: ");
  Serial.print(c_pitch);
  Serial.print(", ");
  Serial.print("D: ");
  Serial.print(lever_d);
  Serial.print(", ");
  Serial.print("R: ");
  Serial.print(lever_r);
  Serial.print(", ");
  Serial.print("l_on: ");
  Serial.print(sw_lights_on);
  Serial.print(", ");
  Serial.print("l_auto: ");
  Serial.print(sw_lights_auto);
  Serial.print(", ");
  Serial.print("warn: ");
  Serial.print(sw_warnings);
  Serial.print(", ");
  Serial.print("ultra: ");
  Serial.print(sw_ultrasonic);
  Serial.print(", ");
  Serial.print("stop: ");
  Serial.print(btn_stop);
  Serial.print(", ");
  Serial.print("claxon: ");
  Serial.print(btn_claxon);

  Serial.print(", ");
  Serial.print("dist: ");
  Serial.print(dist);
  Serial.print(", ");
  Serial.print("ldr: ");
  Serial.println(ldr);
}
