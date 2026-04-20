#include "melody.h"
#include "pitches.h"
#include "pins.h"
#include "Arduino.h"

int melody[] = {
    NOTE_E5, NOTE_D5, NOTE_FS4, NOTE_GS4,
    NOTE_CS5, NOTE_B4, NOTE_D4, NOTE_E4,
    NOTE_B4, NOTE_A4, NOTE_CS4, NOTE_E4,
    NOTE_A4};

int durations[] = {
    8, 8, 4, 4,
    8, 8, 4, 4,
    8, 8, 4, 4,
    2};

static const int MELODY_NOTE_COUNT = sizeof(durations) / sizeof(durations[0]);
static int melodyIndex = 0;
static bool melodyPlaying = false;
static unsigned long nextNoteAtMs = 0;

void playMelody()
{
    melodyIndex = 0;
    melodyPlaying = true;
    nextNoteAtMs = 0;
}

void stopMelody()
{
    melodyPlaying = false;
    melodyIndex = 0;
    noTone(BUZZER);
}

bool isMelodyPlaying()
{
    return melodyPlaying;
}

void updateMelody()
{
    if (!melodyPlaying)
    {
        return;
    }

    unsigned long nowMs = millis();
    if (nextNoteAtMs != 0 && nowMs < nextNoteAtMs)
    {
        return;
    }

    if (melodyIndex >= MELODY_NOTE_COUNT)
    {
        stopMelody();
        return;
    }

    int durationMs = 1000 / durations[melodyIndex];
    tone(BUZZER, melody[melodyIndex], durationMs);

    int pauseBetweenNotesMs = (durationMs * 13) / 10;
    nextNoteAtMs = nowMs + pauseBetweenNotesMs;
    melodyIndex++;
}
