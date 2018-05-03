#include <IRremoteInt.h>
#include <IRremote.h>
#include <Adafruit_NeoPixel.h>

const byte RED_LED_PIN=  12;
const byte GREEN_LED_PIN=  13;
const int BUTTON_PIN = 7;
const byte SPEAKER_PIN= 4;
const byte LED_PIN = 3;
const byte STRIP_LENGTH = 8;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, LED_PIN, NEO_GRB + NEO_KHZ800);

long led_iteration_interval = 100;
long current_time;
long start_time;

int currentState;
const int startState = 0;
const int inGameState = 1;
const int stageCompleteState = 2;
const int lossingState = 3;
const int winnigState = 4;
const int validationState = 5;
const int finalLevel = 3;

int currentLed = 0;
int toStartAGame;
int targetLed = 4;
int currentGameLevel = 1;

char* songToPlay;
long songNextIterationTime;
boolean toneIsUp= false;

uint32_t RED_COLOR = strip.Color(255, 0, 0);
uint32_t GREEN_COLOR = strip.Color(0, 255, 0);
uint32_t BLUE_COLOR = strip.Color(0, 0, 255); 
uint32_t WHITE_COLOR = strip.Color(0, 0, 0); 

// IR settings
const int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results; 

// StarWar song indicates a correct attempt
char *songStarWars = "StarWars:d=4,o=5,b=45:32p,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6";

void setup() {
        
  // put your setup code here, to run once:
    pinMode(BUTTON_PIN, INPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    strip.begin();
    strip.show();
    Serial.begin(9600);
    start_time = millis() + led_iteration_interval;
    strip.setPixelColor(targetLed, GREEN_COLOR); 
    strip.show();
    songToPlay = set_rtttl(songStarWars);
    irrecv.enableIRIn(); // Start the receiver
    currentState = startState;
}

void loop() {
  
  current_time = millis();
  toStartAGame = digitalRead(BUTTON_PIN);
  
  switch(currentState) {
    case startState:
        if( toStartAGame == HIGH){
          digitalWrite(RED_LED_PIN, LOW);  
          digitalWrite(GREEN_LED_PIN, HIGH);
          currentState = inGameState;
        }else{
          digitalWrite(RED_LED_PIN, HIGH);  
          digitalWrite(GREEN_LED_PIN, LOW);
        }
        break;
        
     case inGameState:
        // pressing any button on the remote.
        if (irrecv.decode(&results)) {
          digitalWrite(RED_LED_PIN, HIGH); 
          irrecv.resume(); // Receive the next value
          currentState = validationState;
        }
        //Song iterations
        if(current_time - songNextIterationTime > 0){
          if(toneIsUp){
            noTone(SPEAKER_PIN);
          }
          if(!(*songToPlay)){
            restSongVars();
            songToPlay = set_rtttl(songStarWars);
          }
          playSongIteration(songToPlay);  
        }
        if( abs(current_time - start_time) > led_iteration_interval){
          ledStripIteration(BLUE_COLOR);
          start_time = millis() + led_iteration_interval; 
        }              
        break;
      case validationState:
        if (targetLed == currentLed - 1 && currentGameLevel < finalLevel){
          currentGameLevel++;
          currentState = startState;
        }
        break;
  }
}


void ledStripIteration(uint32_t inputColor){
  
    if( currentLed == 0){
      strip.setPixelColor(STRIP_LENGTH -1, WHITE_COLOR);  
    }else if(currentLed - 1 == targetLed){
       strip.setPixelColor(currentLed -1, GREEN_COLOR);
    }
    else{
      strip.setPixelColor(currentLed -1, WHITE_COLOR);  
    }
    strip.show();
    strip.setPixelColor(currentLed, inputColor);  
    strip.show();
    currentLed++;
    if( currentLed > STRIP_LENGTH -1){
      currentLed = 0;
    }
}




/*
 * This is the RTTL Part
 * We used the code from the following link
 * http://www.instructables.com/id/Aruino-Tone-RTTL-Player/
 */


#define OCTAVE_OFFSET 0
// These values can also be found as constants in the Tone library (Tone.h)
int notes[] = { 0,
262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,
1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951
};
 byte default_dur = 4;
 byte default_oct = 6;
 int bpm = 63;
 int num;
 long wholenote;
 long duration;
 byte note;
 byte scale;

void restSongVars(){
 default_dur = 4;
 default_oct = 6;
 bpm = 63;
 num = 0;
 wholenote = 0;
 duration = 0;
 note = 0;
 scale = 0;
}
#define isdigit(n) (n >= '0' && n <= '9')

char* set_rtttl(char *p)
{
  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while(*p != ':') p++;    // ignore name
  p++;                     // skip ':'

  // get default duration
  if(*p == 'd')
  {
    p++; p++;              // skip "d="
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    if(num > 0) default_dur = num;
    p++;                   // skip comma
  }

  // get default octave
  if(*p == 'o')
  {
    p++; p++;              // skip "o="
    num = *p++ - '0';
    if(num >= 3 && num <=7) default_oct = num;
    p++;                   // skip comma
  }

  // get BPM
  if(*p == 'b')
  {
    p++; p++;              // skip "b="
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++;                   // skip colon
  }

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)
  num = 0;
  return p;
}

void playSongIteration(char *p){
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    if(num) duration = wholenote / num;
    else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after
    // first, get note duration, if available
    // now get the note
    note = 0;

    switch(*p)
    {
      case 'c':
        note = 1;
        break;
      case 'd':
        note = 3;
        break;
      case 'e':
        note = 5;
        break;
      case 'f':
        note = 6;
        break;
      case 'g':
        note = 8;
        break;
      case 'a':
        note = 10;
        break;
      case 'b':
        note = 12;
        break;
      case 'p':
      default:
        note = 0;
    }
    p++;

    // now, get optional '#' sharp
    if(*p == '#')
    {
      note++;
      p++;
    }

    // now, get optional '.' dotted note
    if(*p == '.')
    {
      duration += duration/2;
      p++;
    }
  
    // now, get scale
    if(isdigit(*p))
    {
      scale = *p - '0';
      p++;
    }
    else
    {
      scale = default_oct;
    }

    scale += OCTAVE_OFFSET;

    if(*p == ',')
      p++;       // skip comma for next note (or we may be at the end)

    // now play the note
    if(note)
    {
      tone(SPEAKER_PIN, notes[(scale - 4) * 12 + note]);
      toneIsUp = true;
    }
    songNextIterationTime = millis() + duration;
    songToPlay = p++;
}

