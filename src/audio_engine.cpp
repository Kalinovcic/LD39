
ALCdevice* audio_device;
ALCcontext *audio_context;

constexpr int MAX_SOURCES = 16;
ALuint sources[MAX_SOURCES];
int next_source = 1;

static void check_for_openal_error(const char* where)
{
    auto error = alGetError();
    if (error != AL_NO_ERROR)
    {
        critical("OpenAL error %s: %d\n", where, (int) error);
    }
}

void init_audio_system()
{
    audio_device = alcOpenDevice(NULL);
    if (!audio_device)
    {
        critical("OpenAL failed to open an audio device!\n");
    }

    audio_context = alcCreateContext(audio_device, NULL);
    if (!alcMakeContextCurrent(audio_context))
    {
        critical("OpenAL failed to make context current!\n");
    }
    check_for_openal_error("after creating the context");

    alGenSources(MAX_SOURCES, sources);
    check_for_openal_error("after creating the sources");

    ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
    alListener3f(AL_POSITION, 0, 0, 1.0f);
}

void uninit_audio_system()
{
    alDeleteSources(MAX_SOURCES, sources);
    alcDestroyContext(audio_context);
    alcCloseDevice(audio_device);
}

void load_sound(Sound* sound, const char* path)
{
    SDL_AudioSpec spec;
    uint8* data;
    uint32 length;
    auto bytes = read_all_bytes_from_file(path, false);;
    if (!SDL_LoadWAV_RW(SDL_RWFromMem(bytes.data, bytes.count), 1, &spec, &data, &length))
    {
        critical("SDL2 failed to load WAV \"%s\".\n", path);
    }

    if (spec.format != AUDIO_S16)
    {
        critical("Unsupported audio format in WAV \"%s\": %d\n", path, spec.format);
    }

    alGenBuffers(1, &sound->buffer);
    check_for_openal_error("after creating a buffer");

    ALenum format;
    if (spec.channels == 1)
        format = AL_FORMAT_MONO16;
    else if (spec.channels == 2)
        format = AL_FORMAT_STEREO16;
    else
        critical("Unsupported number of channels in WAV \"%s\": %d\n", path, spec.channels);

    alBufferData(sound->buffer, format, data, length, spec.freq);
    check_for_openal_error("after setting buffer data");
}

void play_music(Sound* sound, float gain)
{
    ALuint source = sources[0];

    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, gain);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_TRUE);

    alSourcei(source, AL_BUFFER, sound->buffer);
    alSourcePlay(source);
}

void play_sound(Sound* sound, float gain)
{
    ALuint source = sources[next_source];
    if (++next_source >= MAX_SOURCES)
        next_source = 1;

    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, gain);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_FALSE);

    alSourcei(source, AL_BUFFER, sound->buffer);
    alSourcePlay(source);
}
