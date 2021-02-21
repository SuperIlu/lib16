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
 *
 * http://bochs.sourceforge.net/techspec/adlib_sb.txt
 * https://github.com/DhrBaksteen/ArduinoOPL2
 */

#include <conio.h>
#include <i86.h>
#include <stdlib.h>

#include "opl2.h"
#include "error.h"

/* ======================================================================
** defines
** ====================================================================== */
#define OPL2_CLAMP(val, vmin, vmax) max(vmin, min(val, vmax))

#define OPL2_PORT_ADDR 0x388  //!< Address/Status port  (R/W)
#define OPL2_PORT_DATA 0x389  //!< Data port

#define OPL2_PORT_ADDR_DELAY 10  //!< address delay
#define OPL2_PORT_DATA_DELAY 40  //!< data delay

/* ======================================================================
** private variables
** ====================================================================== */
static const float opl2_fIntervals[8] = {0.048, 0.095, 0.190, 0.379, 0.759, 1.517, 3.034, 6.069};
static const unsigned int opl2_noteFNumbers[12] = {0x156, 0x16B, 0x181, 0x198, 0x1B0, 0x1CA, 0x1E5, 0x202, 0x220, 0x241, 0x263, 0x287};
static const float opl2_blockFrequencies[8] = {48.503, 97.006, 194.013, 388.026, 776.053, 1552.107, 3104.215, 6208.431};
static const uint8_t opl2_registerOffsets[2][9] = {
    {0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12}, /*  initializers for operator 1 */
    {0x03, 0x04, 0x05, 0x0B, 0x0C, 0x0D, 0x13, 0x14, 0x15}  /*  initializers for operator 2 */
};
static const uint8_t opl2_drumRegisterOffsets[2][5] = {{0x10, 0xFF, 0x12, 0xFF, 0x11}, {0x13, 0x14, 0xFF, 0x15, 0xFF}};
static const uint8_t opl2_drumChannels[5] = {6, 7, 8, 8, 7};
static const uint8_t opl2_drumBits[5] = {OPL2_DRUM_BITS_BASS, OPL2_DRUM_BITS_SNARE, OPL2_DRUM_BITS_TOM, OPL2_DRUM_BITS_CYMBAL, OPL2_DRUM_BITS_HI_HAT};

static uint8_t opl2_chipRegisters[3];
static uint8_t opl2_channelRegisters[3 * OPL2_NUM_CHANNELS];
static uint8_t opl2_operatorRegisters[10 * OPL2_NUM_CHANNELS];

/* ======================================================================
** private functions
** ====================================================================== */
/**
 * @brief write to an AdLib register
 *
 * @param reg register number
 * @param val new value
 */
static void opl2_write(uint8_t reg, uint8_t val) {
    int i;

    outp(OPL2_PORT_ADDR, reg);
    for (i = 0; i < OPL2_PORT_ADDR_DELAY; i++) {
        inp(OPL2_PORT_ADDR);
    }
    outp(OPL2_PORT_DATA, val);
    for (i = 0; i < OPL2_PORT_DATA_DELAY; i++) {
        inp(OPL2_PORT_ADDR);
    }
}

/**
 * @brief read OPL status register
 *
 * @return uint8_t
 */
static uint8_t opl2_read() { return inp(OPL2_PORT_ADDR); }

/**
 * Get the internal register offset for a chip wide register.
 *
 * @param reg - The 9-bit register for which we want to know the internal offset.
 * @return The offset to the internal shadow register or 0 if an illegal chip register was requested.
 */
static uint8_t opl2_getChipRegisterOffset(uint8_t reg) {
    switch (reg & 0xFF) {
        case 0x08:
            return 1;
        case 0xBD:
            return 2;
        case 0x01:
        default:
            return 0;
    }
}

/**
 * Get the internal offset of a channel register.
 *
 * @param baseRegister - The base register where we want to know the offset of.
 * @param channel - The channel [0, numChannels] for which we want to know the offset.
 * @return The internal offset of the channel register or 0 if the baseRegister is invalid.
 */
static uint8_t opl2_getChannelRegisterOffset(uint8_t baseRegister, uint8_t channel) {
    uint8_t offset;

    channel = channel % OPL2_NUM_CHANNELS;
    offset = channel * 3;

    switch (baseRegister) {
        case 0xB0:
            return offset + 1;
        case 0xC0:
            return offset + 2;
        case 0xA0:
        default:
            return offset;
    }
}

/**
 * Get the internal offset of an operator register.
 *
 * @param baseRegister - The base register where we want to know the offset of.
 * @param channel - The channel [0, numChannels] to get the offset to.
 * @param operatorNum - The operator [0, 1] to get the offset to.
 * @return The internal offset of the operator register or 0 if the baseRegister is invalid.
 */
static uint16_t opl2_getOperatorRegisterOffset(uint8_t baseRegister, uint8_t channel, uint8_t operatorNum) {
    uint16_t offset;

    channel = channel % OPL2_NUM_CHANNELS;
    operatorNum = operatorNum & 0x01;
    offset = (channel * 10) + (operatorNum * 5);

    switch (baseRegister) {
        case 0x40:
            return offset + 1;
        case 0x60:
            return offset + 2;
        case 0x80:
            return offset + 3;
        case 0xE0:
            return offset + 4;
        case 0x20:
        default:
            return offset;
    }
}

/**
 * Get the offset from a base register to a channel operator register.
 *
 * @param channel - The channel for which to get the offset [0, 8].
 * @param operatorNum - The operator for which to get the offset [0, 1].
 * @return The offset from the base register to the operator register.
 */
static uint8_t opl2_getRegisterOffset(uint8_t channel, uint8_t operatorNum) { return opl2_registerOffsets[operatorNum % 2][channel % OPL2_CHANNELS_PER_BANK]; }

/**
 * Get the current value of a chip wide register from the shadow registers.
 *
 * @param reg - The 9-bit address of the register.
 * @return The value of the register from shadow registers.
 */
static uint8_t opl2_getChipRegister(uint8_t reg) { return opl2_chipRegisters[opl2_getChipRegisterOffset(reg)]; }

/**
 * Write a given value to a chip wide register.
 *
 * @param reg - The 9-bit register to write to.
 * @param value - The value to write to the register.
 */
static void opl2_setChipRegister(uint8_t reg, uint8_t value) {
    opl2_chipRegisters[opl2_getChipRegisterOffset(reg)] = value;
    opl2_write(reg & 0xFF, value);
}

/**
 * Get the value of a channel register.
 *
 * @param baseAddress - The base address of the register.
 * @param channel - The channel for which to get the register value [0, 8].
 * @return The current value of the from the shadow register.
 */
static uint8_t opl2_getChannelRegister(uint8_t baseRegister, uint8_t channel) { return opl2_channelRegisters[opl2_getChannelRegisterOffset(baseRegister, channel)]; }

/**
 * Write a given value to a channel based register.
 *
 * @param baseRegister - The base address of the register.
 * @param channel - The channel to address [0, 8].
 * @param value - The value to write to the register.
 */
static void opl2_setChannelRegister(uint8_t baseRegister, uint8_t channel, uint8_t value) {
    uint8_t reg;
    opl2_channelRegisters[opl2_getChannelRegisterOffset(baseRegister, channel)] = value;
    reg = baseRegister + (channel % OPL2_CHANNELS_PER_BANK);
    opl2_write(reg, value);
}

/**
 * Get the current value of an operator register of a channel from the shadow registers.
 *
 * @param baseRegister - The base address of the register.
 * @param channel - The channel of the operatpr [0, 17].
 * @param op - The operator [0, 1].
 * @return The operator register value from shadow registers.
 */
static uint8_t opl2_getOperatorRegister(uint8_t baseRegister, uint8_t channel, uint8_t operatorNum) {
    return opl2_operatorRegisters[opl2_getOperatorRegisterOffset(baseRegister, channel, operatorNum)];
}

/**
 * Write a given value to an operator register for a channel.
 *
 * @param baseRegister - The base address of the register.
 * @param channel - The channel of the operator [0, 8]
 * @param operatorNum - The operator to change [0, 1].
 * @param value - The value to write to the operator's register.
 */
static void opl2_setOperatorRegister(uint8_t baseRegister, uint8_t channel, uint8_t operatorNum, uint8_t value) {
    uint8_t reg;
    opl2_operatorRegisters[opl2_getOperatorRegisterOffset(baseRegister, channel, operatorNum)] = value;
    reg = baseRegister + opl2_getRegisterOffset(channel, operatorNum);
    opl2_write(reg, value);
}

/* ======================================================================
** public functions
** ====================================================================== */
/**
 * @brief detect an OPL2/AdLib
 *
 * @return true if a card is found, else false.
 */
bool opl2_init() {
    uint8_t stat1, stat2;
    // 1)  Reset both timers by writing 60h to register 4.
    opl2_write(0x04, 0x60);
    // 2)  Enable the interrupts by writing 80h to register 4.  NOTE: this must be a separate step from number 1.
    opl2_write(0x04, 0x80);
    // 3)  Read the status register (port 388h).  Store the result.
    stat1 = opl2_read();
    // 4)  Write FFh to register 2 (Timer 1).
    opl2_write(0x01, 0xFF);
    // 5)  Start timer 1 by writing 21h to register 4.
    opl2_write(0x04, 0x21);
    // 6)  Delay for at least 80 microseconds.
    delay(80);
    // 7)  Read the status register (port 388h).  Store the result.
    stat2 = opl2_read();
    // 8)  Reset both timers and interrupts (see steps 1 and 2).
    opl2_write(0x04, 0x60);
    opl2_write(0x04, 0x80);
    // 9)  Test the stored results of steps 3 and 7 by ANDing them with E0h.  The result of step 3 should be 00h, and the result of step 7 should be C0h.  If both are correct, an
    // AdLib-compatible board is installed in the computer.

    if (((stat1 & 0xE0) == 0x00) && ((stat2 & 0xE0) == 0xC0)) {
        opl2_reset();
        ERR_OK();
        return true;
    } else {
        ERR_AVAIL();
        return false;
    }
}

/**
 * Hard reset the OPL2 chip and initialize all registers to 0x00. This should be called before sending any data to the
 * chip.
 */
void opl2_reset() {
    int i, j;

    // Initialize chip registers.
    opl2_setChipRegister(0x00, 0x00);
    opl2_setChipRegister(0x08, 0x40);
    opl2_setChipRegister(0xBD, 0x00);

    // Initialize all channel and operator registers.
    for (i = 0; i < OPL2_NUM_CHANNELS; i++) {
        opl2_setChannelRegister(0xA0, i, 0x00);
        opl2_setChannelRegister(0xB0, i, 0x00);
        opl2_setChannelRegister(0xC0, i, 0x00);

        for (j = OPL2_OPERATOR1; j <= OPL2_OPERATOR2; j++) {
            opl2_setOperatorRegister(0x20, i, j, 0x00);
            opl2_setOperatorRegister(0x40, i, j, 0x3F);
            opl2_setOperatorRegister(0x60, i, j, 0x00);
            opl2_setOperatorRegister(0x80, i, j, 0x00);
            opl2_setOperatorRegister(0xE0, i, j, 0x00);
        }
    }
}

/**
 * Get the frequency block of the given channel.
 */
uint8_t opl2_getBlock(uint8_t channel) { return (opl2_getChannelRegister(0xB0, channel) & 0x1C) >> 2; }

/**
 * Set frequency block. 0x00 is lowest, 0x07 is highest. This determines the frequency interval between notes.
 * 0 - 0.048 Hz, Range: 0.047 Hz ->   48.503 Hz
 * 1 - 0.095 Hz, Range: 0.094 Hz ->   97.006 Hz
 * 2 - 0.190 Hz, Range: 0.189 Hz ->  194.013 Hz
 * 3 - 0.379 Hz, Range: 0.379 Hz ->  388.026 Hz
 * 4 - 0.759 Hz, Range: 0.758 Hz ->  776.053 Hz
 * 5 - 1.517 Hz, Range: 1.517 Hz -> 1552.107 Hz
 * 6 - 3.034 Hz, Range: 3.034 Hz -> 3104.215 Hz
 * 7 - 6.069 Hz, Range: 6.068 Hz -> 6208.431 Hz
 */
void opl2_setBlock(uint8_t channel, uint8_t block) {
    uint8_t value = opl2_getChannelRegister(0xB0, channel) & 0xE3;
    opl2_setChannelRegister(0xB0, channel, value + ((block & 0x07) << 2));
}

/**
 * Get the octave split bit.
 */
bool opl2_getNoteSelect() { return opl2_getChipRegister(0x08) & 0x40; }

/**
 * Set the octave split bit. This sets how the chip interprets F-numbers to determine where an octave is split. For note
 * F-numbers used by the OPL2 library this bit should be set.
 *
 * @param enable - Sets the note select bit when true, otherwise reset it.
 */
void opl2_setNoteSelect(bool enable) { opl2_setChipRegister(0x08, enable ? 0x40 : 0x00); }

/**
 * Is the voice of the given channel currently enabled?
 */
bool opl2_getKeyOn(uint8_t channel) { return opl2_getChannelRegister(0xB0, channel) & 0x20; }

/**
 * Enable voice on channel.
 */
void opl2_setKeyOn(uint8_t channel, bool keyOn) {
    uint8_t value = opl2_getChannelRegister(0xB0, channel) & 0xDF;
    opl2_setChannelRegister(0xB0, channel, value + (keyOn ? 0x20 : 0x00));
}

/**
 * Get the feedback strength of the given channel.
 */
uint8_t opl2_getFeedback(uint8_t channel) { return (opl2_getChannelRegister(0xC0, channel) & 0x0E) >> 1; }

/**
 * Set feedback strength. 0x00 is no feedback, 0x07 is strongest.
 */
void opl2_setFeedback(uint8_t channel, uint8_t feedback) {
    uint8_t value = opl2_getChannelRegister(0xC0, channel) & 0xF1;
    opl2_setChannelRegister(0xC0, channel, value + ((feedback & 0x07) << 1));
}

/**
 * Get the synth model that is used for the given channel.
 */
uint8_t opl2_getSynthMode(uint8_t channel) { return opl2_getChannelRegister(0xC0, channel) & 0x01; }

/**
 * Set the synthesizer mode of the given channel.
 */
void opl2_setSynthMode(uint8_t channel, uint8_t synthMode) {
    uint8_t value = opl2_getChannelRegister(0xC0, channel) & 0xFE;
    opl2_setChannelRegister(0xC0, channel, value + (synthMode & 0x01));
}

/**
 * Is deeper amplitude modulation enabled?
 */
bool opl2_getDeepTremolo() { return opl2_getChipRegister(0xBD) & 0x80; }

/**
 * Set deeper aplitude modulation depth. When false modulation depth is 1.0 dB, when true modulation depth is 4.8 dB.
 */
void opl2_setDeepTremolo(bool enable) {
    uint8_t value = opl2_getChipRegister(0xBD) & 0x7F;
    opl2_setChipRegister(0xBD, value + (enable ? 0x80 : 0x00));
}

/**
 * Is deeper vibrato depth enabled?
 */
bool opl2_getDeepVibrato() { return opl2_getChipRegister(0xBD) & 0x40; }

/**
 * Set deeper vibrato depth. When false vibrato depth is 7/100 semi tone, when true vibrato depth is 14/100.
 */
void opl2_setDeepVibrato(bool enable) {
    uint8_t value = opl2_getChipRegister(0xBD) & 0xBF;
    opl2_setChipRegister(0xBD, value + (enable ? 0x40 : 0x00));
}

/**
 * Is percussion mode currently enabled?
 */
bool opl2_getPercussion() { return opl2_getChipRegister(0xBD) & 0x20; }

/**
 * Enable or disable percussion mode. When set to false there are 9 melodic voices, when true there are 6 melodic
 * voices and channels 6 through 8 are used for drum sounds. KeyOn for these channels must be off.
 */
void opl2_setPercussion(bool enable) {
    uint8_t value = opl2_getChipRegister(0xBD) & 0xDF;
    opl2_setChipRegister(0xBD, value + (enable ? 0x20 : 0x00));
}

/**
 * Return which drum sounds are enabled.
 */
uint8_t opl2_getDrums() { return opl2_getChipRegister(0xBD) & 0x1F; }

/**
 * Set the OPL2 drum registers all at once.
 */
void opl2_setDrumsByte(uint8_t drums) {
    uint8_t value = opl2_getChipRegister(0xBD) & 0xE0;
    opl2_setChipRegister(0xBD, value);
    opl2_setChipRegister(0xBD, value + (drums & 0x1F));
}

/**
 * Enable or disable various drum sounds.
 * Note that keyOn for channels 6, 7 and 8 must be false in order to use rhythms.
 */
void opl2_setDrums(bool bass, bool snare, bool tom, bool cymbal, bool hihat) {
    uint8_t drums = 0;
    drums += bass ? OPL2_DRUM_BITS_BASS : 0x00;
    drums += snare ? OPL2_DRUM_BITS_SNARE : 0x00;
    drums += tom ? OPL2_DRUM_BITS_TOM : 0x00;
    drums += cymbal ? OPL2_DRUM_BITS_CYMBAL : 0x00;
    drums += hihat ? OPL2_DRUM_BITS_HI_HAT : 0x00;
    opl2_setDrumsByte(drums);
}

/**
 * Get the wave form currently set for the given channel.
 */
uint8_t opl2_getWaveForm(uint8_t channel, uint8_t operatorNum) { return opl2_getOperatorRegister(0xE0, channel, operatorNum) & 0x07; }

/**
 * Select the wave form to use.
 */
void opl2_setWaveForm(uint8_t channel, uint8_t operatorNum, uint8_t waveForm) {
    uint8_t value = opl2_getOperatorRegister(0xE0, channel, operatorNum) & 0xF8;
    opl2_setOperatorRegister(0xE0, channel, operatorNum, value + (waveForm & 0x07));
}

/**
 * Get the frequency step per F-number for the current block on the given channel.
 */
float opl2_getFrequencyStep(uint8_t channel) { return opl2_fIntervals[opl2_getBlock(channel)]; }

/**
 * Get the F-number for the given frequency for a given channel. When the F-number is calculated the current frequency
 * block of the channel is taken into account.
 */
uint16_t opl2_getFrequencyFNumber(uint8_t channel, float frequency) {
    float fInterval = opl2_getFrequencyStep(channel);
    return OPL2_CLAMP((uint16_t)(frequency / fInterval), (uint16_t)0, (uint16_t)1023);
}

/**
 * Get the F-Number for the given note. In this case the block is assumed to be the octave.
 */
uint16_t opl2_getNoteFNumber(uint8_t note) { return opl2_noteFNumbers[note % OPL2_NUM_NOTES]; }

/**
 * Get the optimal frequency block for the given frequency.
 */
uint8_t opl2_getFrequencyBlock(float frequency) {
    uint8_t i;

    for (i = 0; i < 8; i++) {
        if (frequency < opl2_blockFrequencies[i]) {
            return i;
        }
    }
    return 7;
}

/**
 * Is wave form selection currently enabled.
 */
bool opl2_getWaveFormSelect() { return opl2_getChipRegister(0x01) & 0x20; }

/**
 * Enable wave form selection for each operator.
 */
void opl2_setWaveFormSelect(bool enable) {
    if (enable) {
        opl2_setChipRegister(0x01, opl2_getChipRegister(0x01) | 0x20);
    } else {
        opl2_setChipRegister(0x01, opl2_getChipRegister(0x01) & 0xDF);
    }
}

/**
 * Is amplitude modulation enabled for the given operator?
 */
bool opl2_getTremolo(uint8_t channel, uint8_t operatorNum) { return opl2_getOperatorRegister(0x20, channel, operatorNum) & 0x80; }

/**
 * Apply amplitude modulation when set to true. Modulation depth is controlled globaly by the AM-depth flag in the 0xBD
 * register.
 */
void opl2_setTremolo(uint8_t channel, uint8_t operatorNum, bool enable) {
    uint8_t value = opl2_getOperatorRegister(0x20, channel, operatorNum) & 0x7F;
    opl2_setOperatorRegister(0x20, channel, operatorNum, value + (enable ? 0x80 : 0x00));
}

/**
 * Is vibrator enabled for the given channel?
 */
bool opl2_getVibrato(uint8_t channel, uint8_t operatorNum) { return opl2_getOperatorRegister(0x20, channel, operatorNum) & 0x40; }

/**
 * Apply vibrato when set to true. Vibrato depth is controlled globally by the VIB-depth flag in the 0xBD register.
 */
void opl2_setVibrato(uint8_t channel, uint8_t operatorNum, bool enable) {
    uint8_t value = opl2_getOperatorRegister(0x20, channel, operatorNum) & 0xBF;
    opl2_setOperatorRegister(0x20, channel, operatorNum, value + (enable ? 0x40 : 0x00));
}

/**
 * Is sustain being maintained for the given channel?
 */
bool opl2_getMaintainSustain(uint8_t channel, uint8_t operatorNum) { return opl2_getOperatorRegister(0x20, channel, operatorNum) & 0x20; }

/**
 * When set to true the sustain level of the voice is maintained until released. When false the sound begins to decay
 * immediately after hitting the sustain phase.
 */
void opl2_setMaintainSustain(uint8_t channel, uint8_t operatorNum, bool enable) {
    uint8_t value = opl2_getOperatorRegister(0x20, channel, operatorNum) & 0xDF;
    opl2_setOperatorRegister(0x20, channel, operatorNum, value + (enable ? 0x20 : 0x00));
}

/**
 * Is envelope scaling being applied to the given channel?
 */
bool opl2_getEnvelopeScaling(uint8_t channel, uint8_t operatorNum) { return opl2_getOperatorRegister(0x20, channel, operatorNum) & 0x10; }

/**
 * Enable or disable envelope scaling. When set to true higher notes will be shorter than lower ones.
 */
void opl2_setEnvelopeScaling(uint8_t channel, uint8_t operatorNum, bool enable) {
    uint8_t value = opl2_getOperatorRegister(0x20, channel, operatorNum) & 0xEF;
    opl2_setOperatorRegister(0x20, channel, operatorNum, value + (enable ? 0x10 : 0x00));
}

/**
 * Get the frequency multiplier for the given channel.
 */
uint8_t opl2_getMultiplier(uint8_t channel, uint8_t operatorNum) { return opl2_getOperatorRegister(0x20, channel, operatorNum) & 0x0F; }

/**
 * Set frequency multiplier for the given channel. Note that a multiplier of 0 will apply a 0.5 multiplication.
 */
void opl2_setMultiplier(uint8_t channel, uint8_t operatorNum, uint8_t multiplier) {
    uint8_t value = opl2_getOperatorRegister(0x20, channel, operatorNum) & 0xF0;
    opl2_setOperatorRegister(0x20, channel, operatorNum, value + (multiplier & 0x0F));
}

/**
 * Get the scaling level for the given channel.
 */
uint8_t opl2_getScalingLevel(uint8_t channel, uint8_t operatorNum) { return (opl2_getOperatorRegister(0x40, channel, operatorNum) & 0xC0) >> 6; }

/**
 * Decrease output levels as frequency increases.
 * 00 - No change
 * 01 - 1.5 dB/oct
 * 10 - 3.0 dB/oct
 * 11 - 6.0 dB/oct
 */
void opl2_setScalingLevel(uint8_t channel, uint8_t operatorNum, uint8_t scaling) {
    uint8_t value = opl2_getOperatorRegister(0x40, channel, operatorNum) & 0x3F;
    opl2_setOperatorRegister(0x40, channel, operatorNum, value + ((scaling & 0x03) << 6));
}

/**
 * Get the volume of the given channel operator. 0x00 is laudest, 0x3F is softest.
 */
uint8_t opl2_getVolume(uint8_t channel, uint8_t operatorNum) { return opl2_getOperatorRegister(0x40, channel, operatorNum) & 0x3F; }

/**
 * Set the volume of the channel operator.
 * Note that the scale is inverted! 0x00 for loudest, 0x3F for softest.
 */
void opl2_setVolume(uint8_t channel, uint8_t operatorNum, uint8_t volume) {
    uint8_t value = opl2_getOperatorRegister(0x40, channel, operatorNum) & 0xC0;
    opl2_setOperatorRegister(0x40, channel, operatorNum, value + (volume & 0x3F));
}

/**
 * Get the volume of the given channel.
 */
uint8_t opl2_getChannelVolume(uint8_t channel) { return opl2_getVolume(channel, OPL2_OPERATOR2); }

/**
 * Set the volume for the given channel. Depending on the current synthesis mode this will affect both operators (AM) or
 * only operator 2 (FM).
 */
void opl2_setChannelVolume(uint8_t channel, uint8_t volume) {
    if (opl2_getSynthMode(channel)) {
        opl2_setVolume(channel, OPL2_OPERATOR1, volume);
    }
    opl2_setVolume(channel, OPL2_OPERATOR2, volume);
}

/**
 * Get the attack rate of the given channel.
 */
uint8_t opl2_getAttack(uint8_t channel, uint8_t operatorNum) { return (opl2_getOperatorRegister(0x60, channel, operatorNum) & 0xF0) >> 4; }

/**
 * Attack rate. 0x00 is slowest, 0x0F is fastest.
 */
void opl2_setAttack(uint8_t channel, uint8_t operatorNum, uint8_t attack) {
    uint8_t value = opl2_getOperatorRegister(0x60, channel, operatorNum) & 0x0F;
    opl2_setOperatorRegister(0x60, channel, operatorNum, value + ((attack & 0x0F) << 4));
}

/**
 * Get the decay rate of the given channel.
 */
uint8_t opl2_getDecay(uint8_t channel, uint8_t operatorNum) { return opl2_getOperatorRegister(0x60, channel, operatorNum) & 0x0F; }

/**
 * Decay rate. 0x00 is slowest, 0x0F is fastest.
 */
void opl2_setDecay(uint8_t channel, uint8_t operatorNum, uint8_t decay) {
    uint8_t value = opl2_getOperatorRegister(0x60, channel, operatorNum) & 0xF0;
    opl2_setOperatorRegister(0x60, channel, operatorNum, value + (decay & 0x0F));
}

/**
 * Get the sustain level of the given channel. 0x00 is laudest, 0x0F is softest.
 */
uint8_t opl2_getSustain(uint8_t channel, uint8_t operatorNum) { return (opl2_getOperatorRegister(0x80, channel, operatorNum) & 0xF0) >> 4; }

/**
 * Sustain level. 0x00 is laudest, 0x0F is softest.
 */
void opl2_setSustain(uint8_t channel, uint8_t operatorNum, uint8_t sustain) {
    uint8_t value = opl2_getOperatorRegister(0x80, channel, operatorNum) & 0x0F;
    opl2_setOperatorRegister(0x80, channel, operatorNum, value + ((sustain & 0x0F) << 4));
}

/**
 * Get the release rate of the given channel.
 */
uint8_t opl2_getRelease(uint8_t channel, uint8_t operatorNum) { return opl2_getOperatorRegister(0x80, channel, operatorNum) & 0x0F; }

/**
 * Release rate. 0x00 is flowest, 0x0F is fastest.
 */
void opl2_setRelease(uint8_t channel, uint8_t operatorNum, uint8_t release) {
    uint8_t value = opl2_getOperatorRegister(0x80, channel, operatorNum) & 0xF0;
    opl2_setOperatorRegister(0x80, channel, operatorNum, value + (release & 0x0F));
}

/**
 * Get the frequenct F-number of the given channel.
 */
uint16_t opl2_getFNumber(uint8_t channel) {
    uint16_t value = (opl2_getChannelRegister(0xB0, channel) & 0x03) << 8;
    value += opl2_getChannelRegister(0xA0, channel);
    return value;
}

/**
 * Set frequency F-number [0, 1023] for the given channel.
 */
void opl2_setFNumber(uint8_t channel, uint16_t fNumber) {
    uint8_t value = opl2_getChannelRegister(0xB0, channel) & 0xFC;
    opl2_setChannelRegister(0xB0, channel, value + ((fNumber & 0x0300) >> 8));
    opl2_setChannelRegister(0xA0, channel, fNumber & 0xFF);
}

/**
 * Get the frequency for the given channel.
 */
float opl2_getFrequency(uint8_t channel) {
    float fInterval = opl2_getFrequencyStep(channel);
    return opl2_getFNumber(channel) * fInterval;
}

/**
 * Set the frequenct of the given channel and if needed switch to a different block.
 */
void opl2_setFrequency(uint8_t channel, float frequency) {
    uint16_t fNumber;
    uint8_t block = opl2_getFrequencyBlock(frequency);
    if (opl2_getBlock(channel) != block) {
        opl2_setBlock(channel, block);
    }
    fNumber = opl2_getFrequencyFNumber(channel, frequency);
    opl2_setFNumber(channel, fNumber);
}

/**
 * Create and return a new empty instrument->
 */
void opl2_createInstrument(instrument_t *instrument) {
    uint8_t op;

    for (op = OPL2_OPERATOR1; op <= OPL2_OPERATOR2; op++) {
        instrument->operators[op].hasTremolo = false;
        instrument->operators[op].hasVibrato = false;
        instrument->operators[op].hasSustain = false;
        instrument->operators[op].hasEnvelopeScaling = false;
        instrument->operators[op].frequencyMultiplier = 0;
        instrument->operators[op].keyScaleLevel = 0;
        instrument->operators[op].outputLevel = 0;
        instrument->operators[op].attack = 0;
        instrument->operators[op].decay = 0;
        instrument->operators[op].sustain = 0;
        instrument->operators[op].release = 0;
        instrument->operators[op].waveForm = 0;
    }

    instrument->transpose = 0;
    instrument->feedback = 0;
    instrument->isAdditiveSynth = false;
}

/**
 * Create an instrument and load it with instrument parameters from the given instrument data pointer.
 */
void opl2_loadInstrument(const unsigned char *data, instrument_t *instrument) {
    uint8_t op;

    opl2_createInstrument(instrument);

    for (op = OPL2_OPERATOR1; op <= OPL2_OPERATOR2; op++) {
        instrument->operators[op].hasTremolo = data[op * 5 + 1] & 0x80 ? true : false;
        instrument->operators[op].hasVibrato = data[op * 5 + 1] & 0x40 ? true : false;
        instrument->operators[op].hasSustain = data[op * 5 + 1] & 0x20 ? true : false;
        instrument->operators[op].hasEnvelopeScaling = data[op * 5 + 1] & 0x10 ? true : false;
        instrument->operators[op].frequencyMultiplier = (data[op * 5 + 1] & 0x0F);
        instrument->operators[op].keyScaleLevel = (data[op * 5 + 2] & 0xC0) >> 6;
        instrument->operators[op].outputLevel = data[op * 5 + 2] & 0x3F;
        instrument->operators[op].attack = (data[op * 5 + 3] & 0xF0) >> 4;
        instrument->operators[op].decay = data[op * 5 + 3] & 0x0F;
        instrument->operators[op].sustain = (data[op * 5 + 4] & 0xF0) >> 4;
        instrument->operators[op].release = data[op * 5 + 4] & 0x0F;
    }
    instrument->operators[0].waveForm = data[10] & 0x07;
    instrument->operators[1].waveForm = (data[10] & 0x70) >> 4;

    instrument->transpose = data[0];
    instrument->feedback = (data[5] & 0x0E) >> 1;
    instrument->isAdditiveSynth = data[5] & 0x01 ? true : false;
}

/**
 * Create a new instrument from the given OPL2 channel.
 */
void opl2_getInstrument(uint8_t channel, instrument_t *instrument) {
    uint8_t op;

    for (op = OPL2_OPERATOR1; op <= OPL2_OPERATOR2; op++) {
        instrument->operators[op].hasTremolo = opl2_getTremolo(channel, op);
        instrument->operators[op].hasVibrato = opl2_getVibrato(channel, op);
        instrument->operators[op].hasSustain = opl2_getMaintainSustain(channel, op);
        instrument->operators[op].hasEnvelopeScaling = opl2_getEnvelopeScaling(channel, op);
        instrument->operators[op].frequencyMultiplier = opl2_getMultiplier(channel, op);
        instrument->operators[op].keyScaleLevel = opl2_getScalingLevel(channel, op);
        instrument->operators[op].outputLevel = opl2_getVolume(channel, op);
        instrument->operators[op].attack = opl2_getAttack(channel, op);
        instrument->operators[op].decay = opl2_getDecay(channel, op);
        instrument->operators[op].sustain = opl2_getSustain(channel, op);
        instrument->operators[op].release = opl2_getRelease(channel, op);
        instrument->operators[op].waveForm = opl2_getWaveForm(channel, op);
    }

    instrument->transpose = 0;
    instrument->feedback = opl2_getFeedback(channel);
    instrument->isAdditiveSynth = opl2_getSynthMode(channel) == OPL2_SYNTH_MODE_AM;
}

/**
 * Set the given instrument to a channel. An optional volume may be provided to assign to proper output levels for the
 * operators.
 */
void opl2_setInstrument(uint8_t channel, instrument_t *instrument, float volume) {
    uint8_t op, value;

    volume = OPL2_CLAMP(volume, (float)0.0, (float)1.0);

    opl2_setWaveFormSelect(true);
    for (op = OPL2_OPERATOR1; op <= OPL2_OPERATOR2; op++) {
        uint8_t outputLevel = 63 - (uint8_t)((63.0 - (float)instrument->operators[op].outputLevel) * volume);

        opl2_setOperatorRegister(0x20, channel, op,
                                 (instrument->operators[op].hasTremolo ? 0x80 : 0x00) + (instrument->operators[op].hasVibrato ? 0x40 : 0x00) +
                                     (instrument->operators[op].hasSustain ? 0x20 : 0x00) + (instrument->operators[op].hasEnvelopeScaling ? 0x10 : 0x00) +
                                     (instrument->operators[op].frequencyMultiplier & 0x0F));
        opl2_setOperatorRegister(0x40, channel, op, ((instrument->operators[op].keyScaleLevel & 0x03) << 6) + (outputLevel & 0x3F));
        opl2_setOperatorRegister(0x60, channel, op, ((instrument->operators[op].attack & 0x0F) << 4) + (instrument->operators[op].decay & 0x0F));
        opl2_setOperatorRegister(0x80, channel, op, ((instrument->operators[op].sustain & 0x0F) << 4) + (instrument->operators[op].release & 0x0F));
        opl2_setOperatorRegister(0xE0, channel, op, (instrument->operators[op].waveForm & 0x07));
    }

    value = opl2_getChannelRegister(0xC0, channel) & 0xF0;
    opl2_setChannelRegister(0xC0, channel, value + ((instrument->feedback & 0x07) << 1) + (instrument->isAdditiveSynth ? 0x01 : 0x00));
}

/**
 * Set the given instrument as a drum type for percussive mode. Depending on the drumType the instrument parameters will
 * be loaded into the appropriate channel and operator(s). An optional volume may be provided to set the
 * proper output levels for the operator(s).
 *
 * @param instrument - The instrument to be set.
 * @param drumType - The type of drum instrument to set the parameters of.
 * @param volume - Optional volume parameter for the drum.
 */
void opl2_setDrumInstrument(instrument_t *instrument, uint8_t drumType, float volume) {
    uint8_t channel, value, op;

    drumType = OPL2_CLAMP(drumType, (uint8_t)OPL2_DRUM_BASS, (uint8_t)OPL2_DRUM_HI_HAT);
    volume = OPL2_CLAMP(volume, (float)0.0, (float)1.0);
    channel = opl2_drumChannels[drumType];

    opl2_setWaveFormSelect(true);
    for (op = OPL2_OPERATOR1; op <= OPL2_OPERATOR2; op++) {
        uint8_t outputLevel = 63 - (uint8_t)((63.0 - (float)instrument->operators[op].outputLevel) * volume);
        uint8_t registerOffset = opl2_drumRegisterOffsets[op][drumType];

        if (registerOffset != 0xFF) {
            opl2_setOperatorRegister(0x20, channel, op,
                                     (instrument->operators[op].hasTremolo ? 0x80 : 0x00) + (instrument->operators[op].hasVibrato ? 0x40 : 0x00) +
                                         (instrument->operators[op].hasSustain ? 0x20 : 0x00) + (instrument->operators[op].hasEnvelopeScaling ? 0x10 : 0x00) +
                                         (instrument->operators[op].frequencyMultiplier & 0x0F));
            opl2_setOperatorRegister(0x40, channel, op, ((instrument->operators[op].keyScaleLevel & 0x03) << 6) + (outputLevel & 0x3F));
            opl2_setOperatorRegister(0x60, channel, op, ((instrument->operators[op].attack & 0x0F) << 4) + (instrument->operators[op].decay & 0x0F));
            opl2_setOperatorRegister(0x80, channel, op, ((instrument->operators[op].sustain & 0x0F) << 4) + (instrument->operators[op].release & 0x0F));
            opl2_setOperatorRegister(0xE0, channel, op, (instrument->operators[op].waveForm & 0x07));
        }
    }

    value = opl2_getChannelRegister(0xC0, channel) & 0xF0;
    opl2_setChannelRegister(0xC0, channel, value + ((instrument->feedback & 0x07) << 1) + (instrument->isAdditiveSynth ? 0x01 : 0x00));
}

/**
 * Play a note of a certain octave on the given channel.
 */
void opl2_playNote(uint8_t channel, uint8_t octave, uint8_t note) {
    if (opl2_getKeyOn(channel)) {
        opl2_setKeyOn(channel, false);
    }
    opl2_setBlock(channel, OPL2_CLAMP(octave, (uint8_t)0, (uint8_t)OPL2_NUM_OCTAVES));
    opl2_setFNumber(channel, opl2_noteFNumbers[note % 12]);
    opl2_setKeyOn(channel, true);
}

/**
 * Play a drum sound at a given note and frequency.
 * The OPL2 must be put into percusive mode first and the parameters of the drum sound must be set in the required
 * operator(s). Note that changing octave and note frequenct will influence both drum sounds if they occupy only a
 * single operator (Snare + Hi-hat and Tom + Cymbal).
 */
void opl2_playDrum(uint8_t drum, uint8_t octave, uint8_t note) {
    uint8_t drumState, drumChannel;

    drum = drum % OPL2_NUM_DRUM_SOUNDS;
    drumState = opl2_getDrums();

    opl2_setDrumsByte(drumState & ~opl2_drumBits[drum]);
    drumChannel = opl2_drumChannels[drum];
    opl2_setBlock(drumChannel, OPL2_CLAMP(octave, (uint8_t)0, (uint8_t)OPL2_NUM_OCTAVES));
    opl2_setFNumber(drumChannel, opl2_noteFNumbers[note % OPL2_NUM_NOTES]);
    opl2_setDrumsByte(drumState | opl2_drumBits[drum]);
}
