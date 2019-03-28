#include "PcmWave.hpp"
#include "wav2wav.hpp"
#include <cstdio>
#include <limits>

static void show_info(const char *name, const PcmWave& wave)
{
    fprintf(stderr, "%s: %ld Hz sampling, %d-bit, %d channel\n",
            name, wave.sample_rate(),
            wave.mode(), wave.num_channels());
}

bool mono_to_stereo(PcmWave& wave1, PcmWave& wave2)
{
    // TODO:
    return false;
}

bool stereo_to_mono(PcmWave& wave1, PcmWave& wave2)
{
    switch (wave1.num_channels())
    {
    case 1:
    default:
        // must be stereo
        assert(0);
        return false;
    case 2:
        switch (wave1.mode())
        {
        case 8:
            wave2.set_info(1, wave1.mode(), wave1.sample_rate());
            for (size_t i = 0; i < wave1.num_units() * wave1.num_channels(); i += 2)
            {
                int left = wave1.data_8bit(i);
                int right = wave1.data_8bit(i + 1);
                uint8_t middle = uint8_t((left + right) / 2);
                wave2.push_8bit(middle);
            }
            break;
        case 16:
            wave2.set_info(1, wave1.mode(), wave1.sample_rate());
            for (size_t i = 0; i < wave1.num_units() * wave1.num_channels(); i += 2)
            {
                int left = wave1.data_16bit(i);
                int right = wave1.data_16bit(i + 1);
                int16_t middle = int16_t((left + right) / 2);
                wave2.push_16bit(middle);
            }
            break;
        default:
            assert(0);
            return false;
        }
        break;
    }

    wave2.update_info();
    return true;
}

bool wav2wav_fp(const char *in, const char *out, FILE *fin, FILE *fout, W2W& w2w)
{
    PcmWave wave1, wave2;

    if (!wave1.read_from_fp(fin))
    {
        fprintf(stderr, "ERROR: %s: unable to read\n", in);
        return false;
    }

    show_info(in, wave1);

    bool flag = false;
    switch (w2w.channels)
    {
    case 0:
        wave2 = wave1;
        flag = true;
        break;
    case 1:
        if (wave1.is_mono())
        {
            wave2 = wave1;
            flag = true;
        }
        else
        {
            flag = stereo_to_mono(wave1, wave2);
        }
        break;
    case 2:
        if (wave1.is_stereo())
        {
            wave2 = wave1;
            flag = true;
        }
        else
        {
            flag = mono_to_stereo(wave1, wave2);
        }
        break;
    }

    show_info(out, wave2);
    assert(wave2.is_valid());

    if (!wave2.write_to_fp(fout))
    {
        fprintf(stderr, "ERROR: %s: Unable to write.\n", out);
        return false;
    }

    fprintf(stderr, "'%s' --> '%s' (OK)\n", in, out);

    return true;
}

bool wav2wav(const char *file1, const char *file2, W2W& w2w)
{
    FILE *fin, *fout;

    if (file1)
    {
        fin = fopen(file1, "rb");
        if (!fin)
        {
            fprintf(stderr, "ERROR: Unable to open file '%s'.\n", file1);
            return false;
        }
    }
    else
    {
        file1 = "stdin";
        fin = stdin;
    }

    if (file2)
    {
        fout = fopen(file2, "wb");
        if (!fout)
        {
            fprintf(stderr, "ERROR: Unable to open file '%s'.\n", file2);
            fclose(fin);
            return false;
        }
    }
    else
    {
        if (file1)
        {
            static char out_name[128];
            strcpy(out_name, file1);
            strcat(out_name, ".wav");
            file2 = out_name;
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
            file2 = "stdout";
            fout = stdout;
        }
    }

    bool ret = wav2wav_fp(file1, file2, fin, fout, w2w);

    fclose(fout);
    fclose(fin);

    return ret;
}

#ifdef WAV2WAV
    static void show_help(void)
    {
        printf("wav2wav --- Converts a wave file to a wave file\n");
        printf("Usage #1: txt2wav [options] wave-file-1.txt [wave-file-2.wav]\n");
        printf("Usage #2: cat wave-file-1.wav | txt2wav [options] > wave-file-2.wav\n");
        printf("Options:\n");
        printf("--help          Show this help.\n");
        printf("--version       Show version info.\n");
        printf("--channels XXX  Specify the number of channels.\n");
        printf("--rate XXX      Specify sampling rate.\n");
        printf("--mode XXX      Specify bits per sample.\n");
    }

    static void show_version(void)
    {
        printf("wav2wav version 0.0 by katahiromz\n");
    }

    int main(int argc, char **argv)
    {
        W2W w2w;

        if (argc <= 1)
        {
            if (bool ret = wav2wav_fp("stdin", "stdout", stdin, stdout, w2w))
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
                if (strcmp(argv[i], "--channels") == 0)
                {
                    if (i + 1 >= argc)
                    {
                        fprintf(stderr, "ERROR: No parameter for '%s'.\n", argv[i]);
                        return EXIT_FAILURE;
                    }
                    ++i;
                    w2w.channels = (int)strtoul(argv[i], NULL, 0);
                    if (i >= argc || (w2w.channels != 0 && w2w.channels != 1 && w2w.channels != 2))
                    {
                        fprintf(stderr, "ERROR: Invalid parameter '%s'.\n", argv[i]);
                        return EXIT_FAILURE;
                    }
                    continue;
                }
                if (strcmp(argv[i], "--rate") == 0)
                {
                    if (i + 1 >= argc)
                    {
                        fprintf(stderr, "ERROR: No parameter for '%s'.\n", argv[i]);
                        return EXIT_FAILURE;
                    }
                    ++i;
                    w2w.sampling_rate = (int)strtoul(argv[i], NULL, 0);
                    if (i >= argc)
                    {
                        fprintf(stderr, "ERROR: Invalid parameter '%s'.\n", argv[i]);
                        return EXIT_FAILURE;
                    }
                    continue;
                }
                if (strcmp(argv[i], "--mode") == 0)
                {
                    if (i + 1 >= argc)
                    {
                        fprintf(stderr, "ERROR: No parameter for '%s'.\n", argv[i]);
                        return EXIT_FAILURE;
                    }
                    ++i;
                    w2w.mode = (int)strtoul(argv[i], NULL, 0);
                    if (i >= argc || (w2w.mode != 0 && w2w.mode != 8 && w2w.mode != 16))
                    {
                        fprintf(stderr, "ERROR: Invalid parameter '%s'.\n", argv[i]);
                        return EXIT_FAILURE;
                    }
                    continue;
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

        return wav2wav(arg1, arg2, w2w) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
#endif
