#include "PcmWave.hpp"
#include "wav2txt.hpp"
#include <cstdio>

static void show_info(const char *name, const PcmWave& wave)
{
    fprintf(stderr, "%s: %ld Hz sampling, %d-bit, %d channel\n",
            name, wave.sample_rate(),
            wave.mode(), wave.num_channels());
}

static bool write_1ch_8(FILE *fout, const PcmWave& wave)
{
    for (size_t i = 0; i < wave.num_units() * wave.num_channels(); ++i)
    {
        fprintf(fout, "%u\n", wave.data_8bit(i));
    }
    return true;
}

static bool write_1ch_16(FILE *fout, const PcmWave& wave)
{
    for (size_t i = 0; i < wave.num_units() * wave.num_channels(); ++i)
    {
        fprintf(fout, "%d\n", wave.data_16bit(i));
    }
    return true;
}

static bool write_2ch_8(FILE *fout, const PcmWave& wave)
{
    for (size_t i = 0; i < wave.num_units() * wave.num_channels(); i += 2)
    {
        fprintf(fout, "%u %u\n", wave.data_8bit(i), wave.data_8bit(i + 1));
    }
    return true;
}

static bool write_2ch_16(FILE *fout, const PcmWave& wave)
{
    for (size_t i = 0; i < wave.num_units() * wave.num_channels(); i += 2)
    {
        fprintf(fout, "%d %d\n", wave.data_16bit(i), wave.data_16bit(i + 1));
    }
    return true;
}

bool wav2txt_fp(const char *in, const char *out, FILE *fin, FILE *fout)
{
    PcmWave wave;
    if (!wave.read_from_fp(fin))
    {
        fprintf(stderr, "ERROR: %s: unable to read\n", in);
        return false;
    }

    show_info(in, wave);

    switch (wave.num_channels())
    {
    case 1:
        switch (wave.mode())
        {
        case 8:
            write_1ch_8(fout, wave);
            break;
        case 16:
            write_1ch_16(fout, wave);
            break;
        default:
            assert(0);
            return false;
        }
        break;
    case 2:
        switch (wave.mode())
        {
        case 8:
            write_2ch_8(fout, wave);
            break;
        case 16:
            write_2ch_16(fout, wave);
            break;
        default:
            assert(0);
            return false;
        }
        break;
    default:
        assert(0);
        return false;
    }

    return true;
}

bool wav2txt(const char *wav_file, const char *txt_file)
{
    FILE *fin, *fout;

    if (wav_file)
    {
        fin = fopen(wav_file, "rb");
        if (!fin)
        {
            fprintf(stderr, "ERROR: Unable to open file '%s'.\n", wav_file);
            return false;
        }
    }
    else
    {
        fin = stdin;
        wav_file = "stdin";
    }

    if (txt_file)
    {
        fout = fopen(txt_file, "w");
        if (!fout)
        {
            fprintf(stderr, "ERROR: Unable to open file '%s'.\n", txt_file);
            fclose(fin);
            return false;
        }
    }
    else
    {
        if (wav_file)
        {
            static char out_name[128];
            strcpy(out_name, wav_file);
            strcat(out_name, ".txt");
            txt_file = out_name;
            fout = fopen(out_name, "wb");
            if (!fout)
            {
                fprintf(stderr, "ERROR: Unable to open file '%s'.\n", out_name);
                fclose(fin);
                return false;
            }
        }
        else
        {
            fout = stdout;
            txt_file = "stdout";
        }
    }

    bool ret = wav2txt_fp(wav_file, txt_file, fin, fout);
    if (ret)
    {
        fprintf(stderr, "'%s' --> '%s' (OK)\n", wav_file, txt_file);
    }

    fclose(fout);
    fclose(fin);

    return ret;
}

#ifdef WAV2TXT
    static void show_help(void)
    {
        printf("wav2txt --- Converts a wave file to a text file\n");
        printf("Usage #1: txt2wav [options] sound-file.wav [text-file.txt]\n");
        printf("Usage #2: cat sound-file.wav | txt2wav [options] > text-file.txt\n");
        printf("Options:\n");
        printf("--help      Show this help.\n");
        printf("--version   Show version info.\n");
    }

    static void show_version(void)
    {
        printf("wav2txt version 0.0 by katahiromz\n");
    }

    int main(int argc, char **argv)
    {
        if (argc <= 1)
        {
            if (bool ret = wav2txt_fp("stdin", "stdout", stdin, stdout))
                return EXIT_SUCCESS;
            return EXIT_FAILURE;
        }

        const char *arg1 = NULL;
        const char *arg2 = NULL;
        for (int i = 1; i < argc; ++i)
        {
            if (argv[i][0] == '-')
            {
                if (strcmp(argv[i], "--help") == 0)
                {
                    show_help();
                    return EXIT_SUCCESS;
                }
                if (strcmp(argv[i], "--version") == 0)
                {
                    show_version();
                    return EXIT_SUCCESS;
                }

                fprintf(stderr, "ERROR: Invalid argument '%s'.\n", argv[i]);
                return EXIT_FAILURE;
            }
            if (arg1 == NULL)
            {
                arg1 = argv[i];
            }
            else if (arg2 == NULL)
            {
                arg2 = argv[i];
            }
            else
            {
                fprintf(stderr, "ERROR: Too many argument.\n");
                return EXIT_FAILURE;
            }
        }

        return wav2txt(arg1, arg2) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
#endif