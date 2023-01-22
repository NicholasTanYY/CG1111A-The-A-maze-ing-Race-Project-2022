#include "MeMCore.h"
#define TURNING_TIME_MS 451             // The time duration (ms) for turning
#define SHIFTING_TIME_MS 30             // The time duration (ms) for turning

#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_AS4 466 

#define RGBWait 400                     // Duration to wait for LED reflection

MeDCMotor left_motor(M1);               // Left motor to port M1
MeDCMotor right_motor(M2);              // Right motor to port M2
MeLineFollower lineFinder(2);           // Line finder to port 2
MeUltrasonicSensor ultraSensor(1);      // Ultrasonic distance sensor to port 1
MeRGBLed led(PORT_7);                   // LED to port 7
MeBuzzer buzzer;

// Values were found by calibrating the sensor using White and Black Paper
float whiteArray[3] = {971, 970, 1000};
float blackArray[3] = {656, 639, 751};
float greyDiff[3] = {315, 331, 249};

int LDRpin = A0;                        // Assign LDR pin to A0
int IRPin = A1;                         // Assign IR pin to A1
int sigA = A2;                          // Assign signal A to A2
int sigB = A3;                          // Assign signal B to A3
double distance_ultra = 0.00;
double IR_threshold = 3.15;             // Threshold voltage before IR sensor sends feedback to shift left
uint8_t motor_speed = 190;              // Assign speed at which the motor rotates (max 255)
double distance_threshold = 7.0;        // Threshold distance before ultrasonic sensor sends feedback to shift right

char color_detected = '0';              // Initialise char to store detected colour

void move_forward()
{
    left_motor.run(motor_speed);        // Negative: wheel turns anti-clockwise
    right_motor.run(-motor_speed);      // Positive: wheel turns clockwise
}

void turn_left()
{
    left_motor.run(motor_speed);        // Positive: wheel turns clockwise
    right_motor.run(motor_speed);       // Positive: wheel turns clockwise
    delay(TURNING_TIME_MS + 7);         // Keep turning left for this time duration
}

void turn_left_twice()
{
    turn_left();
    delay(23);
    move_forward();
    delay(1100);
    turn_left();
    delay(23);
}

void turn_right()
{
    left_motor.run(-motor_speed);       // Negative: wheel turns anti-clockwise
    right_motor.run(-motor_speed);      // Negative: wheel turns anti-clockwise
    delay(TURNING_TIME_MS - 2);         // Keep turning left for this time duration
}

void turn_right_twice()
{
    turn_right();
    delay(33);
    move_forward();
    delay(1150);
    turn_right();
    delay(33);
}

void u_turn_left()
{
    left_motor.run(motor_speed);        // Positive: wheel turns clockwise
    right_motor.run(motor_speed);       // Positive: wheel turns clockwise
    delay(TURNING_TIME_MS * 2 - 10);    // Keep turning left for this time duration
}

void u_turn_right()
{
    left_motor.run(-motor_speed);       // Positive: wheel turns clockwise
    right_motor.run(-motor_speed);      // Positive: wheel turns clockwise
    delay(TURNING_TIME_MS * 2 - 15);    // Keep turning left for this time duration
}

void shift_left()
{
    left_motor.run(motor_speed);        // Positive: wheel turns clockwise
    right_motor.run(motor_speed);       // Positive: wheel turns clockwise
    delay(SHIFTING_TIME_MS);            // Keep turning left for this time duration
}

void shift_right()
{
    left_motor.run(-motor_speed);       // Positive: wheel turns clockwise
    right_motor.run(-motor_speed);      // Positive: wheel turns clockwise
    delay(SHIFTING_TIME_MS);            // Keep turning left for this time duration
}

void stop_robot()
{
    left_motor.stop();                  // Stop left motor
    right_motor.stop();                 // Stop right motor
}

// Sensing black strip on ground
bool detected_black_line()
{
    // Return true when both sensors detect the black line
    int sensor_state = lineFinder.readSensors();
    return sensor_state == S1_IN_S2_IN;
}

int melody[] = {
    NOTE_G4, NOTE_A4, NOTE_F4, NOTE_G4, 0, NOTE_AS4, NOTE_A4, NOTE_F4, NOTE_G4
};

// note durations: 2 = half note, 8 = eighth note, 12 = twelth note
int noteDurations[] = {
    8, 12, 8, 2, 2, 8, 12, 8, 2
};

void tune() 
{
  // iterate over the notes of the melody:
  for (int curr_note = 0; curr_note < 9; curr_note++) 
  {
    // Calculate the note duration, take one second divided by the note type.
    int note_duration = 1000 / noteDurations[curr_note];
    buzzer.tone(8, melody[curr_note], note_duration);

    // We set a duration between each note
    int pause_tone = note_duration * 1.30;
    delay(pause_tone);
    
    // stop the tone playing
    buzzer.noTone(8);
  }
}

void colour(int i)
{
    // Y3 LOW - Turning the R LED on
    if (i == 0)
    {
        digitalWrite(A2, HIGH);
        digitalWrite(A3, HIGH);
    }

    // Y1 LOW - Turning the G LED on
    if (i == 1)
    {
        digitalWrite(A2, HIGH);
        digitalWrite(A3, LOW);
    }

    // Y2 LOW - Turning the B LED on
    if (i == 2)
    {
        digitalWrite(A2, LOW);
        digitalWrite(A3, HIGH);
    }

    // Y0 LOW - Turning the IR emittor on
    if (i == 3)
    {
        digitalWrite(A2, LOW);
        digitalWrite(A3, LOW);
    }
}

void execute_manoeuvre(char color)
{
    // Detected colour is Red - turn left
    if (color == 'R')
    {
        Serial.println("turning left...");
        turn_left();
    }

    // Detected colour is Green - turn right
    else if (color == 'G')
    {
        Serial.println("turning right...");
        turn_right();
    }

    // Detected colour is Orange - u-turn
    else if (color == 'O')
    {
        Serial.println(ultraSensor.distanceCm());

        // U-turn right if it is too narrow on the left
        if (ultraSensor.distanceCm() <= 12.2)
        {
            Serial.println("u-turning right...");
            u_turn_right();
        }

        // U-turn left if it is too narrow on the right
        else
        {
            Serial.println("u-turning left...");
            u_turn_left();
        }
    }

    // Detected colour is Purple - turn left twice
    else if (color == 'P')
    {
        Serial.println("turning left twice...");
        turn_left_twice();
    }

    // Detected colour is Red - turn right twice
    else if (color == 'B')
    {
        Serial.println("turning right twice...");
        turn_right_twice();
    }

    // Detected colour is White - stop and play tune
    else if (color == 'W')
    {
        Serial.println("playing tune...");
        tune();
        Serial.println("tune finish. Let's observe a moment of silence...");
        delay(9999999999);
    }
}

char detectColor(int RGB[3])
{
    char res = '0';
    Serial.println(RGB[0]);
    Serial.println(RGB[1]);
    Serial.println(RGB[2]);

    // Colour ranges which enable colour sensors to detect colours accurately in the given lighting
    // Colour range for White
    if ((RGB[0] >= 212 && RGB[0] <= 251) && (RGB[1] >= 200 && RGB[1] <= 250) && (RGB[2] >= 191 && RGB[2] <= 244)) 
    {
        res = 'W';
        Serial.println("White");
    }

    // Colour range for Orange
    else if ((RGB[0] >= 214 && RGB[0] <= 246) && (RGB[1] >= 124 && RGB[1] <= 178) && (RGB[2] >= 95 && RGB[2] <= 156)) 
    {
        res = 'O';
        Serial.println("Orange");
    }

    // Colour range for Red
    else if ((RGB[0] >= 212 && RGB[0] <= 246) && (RGB[1] >= 77 && RGB[1] <= 123) && (RGB[2] >= 95 && RGB[2] <= 157)) 
    {
        res = 'R';
        Serial.println("Red");
    }

    // Colour range for Purple
    else if ((RGB[0] >= 162 && RGB[0] <= 203) && (RGB[1] >= 130 && RGB[1] <= 180) && (RGB[2] >= 167 && RGB[2] <= 213)) 
    {
        res = 'P';
        Serial.println("Purple");
    }

    // Colour range for Blue
    else if ((RGB[0] >= 124 && RGB[0] <= 156) && (RGB[1] >= 169 && RGB[1] <= 216) && (RGB[2] >= 187 && RGB[2] <= 235)) 
    {
        res = 'B';
        Serial.println("Blue");
    }

    // Colour range for Green
    else if ((RGB[0] >= 30 && RGB[0] <= 79) && (RGB[1] >= 134 && RGB[1] <= 194) && (RGB[2] >= 127 && RGB[2] <= 181)) 
    {
        res = 'G';
        Serial.println("Green");
    } 
    return res;
}

int get_average_reading(int times)
{
    // Find the average reading for the requested number of times of scanning LDR
    int reading;
    int total = 0;

    // Take the reading as many times as requested and add them up
    for(int i = 0 ; i < times ; i++){
        reading = analogRead(LDRpin);
        total = reading + total;
    }

    // Return the calculated average
    return total/times;
}

char read_color()
{
    Serial.println("reading colour");
    int final_RGB[3] = {0};
    for (int i = 0; i <= 2; i++)
    {
        // Turn on different coloured LEDs one at a time
        colour(i);
        delay(RGBWait);

        // Get the average of 10 readings and rebase to 255
        final_RGB[i] = (get_average_reading(10) - blackArray[i])/(greyDiff[i])*255;
    }
    return detectColor(final_RGB);
}

// Stop at black line and make the appropriate turns
void travel() 
{
    // Black strip detected. Stop moving and do colour challenge
    if (detected_black_line()) 
    {    
        Serial.println("Sensor 1 and 2 detect the black line");
        stop_robot();

        // Colour challenge: detecting the colour of the paper. Returns a characters representing the colour
        color_detected = read_color();  
        Serial.println(color_detected);

        // Colour challenge: Performing the appropriate turns based on the colour of the paper
        execute_manoeuvre(color_detected);  
    }
    // The mBot continues moving if no black line is detected
    else
    {
        Serial.println("No detection");
        move_forward();  
    }

    // Keep the IR on when the mBot is moving forward
    colour(3);  

    // Measure the distance from the left of the mBot to the wall
    distance_ultra = ultraSensor.distanceCm();

    // Robot is too close to the left wall 
    if (distance_ultra <= distance_threshold)  
    {
        Serial.println("Too close to left wall, shifting right...");
        shift_right();
    }

    // Calculates IR voltage based on the IR reading - More details in the report
    int IR_reading = analogRead(IRPin);
    float IR_voltage = (IR_reading * (1.0) / 1023) * 5;  
    Serial.println(IR_voltage);

    // magnitude of IR_voltage increases as distance from the right wall increases
    if (IR_voltage <= IR_threshold)
    {
        Serial.println("Too close to right wall, shifting left...");  
        shift_left();
    }
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Robot started...");
    delay(1000);
    pinMode(sigA, OUTPUT);
    pinMode(sigB, OUTPUT);
}

void loop()
{  
    travel();
}