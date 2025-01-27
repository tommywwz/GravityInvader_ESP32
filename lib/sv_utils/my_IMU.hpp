#ifndef MY_IMU_HPP
#define MY_IMU_HPP

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

volatile double pitch, roll = 0;

void getAngle(const sensors_event_t& accel)
{
    double x = accel.acceleration.x;
    double y = accel.acceleration.y;
    double z = accel.acceleration.z;
    pitch = atan(x / sqrt((y * y) + (z * z)));
    roll = atan(y / sqrt((x * x) + (z * z)));
    pitch = pitch * (180.0 / PI);
    roll = roll * (180.0 / PI);
}

void setupGyro()
{
    if (!mpu.begin())
    {
        Serial.println("Failed to find MPU6050 chip");
        for (;;)
            ; // Don't proceed, loop forever
    }
    Serial.println("MPU6050 Found!");

    // set accelerometer range to +-8G
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

    // set gyro range to +- 500 deg/s
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);

    // set filter bandwidth to 21 Hz
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    delay(100);
}

#endif