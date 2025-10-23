#include <raylib.h>
#include <flecs.h>
#include <string>

#include "audio.h"

struct MusicFile
{
  std::string fname;
};

struct SoundFile
{
  std::string fname;
};


struct audio
{
  audio(flecs::world& ecs)
  {
    ecs.module<audio>();

    ecs.component<MusicFile>()
      .on_set([&](flecs::entity ent, const MusicFile& tf)
      {
        Music mus = LoadMusicStream(tf.fname.c_str());
        ent.set(mus);
      })
      .member<std::string>("filename")
      .add(flecs::OnInstantiate, flecs::Inherit);

    ecs.component<SoundFile>()
      .on_set([&](flecs::entity ent, const SoundFile& tf)
      {
        Sound snd = LoadSound(tf.fname.c_str());
        ent.set(snd);
      })
      .member<std::string>("filename")
      .add(flecs::OnInstantiate, flecs::Inherit);

      ecs.component<StepSound>();
      ecs.component<DoorSound>();
      ecs.component<ClickSound>();
      ecs.component<ShootSound>();
      ecs.component<HitSound>();
      ecs.component<TriggerSound>();
      ecs.component<LockedSound>();
      ecs.component<DialogSound>();
  }
};

void play_snd(flecs::entity e)
{
  if (e)
    e.get([&](const Sound& snd)
    {
      PlaySound(snd);
    });
}

void stop_snd(flecs::entity e)
{
  if (e)
    e.get([&](const Sound& snd)
    {
      StopSound(snd);
    });
}


void register_audio(flecs::world& ecs)
{
  ecs.import<audio>();
}

/*
#define MAX_SAMPLES               512
#define MAX_SAMPLES_PER_UPDATE   4096

uint8_t bytebeat(int t)
{
    return 10*(t>>7|t|t>>6)+4*(t&t>>13|t>>6);
}

static int toffs = 0;

template<typename T>
T clamp(T v, T lo, T hi)
{
    return v < lo ? lo : v > hi ? hi : v;
}

constexpr int sampleRate = 44100;

uint8_t saw(float freq, int t)
{
    return t * (freq / float(sampleRate)) * 255;
}

uint8_t sine(float freq, int t)
{
    return (sinf(float(t) * (freq / float(sampleRate)) * (PI * 2.f)) + 1.f) * 0.5f * 255;
}

static const char* notes[] =
{
    "C5",
    "",
    "C5",
    "",

    "C5",
    "",
    "C5",
    "",

    "",
    "",
    "",
    "",

    "G4#",
    "",
    "",
    "",

    "",
    "",
    "A4#",
    "",

    "",
    "",
    "",
    "",

    "C5",
    "",
    "",
    "",

    "A4#",
    "",
    "C5",
    "C5",

    "C5",
    "C5",
    "",
    "",

    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

const int bpm = 130;
const int lpb = 8; // lines per beat

int frame_to_note_idx(int frame)
{
    return int(float(frame) / float(sampleRate) * (bpm / 60.f) * lpb) % (sizeof(notes) / sizeof(notes[0]));
}

int note_to_freq(const char* note)
{
    // letter intervals
    constexpr int intervals[7] = {0, 2, 3, 5, 7, 8, 10};
    constexpr float basis = 440.f; // A440
    const char noteLetter = note[0];
    if (noteLetter == '\0')
        return 0;
    const char noteOctave = note[1];
    // we only recognize sharps
    const bool sharp = note[2] != '\0' && note[2] == '#';
    const int diffFromBasis = intervals[noteLetter - 'A'] + (noteOctave - '4') * 12 + (sharp ? 1 : 0);

    return float(basis) * powf(2.f, float(diffFromBasis) / 12.f);
}

static int lastFreq = 0;
static int lastFreqTick = 0;

template<bool doDelta>
void audioCallback(void *buffer, unsigned int frames)
{
    uint8_t *d = (uint8_t*)buffer;

    int prevFrame = 0;
    for (size_t i = 0; i < frames; ++i)
    {
      /*
        int curFrame = bytebeat(toffs + i);
        if constexpr (doDelta)
        {
            int delta = curFrame - prevFrame + 128;
            d[i] = (uint8_t)clamp(delta, 0, 255);
            prevFrame = curFrame;
        }
        else
        {
            d[i] = (uint8_t)clamp(curFrame, 0, 255);
        }
        *//*
        int ct = toffs + i;
        int noteIdx = frame_to_note_idx(ct);
        int curFreq = note_to_freq(notes[noteIdx]);
        uint8_t curSaw = saw(curFreq, ct);
        const int relTicks = sampleRate / 10;
        if (curFreq != 0)
        {
          lastFreq = curFreq;
          lastFreqTick = ct;
        }
        if (lastFreq != 0 && lastFreqTick + relTicks >= ct)
        {
          float vol = 0.7f * (1.f - float(ct - lastFreqTick) / float(relTicks));
          d[i] = std::min(int(saw(lastFreq, toffs + i) * vol), 255);
        }
    }
    toffs += frames;
}


void init_audio()
{
  SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);
  AudioStream as = LoadAudioStream(sampleRate, 8, 1);
  SetAudioStreamCallback(as, audioCallback<true>);
  PlayAudioStream(as);
  uint8_t *buffer = new uint8_t[MAX_SAMPLES_PER_UPDATE];
  {
      audioCallback<false>(buffer, MAX_SAMPLES_PER_UPDATE);
      FILE* f = fopen("bytebeat.raw", "wb");
      fwrite(buffer, MAX_SAMPLES_PER_UPDATE, 1, f);
      fclose(f);
  }

  {
      toffs = 0;
      audioCallback<true>(buffer, MAX_SAMPLES_PER_UPDATE);
      FILE* f = fopen("bytebeat_filt.raw", "wb");
      fwrite(buffer, MAX_SAMPLES_PER_UPDATE, 1, f);
      fclose(f);
  }
  delete[] buffer;
}
*/

