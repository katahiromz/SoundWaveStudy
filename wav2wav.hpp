#ifndef WAV2WAV_HPP_
#define WAV2WAV_HPP_

#include <cstdio>

struct W2W
{
    int channels = 0;       // default if zero
    int mode = 0;           // default if zero
    int sampling_rate = 0;  // default if zero
};

bool mono_to_stereo(PcmWave& wave1, PcmWave& wave2);
bool stereo_to_mono(PcmWave& wave1, PcmWave& wave2);
bool mode_8bit_to_16bit(PcmWave& wave1, PcmWave& wave2);
bool mode_16bit_to_8bit(PcmWave& wave1, PcmWave& wave2);
bool wav2wav_fp(const char *in, const char *out, FILE *fin, FILE *fout, W2W& w2w);
bool wav2wav(const char *txt_file, const char *wav_file, W2W& w2w);

#endif  // ndef WAV2WAV_HPP_
