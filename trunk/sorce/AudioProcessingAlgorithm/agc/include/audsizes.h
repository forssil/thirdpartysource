#ifndef AUDSIZES_H
#define AUDSIZES_H

/*
 * AUD_MAX_20MS_AUDIO defines maximum number of bytes with 20ms audio. Max
 * is 16bit PCM at 16kHz, i.e. (16 kHz) * (16bit/sample) * (20ms) / (8bit) =
 * 640 bytes.
 */
#define AUD_MAX_20MS_AUDIO 1920
#define AUD_MAX_10MS_AUDIO AUD_MAX_20MS_AUDIO/2

// Maximum number of samples in one uncompressed audioframe
#define AUD_MAXFRAMELEN     960

// Internal buffer size in ms for uncompressed audio sent internally
// in the audio system (after decoding).
#define AUD_INT_BUFSIZE_MS  10

// External buffer size in ms for audio sent between the stack and the
// audio codec.
#define AUD_EXT_BUFSIZE_MS  20

// Max number of audio channels supported in this product.  1 for
// mono, 2 for stereo
#define AUD_MAX_CHANNELS        2
#define AUD_MAX_EC_CHANNELS     3

#define AUD_INT_SAMPLERATE_HZ   48000
#define AUD_INT_SAMPLERATE_KHZ  48

#define AUD_INT_BUFSIZE (AUD_INT_SAMPLERATE_KHZ * AUD_INT_BUFSIZE_MS)

#define OPUSPLAY_MAX_NUM_CHANNELS 3
#define OPUSPLAY_MAX_NUM_SAMPLES 960 /* allow 20 msec packets */

#endif
