/*
 * dev_tone_alarm.c
 *
 *  Created on: 2019Äê8ÔÂ2ÈÕ
 *      Author: Administrator
 */

/**
 * Driver for the PX4 audio alarm port, /dev/tone_alarm.
 *
 * The tone_alarm driver supports a set of predefined "alarm"
 * tunes and one user-supplied tune.
 *
 * The TONE_SET_ALARM ioctl can be used to select a predefined
 * alarm tune, from 1 - <TBD>.  Selecting tune zero silences
 * the alarm.
 *
 * Tunes follow the syntax of the Microsoft GWBasic/QBasic PLAY
 * statement, with some exceptions and extensions.
 *
 * From Wikibooks:
 *
 * PLAY "[string expression]"
 *
 * Used to play notes and a score ... The tones are indicated by letters A through G.
 * Accidentals are indicated with a "+" or "#" (for sharp) or "-" (for flat)
 * immediately after the note letter. See this example:
 *
 *   PLAY "C C# C C#"
 *
 * Whitespaces are ignored inside the string expression. There are also codes that
 * set the duration, octave and tempo. They are all case-insensitive. PLAY executes
 * the commands or notes the order in which they appear in the string. Any indicators
 * that change the properties are effective for the notes following that indicator.
 *
 * Ln     Sets the duration (length) of the notes. The variable n does not indicate an actual duration
 *        amount but rather a note type; L1 - whole note, L2 - half note, L4 - quarter note, etc.
 *        (L8, L16, L32, L64, ...). By default, n = 4.
 *        For triplets and quintets, use L3, L6, L12, ... and L5, L10, L20, ... series respectively.
 *        The shorthand notation of length is also provided for a note. For example, "L4 CDE L8 FG L4 AB"
 *        can be shortened to "L4 CDE F8G8 AB". F and G play as eighth notes while others play as quarter notes.
 * On     Sets the current octave. Valid values for n are 0 through 6. An octave begins with C and ends with B.
 *        Remember that C- is equivalent to B.
 * < >    Changes the current octave respectively down or up one level.
 * Nn     Plays a specified note in the seven-octave range. Valid values are from 0 to 84. (0 is a pause.)
 *        Cannot use with sharp and flat. Cannot use with the shorthand notation neither.
 * MN     Stand for Music Normal. Note duration is 7/8ths of the length indicated by Ln. It is the default mode.
 * ML     Stand for Music Legato. Note duration is full length of that indicated by Ln.
 * MS     Stand for Music Staccato. Note duration is 3/4ths of the length indicated by Ln.
 * Pn     Causes a silence (pause) for the length of note indicated (same as Ln).
 * Tn     Sets the number of "L4"s per minute (tempo). Valid values are from 32 to 255. The default value is T120.
 * .      When placed after a note, it causes the duration of the note to be 3/2 of the set duration.
 *        This is how to get "dotted" notes. "L4 C#." would play C sharp as a dotted quarter note.
 *        It can be used for a pause as well.
 *
 * Extensions/variations:
 *
 * MB MF  The MF command causes the tune to play once and then stop. The MB command causes the
 *        tune to repeat when it ends.
 *
 */

#include "dev_tone_alarm.h"
#include "hr_timer.h"
#include "drv_hrt.h"
#include "drv_pwm.h"

#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#include <rtthread.h>
#include "board.h"

/*----------------------------- Private define -------------------------------*/

#define TUNE_MAX        (1024 * 8)
#define nullptr         RT_NULL

/*----------------------------- Private typedef ------------------------------*/

typedef enum
{
    MODE_NORMAL,
    MODE_LEGATO,
    MODE_STACCATO
} note_mode_e;

typedef struct
{
    char      *_user_tune;

    const char      *_tune;     // current tune string
    const char      *_next;     // next note in the string

    unsigned        _tempo;
    unsigned        _note_length;

    note_mode_e     _note_mode;

    unsigned        _octave;
    unsigned        _silence_length; // if nonzero, silence before next note
    bool            _repeat;    // if true, tune restarts at end

    struct hrt_call _note_call; // HRT callout for note completion

    int             pwm_index;

}tone_alarm_t;

/*------------------------------ Private variables ---------------------------*/

const uint8_t _note_tab[] = {9, 11, 0, 2, 4, 5, 7};

static tone_alarm_t tone_alarm =
{
     ._user_tune = RT_NULL,
     ._tune = RT_NULL,
     ._next = RT_NULL,
     .pwm_index = 0,
};

/* ----------------------- Private function prototypes ---------------------*/

static int              play(tone_alarm_t *tone_alarm, const char *buffer, int len);

//// Convert a note value in the range C1 to B7 into a divisor for
//// the configured timer's clock.
////
//static unsigned        note_to_divisor(tone_alarm_t *tone_alarm, unsigned note);

// Calculate the duration in microseconds of play and silence for a
// note given the current tempo, length and mode and the number of
// dots following in the play string.
//
static unsigned        note_duration(tone_alarm_t *tone_alarm, unsigned *silence, unsigned note_length, unsigned dots);

// Calculate the duration in microseconds of a rest corresponding to
// a given note length.
//
static unsigned        rest_duration(tone_alarm_t *tone_alarm, unsigned rest_length, unsigned dots);

// Start playing the note
//
static void            start_note(tone_alarm_t *tone_alarm, unsigned note);

// Stop playing the current note and make the player 'safe'
//
static void            stop_note(tone_alarm_t *tone_alarm);

// Start playing the tune
//
static void            start_tune(tone_alarm_t *tone_alarm, const char *tune);

static void            stop_tune(tone_alarm_t *tone_alarm);

// Parse the next note out of the string and play it
//
static void            next_note(tone_alarm_t *tone_alarm);

// Find the next character in the string, discard any whitespace and
// return the canonical (uppercase) version.
//
static int         next_char(tone_alarm_t *tone_alarm);

// Extract a number from the string, consuming all the digit characters.
//
static unsigned        next_number(tone_alarm_t *tone_alarm);

// Consume dot characters from the string, returning the number consumed.
//
static unsigned        next_dots(tone_alarm_t *tone_alarm);

// hrt_call trampoline for next_note
//
static void     next_trampoline(void *arg);

/*-------------------------------- Functions -------------------------------*/

int dev_tone_alarm_init(void)
{
    rt_memset(&tone_alarm._note_call, 0, sizeof(tone_alarm._note_call));
    drv_pwm_set_frequency(tone_alarm.pwm_index, 0);
    return 0;
}
INIT_DEVICE_EXPORT(dev_tone_alarm_init);

int dev_tone_alarm_play(const char *buffer, int len)
{
    return play(&tone_alarm, buffer, len);
}

/*-------------------------------- Private  Functions -------------------------------*/

static int              play(tone_alarm_t *tone_alarm, const char *buffer, int len)
{
    // sanity-check the buffer for length and nul-termination
    if (len > TUNE_MAX) {
        return -EFBIG;
    }

    // if we have an existing user tune, free it
    if (tone_alarm->_user_tune != nullptr) {

        stop_tune(tone_alarm);

//        // if we are playing the user tune, stop
//        if (tone_alarm->_tune == tone_alarm->_user_tune) {
//            tone_alarm->_tune = nullptr;
//            tone_alarm->_next = nullptr;
//        }

        // free the old user tune
        rt_free((void *)tone_alarm->_user_tune);
        tone_alarm->_user_tune = nullptr;
    }

    // if the new tune is empty, we're done
    if (buffer[0] == '\0') {
        return 0;
    }

    // allocate a copy of the new tune
    tone_alarm->_user_tune = rt_malloc(len);

    if (tone_alarm->_user_tune == nullptr) {
        return -RT_ENOMEM;
    }
    rt_memcpy(tone_alarm->_user_tune, buffer, len);

    // and play it
//    rt_kprintf("play:%s\n", tone_alarm->_user_tune);
    start_tune(tone_alarm, tone_alarm->_user_tune);

    return len;
}

//static unsigned        note_to_divisor(tone_alarm_t *tone_alarm, unsigned note)
//{
//    // compute the frequency first (Hz)
//    float freq = 880.0f * expf(logf(2.0f) * ((int)note - 46) / 12.0f);
//
//    float period = 0.5f / freq;
//
//    // and the divisor, rounded to the nearest integer
//    unsigned divisor = (period * TONE_ALARM_CLOCK) + 0.5f;
//
//    return divisor;
//
//}

static unsigned        note_duration(tone_alarm_t *tone_alarm, unsigned *silence, unsigned note_length, unsigned dots)
{
    unsigned whole_note_period = (60 * 1000000 * 4) / tone_alarm->_tempo;

    if (note_length == 0) {
        note_length = 1;
    }

    unsigned note_period = whole_note_period / note_length;

    switch (tone_alarm->_note_mode) {
    case MODE_NORMAL:
        *silence = note_period / 8;
        break;

    case MODE_STACCATO:
        *silence = note_period / 4;
        break;

    default:
    case MODE_LEGATO:
        *silence = 0;
        break;
    }

    note_period -= *silence;

    unsigned dot_extension = note_period / 2;

    while (dots--) {
        note_period += dot_extension;
        dot_extension /= 2;
    }

    return note_period;
}

static unsigned        rest_duration(tone_alarm_t *tone_alarm, unsigned rest_length, unsigned dots)
{
    unsigned whole_note_period = (60 * 1000000 * 4) / tone_alarm->_tempo;

    if (rest_length == 0) {
        rest_length = 1;
    }

    unsigned rest_period = whole_note_period / rest_length;

    unsigned dot_extension = rest_period / 2;

    while (dots--) {
        rest_period += dot_extension;
        dot_extension /= 2;
    }

    return rest_period;
}

static void            start_note(tone_alarm_t *tone_alarm, unsigned note)
{
//    // check if circuit breaker is enabled
//    if (_cbrk == CBRK_UNINIT) {
//        _cbrk = circuit_breaker_enabled("CBRK_BUZZER", CBRK_BUZZER_KEY);
//    }
//
//    if (_cbrk != CBRK_OFF) { return; }
//
//    // compute the divisor
//    unsigned divisor = note_to_divisor(note);
//
//    // pick the lowest prescaler value that we can use
//    // (note that the effective prescale value is 1 greater)
//    unsigned prescale = divisor / 65536;
//
//    // calculate the timer period for the selected prescaler value
//    unsigned period = (divisor / (prescale + 1)) - 1;
//
//    rPSC = prescale;    // load new prescaler
//    rARR = period;      // load new toggle period
//    rEGR = GTIM_EGR_UG; // force a reload of the period
//    rCCER |= TONE_CCER; // enable the output
//
//    // configure the GPIO to enable timer output
//    stm32_configgpio(GPIO_TONE_ALARM);

    float freq = 880.0f * expf(logf(2.0f) * ((int)note - 46) / 12.0f);
    drv_pwm_set_frequency(tone_alarm->pwm_index, (int)freq);
}

static void            stop_note(tone_alarm_t *tone_alarm)
{
    drv_pwm_set_frequency(tone_alarm->pwm_index, 0);
}

static void            start_tune(tone_alarm_t *tone_alarm, const char *tune)
{
    // kill any current playback
    hrt_cancel(&tone_alarm->_note_call);

    // record the tune
    tone_alarm->_tune = tune;
    tone_alarm->_next = tune;

    // initialise player state
    tone_alarm->_tempo = 120;
    tone_alarm->_note_length = 4;
    tone_alarm->_note_mode = MODE_NORMAL;
    tone_alarm->_octave = 4;
    tone_alarm->_silence_length = 0;
    tone_alarm->_repeat = false;        // otherwise command-line tunes repeat forever...

    // schedule a callback to start playing
    hrt_call_after(&tone_alarm->_note_call, 0, (hrt_callout)next_trampoline, tone_alarm);
}

static void            stop_tune(tone_alarm_t *tone_alarm)
{
    // kill any current playback
    hrt_cancel(&tone_alarm->_note_call);

    tone_alarm->_tune = tone_alarm->_next = nullptr;

    stop_note(tone_alarm);
}

static void            next_note(tone_alarm_t *tone_alarm)
{
    // do we have an inter-note gap to wait for?
    if (tone_alarm->_silence_length > 0) {
        stop_note(tone_alarm);
        hrt_call_after(&tone_alarm->_note_call, (hrt_abstime)tone_alarm->_silence_length, (hrt_callout)next_trampoline, tone_alarm);
        tone_alarm->_silence_length = 0;
        return;
    }

    // make sure we still have a tune - may be removed by the write / ioctl handler
    if ((tone_alarm->_next == nullptr) || (tone_alarm->_tune == nullptr)) {
        stop_note(tone_alarm);
        return;
    }

    // parse characters out of the string until we have resolved a note
    unsigned note = 0;
    unsigned note_length = tone_alarm->_note_length;
    unsigned duration;

    while (note == 0) {
        // we always need at least one character from the string
        int c = next_char(tone_alarm);

        if (c == 0) {
            goto tune_end;
        }

        tone_alarm->_next++;

        switch (c) {
        case 'L':   // select note length
            tone_alarm->_note_length = next_number(tone_alarm);

            if (tone_alarm->_note_length < 1) {
                goto tune_error;
            }

            break;

        case 'O':   // select octave
            tone_alarm->_octave = next_number(tone_alarm);

            if (tone_alarm->_octave > 6) {
                tone_alarm->_octave = 6;
            }

            break;

        case '<':   // decrease octave
            if (tone_alarm->_octave > 0) {
                tone_alarm->_octave--;
            }

            break;

        case '>':   // increase octave
            if (tone_alarm->_octave < 6) {
                tone_alarm->_octave++;
            }

            break;

        case 'M':   // select inter-note gap
            c = next_char(tone_alarm);

            if (c == 0) {
                goto tune_error;
            }

            tone_alarm->_next++;

            switch (c) {
            case 'N':
                tone_alarm->_note_mode = MODE_NORMAL;
                break;

            case 'L':
                tone_alarm->_note_mode = MODE_LEGATO;
                break;

            case 'S':
                tone_alarm->_note_mode = MODE_STACCATO;
                break;

            case 'F':
                tone_alarm->_repeat = false;
                break;

            case 'B':
                tone_alarm->_repeat = true;
                break;

            default:
                goto tune_error;
            }

            break;

        case 'P':   // pause for a note length
            stop_note(tone_alarm);
            hrt_call_after(&tone_alarm->_note_call,
                       (hrt_abstime)rest_duration(tone_alarm, next_number(tone_alarm), next_dots(tone_alarm)),
                       (hrt_callout)next_trampoline,
                       tone_alarm);
            return;

        case 'T': { // change tempo
                unsigned nt = next_number(tone_alarm);

                if ((nt >= 32) && (nt <= 255)) {
                    tone_alarm->_tempo = nt;

                } else {
                    goto tune_error;
                }

                break;
            }

        case 'N':   // play an arbitrary note
            note = next_number(tone_alarm);

            if (note > 84) {
                goto tune_error;
            }

            if (note == 0) {
                // this is a rest - pause for the current note length
                hrt_call_after(&tone_alarm->_note_call,
                           (hrt_abstime)rest_duration(tone_alarm, tone_alarm->_note_length, next_dots(tone_alarm)),
                           (hrt_callout)next_trampoline,
                           tone_alarm);
                return;
            }

            break;

        case 'A'...'G': // play a note in the current octave
            note = _note_tab[c - 'A'] + (tone_alarm->_octave * 12) + 1;
            c = next_char(tone_alarm);

            switch (c) {
            case '#':   // up a semitone
            case '+':
                if (note < 84) {
                    note++;
                }

                tone_alarm->_next++;
                break;

            case '-':   // down a semitone
                if (note > 1) {
                    note--;
                }

                tone_alarm->_next++;
                break;

            default:
                // 0 / no next char here is OK
                break;
            }

            // shorthand length notation
            note_length = next_number(tone_alarm);

            if (note_length == 0) {
                note_length = tone_alarm->_note_length;
            }

            break;

        default:
            goto tune_error;
        }
    }

    // compute the duration of the note and the following silence (if any)
    duration = note_duration(tone_alarm, &tone_alarm->_silence_length, note_length, next_dots(tone_alarm));

    // start playing the note
    start_note(tone_alarm, note);

    // and arrange a callback when the note should stop
    hrt_call_after(&tone_alarm->_note_call, (hrt_abstime)duration, (hrt_callout)next_trampoline, tone_alarm);
    return;

    // tune looks bad (unexpected EOF, bad character, etc.)
tune_error:
    rt_kprintf("tune error\n");
    tone_alarm->_repeat = false;        // don't loop on error

    // stop (and potentially restart) the tune
tune_end:
    stop_note(tone_alarm);

    if (tone_alarm->_repeat) {
        start_tune(tone_alarm, tone_alarm->_tune);

    } else {
        tone_alarm->_tune = nullptr;
//        _default_tune_number = 0;
    }

    return;
}

static int         next_char(tone_alarm_t *tone_alarm)
{
    while (isspace(*tone_alarm->_next)) {
        tone_alarm->_next++;
    }

    return toupper(*tone_alarm->_next);
}

static unsigned        next_number(tone_alarm_t *tone_alarm)
{
    unsigned number = 0;
    int c;

    for (;;) {
        c = next_char(tone_alarm);

        if (!isdigit(c)) {
            return number;
        }

        tone_alarm->_next++;
        number = (number * 10) + (c - '0');
    }
}

static unsigned        next_dots(tone_alarm_t *tone_alarm)
{
    unsigned dots = 0;

    while (next_char(tone_alarm) == '.') {
        tone_alarm->_next++;
        dots++;
    }

    return dots;
}

static void     next_trampoline(void *arg)
{
    next_note((tone_alarm_t *)arg);
}

#include <finsh.h>
int cmd_play_tune(int argc, char **argv)
{
    if(argc != 2)
    {
        rt_kprintf("usage:\n  play_tune string\r\n");
        return -1;
    }

    dev_tone_alarm_play(argv[1], strlen(argv[1]) + 1);
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_play_tune, play_tune, play tune);
