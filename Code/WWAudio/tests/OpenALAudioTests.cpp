#include "AudibleSound.h"
#include "Sound3D.h"
#include "WWAudio.h"
#include "openal/FFMpegBuffer.h"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace {
template<typename T>
struct ReleaseRef
{
    void operator()(T *value) const
    {
        if (value) {
            value->Release_Ref();
        }
    }
};

class TemporaryWav
{
public:
    TemporaryWav()
    {
        const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
        _path = std::filesystem::temp_directory_path() /
                ("openw3d-openal-" + std::to_string(suffix) + ".wav");
    }

    ~TemporaryWav()
    {
        std::error_code error;
        std::filesystem::remove(_path, error);
    }

    bool write()
    {
        constexpr std::uint32_t sampleRate = 22050;
        constexpr std::uint16_t channels = 1;
        constexpr std::uint16_t bitsPerSample = 16;
        constexpr std::uint32_t sampleCount = sampleRate;
        constexpr std::uint32_t bytesPerSample = bitsPerSample / 8;
        constexpr std::uint32_t dataSize = sampleCount * bytesPerSample;

        std::ofstream stream(_path, std::ios::binary | std::ios::trunc);
        if (!stream) {
            return false;
        }

        const auto write16 = [&stream](std::uint16_t value) {
            const char bytes[] = {
                static_cast<char>(value & 0xff),
                static_cast<char>((value >> 8) & 0xff),
            };
            stream.write(bytes, sizeof(bytes));
        };
        const auto write32 = [&stream](std::uint32_t value) {
            const char bytes[] = {
                static_cast<char>(value & 0xff),
                static_cast<char>((value >> 8) & 0xff),
                static_cast<char>((value >> 16) & 0xff),
                static_cast<char>((value >> 24) & 0xff),
            };
            stream.write(bytes, sizeof(bytes));
        };

        stream.write("RIFF", 4);
        write32(36 + dataSize);
        stream.write("WAVE", 4);
        stream.write("fmt ", 4);
        write32(16);
        write16(1);
        write16(channels);
        write32(sampleRate);
        write32(sampleRate * channels * bytesPerSample);
        write16(channels * bytesPerSample);
        write16(bitsPerSample);
        stream.write("data", 4);
        write32(dataSize);

        for (std::uint32_t index = 0; index < sampleCount; ++index) {
            const std::int16_t sample = ((index / 55) % 2) ? 4096 : -4096;
            write16(static_cast<std::uint16_t>(sample));
        }

        return stream.good();
    }

    const std::filesystem::path &path() const
    {
        return _path;
    }

private:
    std::filesystem::path _path;
};

bool expect(bool condition, const char *message, int &failures)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
    return condition;
}
} // namespace

int main()
{
    int failures = 0;
    TemporaryWav fixture;
    if (!expect(fixture.write(), "could not create the generated PCM WAV fixture", failures)) {
        return 1;
    }

    const std::string filename = fixture.path().string();
    std::error_code sizeError;
    const std::uintmax_t fixtureSize = std::filesystem::file_size(fixture.path(), sizeError);
    expect(!sizeError && fixtureSize > DEF_MAX_2D_BUFFER_SIZE &&
               fixtureSize <= DEF_MAX_3D_BUFFER_SIZE * 2,
           "generated WAV did not straddle the 2D-streaming and 3D-static thresholds",
           failures);

    {
        std::unique_ptr<FFMpegBufferClass, ReleaseRef<FFMpegBufferClass>> decoded(
            new FFMpegBufferClass);
        const bool loaded = decoded->Load_From_File(filename.c_str(), false);
        expect(loaded, "FFmpeg could not decode the generated PCM WAV", failures);
        if (loaded) {
            expect(decoded->Get_Channels() == 1, "decoded WAV was not mono", failures);
            expect(decoded->Get_Rate() == 22050, "decoded WAV sample rate changed", failures);
            expect(decoded->Get_Bits() == 16, "decoded WAV was not converted to PCM16", failures);
            expect(decoded->Get_Duration() >= 990 && decoded->Get_Duration() <= 1010,
                   "decoded WAV duration was not approximately one second",
                   failures);
            expect(decoded->Get_Raw_Length() > 0, "decoded WAV had no PCM payload", failures);
        }
    }

    {
        std::unique_ptr<FFMpegBufferClass, ReleaseRef<FFMpegBufferClass>> streaming(
            new FFMpegBufferClass);
        const bool loaded = streaming->Load_From_File(filename.c_str(), true);
        expect(loaded, "FFmpeg could not open the generated WAV for streaming", failures);
        if (loaded) {
            expect(streaming->Is_Streaming(),
                   "FFmpeg did not mark the generated WAV as streaming",
                   failures);
            expect(streaming->Get_Raw_Length() == 0,
                   "streaming WAV unexpectedly predecoded a static PCM payload",
                   failures);
            expect(streaming->Get_Channels() == 1,
                   "streaming WAV metadata was not mono",
                   failures);
        }
    }

    std::unique_ptr<WWAudioClass> audio(WWAudioClass::Create_Instance());
    if (!expect(audio != nullptr, "WWAudioClass::Create_Instance returned null", failures)) {
        return 1;
    }
    audio->Initialize();

    expect(WWAudioClass::Get_Instance() == audio.get(),
           "WWAudio singleton did not reference the OpenAL instance",
           failures);
    expect(std::strcmp(audio->Get_3D_Driver_Name().Peek_Buffer(), "OpenAL 3D Audio") == 0,
           "configured WWAudio backend was not OpenAL",
           failures);
    expect(audio->Get_2D_Sample_Count() == DEF_2D_SAMPLE_COUNT,
           "OpenAL did not allocate the expected 2D source pool",
           failures);
    expect(audio->Get_3D_Sample_Count() == DEF_3D_SAMPLE_COUNT,
           "OpenAL did not allocate the expected 3D source pool",
           failures);

    {
        std::unique_ptr<AudibleSoundClass, ReleaseRef<AudibleSoundClass>> sound(
            audio->Create_Sound_Effect(filename.c_str()));
        if (expect(sound != nullptr, "OpenAL could not create a 2D sound", failures)) {
            expect(sound->Play(), "OpenAL 2D Play returned false", failures);
            expect(sound->Is_Playing(), "OpenAL 2D sound did not enter playing state", failures);

            bool ownsSource = false;
            for (int index = 0; index < audio->Get_2D_Sample_Count(); ++index) {
                ownsSource |= audio->Peek_2D_Sample(index) == sound.get();
            }
            expect(ownsSource, "OpenAL 2D sound did not acquire a real source", failures);
            expect(sound->Stop(), "OpenAL 2D Stop returned false", failures);
            expect(!sound->Is_Playing(), "OpenAL 2D sound did not stop", failures);
            expect(sound->Play(), "OpenAL 2D replay returned false", failures);
            expect(sound->Is_Playing(), "OpenAL 2D sound did not resume on replay", failures);
            expect(sound->Stop(), "OpenAL 2D second Stop returned false", failures);
            expect(!sound->Is_Playing(), "OpenAL 2D sound did not stop after replay", failures);
        }
    }

    {
        std::unique_ptr<Sound3DClass, ReleaseRef<Sound3DClass>> sound(
            audio->Create_3D_Sound(filename.c_str(), CLASSID_3D));
        if (expect(sound != nullptr, "OpenAL could not create a 3D sound", failures)) {
            expect(sound->Get_Class_ID() == CLASSID_3D,
                   "mono PCM WAV fell back to pseudo-3D",
                   failures);
            sound->Cull_Sound(false);
            expect(sound->Play(), "OpenAL 3D Play returned false", failures);
            expect(sound->Is_Playing(), "OpenAL 3D sound did not enter playing state", failures);

            bool ownsSource = false;
            for (int index = 0; index < audio->Get_3D_Sample_Count(); ++index) {
                ownsSource |= audio->Peek_3D_Sample(index) == sound.get();
            }
            expect(ownsSource, "OpenAL 3D sound did not acquire a real source", failures);
            expect(sound->Stop(), "OpenAL 3D Stop returned false", failures);
            expect(!sound->Is_Playing(), "OpenAL 3D sound did not stop", failures);
        }
    }

    audio.reset();
    expect(WWAudioClass::Get_Instance() == nullptr,
           "OpenAL destruction did not clear the WWAudio singleton",
           failures);

    if (failures != 0) {
        std::cerr << failures << " OpenAL regression assertion(s) failed.\n";
        return 1;
    }

    std::cout << "OpenAL backend initialization, decode, 2D, 3D, and teardown passed.\n";
    return 0;
}
