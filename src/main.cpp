#include "Arduino.h"
#include "config.h"

//Config Macros
#define PADDLE_PRESSED_LEFT !digitalRead(KEY_LEFT_PIN)
#define PADDLE_PRESSED_RIGHT !digitalRead(KEY_RIGHT_PIN)
#define START_TONE tone(SPEAKER_OUT, BEEP_FREQ);
#define END_TONE noTone(SPEAKER_OUT);



typedef enum class STATE {
    IDLE = 0, // doing nothing
    DASH = 1,  // playing a dash
    DOT = 2,   // playing a dot
    DELAY = 3  // in the dot-length delay between two dot/dashes
} State_t;


void HandleIambicMorse(int DitLength, int DahLength) {
    static State_t CurrentState = STATE::IDLE; // state of what the keyer output is sending right now
    static State_t NextState = STATE::IDLE; // state the keyer will go into when current element ends
    static State_t LastState = STATE::IDLE;        // previous state of the keyer
    static unsigned long NextStateChangeTime; // what CurrentTime did the current element start    sounding (in milliseconds since powerup)

    bool LeftPaddlePressed = PADDLE_PRESSED_LEFT;  // read the current values of the paddles
    bool RightPaddlePressed = PADDLE_PRESSED_RIGHT;

    unsigned long CurrentTime = millis();

    switch (CurrentState) {  // cases based on what current state is

        case STATE::DASH:
            if (LeftPaddlePressed && (NextState == STATE::IDLE)) {  // going from dash to iambic mode
                NextState = STATE::DOT;
            }

            if (CurrentTime >= NextStateChangeTime) {  // at end of current dash
                LastState = STATE::DASH;  // a delay will follow the dash
                CurrentState = STATE::DELAY;
                NextStateChangeTime = CurrentTime + DitLength;
            }

            START_TONE;
            break;

        case STATE::DOT:
            if (RightPaddlePressed && (NextState == STATE::IDLE)) {  // going from dot to iambic mode
                NextState = STATE::DASH;
            }
            if (CurrentTime >= NextStateChangeTime) {  // at end of current dot
                LastState = STATE::DOT;  // a delay will follow the dot
                CurrentState = STATE::DELAY;
                NextStateChangeTime = CurrentTime + DitLength;
            }

            START_TONE;
            break;

        case STATE::IDLE:   // not sending, nor finishing the delay after a dot or dash
            if (LeftPaddlePressed &&
                (!RightPaddlePressed)) { // only dot paddle pressed, go to DOT mode
                LastState = STATE::IDLE;
                CurrentState = STATE::DOT;
                NextStateChangeTime = CurrentTime + DitLength;

            } else if ((!LeftPaddlePressed) &&
                       RightPaddlePressed) { // only dash paddle pressed, go to DASH mode
                LastState = STATE::IDLE;
                CurrentState = STATE::DASH;
                NextStateChangeTime = CurrentTime + DahLength;

            } else if (LeftPaddlePressed && RightPaddlePressed && (NextState == STATE::IDLE)) {
                // if both paddles hit at same CurrentTime (rare, but happens)
                LastState = STATE::IDLE;
                CurrentState = STATE::DOT;
                NextState = STATE::DASH;
                NextStateChangeTime = CurrentTime + DahLength; // it is an iambic keyer, not a trochaic keyer
            }

            END_TONE;
            break;

        case STATE::DELAY:  // waiting for a dot-length delay after sending a dot or dash
            if (CurrentTime >= NextStateChangeTime) {  // check to see if there is a next element to play
                CurrentState = NextState;
                if (CurrentState == STATE::DOT) {
                    NextStateChangeTime = CurrentTime + DitLength;

                } else if (CurrentState == STATE::DASH) {
                    NextStateChangeTime = CurrentTime + DahLength;
                }

                LastState = STATE::DELAY;
                NextState = STATE::IDLE;
            }
            // during the delay, if either paddle is pressed, save it to play after the delay
            if ((LastState == STATE::DOT) && RightPaddlePressed && (NextState == STATE::IDLE)) {
                NextState = STATE::DASH;

            } else if ((LastState == STATE::DASH) && LeftPaddlePressed && (NextState == STATE::IDLE)) {
                NextState = STATE::DOT;
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
    if (ModeVal MODE_THRESHOLD) {
        HandleStraightMorse();
    } else {
        int DitLength = ((ModeVal * ((float) (MAX_WPM - MIN_WPM) / 1023.) + MIN_WPM));
        int DahLength = DitLength * DIT_DAH_RATIO;
        HandleIambicMorse(DitLength, DahLength);
    }

}


   

