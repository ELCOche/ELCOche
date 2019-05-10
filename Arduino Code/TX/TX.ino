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
 * In this file you can find the code for the System transmitter (steering wheel, Throttle and VR glasses).
 * The refered components are:
 * - Arduino Mega
 * - Transmitter (o transceptor) of radio with serial communication
 * - Accelerometer (MPU6050)
 * - 2 potentiometer
 * - 4 buttons
 * - 3 switches spdt (2 positions)
 * - 2 switches spdt (3 positions)
 * - Joystick
 */

// Libraries  
#include <SoftwareSerial.h>
#include <MPU6050.h>
#include <LiquidCrystal.h> 

// Configuration
/*
 * You can change all the parameters of the TX in this section.
 */
#define SERIAL_BAUDRATE     9600
#define RADIO_BAUDRATE      57600
#define MIN_MOTOR           0
#define MAX_MOTOR           255
#define MIN_DIR             60
#define MAX_DIR             120
#define MIN_PEDAL           0
#define MAX_PEDAL           829
#define MIN_WHEEL           230
#define CEN_WHEEL           540
#define MAX_WHEEL           832
#define DATA_HIGH           3
#define DATA_LOW            2

#define COLS                20
#define ROWS                4
#define E                   50
#define RS                  52
#define D4                  48
#define D5                  46
#define D6                  44
#define D7                  42

#define PIN_RADIO_TX        10
#define PIN_RADIO_RX        11
#define PIN_MOTOR           A1
#define PIN_DIR             A0
#define PIN_JOY_X           A3
#define PIN_JOY_Y           A4
#define PIN_LEVER_D         53
#define PIN_LEVER_R         51
#define PIN_SW_LIGHTS_ON    41
#define PIN_SW_LIGHTS_AUTO  47
#define PIN_SW_WARNINGS     49
#define PIN_SW_ULTRASONIC   45
#define PIN_SW_CAM          43
#define PIN_BTN_STOP        35
#define PIN_BTN_CAL         39
#define PIN_BTN_CAL_MPU     37
#define PIN_BTN_CLAXON      31

LiquidCrystal lcd(E, RS, D4, D5, D6, D7);
SoftwareSerial radio(PIN_RADIO_TX, PIN_RADIO_RX);
MPU6050 mpu;

int motor, dir, c_yaw, c_pitch;
int lever_d, lever_r, sw_lights_on, sw_lights_auto, sw_warnings, sw_ultrasonic, sw_cam, btn_stop, btn_cal, btn_cal_mpu, btn_claxon;

float timeStep = 0.01;
float yaw = 0;
float pitch = 0;

String data = "";
String text_0 = "ELCOche - 2019";
String text_1, text_2, text_3;

void setup() {
   
  Serial.begin(SERIAL_BAUDRATE);
  radio.begin(RADIO_BAUDRATE);
  lcd.begin(COLS, ROWS);
  
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)) {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(text_0);
    lcd.setCursor(0, 2);
    lcd.print("MPU ERROR!");
    lcd.setCursor(0, 3);
    lcd.print("Please, check wiring");
    delay(500);
  }
  mpu.calibrateGyro();
  mpu.setThreshold(3); //Default: 3
  
  pinMode(PIN_MOTOR,              INPUT);
  pinMode(PIN_DIR,                INPUT);
  pinMode(PIN_JOY_X,              INPUT);
  pinMode(PIN_JOY_Y,              INPUT);
  pinMode(PIN_LEVER_D,            INPUT_PULLUP);
  pinMode(PIN_LEVER_R,            INPUT_PULLUP);
  pinMode(PIN_SW_LIGHTS_ON,       INPUT_PULLUP);
  pinMode(PIN_SW_LIGHTS_AUTO,     INPUT_PULLUP);
  pinMode(PIN_SW_WARNINGS,        INPUT_PULLUP);
  pinMode(PIN_SW_ULTRASONIC,      INPUT_PULLUP);
  pinMode(PIN_BTN_STOP,           INPUT_PULLUP);
  pinMode(PIN_BTN_CAL,            INPUT_PULLUP);
  pinMode(PIN_BTN_CAL_MPU,        INPUT_PULLUP);
  pinMode(PIN_BTN_CLAXON,         INPUT_PULLUP);
  pinMode(PIN_SW_CAM,             INPUT_PULLUP);
}

void loop() {
  /*
   * Getting yaw an pitch values from the accelerometer.
   */
  getYPR();

  /*
   * With the calibration data, defining the max turn for both directions of the steering 
   * wheel, in order to set the max value.
   */
  int max_turn = min(abs(MIN_WHEEL - CEN_WHEEL), abs(MAX_WHEEL - CEN_WHEEL));
  
  motor =           map(analogRead(PIN_MOTOR), MIN_PEDAL, MAX_PEDAL, MIN_MOTOR, MAX_MOTOR);
  dir =             map(analogRead(PIN_DIR), CEN_WHEEL - max_turn, CEN_WHEEL + max_turn, MIN_DIR, MAX_DIR);
  lever_d =         readValue(PIN_LEVER_D);
  lever_r =         readValue(PIN_LEVER_R);
  sw_lights_on =    readValue(PIN_SW_LIGHTS_ON);
  sw_lights_auto =  readValue(PIN_SW_LIGHTS_AUTO);
  sw_warnings =     readValue(PIN_SW_WARNINGS);
  sw_ultrasonic =   readValue(PIN_SW_ULTRASONIC);
  sw_cam =          readValue(PIN_SW_CAM);
  btn_stop =        readValue(PIN_BTN_STOP);
  btn_cal =         readValue(PIN_BTN_CAL);
  btn_cal_mpu =     readValue(PIN_BTN_CAL_MPU);
  btn_claxon =      readValue(PIN_BTN_CLAXON);

  /*
   * If the value of the pedal potentiometer is very low, ignore it.
   */
  if (motor < 5) {
    motor = 0;
  }

  /*
   * dir (direction) can´t be out of the definited range.
   */
  if (dir > MAX_DIR) {
    dir = MAX_DIR;
  } else if (dir < MIN_DIR) {
    dir = MIN_DIR;
  }

  /*
   * If the switch that checks the cam mode is in joystick mode, map the analog inputs 
   * of the joystick. If it is in VR mode, map the yaw and pitch values sent from the 
   * accelerometer. 
   */
  if (sw_cam == DATA_LOW) {
    c_yaw =         map(analogRead(PIN_JOY_X), 0, 1023, 0, 180);
    c_pitch =       map(analogRead(PIN_JOY_Y), 1023, 0, 0, 180);
  } else {
    c_yaw =         map(yaw, -10, 10, 0, 180);
    c_pitch =       map(pitch, -10, 10, 0, 180);
  }
  
  sendData();
  printLogs();
  printLCD();

  if (btn_cal_mpu == DATA_HIGH){
    Serial.println("Calibrado de MPU");
    // It doesn´t work very well...
    mpu.calibrateGyro();
    mpu.setThreshold(3); //Default: 3
  }
  delay(50);
}

/*
 * Function that converts '1' and '0 to another integer values.
 */
int readValue(int pin) {
  return digitalRead(pin) ? DATA_LOW : DATA_HIGH;
}

/*
 * Function that creates the data string that is sent to the RX. 
 */
void sendData() {
  data = "";
  data += String(motor);
  data += ",";
  data += String(dir);
  data += ",";
  data += String(c_yaw);
  data += ",";
  data += String(c_pitch);
  data += ",";
  data += String(lever_d);
  data += ",";
  data += String(lever_r);
  data += ",";
  data += String(sw_lights_on);
  data += ",";
  data += String(sw_lights_auto);
  data += ",";
  data += String(sw_warnings);
  data += ",";
  data += String(sw_ultrasonic);
  data += ",";
  data += String(btn_stop);
  data += ",";
  data += String(btn_claxon);
  radio.print("s" + data + "e");
}

/*
 * Function that writes information on the LCD display. 
 */
void printLCD() {
  if (lever_d == DATA_HIGH) {
    text_1 = "Gear lever: D";
  } else if (lever_r == DATA_HIGH){
    text_1 = "Gear lever: R";
  } else {
    text_1 = "Gear lever: N";
  }
  if (sw_cam == DATA_HIGH) {
    text_2 = "Cam mode: VR";
  } else {
    text_2 = "Cam mode: Joystick";
  }
  if (sw_lights_on == DATA_HIGH) {
    text_3 = "Lights: ON";
  } else if (sw_lights_auto == DATA_HIGH) {
    text_3 = "Lights: AUTO";
  } else {
    text_3 = "Lights: OFF";
  }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(text_0);
  lcd.setCursor(0, 1);
  lcd.print(text_1);
  lcd.setCursor(0, 2);
  lcd.print(text_2);
  lcd.setCursor(0, 3);
  lcd.print(text_3);
}

/*
 * Function that prints in the console all necesary information for debug.
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
  Serial.print("cam: ");
  Serial.print(sw_cam);
  Serial.print(", ");
  Serial.print("stop: ");
  Serial.print(btn_stop);
  Serial.print(", ");
  Serial.print("cal: ");
  Serial.print(btn_cal);
  Serial.print(", ");
  Serial.print("cal_mpu: ");
  Serial.print(btn_cal_mpu);
  Serial.print(", ");
  Serial.print("claxon: ");
  Serial.println(btn_claxon);
}

/*
 * Function that gets yaw and pitch values from the accelerometer. 
 */
void getYPR() {
  Vector norm = mpu.readNormalizeGyro();
  yaw = yaw + norm.ZAxis * timeStep;
  pitch = pitch + norm.YAxis * timeStep;
}
