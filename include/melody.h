#ifndef MELODY_H
#define MELODY_H

#include "pins.h"

#include "Arduino.h"
#include "pitches.h"

extern int melody[];
extern int durations[];
// Starts melody playback without blocking.
void playMelody();
// Advances melody state machine; call frequently from loop.
void updateMelody();
bool isMelodyPlaying();
void stopMelody();

#endif // MELODY_H