#ifndef WAV2TXT_HPP_
#define WAV2TXT_HPP_

#include <cstdio>

bool wav2txt_fp(const char *in, const char *out, FILE *fin, FILE *fout);
bool wav2txt(const char *wav_file, const char *txt_file);

#endif  // ndef WAV2TXT_HPP_
