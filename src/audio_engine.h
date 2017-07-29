#pragma once

struct Sound
{
    ALuint buffer;

};

void init_audio_system();
void uninit_audio_system();
void load_sound(Sound* sound, const char* path);
void play_music(Sound* sound, float gain = 1.0);
void play_sound(Sound* sound, float gain = 1.0);
