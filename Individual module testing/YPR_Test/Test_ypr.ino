// Roll, Pitch and Yaw with MPU6050
// http://www.giuseppecaccavale.it
// Giuseppe Caccavale

#include <Wire.h>
#include <MPU6050.h>
#include <Servo.h>

MPU6050 mpu;

Servo servo0;
Servo servo1;

// Pitch, Roll and Yaw values
int pitch = 0;
int roll = 0;
float yaw = 0;

void setup() 
{
  Serial.begin(115200);

  Serial.println("Initialize MPU6050");

  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  // Calibrate gyroscope. The calibration must be at rest.
  // If you don't want calibrate, comment this line.
  mpu.calibrateGyro();
  
  // Set threshold sensivty. Default 3.
  // If you don't want use threshold, comment this line or set 0.
  mpu.setThreshold(1);

  servo0.attach(10);
  servo1.attach(9);
}

void loop() {
  // Read normalized values 
  Vector normAccel = mpu.readNormalizeAccel();
  Vector normGyro = mpu.readNormalizeGyro();

  // Calculate Pitch & Roll
  pitch = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis*normAccel.YAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
  roll = (atan2(normAccel.YAxis, normAccel.ZAxis)*180.0)/M_PI;
  
  //Ignore the gyro if our angular velocity does not meet our threshold
  if (normGyro.ZAxis > 1 || normGyro.ZAxis < -1) {
    normGyro.ZAxis /= 100;
    yaw += normGyro.ZAxis;
  }

   //Keep our angle between 0-359 degrees
  if (yaw < 0)
    yaw += 360;
  else if (yaw > 359)
    yaw -= 360;
    
  // Output
  Serial.print("Pitch = ");
  Serial.print(pitch);
  Serial.print("\tRoll = ");
  Serial.print(roll);
  Serial.print("\tYaw = ");
  Serial.print(yaw);
  
  Serial.println();
  
  int servo0Value = map(yaw, -90, 90, 0, 180);
  int servo1Value = map(pitch, -90, 90, 0, 180);

  servo0.write(servo0Value);
  servo1.write(servo1Value);
  
  delay(10);
}
