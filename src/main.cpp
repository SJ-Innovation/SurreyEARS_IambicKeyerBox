#include "Arduino.h"

#define SPEAKER_OUT 3
#define POT_LOW_PIN A0
#define POT_WIPER_PIN A1
#define POT_HIGH_PIN A2

#define KEY_LOW_PIN 10
#define KEY_LEFT_PIN 9
#define KEY_RIGHT_PIN 8

#define DIT_DAH_RATIO 3

#define PADDLE_PRESSED_LEFT !digitalRead(KEY_LEFT_PIN)
#define PADDLE_PRESSED_RIGHT !digitalRead(KEY_RIGHT_PIN)

#define BEEP_FREQ 746.25

#define START_TONE tone(SPEAKER_OUT, BEEP_FREQ);
#define END_TONE noTone(SPEAKER_OUT);


#define MODE_THRESHOLD 1000

// state of the machine
typedef enum {
    IDLE = 0, // doing nothing
    DASH = 1,  // playing a dash
    DOT = 2,   // playing a dot
    DELAY = 3  // in the dot-length delay between two dot/dashes
} State_t;


void HandleIambicMorse(int DitLength, int DahLength) {
    static State_t CurrentState = IDLE; // sate of what the keyer output is sending right now
    static State_t NextState = IDLE; // state the keyer will go into when current element ends
    static State_t LastState = IDLE;        // previous state of the keyer
    static unsigned long NextStateChangeTime; // what CurrentTime did the current element start    sounding (in milliseconds since powerup)

    bool LeftPaddlePressed = PADDLE_PRESSED_LEFT;  // read the current values of the paddles
    bool RightPaddlePressed = PADDLE_PRESSED_RIGHT;

    unsigned long CurrentTime = millis();

    switch (CurrentState) {  // cases based on what current state is

        case DASH:
            if ((LeftPaddlePressed == HIGH) && (NextState == IDLE)) {  // going from dash to iambic mode
                NextState = DOT;
            }

            if (CurrentTime >= NextStateChangeTime) {  // at end of current dash
                LastState = DASH;  // a delay will follow the dash
                CurrentState = DELAY;
                NextStateChangeTime = CurrentTime + DitLength;
            }

            START_TONE;
            break;

        case DOT:
            if ((RightPaddlePressed == HIGH) && (NextState == IDLE)) {  // going from dot to iambic mode
                NextState = DASH;
            }
            if (CurrentTime >= NextStateChangeTime) {  // at end of current dot
                LastState = DOT;  // a delay will follow the dot
                CurrentState = DELAY;
                NextStateChangeTime = CurrentTime + DitLength;
            }

            START_TONE;
            break;

        case IDLE:   // not sending, nor finishing the delay after a dot or dash
            if ((LeftPaddlePressed == HIGH) &&
                (RightPaddlePressed == LOW)) { // only dot paddle pressed, go to DOT mode
                LastState = IDLE;
                CurrentState = DOT;
                NextStateChangeTime = CurrentTime + DitLength;

            } else if ((LeftPaddlePressed == LOW) &&
                       (RightPaddlePressed == HIGH)) { // only dash paddle pressed, go to DASH mode
                LastState = IDLE;
                CurrentState = DASH;
                NextStateChangeTime = CurrentTime + DahLength;

            } else if ((LeftPaddlePressed == HIGH) && (RightPaddlePressed == HIGH) && (NextState == IDLE)) {
                // if both paddles hit at same CurrentTime (rare, but happens)
                LastState = IDLE;
                CurrentState = DOT;
                NextState = DASH;
                NextStateChangeTime = CurrentTime + DahLength; // it is an iambic keyer, not a trochaic keyer
            }

            END_TONE;
            break;

        case DELAY:  // waiting for a dot-length delay after sending a dot or dash 
            if (CurrentTime >= NextStateChangeTime) {  // check to see if there is a next element to play
                CurrentState = NextState;
                if (CurrentState == DOT) {
                    NextStateChangeTime = CurrentTime + DitLength;

                } else if (CurrentState == DASH) {
                    NextStateChangeTime = CurrentTime + DahLength;
                }

                LastState = DELAY;
                NextState = IDLE;
            }
            // during the delay, if either paddle is pressed, save it to play after the delay
            if ((LastState == DOT) && (RightPaddlePressed == HIGH) && (NextState == NULL)) {
                NextState = DASH;

            } else if ((LastState == DASH) && (LeftPaddlePressed == HIGH) && (NextState == NULL)) {
                NextState = DOT;
            }

            END_TONE;
            break;

        default:
            END_TONE;
            break;
    }
}

void HandleStraightMorse() {
    if (PADDLE_PRESSED_LEFT || PADDLE_PRESSED_RIGHT) {
        START_TONE;
    } else {
        END_TONE;
    }
}

void setup() {
    pinMode(KEY_LOW_PIN, OUTPUT);
    digitalWrite(KEY_LOW_PIN, LOW);

    pinMode(KEY_LEFT_PIN, INPUT_PULLUP);
    digitalWrite(KEY_LEFT_PIN, HIGH);

    pinMode(KEY_RIGHT_PIN, INPUT_PULLUP);
    digitalWrite(KEY_RIGHT_PIN, HIGH);

    pinMode(POT_LOW_PIN, OUTPUT);
    digitalWrite(POT_LOW_PIN, LOW);

    pinMode(POT_HIGH_PIN, OUTPUT);
    digitalWrite(POT_HIGH_PIN, HIGH);

    digitalWrite(POT_WIPER_PIN, INPUT);

    pinMode(SPEAKER_OUT, OUTPUT);
    digitalWrite(SPEAKER_OUT, LOW);
}



void loop() {
    int ModeVal = analogRead(POT_WIPER_PIN);
    if (ModeVal >= MODE_THRESHOLD) {
        HandleStraightMorse();
    } else {
        int DitLength = ((ModeVal * (65. / 1023.) + 35)); //load CW message WPM
        int DahLength = DitLength * DIT_DAH_RATIO;
        HandleIambicMorse(DitLength, DahLength);
    }

}


   

