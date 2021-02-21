/**
 * OPL2 Audio Lib for Arduino & Pi
 *
 * YM3812 OPL2 Audio Library for Arduino, Raspberry Pi and Orange Pi v2.1.0
 * Code by Maarten Janssen (maarten@cheerful.nl) 2016-12-18
 * WWW.CHEERFUL.NL
 *
 * [...]
 *
 * Last updated 2021-01-23
 * Most recent version of the library can be found at my GitHub: https://github.com/DhrBaksteen/ArduinoOPL2
 * Details about the YM3812 and YMF262 chips can be found at http://www.shikadi.net/moddingwiki/OPL_chip
 *
 * This library is open source and provided as is under the MIT software license, a copy of which is provided as part of
 * the project's repository. This library makes use of Gordon Henderson's Wiring Pi.
 * WWW.CHEERFUL.NL
 */
#ifndef __OPL2_H_
#define __OPL2_H_

#include <stdbool.h>
#include <stdint.h>

/* ======================================================================
** defines
** ====================================================================== */
// Generic OPL2 definitions.
#define OPL2_NUM_CHANNELS 9
#define OPL2_CHANNELS_PER_BANK 9

// Operator definitions.
#define OPL2_OPERATOR1 0
#define OPL2_OPERATOR2 1
#define OPL2_MODULATOR 0
#define OPL2_CARRIER 1

// Synthesis type definitions.
#define OPL2_SYNTH_MODE_FM 0
#define OPL2_SYNTH_MODE_AM 1

// Drum sounds.
#define OPL2_DRUM_BASS 0
#define OPL2_DRUM_SNARE 1
#define OPL2_DRUM_TOM 2
#define OPL2_DRUM_CYMBAL 3
#define OPL2_DRUM_HI_HAT 4

// Drum sound bit masks.
#define OPL2_DRUM_BITS_BASS 0x10
#define OPL2_DRUM_BITS_SNARE 0x08
#define OPL2_DRUM_BITS_TOM 0x04
#define OPL2_DRUM_BITS_CYMBAL 0x02
#define OPL2_DRUM_BITS_HI_HAT 0x01

// Note to frequency mapping.
#define OPL2_NOTE_C 0
#define OPL2_NOTE_CS 1
#define OPL2_NOTE_D 2
#define OPL2_NOTE_DS 3
#define OPL2_NOTE_E 4
#define OPL2_NOTE_F 5
#define OPL2_NOTE_FS 6
#define OPL2_NOTE_G 7
#define OPL2_NOTE_GS 8
#define OPL2_NOTE_A 9
#define OPL2_NOTE_AS 10
#define OPL2_NOTE_B 11

// Tune specific declarations.
#define OPL2_NUM_OCTAVES 7
#define OPL2_NUM_NOTES 12
#define OPL2_NUM_DRUM_SOUNDS 5

/* ======================================================================
** typedefs
** ====================================================================== */
typedef struct __operator {
    bool hasTremolo;
    bool hasVibrato;
    bool hasSustain;
    bool hasEnvelopeScaling;
    uint8_t frequencyMultiplier;
    uint8_t keyScaleLevel;
    uint8_t outputLevel;
    uint8_t attack;
    uint8_t decay;
    uint8_t sustain;
    uint8_t release;
    uint8_t waveForm;
} operator_t;

typedef struct __instrument {
    operator_t operators[2];
    uint8_t feedback;
    bool isAdditiveSynth;
    uint8_t transpose;
} instrument_t;

/* ======================================================================
** prototypes
** ====================================================================== */
extern bool opl2_getDeepTremolo();
extern bool opl2_getDeepVibrato();
extern bool opl2_getEnvelopeScaling(uint8_t channel, uint8_t operatorNum);
extern bool opl2_getKeyOn(uint8_t channel);
extern bool opl2_getMaintainSustain(uint8_t channel, uint8_t operatorNum);
extern bool opl2_getNoteSelect();
extern bool opl2_getPercussion();
extern bool opl2_getTremolo(uint8_t channel, uint8_t operatorNum);
extern bool opl2_getVibrato(uint8_t channel, uint8_t operatorNum);
extern bool opl2_getWaveFormSelect();
extern bool opl2_init();
extern float opl2_getFrequency(uint8_t channel);
extern float opl2_getFrequencyStep(uint8_t channel);
extern uint16_t opl2_getFNumber(uint8_t channel);
extern uint16_t opl2_getFrequencyFNumber(uint8_t channel, float frequency);
extern uint16_t opl2_getNoteFNumber(uint8_t note);
extern uint8_t opl2_getAttack(uint8_t channel, uint8_t operatorNum);
extern uint8_t opl2_getBlock(uint8_t channel);
extern uint8_t opl2_getChannelVolume(uint8_t channel);
extern uint8_t opl2_getDecay(uint8_t channel, uint8_t operatorNum);
extern uint8_t opl2_getDrums();
extern uint8_t opl2_getFeedback(uint8_t channel);
extern uint8_t opl2_getFrequencyBlock(float frequency);
extern uint8_t opl2_getMultiplier(uint8_t channel, uint8_t operatorNum);
extern uint8_t opl2_getRelease(uint8_t channel, uint8_t operatorNum);
extern uint8_t opl2_getScalingLevel(uint8_t channel, uint8_t operatorNum);
extern uint8_t opl2_getSustain(uint8_t channel, uint8_t operatorNum);
extern uint8_t opl2_getSynthMode(uint8_t channel);
extern uint8_t opl2_getVolume(uint8_t channel, uint8_t operatorNum);
extern uint8_t opl2_getWaveForm(uint8_t channel, uint8_t operatorNum);
extern void opl2_createInstrument(instrument_t *instrument);
extern void opl2_getInstrument(uint8_t channel, instrument_t *instrument);
extern void opl2_loadInstrument(const unsigned char *data, instrument_t *instrument);
extern void opl2_playDrum(uint8_t drum, uint8_t octave, uint8_t note);
extern void opl2_playNote(uint8_t channel, uint8_t octave, uint8_t note);
extern void opl2_reset();
extern void opl2_setAttack(uint8_t channel, uint8_t operatorNum, uint8_t attack);
extern void opl2_setBlock(uint8_t channel, uint8_t block);
extern void opl2_setChannelVolume(uint8_t channel, uint8_t volume);
extern void opl2_setDecay(uint8_t channel, uint8_t operatorNum, uint8_t decay);
extern void opl2_setDeepTremolo(bool enable);
extern void opl2_setDeepVibrato(bool enable);
extern void opl2_setDrumInstrument(instrument_t *instrument, uint8_t drumType, float volume);
extern void opl2_setDrums(bool bass, bool snare, bool tom, bool cymbal, bool hihat);
extern void opl2_setDrumsByte(uint8_t drums);
extern void opl2_setEnvelopeScaling(uint8_t channel, uint8_t operatorNum, bool enable);
extern void opl2_setFeedback(uint8_t channel, uint8_t feedback);
extern void opl2_setFNumber(uint8_t channel, uint16_t fNumber);
extern void opl2_setFrequency(uint8_t channel, float frequency);
extern void opl2_setInstrument(uint8_t channel, instrument_t *instrument, float volume);
extern void opl2_setKeyOn(uint8_t channel, bool keyOn);
extern void opl2_setMaintainSustain(uint8_t channel, uint8_t operatorNum, bool enable);
extern void opl2_setMultiplier(uint8_t channel, uint8_t operatorNum, uint8_t multiplier);
extern void opl2_setNoteSelect(bool enable);
extern void opl2_setPercussion(bool enable);
extern void opl2_setRelease(uint8_t channel, uint8_t operatorNum, uint8_t release);
extern void opl2_setScalingLevel(uint8_t channel, uint8_t operatorNum, uint8_t scaling);
extern void opl2_setSustain(uint8_t channel, uint8_t operatorNum, uint8_t sustain);
extern void opl2_setSynthMode(uint8_t channel, uint8_t synthMode);
extern void opl2_setTremolo(uint8_t channel, uint8_t operatorNum, bool enable);
extern void opl2_setVibrato(uint8_t channel, uint8_t operatorNum, bool enable);
extern void opl2_setVolume(uint8_t channel, uint8_t operatorNum, uint8_t volume);
extern void opl2_setWaveFormSelect(bool enable);

#endif  // __OPL2_H_
