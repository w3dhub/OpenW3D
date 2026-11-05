#include "AudibleSound.h"
#include "WWAudio.h"

#include <iostream>

namespace {

class TestAudioClass final : public WWAudioClass
{
public:
    TestAudioClass()
        : WWAudioClass(true), _driverName("Lifetime test")
    {
    }

    void Initialize(bool = true, int = 16, int = 44100) override {}
    void Initialize(const char *) override {}
    void Shutdown() override {}
    const StringClass &Get_3D_Driver_Name() const override { return _driverName; }
    DRIVER_TYPE_2D Open_2D_Device(bool, int, int) override { return DRIVER2D_ERROR; }
    int Get_3D_Device_Count() const override { return 0; }
    bool Get_3D_Device(int, const char **info) override
    {
        if (info) {
            *info = _driverName.Peek_Buffer();
        }
        return false;
    }
    bool Select_3D_Device(const char *) override { return false; }
    void Set_Speaker_Type(int) override {}
    int Get_Speaker_Type() const override { return W3D_3D_2_SPEAKER; }
    float Get_Effects_Level() override { return 0.0F; }
    void Flush_Cache() override {}
    int Get_2D_Sample_Count() const override { return 0; }
    int Get_3D_Sample_Count() const override { return 0; }
    AudibleSoundClass *Peek_2D_Sample(int) override { return nullptr; }
    AudibleSoundClass *Peek_3D_Sample(int) override { return nullptr; }

    int Completed_Sound_Count() const { return m_CompletedSounds.Count(); }

protected:
    bool Validate_3D_Sound_Buffer(SoundBufferClass *) override { return false; }
    SoundHandleClass *Get_2D_Handle(const AudibleSoundClass &, bool) override { return nullptr; }
    SoundHandleClass *Get_3D_Handle(const Sound3DClass &) override { return nullptr; }
    SoundBufferClass *Get_Sound_Buffer(const char *, bool) override { return nullptr; }

private:
    StringClass _driverName;
};

} // namespace

int main()
{
    TestAudioClass audio;
    AudibleSoundClass *sound = new AudibleSoundClass;

    audio.Add_To_Playlist(sound);
    if (audio.Get_Playlist_Count() != 1) {
        std::cerr << "Sound was not added to the playlist exactly once.\n";
        return 1;
    }

    if (!audio.Remove_From_Playlist(sound) || !audio.Remove_From_Playlist(sound)) {
        std::cerr << "Repeated removal did not find the queued sound.\n";
        return 1;
    }

    if (audio.Completed_Sound_Count() != 1) {
        std::cerr << "A sound was queued for deferred release more than once.\n";
        return 1;
    }

    // Leave the playlist as the final owner, then drain its single deferred
    // release. Duplicate completed entries would dereference a freed object.
    sound->Release_Ref();
    audio.Free_Completed_Sounds();

    if (audio.Get_Playlist_Count() != 0 || audio.Completed_Sound_Count() != 0) {
        std::cerr << "Completed-sound cleanup left stale ownership state.\n";
        return 1;
    }

    return 0;
}
