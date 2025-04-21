/**
 * @file main.c
 * @author Maarten Janssen / DhrBaksteen, SuperIlu
 * @brief lib16 port of https://github.com/DhrBaksteen/ArduinoOPL2/blob/master/examples_pi/demotune/demotune.cpp
 *
 * @copyright Copyright (c) 2021 Maarten Janssen / DhrBaksteen
 */
#include <stddef.h>
#include <stdlib.h>
#include <dos.h>
#include <stdio.h>
#include <strings.h>
#include <time.h>

#include "lib16.h"
#include "midi_instruments.h"

const int noteDefs[21] = {OPL2_NOTE_A,  OPL2_NOTE_GS, OPL2_NOTE_AS, OPL2_NOTE_B, OPL2_NOTE_AS, OPL2_NOTE_B, OPL2_NOTE_C,  OPL2_NOTE_C, OPL2_NOTE_CS, OPL2_NOTE_D, OPL2_NOTE_CS,
                          OPL2_NOTE_DS, OPL2_NOTE_E,  OPL2_NOTE_DS, OPL2_NOTE_F, OPL2_NOTE_F,  OPL2_NOTE_E, OPL2_NOTE_FS, OPL2_NOTE_G, OPL2_NOTE_FS, OPL2_NOTE_GS};

float tempo;

typedef struct __Tune {
    const char *data;
    unsigned int channel;
    unsigned int octave;
    float noteDuration;
    float noteLength;
    unsigned int nextNoteTime;
    unsigned int releaseTime;
    unsigned int index;
} Tune;

const char tuneData[3][200] = {
    "t150m200o5l8egredgrdcerc<b>er<ba>a<a>agdefefedr4.regredgrdcerc<b>er<ba>a<a>agdedcr4.c<g>cea>cr<ag>cr<gfarfearedgrdcfrc<bagab>cdfegredgrdcerc<b>er<ba>a<a>agdedcr4.cro3c2",
    "m85o3l8crer<br>dr<ar>cr<grbrfr>cr<grbr>crer<gb>dgcrer<br>dr<ar>cr<grbrfr>cr<grbr>ceger4.rfrafergedrfdcrec<br>d<bar>c<agrgd<gr4.o4crer<br>dr<ar>cr<grbrfr>cr<grbr>cege",
    "m85o3l8r4gr4.gr4.er4.err4fr4.gr4.gr4.grr4gr4.er4.er4.frr4gr4>ccr4ccr4<aarraar4ggr4ffr4.ro4gab>dr4.r<gr4.gr4.err4er4.fr4.g"};

Tune music[3];

float parseNumber(Tune *tune) {
    float number = 0.0f;
    if (tune->data[tune->index] != 0 && tune->data[tune->index] >= '0' && tune->data[tune->index] <= '9') {
        while (tune->data[tune->index] != 0 && tune->data[tune->index] >= '0' && tune->data[tune->index] <= '9') {
            number = number * 10 + (tune->data[tune->index++] - 48);
        }
        tune->index--;
    }
    return number;
}

float parseDuration(Tune *tune) {
    // Get custom note duration or use default note duration.
    float duration = parseNumber(tune);
    if (duration == 0) {
        duration = 4.0f / tune->noteDuration;
    } else {
        duration = 4.0f / duration;
    }

    // See whether we need to double the duration
    if (tune->data[++tune->index] == '.') {
        duration *= 1.5f;
    } else {
        tune->index--;
    }

    // Calculate note duration in ms.
    duration = (60.0f / tempo) * duration * 1000;
    return duration;
}

void parseNote(Tune *tune) {
    float duration;

    // Get index of note in base frequenct table.
    int note = (tune->data[tune->index++] - 97) * 3;
    if (tune->data[tune->index] == '-') {
        note++;
        tune->index++;
    } else if (tune->data[tune->index] == '+') {
        note += 2;
        tune->index++;
    }

    // Get duration, set delay and play note.
    duration = parseDuration(tune);
    tune->nextNoteTime = clock() + duration;
    tune->releaseTime = clock() + (duration * tune->noteLength);
    opl2_playNote(tune->channel, tune->octave, noteDefs[note]);
}

void parseTune(Tune *tune) {
    while (tune->data[tune->index] != 0) {
        // Decrease octave if greater than 1.
        if (tune->data[tune->index] == '<' && tune->octave > 1) {
            tune->octave--;

            // Increase octave if less than 7.
        } else if (tune->data[tune->index] == '>' && tune->octave < 7) {
            tune->octave++;

            // Set octave.
        } else if (tune->data[tune->index] == 'o' && tune->data[tune->index + 1] >= '1' && tune->data[tune->index + 1] <= '7') {
            tune->octave = tune->data[++tune->index] - 48;

            // Set default note duration.
        } else if (tune->data[tune->index] == 'l') {
            float duration;
            tune->index++;
            duration = parseNumber(tune);
            if (duration != 0) tune->noteDuration = duration;

            // Set note length in percent.
        } else if (tune->data[tune->index] == 'm') {
            tune->index++;
            tune->noteLength = parseNumber(tune) / 100.0;

            // Set song tempo.
        } else if (tune->data[tune->index] == 't') {
            tune->index++;
            tempo = parseNumber(tune);

            // Pause.
        } else if (tune->data[tune->index] == 'p' || tune->data[tune->index] == 'r') {
            tune->index++;
            tune->nextNoteTime = clock() + parseDuration(tune);
            break;

            // Next character is a note A..G so play it.
        } else if (tune->data[tune->index] >= 'a' && tune->data[tune->index] <= 'g') {
            parseNote(tune);
            break;
        }

        tune->index++;
    }
}

int main(int argc, char *argv[]) {
    instrument_t piano;
    int i;
    int hasData = 1;

    if (!opl2_init()) {
        puts("No sound");
        exit(1);
    }
    puts("AdLib found");

    tempo = 120.0f;

    // Initialize 3 channels of the tune.
    for (i = 0; i < 3; i++) {
        music[i].data = tuneData[i];
        music[i].channel = i;
        music[i].octave = 4;
        music[i].noteDuration = 4.0f;
        music[i].noteLength = 0.85f;
        music[i].releaseTime = 0;
        music[i].nextNoteTime = 0;
        music[i].index = 0;
    }

    // Setup channels 0, 1 and 2 instruments.
    opl2_loadInstrument(INSTRUMENT_PIANO1, &piano);
    for (i = 0; i < 3; i++) {
        opl2_setInstrument(i, &piano, 1.0);
    }

    while (hasData) {
        hasData = 0;
        for (i = 0; i < 3; i++) {
            if (clock() >= music[i].releaseTime && opl2_getKeyOn(music[i].channel)) {
                opl2_setKeyOn(music[i].channel, false);
            }
            if (clock() >= music[i].nextNoteTime && music[i].data[music[i].index] != 0) {
                parseTune(&music[i]);
            }
            hasData += music[i].data[music[i].index];
        }
        delay(1);
    }

    exit(0);
}
