/*
Theramin - ElctroPhysical Instrument

This is a an electric instrument, which plays a note
on a range between C4 to B4 according to a light sensor input.
In addition, the note will be vibrated according to Two potentiometers input.
One controls the vibrato's depth, the other controls the speed (delay between changes).

The Circuit:
  * Input: 
      Potentiometer - A0 - Depth of vibrato. 
      Potentiometer - A2 - Speed of vibrato.
      LightSensor - A4 - Map light intesity to specifc note.
  * Output:
      Speaker - 9 - plays the note with vibrato.

Video link: #LINK#

Created By:
  Gilad_Ram #305433260 
  Tomer_Maimon #308301498
  Alon_Shprung #203349048
*/ 

int vibratoDepthInput;
int vibratoSpeedInput;
double noteChangeValue;

// notes to play from C4 to B4
double notes[] = {261.63,277.18, 293.66, 311.13, 329.63,
                      349.23, 369.99, 392.0, 415.3, 440.0, 466.16, 493.88};
void setup() {
  // constant depth delta between iterations
  noteChangeValue = 0.5;
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

void loop() {                    
  // Read light sensor input and map to a note
  int lightSensorInput = analogRead(A4);
  int mappedNoteIndex = map(lightSensorInput, 0, 1023, 0, 12);
  double mappedNoteValue = notes[mappedNoteIndex];
  double currentNote = mappedNoteValue;

  // Vibrato loops as long as playing the same note
  while (mappedNoteIndex == map(analogRead(A4), 0, 1023, 0, 12)){
    double vibratoMaxDepth = currentNote * 0.1;
    // note may go up and down up to half of the total vibrato depth
    double vibratoChangeLimit = vibratoMaxDepth * 0.5;

    // Change vibrato depth and speed based on live sensor input
    vibratoDepthInput = analogRead(A0);
    vibratoSpeedInput = analogRead(A2);  
    int mappedDelaySpeed = map(vibratoSpeedInput, 0, 1023, 1, 50);
    int currentVibratoDepthLimit = map(vibratoDepthInput, 0, 1023, 0, vibratoChangeLimit);

    // prevent changes to the original note when vibrato depth sensor input is zero
    if (currentVibratoDepthLimit != 0) {
      currentNote = currentNote + noteChangeValue;
    }

    // up and down movement for the vibrato based on depth input
    // change direction when reaching limits
    if (currentNote <=  mappedNoteValue - currentVibratoDepthLimit 
          || currentNote >= mappedNoteValue + currentVibratoDepthLimit) {
       noteChangeValue = -noteChangeValue; 
    }
    
    tone(9, currentNote); // play sound
    
    // delay in between reads for stability based on speed input
    delay(mappedDelaySpeed);  
  }
}
