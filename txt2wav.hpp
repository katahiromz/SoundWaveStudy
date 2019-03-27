#ifndef TXT2WAV_HPP_
#define TXT2WAV_HPP_

#include <cstdio>

bool txt2wav_fp(const char *in, const char *out, FILE *fin, FILE *fout, int sampling_rate);
bool txt2wav(const char *txt_file, const char *wav_file, int sampling_rate);

#endif  // ndef TXT2WAV_HPP_
