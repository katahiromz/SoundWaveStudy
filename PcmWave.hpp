#ifndef PCM_WAVE_HPP_
#define PCM_WAVE_HPP_     7   /* Version 7 */

#if __cplusplus >= 201103L  /* C++11 */
    #include <cstdint>
#elif __STDC_VERSION__ >= 199901L   /* C99 */
    #include <stdint.h>
    #include <stdbool.h>
#else
    #include "pstdint.h"
    #include "pstdbool.h"
#endif

#ifdef __cplusplus
    #include <cstdio>
    #include <cstring>
    #include <vector>
    #include <cassert>
#else
    #include <stdio.h>
    #include <string.h>
    #include <assert.h>
#endif

/* predefinable default values */
#ifndef PCM_WAVE_DEFAULT_CHANNELS
    #define PCM_WAVE_DEFAULT_CHANNELS 1
#endif
#ifndef PCM_WAVE_DEFAULT_BITSPERSAMPLE
    #define PCM_WAVE_DEFAULT_BITSPERSAMPLE 8
#endif
#ifndef PCM_WAVE_DEFAULT_SAMPLE_RATE
    #define PCM_WAVE_DEFAULT_SAMPLE_RATE 8000
#endif

/* See also: http://soundfile.sapp.org/doc/WaveFormat/ */
typedef struct PCM_WAVE
{
    uint32_t ChunkID;           /* "RIFF" 0x52494646 */
    uint32_t ChunkSize;         /* 36 + Subchunk2Size */
    uint32_t Format;            /* "WAVE" 0x57415645 */
    uint32_t Subchunk1ID;       /* "fmt " 0x666d7420 */
    uint32_t Subchunk1Size;     /* 16 for PCM */
    uint16_t AudioFormat;       /* PCM = 1 */
    uint16_t NumChannels;       /* Mono = 1, Stereo = 2, etc. */
    uint32_t SampleRate;        /* 8000, 44100, etc. */
    uint32_t ByteRate;          /* == SampleRate * NumChannels * BitsPerSample/8 */
    uint16_t BlockAlign;        /* == NumChannels * BitsPerSample/8 */
    uint16_t BitsPerSample;     /* 8 bits = 8, 16 bits = 16, etc. */
    uint32_t Subchunk2ID;       /* "data" 0x64617461 */
    uint32_t Subchunk2Size;     /* == NumSamples * NumChannels * BitsPerSample/8 */
} PCM_WAVE;

#ifdef __cplusplus
    class PcmWave
    {
    public:
        typedef std::vector<uint8_t> data_type;

        PcmWave(uint16_t NumChannels_ = PCM_WAVE_DEFAULT_CHANNELS,
                uint16_t BitsPerSample_ = PCM_WAVE_DEFAULT_BITSPERSAMPLE,
                uint32_t SampleRate_ = PCM_WAVE_DEFAULT_SAMPLE_RATE,
                const void *data = NULL, size_t data_size = 0);
        PcmWave(PcmWave&& wave);
        PcmWave& operator=(PcmWave&& wave);

        bool load_from_file(const char *file);
        bool save_to_file(const char *file) const;
#ifdef _WIN32
        bool load_from_file(const wchar_t *file);
        bool save_to_file(const wchar_t *file) const;
#endif

        bool empty() const;
        size_t size() const;
        void resize(size_t data_size);
        void clear();
        void push_8bit(uint8_t byte);
        void push_16bit(int16_t word);
        void reserve(size_t data_size);

        bool is_valid0() const;
        bool is_valid() const;

        bool is_mono() const;
        bool is_stereo() const;
        uint16_t num_channels() const;
        void num_channels(uint16_t ch);

        uint32_t sample_rate() const;
        void sample_rate(uint32_t rate);

        bool mode_8bit() const;
        bool mode_16bit() const;
        uint16_t mode() const;
        void mode(uint16_t bits);

        uint16_t data_unit() const;
        uint32_t num_units() const;

        void get_info(uint16_t *NumChannels_ = NULL,
                      uint16_t *BitsPerSample_ = NULL,
                      uint32_t *SampleRate_ = NULL);
        void set_info(uint16_t NumChannels_ = PCM_WAVE_DEFAULT_CHANNELS,
                      uint16_t BitsPerSample_ = PCM_WAVE_DEFAULT_BITSPERSAMPLE,
                      uint32_t SampleRate_ = PCM_WAVE_DEFAULT_SAMPLE_RATE);
        void update_info();

        void get_data(void *data, size_t data_size);
        void get_data(data_type& data);
        void set_data(const void *data, size_t data_size);
        void set_data(const data_type& data);
              uint8_t& data_8bit(size_t index);
        const uint8_t& data_8bit(size_t index) const;
              int16_t& data_16bit(size_t index);
        const int16_t& data_16bit(size_t index) const;

        bool read_from_fp(std::FILE *fp);
        bool write_to_fp(std::FILE *fp) const;

        float seconds() const;

    protected:
        PCM_WAVE m_wave;
        std::vector<uint8_t> m_data;
    }; // class PcmWave

    inline
    PcmWave::PcmWave(uint16_t NumChannels_,
                     uint16_t BitsPerSample_,
                     uint32_t SampleRate_,
                     const void *data, size_t data_size)
    {
        set_info(NumChannels_, BitsPerSample_, SampleRate_);
        set_data(data, data_size);
    }

    inline
    PcmWave::PcmWave(PcmWave&& wave)
    {
        m_wave = wave.m_wave;
        m_data = std::move(wave.m_data);
    }

    inline
    PcmWave& PcmWave::operator=(PcmWave&& wave)
    {
        m_wave = wave.m_wave;
        m_data = std::move(wave.m_data);
        return *this;
    }

    inline
    bool PcmWave::read_from_fp(std::FILE *fp)
    {
        if (std::fread(&m_wave, sizeof(m_wave), 1, fp))
        {
            if (is_valid0())
            {
                m_data.resize(m_wave.Subchunk2Size);
                if (std::fread(&m_data[0], m_wave.Subchunk2Size, 1, fp))
                {
                    if (uint32_t bits = m_wave.NumChannels * m_wave.BitsPerSample)
                    {
                        return is_valid();
                    }
                }
            }
        }
        clear();
        return false;
    }

    inline
    bool PcmWave::write_to_fp(std::FILE *fp) const
    {
        using namespace std;
        if (!is_valid())
            return false;

        PCM_WAVE wave = m_wave;

        if (std::fwrite(&wave, sizeof(wave), 1, fp) &&
            std::fwrite(&m_data[0], wave.Subchunk2Size, 1, fp))
        {
            return true;
        }

        return false;
    }

    inline
    bool PcmWave::load_from_file(const char *file)
    {
        using namespace std;
        clear();

        bool flag = false;
        if (FILE *fp = fopen(file, "rb"))
        {
            flag = read_from_fp(fp);
            fclose(fp);
        }
        return flag;
    }

    inline
    bool PcmWave::save_to_file(const char *file) const
    {
        using namespace std;
        if (!is_valid())
            return false;

        bool flag = false;
        if (FILE *fp = fopen(file, "wb"))
        {
            flag = write_to_fp(fp);
            fclose(fp);
        }
        return flag;
    }

    #ifdef _WIN32
        inline
        bool PcmWave::load_from_file(const wchar_t *file)
        {
            using namespace std;
            clear();

            bool flag = false;
            if (FILE *fp = _wfopen(file, L"rb"))
            {
                flag = read_from_fp(fp);
                fclose(fp);
            }
            return flag;
        }

        inline
        bool PcmWave::save_to_file(const wchar_t *file) const
        {
            using namespace std;
            if (!is_valid())
                return false;

            bool flag = false;
            if (FILE *fp = _wfopen(file, L"wb"))
            {
                flag = write_to_fp(fp);
                fclose(fp);
            }
            return flag;
        }
    #endif  // def _WIN32

    inline
    void
    PcmWave::get_info(uint16_t *NumChannels_,
                      uint16_t *BitsPerSample_,
                      uint32_t *SampleRate_)
    {
        if (NumChannels_)
            *NumChannels_ = m_wave.NumChannels;
        if (BitsPerSample_)
            *BitsPerSample_ = m_wave.BitsPerSample;
        if (SampleRate_)
            *SampleRate_ = m_wave.SampleRate;
    }

    inline
    void
    PcmWave::set_info(uint16_t NumChannels_,
                      uint16_t BitsPerSample_,
                      uint32_t SampleRate_)
    {
        m_wave.ChunkID = 0x46464952;
        m_wave.Format = 0x45564157;
        m_wave.Subchunk1ID = 0x20746d66;
        m_wave.Subchunk2ID = 0x61746164;
        m_wave.Subchunk1Size = 16;
        m_wave.AudioFormat = 1;
        m_wave.NumChannels = NumChannels_;
        m_wave.SampleRate = SampleRate_;
        m_wave.BitsPerSample = BitsPerSample_;
        m_wave.Subchunk2Size = 0;  // for set_data
        m_wave.ChunkSize = 0;  // for set_data
        update_info();
    }

    inline
    void PcmWave::clear()
    {
        set_info();
        set_data(NULL, 0);
    }

    inline
    void PcmWave::get_data(void *data, size_t data_size)
    {
        if (!data || !data_size || data == &m_data[0])
            return;
        if (data_size > m_data.size())
            data_size = m_data.size();
        memcpy(data, &m_data[0], data_size);
    }

    inline
    void PcmWave::get_data(data_type& data)
    {
        data = m_data;
    }

    inline
    void PcmWave::set_data(const void *data, size_t data_size)
    {
        if (&m_data[0] != data)
        {
            if (!data || !data_size)
            {
                m_data.clear();
                data_size = 0;
            }
            else
            {
                m_data.assign((const uint8_t *)data, (const uint8_t *)data + data_size);
            }
        }

        update_info();
    }

    inline
    void PcmWave::resize(size_t data_size)
    {
        m_data.resize(data_size);
        update_info();
    }

    inline
    void PcmWave::set_data(const data_type& data)
    {
        set_data(&data[0], data.size());
    }

    inline
    bool PcmWave::is_valid() const
    {
        if (!is_valid0())
            return false;

        if (empty())
        {
            assert(0);
            return false;
        }
        if (m_wave.Subchunk2Size != m_data.size())
        {
            assert(0);
            return false;
        }
        if (0 && m_wave.ChunkSize != 36 + m_wave.Subchunk2Size)
        {
            assert(0);
            return false;
        }
        return true;
    }

    inline
    bool PcmWave::is_valid0() const
    {
        if (m_wave.ChunkID != 0x46464952)
        {
            assert(0);
            return false;
        }
        if (m_wave.Format != 0x45564157)
        {
            assert(0);
            return false;
        }
        if (m_wave.Subchunk1ID != 0x20746d66)
        {
            assert(0);
            return false;
        }
        if (m_wave.Subchunk2ID != 0x61746164)
        {
            assert(0);
            return false;
        }
        if (m_wave.Subchunk1Size != 16)
        {
            assert(0);
            return false;
        }
        if (m_wave.AudioFormat != 1)
        {
            assert(0);
            return false;
        }
        if (m_wave.ByteRate != m_wave.SampleRate * m_wave.NumChannels * m_wave.BitsPerSample / 8)
        {
            assert(0);
            return false;
        }
        if (m_wave.BlockAlign != m_wave.NumChannels * m_wave.BitsPerSample / 8)
        {
            assert(0);
            return false;
        }
        return true;
    }

    inline
    bool PcmWave::empty() const
    {
        return size() == 0;
    }

    inline
    size_t PcmWave::size() const
    {
        return m_data.size();
    }

    inline
    void PcmWave::push_8bit(uint8_t byte)
    {
        m_data.push_back(byte);
    }

    inline
    void PcmWave::push_16bit(int16_t word)
    {
        uint16_t w = word;
        m_data.insert(m_data.end(), (uint8_t *)&w, ((uint8_t *)&w) + 2);
    }

    inline
    void PcmWave::update_info()
    {
        m_wave.ByteRate = m_wave.SampleRate * m_wave.NumChannels * m_wave.BitsPerSample / 8;
        m_wave.BlockAlign = m_wave.NumChannels * m_wave.BitsPerSample / 8;

        m_wave.Subchunk2Size = m_data.size();
        m_wave.ChunkSize = 36 + m_wave.Subchunk2Size;
    }

    inline
    uint16_t PcmWave::num_channels() const
    {
        return m_wave.NumChannels;
    }

    inline
    void PcmWave::num_channels(uint16_t ch)
    {
        m_wave.NumChannels = ch;
    }

    inline
    bool PcmWave::is_mono() const
    {
        return num_channels() == 1;
    }

    inline
    bool PcmWave::is_stereo() const
    {
        return num_channels() == 2;
    }

    inline
    uint32_t PcmWave::sample_rate() const
    {
        return m_wave.SampleRate;
    }

    inline
    void PcmWave::sample_rate(uint32_t rate)
    {
        m_wave.SampleRate = rate;
    }

    inline
    bool PcmWave::mode_8bit() const
    {
        return mode() == 8;
    }

    inline
    bool PcmWave::mode_16bit() const
    {
        return mode() == 16;
    }

    inline
    uint16_t PcmWave::mode() const
    {
        return m_wave.BitsPerSample;
    }

    inline
    void PcmWave::mode(uint16_t bits)
    {
        m_wave.BitsPerSample = bits;
    }

    inline
    uint16_t PcmWave::data_unit() const
    {
        return num_channels() * mode() / 8;
    }

    inline
    uint32_t PcmWave::num_units() const
    {
        return size() / data_unit();
    }

    inline
    void PcmWave::reserve(size_t data_size)
    {
        m_data.reserve(data_size);
    }

    inline
    uint8_t& PcmWave::data_8bit(size_t index)
    {
        return reinterpret_cast<uint8_t&>(m_data[index * sizeof(uint8_t)]);
    }

    inline
    const uint8_t& PcmWave::data_8bit(size_t index) const
    {
        return reinterpret_cast<const uint8_t&>(m_data[index * sizeof(uint8_t)]);
    }

    inline
    int16_t& PcmWave::data_16bit(size_t index)
    {
        return reinterpret_cast<int16_t&>(m_data[index * sizeof(int16_t)]);
    }

    inline
    const int16_t& PcmWave::data_16bit(size_t index) const
    {
        return reinterpret_cast<const int16_t&>(m_data[index * sizeof(int16_t)]);
    }

    inline
    float PcmWave::seconds() const
    {
        return float(m_wave.Subchunk2Size) / m_wave.NumChannels /
               m_wave.SampleRate / (m_wave.BitsPerSample / 8); 
    }
#endif  /* C++ */

#endif  /* ndef PCM_WAVE_HPP_ */
