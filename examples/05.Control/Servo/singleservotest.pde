// Servo library test with Pinguino
// Connect servo control lines to pinguino pins.
// Feed servos with +5 V:
//
//   +-----+
//   |servo|--------- PWM Servo control ---> to pinguino pin.
//   |     |--------- +5V
//   |  o  |--------- GND
//   |     | 
//   +-----+

#define MYSERVO 0   // servo attached to pin PWM0 (D8)

u16 MaxPulse, MinPulse;

void setup(void)
{
    servo.attach(MYSERVO);
    /* If you need special values */
    //servo.setMinimumPulse(MYSERVO, 1000); // 1.0ms = 1000 us =   0 deg
    //servo.setMaximumPulse(MYSERVO, 2000); // 2.0ms = 2000 us = 180 deg
    /* If you need to know servo values */
    MinPulse = servo.getMinimumPulse(MYSERVO);
    MaxPulse = servo.getMaximumPulse(MYSERVO);
    
    pinMode(USERLED, OUTPUT);    
}  

void loop(void)
{
    u16 angle;
    u16 pulse;

    /* from 0 to 180 degrees */
    digitalWrite(USERLED, 1);
    for (angle = 0; angle < 180; angle++)
        servo.write(MYSERVO, angle);

    /* from MaximumPulse (180 deg) to MinimumPulse (0 deg) */
    digitalWrite(USERLED, 0);
    for (pulse = MaxPulse; pulse > MinPulse; pulse -= 10)
        servo.pulse(MYSERVO, pulse);
}
