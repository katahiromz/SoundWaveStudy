#include "PcmWave.hpp"
#include "txt2wav.hpp"
#include <cstdio>
#include <limits>

#define BUFSIZE 128

static void show_info(const char *name, const PcmWave& wave)
{
    fprintf(stderr, "%s: %ld Hz sampling, %d-bit, %d channel (%.1f seconds)\n",
            name, wave.sample_rate(),
            wave.mode(), wave.num_channels(), wave.seconds());
}

static uint16_t scan_mode(FILE *fin)
{
    int data;
    char buf[BUFSIZE];

    while (fscanf(fin, "%d", &data) != EOF)
    {
        if (data < std::numeric_limits<uint8_t>::min() ||
            std::numeric_limits<uint8_t>::max() < data)
        {
            return 16;
        }
    }

    return 8;
}

static uint16_t scan_channels(FILE *fin)
{
    int a, b;
    short ch;
    char buf[BUFSIZE];

    fgets(buf, BUFSIZE, fin);
    ch = sscanf(buf, "%d %d", &a, &b);

    return ch;
}

bool read_1ch_8(FILE *fin, PcmWave& wave)
{
    int left, right;
    char buf[BUFSIZE];
    size_t count = 0;
    const int ch = 1;

    while (fgets(buf, BUFSIZE, fin) != NULL)
    {
        int n = sscanf(buf, "%d %d", &left, &right);
        assert(n == ch);

        assert(std::numeric_limits<uint8_t>::min() <= left);
        assert(left <= std::numeric_limits<uint8_t>::max());

        wave.push_8bit(uint8_t(left));
    }

    wave.update_info();
    return true;
}

bool read_1ch_16(FILE *fin, PcmWave& wave)
{
    int left, right;
    char buf[BUFSIZE];
    size_t count = 0;
    const int ch = 1;

    while (fgets(buf, BUFSIZE, fin) != NULL)
    {
        int n = sscanf(buf, "%d %d", &left, &right);
        assert(n == ch);

        assert(std::numeric_limits<int16_t>::min() <= left);
        assert(left <= std::numeric_limits<int16_t>::max());

        wave.push_16bit(int16_t(left));
    }

    wave.update_info();
    return true;
}

bool read_2ch_8(FILE *fin, PcmWave& wave)
{
    int left, right;
    char buf[BUFSIZE];
    size_t count = 0;
    const int ch = 2;

    while (fgets(buf, BUFSIZE, fin) != NULL)
    {
        int n = sscanf(buf, "%d %d", &left, &right);
        assert(n == ch);

        assert(std::numeric_limits<uint8_t>::min() <= left);
        assert(left <= std::numeric_limits<uint8_t>::max());

        assert(std::numeric_limits<uint8_t>::min() <= right);
        assert(right <= std::numeric_limits<uint8_t>::max());

        wave.push_8bit(uint8_t(left));
        wave.push_8bit(uint8_t(right));
        count++;
    }

    wave.update_info();
    return true;
}

bool read_2ch_16(FILE *fin, PcmWave& wave)
{
    int left, right;
    char buf[BUFSIZE];
    size_t count = 0;
    const int ch = 2;

    while (fgets(buf, BUFSIZE, fin) != NULL)
    {
        int n = sscanf(buf, "%d %d", &left, &right);
        assert(n == ch);

        assert(std::numeric_limits<int16_t>::min() <= left);
        assert(left <= std::numeric_limits<int16_t>::max());

        assert(std::numeric_limits<int16_t>::min() <= right);
        assert(right <= std::numeric_limits<int16_t>::max());

        wave.push_16bit(int16_t(left));
        wave.push_16bit(int16_t(right));
        count++;
    }

    printf("count: %u\n", count);
    wave.update_info();
    return true;
}

bool txt2wav_fp(const char *in, const char *out, FILE *fin, FILE *fout, int sampling_rate)
{
    if (sampling_rate == 0)
        sampling_rate = 44100;

    uint32_t mode = scan_mode(fin);
    rewind(fin);
    uint32_t channels = scan_channels(fin);
    rewind(fin);

    PcmWave wave(channels, mode, sampling_rate);

    show_info(in, wave);

    bool flag = false;
    switch (channels)
    {
    case 1:
        switch (mode)
        {
        case 8:
            flag = read_1ch_8(fin, wave);
            break;
        case 16:
            flag = read_1ch_16(fin, wave);
            break;
        }
        break;
    case 2:
        switch (mode)
        {
        case 8:
            flag = read_2ch_8(fin, wave);
            break;
        case 16:
            flag = read_2ch_16(fin, wave);
            break;
        }
        break;
    }

    if (!wave.write_to_fp(fout))
    {
        fprintf(stderr, "ERROR: %s: Unable to write.\n", out);
        return false;
    }

    fprintf(stderr, "'%s' --> '%s' (OK)\n", in, out);

    return true;
}

bool txt2wav(const char *txt_file, const char *wav_file, int sampling_rate)
{
    FILE *fin, *fout;

    if (txt_file)
    {
        fin = fopen(txt_file, "r");
        if (!fin)
        {
            fprintf(stderr, "ERROR: Unable to open file '%s'.\n", txt_file);
            return false;
        }
    }
    else
    {
        txt_file = "stdin";
        fin = stdin;
    }

    if (wav_file)
    {
        fout = fopen(wav_file, "wb");
        if (!fout)
        {
            fprintf(stderr, "ERROR: Unable to open file '%s'.\n", wav_file);
            fclose(fin);
            return false;
        }
    }
    else
    {
        if (txt_file)
        {
            static char out_name[128];
            strcpy(out_name, txt_file);
            strcat(out_name, ".wav");
            wav_file = out_name;
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
            wav_file = "stdout";
            fout = stdout;
        }
    }

    bool ret = txt2wav_fp(txt_file, wav_file, fin, fout, sampling_rate);

    fclose(fout);
    fclose(fin);

    return ret;
}

#ifdef TXT2WAV
    static void show_help(void)
    {
        printf("txt2wav --- Converts a text file to a wave file\n");
        printf("Usage: txt2wav [options] text-file.txt [sound-file.wav]\n");
        printf("Options:\n");
        printf("--help      Show this help.\n");
        printf("--version   Show version info.\n");
        printf("--rate XXX  Specify sampling rate.\n");
    }

    static void show_version(void)
    {
        printf("txt2wav version 0.0 by katahiromz\n");
    }

    int main(int argc, char **argv)
    {
        int sampling_rate = 0;

        if (argc <= 1)
        {
            if (bool ret = txt2wav_fp("stdin", "stdout", stdin, stdout, sampling_rate))
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
                if (strcmp(argv[i], "--rate") == 0)
                {
                    if (i + 1 >= argc)
                    {
                        fprintf(stderr, "ERROR: No parameter for '%s'.\n", argv[i]);
                        return EXIT_FAILURE;
                    }
                    ++i;
                    sampling_rate = (int)strtoul(argv[i], NULL, 0);
                    if (i >= argc || sampling_rate == 0)
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

        return txt2wav(arg1, arg2, sampling_rate) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
#endif
