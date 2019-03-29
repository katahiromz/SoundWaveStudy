#include "PcmWave.hpp"
#include "wav2wav.hpp"
#include <cstdio>
#include <limits>

static void show_info(const char *name, const PcmWave& wave)
{
    fprintf(stderr, "%s: %ld Hz sampling, %d-bit, %d channel (%.1f seconds)\n",
            name, wave.sample_rate(),
            wave.mode(), wave.num_channels(), wave.seconds());
}

bool mono_to_stereo(PcmWave& wave1, PcmWave& wave2)
{
    wave2.clear();

    if (wave1.num_channels() != 1)
    {
        assert(0);
        return false;
    }

    switch (wave1.mode())
    {
    case 8:
        wave2.set_info(2, wave1.mode(), wave1.sample_rate());
        wave2.reserve(wave1.num_units() * wave1.num_channels() * 2);
        for (size_t i = 0; i < wave1.num_units() * wave1.num_channels(); ++i)
        {
            uint8_t value = wave1.data_8bit(i);
            wave2.push_8bit(value);
            wave2.push_8bit(value);
        }
        break;
    case 16:
        wave2.set_info(2, wave1.mode(), wave1.sample_rate());
        wave2.reserve(wave1.num_units() * wave1.num_channels() * 2);
        for (size_t i = 0; i < wave1.num_units() * wave1.num_channels(); ++i)
        {
            int16_t value = wave1.data_16bit(i);
            wave2.push_16bit(value);
            wave2.push_16bit(value);
        }
        break;
    default:
        assert(0);
        return false;
    }

    wave2.update_info();
    return true;
}

bool stereo_to_mono(PcmWave& wave1, PcmWave& wave2)
{
    wave2.clear();

    if (wave1.num_channels() != 2)
    {
        assert(0);
        return false;
    }

    switch (wave1.mode())
    {
    case 8:
        wave2.set_info(1, wave1.mode(), wave1.sample_rate());
        wave2.reserve(wave1.num_units() * wave1.num_channels() / 2);
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
        wave2.reserve(wave1.num_units() * wave1.num_channels() / 2);
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

    wave2.update_info();
    return true;
}

inline int
linear_interpolation(int value, int min1, int max1, int min2, int max2)
{
    // [min1, max1] --> [min2, max2]
    return (value - min1) * (max2 - min2) / (max1 - min1) + min2;
}

inline void interpolation_test(void)
{
    assert(linear_interpolation(0, 0, 255, -32768, 32767) == -32768);
    assert(linear_interpolation(255, 0, 255, -32768, 32767) == 32767);
    assert(linear_interpolation(-32768, -32768, 32767, 0, 255) == 0);
    assert(linear_interpolation(32767, -32768, 32767, 0, 255) == 255);
}

bool mode_8bit_to_16bit(PcmWave& wave1, PcmWave& wave2)
{
    interpolation_test();

    wave2.clear();

    if (wave1.mode() != 8)
    {
        assert(0);
        return false;
    }

    switch (wave1.num_channels())
    {
    case 1:
        wave2.set_info(1, 16, wave1.sample_rate());
        wave2.reserve(wave1.num_units() * wave1.num_channels() * 2);
        for (size_t i = 0; i < wave1.num_units() * wave1.num_channels(); ++i)
        {
            int value = wave1.data_16bit(i);
            assert(0 <= value && value <= 255);
            // [0, 255] --> [-32768, 32767]
            value = linear_interpolation(value, 0, 255, -32768, 32767);
            assert(-32768 <= value && value <= 32767);
            wave2.push_16bit(int16_t(value));
        }
        break;
    case 2:
        wave2.set_info(2, 16, wave1.sample_rate());
        wave2.reserve(wave1.num_units() * wave1.num_channels() * 2);
        for (size_t i = 0; i < wave1.num_units() * wave1.num_channels(); i += 2)
        {
            int left = wave1.data_16bit(i);
            int right = wave1.data_16bit(i + 1);
            assert(0 <= left && left <= 255);
            assert(0 <= right && right <= 255);
            // [0, 255] --> [-32768, 32767]
            left = linear_interpolation(left, 0, 255, -32768, 32767);
            right = linear_interpolation(right, 0, 255, -32768, 32767);
            assert(-32768 <= left && left <= 32767);
            assert(-32768 <= right && right <= 32767);
            wave2.push_16bit(int16_t(left));
            wave2.push_16bit(int16_t(right));
        }
        break;
    default:
        assert(0);
        return false;
    }

    wave2.update_info();
    return true;
}

bool mode_16bit_to_8bit(PcmWave& wave1, PcmWave& wave2)
{
    interpolation_test();

    wave2.clear();

    if (wave1.mode() != 16)
    {
        assert(0);
        return false;
    }

    switch (wave1.num_channels())
    {
    case 1:
        wave2.set_info(1, 8, wave1.sample_rate());
        wave2.reserve(wave1.num_units() * wave1.num_channels() / 2);
        for (size_t i = 0; i < wave1.num_units() * wave1.num_channels(); ++i)
        {
            int value = wave1.data_16bit(i);
            assert(-32768 <= value && value <= 32767);
            // [-32768, 32767] --> [0, 255]
            value = linear_interpolation(value, -32768, 32767, 0, 255);
            assert(0 <= value && value <= 255);
            wave2.push_8bit(uint8_t(value));
        }
        break;
    case 2:
        wave2.set_info(2, 8, wave1.sample_rate());
        wave2.reserve(wave1.num_units() * wave1.num_channels() / 2);
        for (size_t i = 0; i < wave1.num_units() * wave1.num_channels(); i += 2)
        {
            int left = wave1.data_16bit(i);
            int right = wave1.data_16bit(i + 1);
            assert(-32768 <= left && left <= 32767);
            assert(-32768 <= right && right <= 32767);
            // [-32768, 32767] --> [0, 255]
            left = linear_interpolation(left, -32768, 32767, 0, 255);
            right = linear_interpolation(right, -32768, 32767, 0, 255);
            assert(0 <= left && left <= 255);
            assert(0 <= right && right <= 255);
            wave2.push_8bit(uint8_t(left));
            wave2.push_8bit(uint8_t(right));
        }
        break;
    default:
        assert(0);
        return false;
    }

    wave2.update_info();
    return true;
}

bool wav2wav_fp(const char *in, const char *out, FILE *fin, FILE *fout, W2W& w2w)
{
    PcmWave wave1, wave2, wave3;

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
        wave2 = std::move(wave1);
        flag = true;
        break;
    case 1:
        if (wave1.is_mono())
        {
            wave2 = std::move(wave1);
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
            wave2 = std::move(wave1);
            flag = true;
        }
        else
        {
            flag = mono_to_stereo(wave1, wave2);
        }
        break;
    }

    if (!flag)
    {
        fprintf(stderr, "ERROR: %s: Unable to convert.\n", in);
        return false;
    }

    flag = false;
    switch (w2w.mode)
    {
    case 0:
        wave3 = std::move(wave2);
        flag = true;
        break;
    case 8:
        if (wave2.mode_8bit())
        {
            wave3 = std::move(wave2);
            flag = true;
        }
        else
        {
            flag = mode_16bit_to_8bit(wave2, wave3);
        }
        break;
    case 16:
        if (wave2.mode_16bit())
        {
            wave3 = std::move(wave2);
            flag = true;
        }
        else
        {
            flag = mode_8bit_to_16bit(wave2, wave3);
        }
        break;
    }

    if (!flag)
    {
        fprintf(stderr, "ERROR: %s: Unable to convert.\n", in);
        return false;
    }

    if (w2w.sampling_rate)
    {
        wave3.sample_rate(w2w.sampling_rate);
        wave3.update_info();
    }

    show_info(out, wave3);
    assert(wave3.is_valid());

    if (!wave3.write_to_fp(fout))
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

    assert(file1);
    fin = fopen(file1, "rb");
    if (!fin)
    {
        fprintf(stderr, "ERROR: Unable to open file '%s'.\n", file1);
        return false;
    }

    char out_name[256];
    if (!file2)
    {
        strcpy(out_name, file1);
        strcat(out_name, ".wav");
        file2 = out_name;
    }

    fout = fopen(file2, "wb");
    if (!fout)
    {
        fprintf(stderr, "ERROR: Unable to open file '%s'.\n", file2);
        fclose(fin);
        return false;
    }

    bool ret = wav2wav_fp(file1, file2, fin, fout, w2w);

    fclose(fout);
    fclose(fin);

    return ret;
}

#ifdef WAV2WAV
    static void show_help(void)
    {
        printf("wav2wav --- Converts a wave file to another wave file\n");
        printf("Usage: txt2wav [options] wave-file-1.txt [wave-file-2.wav]\n");
        printf("Options:\n");
        printf("--help          Show this help.\n");
        printf("--version       Show version info.\n");
        printf("--channels XXX  Specify the number of channels.\n");
        printf("--rate XXX      Specify sampling rate.\n");
        printf("--mode XXX      Specify bits per sample.\n");
    }

    static void show_version(void)
    {
        printf("wav2wav version 0.4 by katahiromz\n");
    }

    int main(int argc, char **argv)
    {
        W2W w2w;

        if (argc <= 1)
        {
            show_help();
            return EXIT_SUCCESS;
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

        if (arg1 == NULL)
        {
            fprintf(stderr, "ERROR: No input file.\n");
            return EXIT_FAILURE;
        }

        return wav2wav(arg1, arg2, w2w) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
#endif
