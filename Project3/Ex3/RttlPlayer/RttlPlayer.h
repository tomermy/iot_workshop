/*
   This is the RTTL Part
   We used the code from the following link
   http://www.instructables.com/id/Aruino-Tone-RTTL-Player/
   In this assigment we also moddifed RTTL tone to support concurrence!
*/

#ifndef RttlPlayer_h
#define RttlPlayer_h

#include "Arduino.h"


class RttlPlayer
{
  public:
    RttlPlayer(int pin);
    void restSongVars();
    char* set_rtttl(char *p);
    void playSongIteration();
    void play_rtttl(char *p);
    bool toneIsUp;
    char* songToPlay;
    long songNextIterationTime;
};

#endif
