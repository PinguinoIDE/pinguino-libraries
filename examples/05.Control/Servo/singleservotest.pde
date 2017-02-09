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

#define MYSERVO 0   // servo attached to pin 0

u8 angle=90;
s8 dir=1;

void setup(void)
{
    pinMode(USERLED, OUTPUT);    
    servo.attach(MYSERVO);
    //servo.setMinimumPulse(MYSERVO, 1000); // 1.0ms = 0 deg
    //servo.setMaximumPulse(MYSERVO, 1800); // 1.8ms = 180 deg
}  

void loop(void)
{
    servo.write(MYSERVO, angle);
    angle= angle + dir;
    if (angle>180) dir = -1;
    if (angle<1  ) dir =  1;
    toggle(USERLED);
    delay(20);
}
