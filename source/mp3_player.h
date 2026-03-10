#pragma once

#define AMP_DECAY 0.05f
#define AMP_I_DECAY (1.f - AMP_DECAY)

#define POWER_THRESH_MULTIPLIER 1.f

extern volatile float amplitude;

void audio_init();
int play_mp3(char *path, bool loop);
void seek_mp3(float time);
void stop_mp3();
void toggle_playback_mp3();