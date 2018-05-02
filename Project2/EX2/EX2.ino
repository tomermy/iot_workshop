/*
                The Secret Knock Detecting Box
        
The knocking box embedded a piezo sensor for knocking detection. 
The Idea behind it is to improve piezo sensitivity and create a more reliable sensor.
The Arduino uses the knocking box as a sensor to record a knocking pattern. 
The user can set a recording knocking pattern as a locking code by pressing the program mode button.
Once the user press the program button, the device will play a ringtone and turn on the yellow LED.
The device starts to record the user secret sequence right after the tone and until yellow LED is off.
To enter a secret code attempt to the device, the user has to make a trigger knock.
Once such a trigger established, the device plays a code attempt ringtone, and all LEDs are on.
The device stops to record the attempt once all LEDs are off. 
If the attempt succeeded, a Starwar ringtone is played and turn on the green LED. 
Otherwise, a failure ringtone is played and turn on the red LED.

* The device uses EEPROM to store the secret code,
 by that, the code does not change when the device is off.

The Circuit:

  * Input: 
      Button - 6 - turn into program mode. 
      Piezo - A0 - knock sensor.
      
  * Output:
      Speaker - 9 - play ringtones to indicate user.
      Red Led - 13 
      Yello Led - 12
      Green Led - 11
     
Video link: https://youtu.be/RMhu9U0CG5A


Created By:
  Gilad_Ram #305433260 
  Tomer_Maimon #308301498
  Alon_Shprung #203349048
*/ 


#include <EEPROM.h>

// constants
const byte RED_LED_PIN=  13;
const byte YELLOW_LED_PIN=  12;
const byte GREEN_LED_PIN=  11;
const byte PIEZO_SENSOR_PIN= A0;
const byte SPEAKER_PIN= 4;
const int BUTTON_PIN = 6;
// logic constants
const int rejectValue = 37;        // The max diffrenet value between secret knock and input knock
const int averageRejectValue = 25; // The max average error
const int knockFadeTime = 150;     // Debounce timer between knocks.
const byte INTERVAL_NORMALIZATION_BOUND = 200;
const int MAX_KNOCKS = 30;
const int THRESHOLD = 45;
const int KNOCK_SAMPLE_INTERVAL = 2000;

// Knock variables 
byte knockKnockCode[MAX_KNOCKS] = {0, 0, 0, 0, 0}; // Initial with no code... Red Light is ON.
int knockReading[MAX_KNOCKS];
boolean inProgramMode = false;
int pastSoundValue=0;
int currentSoundValue;
int toProgramModebuttonState;
int diffBetweenSensorValues; 

// RTTL 
char *songATeam = "A-Team:d=8,o=5,b=125:4d#6,a#,2d#6,16p,g#,4a#,4d#";
char *songStarWars = "StarWars:d=4,o=5,b=45:32p,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6";
char *songGadget = "Gadget:d=16,o=5,b=50:32d#,32f,32f#,32g#,a#,f#";
char *songZelda = "shinobi:d=4,o=5,b=140:b,f#6,d6,b,g,f#,e,2f#.,a,1f#";

// Leds initial state 
int redLedState = LOW;             
int yellowLedState = LOW;             
int greenLedState = LOW;             


void setup() {
  
  // set the digital pin as output:
  pinMode(PIEZO_SENSOR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  
  // load date from memory
  loadDataFromMemory();
  Serial.begin(9600);
}


void loop() {

  currentSoundValue  = analogRead(PIEZO_SENSOR_PIN);
  toProgramModebuttonState = digitalRead(BUTTON_PIN);
  diffBetweenSensorValues = currentSoundValue - pastSoundValue;
  
  if (diffBetweenSensorValues > THRESHOLD){
    // A trigger knock accrued 
    play_rtttl(songGadget);
    unlockAttempt();
  }
  
  pastSoundValue= currentSoundValue;
  
  if( toProgramModebuttonState == HIGH){
    // Program a new secret code
    digitalWrite(YELLOW_LED_PIN, HIGH);
    play_rtttl(songATeam);
    programNewCode();
  }
}


void resetListeningArray(){
  Serial.println("reset the listening array."); 
  int i = 0;
  // First lets reset the listening array.
  for (i=0;i<MAX_KNOCKS;i++){
    knockReading[i]=0;
  }
}


void printListeningArray(){
  Serial.println("printing the listening array."); 
  int i = 0;
  // First lets reset the listening array.
  for (i=0;i<MAX_KNOCKS;i++){
    Serial.println(knockKnockCode[i]);
  }
}


void programNewCode(){
  listenToSecretKnock();
  saveNewCodeData();
  // and we blink the green and red alternately to show that program is complete.
  Serial.println("New code stored.");
  printListeningArray();
  digitalWrite(YELLOW_LED_PIN, LOW);
  inProgramMode = false;
}


void unlockAttempt(){
  setAllLeds(HIGH);
  listenToSecretKnock();
  setAllLeds(LOW);
  if (validateKnock() == true){
      // The Knock code is valid!!!
      digitalWrite(GREEN_LED_PIN, HIGH);
      Serial.println("Secret knock succeeded.");
      play_rtttl(songStarWars);
      digitalWrite(GREEN_LED_PIN, LOW);
      
    }else{
      // The Knock code is not valid!!!
      digitalWrite(RED_LED_PIN, HIGH);
      Serial.println("Secret knock failed.");
      play_rtttl(songZelda);
      digitalWrite(RED_LED_PIN, LOW);
    }
}

// Set all leds to the given value
void setAllLeds(int value){
  digitalWrite(RED_LED_PIN, value);
  digitalWrite(YELLOW_LED_PIN, value);
  digitalWrite(GREEN_LED_PIN, value);
}

void listenToSecretKnock(){
  Serial.println("listen to secret knock starting...");  
  int currentNumberOfKnocks=0;               // Incrementer for the array.
  int startTime=millis();                 // Reference for when this knock started.
  int now=millis();
  int timePassed = now - startTime;
  
  resetListeningArray();

  delay(knockFadeTime);                                 // wait for this peak to fade before we listen to the next one.
  do {
    //listen for the next knock or wait for it to timeout. 
    currentSoundValue = analogRead(PIEZO_SENSOR_PIN);
    diffBetweenSensorValues = currentSoundValue - pastSoundValue;
    if (diffBetweenSensorValues >= THRESHOLD){ 
      // A Knock 
      //record the delay time.
      Serial.println(diffBetweenSensorValues);
      Serial.println("knock.");
      now=millis();
      knockReading[currentNumberOfKnocks] = now-startTime;
      currentNumberOfKnocks ++;                             //increment the counter
      startTime=now;          
      // and reset our timer for the next knock
      delay(knockFadeTime);                              // again, a little delay to let the knock decay.
    }
    pastSoundValue = currentSoundValue;
    now=millis();
    timePassed = now -startTime; 
  } while ((timePassed < KNOCK_SAMPLE_INTERVAL) 
              && (currentNumberOfKnocks < MAX_KNOCKS));
}

void saveNewCodeData(){
  Serial.println("Storing new code.");
  int maxInterval = getMaxKnockInterval();
  int i = 0;
  for (i=0;i<MAX_KNOCKS;i++){ 
        // normalize the times
        knockKnockCode[i]= map(knockReading[i],0, maxInterval, 0, INTERVAL_NORMALIZATION_BOUND);
        EEPROM.write(i, knockKnockCode[i]);
      }
}

void loadDataFromMemory(){
  Serial.println("Loading code from memory...");
  int i = 0;
  for (i=0;i<MAX_KNOCKS;i++){ 
        knockKnockCode[i]= EEPROM.read(i);
      }
}

int getMaxKnockInterval(){
  int maxKnockInterval = 0;
  int currentInterval = 0; 
  int i=0;
  for (i=0; i< MAX_KNOCKS; i++){ 
    // normalize the times
    currentInterval = knockReading[i];
    if (knockReading[i] > maxKnockInterval){ 
        maxKnockInterval = knockReading[i];
    }
  }
  return maxKnockInterval;
}

/* Compare the reading knock to the secret knock code.
   The comparison of the knocks is calculated with an error rate.
   returns true - valid code
           false - invalid code.
*/      
boolean validateKnock(){
  int secretKnockCount = compareLengths();
  if(secretKnockCount == -1){
    // There is a diffrence in the length of the codes
    return false;
  }
  // Once the length is the same we can check the content.
  int i;
  int totaltimeDifferences=0;
  int timeDiff=0;
  int maxKnockInterval = getMaxKnockInterval();
  
  for (i=0;i<MAX_KNOCKS;i++){ 
    // normalize the times
    knockReading[i]= map(knockReading[i],0, maxKnockInterval, 0, INTERVAL_NORMALIZATION_BOUND);      
    timeDiff = abs(knockReading[i]-knockKnockCode[i]);
    if (timeDiff > rejectValue){ 
      Serial.println(timeDiff);
      // a value diff is to big
      return false;
    }
    totaltimeDifferences += timeDiff;
  }
  int totalErrorAverage = totaltimeDifferences/secretKnockCount;
  Serial.print("The total error Average is: "); Serial.println(totalErrorAverage);
  if (totalErrorAverage > averageRejectValue){
    return false; 
  }
  return true;
}

int compareLengths(){
   
  // simply compares the length of the codes
  int currentKnockCount = 0;
  int secretKnockCount = 0;
  int maxKnockInterval = 0;                // We use this later to normalize the times.
  int i=0;
  
  for (i=0;i<MAX_KNOCKS;i++){
    if (knockReading[i] > 0){
      currentKnockCount++;
    }
    if (knockKnockCode[i] > 0){           //todo: precalculate this.
      secretKnockCount++;
    }
  }
  
  if (currentKnockCount != secretKnockCount){
    Serial.println("The length is not equale");
    return -1; 
  }else{
    Serial.println("The length is equale");
    return secretKnockCount;    
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

#define isdigit(n) (n >= '0' && n <= '9')

void play_rtttl(char *p)
{
  // Absolutely no error checking in here

  byte default_dur = 4;
  byte default_oct = 6;
  int bpm = 63;
  int num;
  long wholenote;
  long duration;
  byte note;
  byte scale;

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

  // now begin note loop
  while(*p)
  {
    // first, get note duration, if available
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    
    if(num) duration = wholenote / num;
    else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

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
      delay(duration);
      noTone(SPEAKER_PIN);
    }
    else
    {
      delay(duration);
    }
  }
}

