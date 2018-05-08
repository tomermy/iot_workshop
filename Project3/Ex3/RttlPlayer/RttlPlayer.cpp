/*
   This is the RTTL Part
   We used the code from the following link
   http://www.instructables.com/id/Aruino-Tone-RTTL-Player/
   In this assigment we also moddifed RTTL tone to support concurrence!
*/


#include "Arduino.h"
#include "RttlPlayer.h"
#include "Tone/Tone.h"

#define OCTAVE_OFFSET 0
int SPEAKER_PIN;

RttlPlayer::RttlPlayer(int pin){
  SPEAKER_PIN = pin;
}
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

void RttlPlayer::restSongVars() {
  songNextIterationTime = 0;
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

/* Modified for concurrence runnig.
   set_rtttl sets a given song as the
   song to play in the iteration function
*/
char* RttlPlayer::set_rtttl(char *p)
{
  restSongVars();
  Serial.println(p);
  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while (*p != ':') p++;   // ignore name
  p++;                     // skip ':'

  // get default duration
  if (*p == 'd')
  {
    p++; p++;              // skip "d="
    num = 0;
    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    if (num > 0) default_dur = num;
    p++;                   // skip comma
  }

  // get default octave
  if (*p == 'o')
  {
    p++; p++;              // skip "o="
    num = *p++ - '0';
    if (num >= 3 && num <= 7) default_oct = num;
    p++;                   // skip comma
  }

  // get BPM
  if (*p == 'b')
  {
    p++; p++;              // skip "b="
    num = 0;
    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++;                   // skip colon
  }

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)
  songToPlay = p;
}

/* Modified for concurrence runnig.
   playSongIteration iterate over songToPlay 
   and play it tone by tone.
*/
void RttlPlayer::playSongIteration() {
  Serial.println(songToPlay);
  num = 0;
  while (isdigit(*songToPlay))
  {
    num = (num * 10) + (*songToPlay++ - '0');
  }
  if (num) duration = wholenote / num;
  else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after
  // first, get note duration, if available
  // now get the note
  note = 0;

  switch (*songToPlay)
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
  songToPlay++;

  // now, get optional '#' sharp
  if (*songToPlay == '#')
  {
    note++;
    songToPlay++;
  }

  // now, get optional '.' dotted note
  if (*songToPlay == '.')
  {
    duration += duration / 2;
    songToPlay++;
  }

  // now, get scale
  if (isdigit(*songToPlay))
  {
    scale = *songToPlay - '0';
    songToPlay++;
  }
  else
  {
    scale = default_oct;
  }

  scale += OCTAVE_OFFSET;

  if (*songToPlay == ',')
    songToPlay++;       // skip comma for next note (or we may be at the end)

  // now play the note
  if (note)
  {
    tone(SPEAKER_PIN, notes[(scale - 4) * 12 + note]);
    toneIsUp = true;
  }
  songNextIterationTime = millis() + duration;
  Serial.println(duration);
}

// The original code
void RttlPlayer::play_rtttl(char *p)
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

  while (*p != ':') p++;   // ignore name
  p++;                     // skip ':'

  // get default duration
  if (*p == 'd')
  {
    p++; p++;              // skip "d="
    num = 0;
    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    if (num > 0) default_dur = num;
    p++;                   // skip comma
  }

  // get default octave
  if (*p == 'o')
  {
    p++; p++;              // skip "o="
    num = *p++ - '0';
    if (num >= 3 && num <= 7) default_oct = num;
    p++;                   // skip comma
  }

  // get BPM
  if (*p == 'b')
  {
    p++; p++;              // skip "b="
    num = 0;
    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++;                   // skip colon
  }

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)
  Serial.print("full song: ");Serial.println(songToPlay);
  // now begin note loop
  while (*p)
  {
    // first, get note duration, if available
    num = 0;
    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }

    if (num) duration = wholenote / num;
    else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

    // now get the note
    note = 0;

    switch (*p)
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
    if (*p == '#')
    {
      note++;
      p++;
    }

    // now, get optional '.' dotted note
    if (*p == '.')
    {
      duration += duration / 2;
      p++;
    }

    // now, get scale
    if (isdigit(*p))
    {
      scale = *p - '0';
      p++;
    }
    else
    {
      scale = default_oct;
    }

    scale += OCTAVE_OFFSET;

    if (*p == ',')
      p++;       // skip comma for next note (or we may be at the end)

    // now play the note
    if (note)
    {
      tone(SPEAKER_PIN, notes[(scale - 4) * 12 + note]);
      delay(duration);
      noTone(SPEAKER_PIN);
    }
    else
    {
      delay(duration);
    }
    Serial.println(duration);
  }
}
