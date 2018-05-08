/*
                The Lights Game - The Runner
We choose to implement the light runner. The game is a one player game.
The idea behind the game is to stop the runner on the target, in our
case the blue led on the green led.The user can stop the runner by pressing
any button on the remote. When the runner stays on the target,a winning level
song is played, and all LEDs turn green.
Otherwise, a losing level song is played, and all LEDs turn red.
To win the game, the user must win all three levels. 
Where, as levels escalate the running pace increases as well.

* To implement a concurrence running we modified the tone and rttlplay libraries.
* In the tone library we chnged the callback pin id, to enable IR and tone to work  
* at the same sketch.
* In the rttlplay (The one we used the last exercise as well), we have modified an iteration rttl player
* to enable playing a song while the user plays the game.

The Circuit:

  * Input: 
      Button - 7 - Start a game. 
      IRReciverSensor - 11.
      
  * Output:
      Speaker - 9 - play ringtones to indicate user level and state.
      Red Led - 13 
      Yello Led - 12
      Green Led - 11
      Leds strip - 3
     
Video link: https://www.youtube.com/watch?v=7J3qjh4jp44&feature=youtu.be

Created By:
  Tomer_Maimon #308301498
  Alon_Shprung #203349048
  Gilad_Ram #305433260 
*/ 

#include <RttlPlayer.h>
#include <IRremoteInt.h>
#include <IRremote.h>
#include <Adafruit_NeoPixel.h>

// CONST GPIO PINS
const byte RED_LED_PIN =  12;
const byte GREEN_LED_PIN =  13;
const int BUTTON_PIN = 7;
const byte SPEAKER_PIN = 4;
const byte LED_PIN = 3;
const byte STRIP_LENGTH = 8;
const int RECV_PIN = 11;
const long winnig_ledIterationInterval = 50;

//Setting a strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, LED_PIN, NEO_GRB + NEO_KHZ800);
RttlPlayer player = RttlPlayer(SPEAKER_PIN);
// Color Consts
const uint32_t RED_COLOR = strip.Color(255, 0, 0);
const uint32_t GREEN_COLOR = strip.Color(0, 255, 0);
const uint32_t BLUE_COLOR = strip.Color(0, 0, 255);
const uint32_t WHITE_COLOR = strip.Color(0, 0, 0);

// Time vars for concurrence running
long currentTime;
long startTime;

int const levelIntervalMultipler = 50;
long ledIterationInterval = 200;

// The Game's States
int currentState;
const int startState = 0;
const int inGameState = 1;
const int stageCompleteState = 2;
const int lossingState = 3;
const int winnigState = 4;
const int winnigGame = 6;
const int validationState = 5;

// Game Level settings
const int finalLevel = 2;
int currentGameLevel = 0;
// Start a game button variable
int toStartAGame;

// Game Led vars
int currentLed = 0;
const int targetLed = 4;

// IR settings
const int REMOTE_NOISE_NUMEBR = 4294967295;
IRrecv irrecv(RECV_PIN);
decode_results results;


// in game song
char *songStarWars = "StarWars:d=4,o=5,b=45:32p,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6";
// losing a game song
char *losingSong = "lose:d=4,o=5,b=100:c.,c,8c,c.,d#,8d,d,8c,c,8c,2c.";
// winning a level song
char *winnigStateSong = "Agadoo:d=4,o=5,b=125:8b,8g#,e,8e,8e,e,8e,8e,8e,8e,8d#,8e,f#,8a,8f#,d#,8d#,8d#,d#,8d#,8d#,8d#,8d#,8c#,8d#,e";
// winnig a game song
char *winnigGameSong = "Macarena:d=4,o=5,b=180:f,8f,8f,f,8f,8f,8f,8f,8f,8f,8f,8a,8c,8c,f,8f,8f,f,8f,8f,8f,8f,8f,8f,8d,8c,p,f,8f,8f,f,8f,8f,8f,8f,8f,8f,8f,8a,p,2c.6,a,8c6,8a,8f,p,2p";

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  strip.begin();
  strip.show();
  Serial.begin(9600);
  startTime = millis() + ledIterationInterval;
  strip.show();
  player.set_rtttl(songStarWars);
  currentState = startState;
}

void loop() {

  currentTime = millis();
  toStartAGame = digitalRead(BUTTON_PIN);
  // State Machine 
  switch (currentState) {
    case startState:
      startStateFunc();
      break;
    case inGameState:
      inGameStateFunc();
      break;
    case validationState:
      validatioStateFunc();
      break;
    case lossingState:
      losingStateFunc();
      break;
    case winnigState:
      winnigStateFunc();
      break;
    case winnigGame:
      winnigGameStateFunc();
      break;
  }
}

/*
  only Red led is on indicates first state 
  waiting for the user to press the start game
  button
*/ 
void startStateFunc() {
  // Press the button to start the game
  if (toStartAGame == HIGH) {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
    strip.setPixelColor(targetLed, GREEN_COLOR);
    currentState = inGameState;
    irrecv.enableIRIn(); // Start the receiver
  } 
  else if(digitalRead(RED_LED_PIN) != HIGH ||
          digitalRead(GREEN_LED_PIN) != LOW){
            
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
  }
}

/*
  Play the game music (star wars) and run 
  the runner.
  Wait for the user to press any key on the remote.
*/ 
void inGameStateFunc() {
  // pressing any button on the remote.
  if (irrecv.decode(&results)) {
    Serial.println(results.value);
    if (results.value != REMOTE_NOISE_NUMEBR) {
      digitalWrite(RED_LED_PIN, HIGH);
      irrecv.resume(); // Receive the next value
      currentState = validationState;
      noTone(SPEAKER_PIN);
      return;
    }
    irrecv.resume(); // Receive the next value
  }
  //Song iterations
  if (currentTime > player.songNextIterationTime) {
    if (player.toneIsUp) {
      noTone(SPEAKER_PIN);
    }
    if (!(*player.songToPlay)) {
      player.set_rtttl(songStarWars);
    }
    player.playSongIteration();
  }
  if (currentTime > startTime) {
    ledStripIteration(BLUE_COLOR);
    startTime = millis() + ledIterationInterval - (currentGameLevel * levelIntervalMultipler);
  }
}

/*
  Validate where the user stoped the runner
  and sets next state. 
*/ 
void validatioStateFunc() {
  if (targetLed == currentLed - 1 && currentGameLevel <= finalLevel) {
    if (currentGameLevel == finalLevel) {
      // user won last level
      // won the game!!!
      currentState = winnigGame;
      player.set_rtttl(winnigGameSong);
      return;
    } else {
      currentState = winnigState;
    }
  } else {
    currentState = lossingState;
  }
}

/*
  Play lossing level song and turn strip of leds to red. 
*/ 
void losingStateFunc() {
  setAllLedStripToColor(RED_COLOR);
  player.play_rtttl(losingSong);
  setAllLedStripToColor(WHITE_COLOR);
  currentGameLevel = 0;
  currentState = startState;
}

/*
  Play winnig level song and turn strip of leds to green. 
*/ 
void winnigStateFunc() {
  setAllLedStripToColor(GREEN_COLOR);
  player.play_rtttl(winnigStateSong);
  setAllLedStripToColor(WHITE_COLOR);
  currentState = inGameState;
  currentGameLevel++;
}

/*
  Play winnig game song and at the same time
  run all leds green.
  set state to start state.
*/ 
void winnigGameStateFunc() {
  //Song iterations
  currentTime = millis();
  if (currentTime > player.songNextIterationTime) {
    if (player.toneIsUp) {
      noTone(SPEAKER_PIN);
    }
    if (!(*player.songToPlay)) {
      currentGameLevel = 0;
      currentState = startState;
      setAllLedStripToColor(GREEN_COLOR);
      delay(1000);
      setAllLedStripToColor(WHITE_COLOR);
      return;
    }
    player.playSongIteration();
  }
  // give priorty to song iteration
  if (currentTime < player.songNextIterationTime && currentTime >  startTime)
  {
    ledStripIteration(GREEN_COLOR);
    startTime = millis() + winnig_ledIterationInterval;
  }
}

/*
  This is an iterator function.
  The function moves turn the next led on 
  and past led off.
  We used it to implement the runner.
*/ 
void ledStripIteration(uint32_t inputColor) {

  if ( currentLed == 0) {
    strip.setPixelColor(STRIP_LENGTH - 1, WHITE_COLOR);
  } else if (currentLed - 1 == targetLed) {
    strip.setPixelColor(currentLed - 1, GREEN_COLOR);
  }
  else {
    strip.setPixelColor(currentLed - 1, WHITE_COLOR);
  }
  strip.show();
  strip.setPixelColor(currentLed, inputColor);
  strip.show();
  currentLed++;
  if ( currentLed > STRIP_LENGTH - 1) {
    currentLed = 0;
  }
}

/*
  Set all strip to a given color
*/ 
void setAllLedStripToColor(uint32_t inputColor) {
  for ( int i = 0; i < STRIP_LENGTH; i++) {
    strip.setPixelColor(i, inputColor);
    strip.show();
  }
}


