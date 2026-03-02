#pragma once

void audio_init();
int play_mp3(char *path, bool loop);
void seek_mp3(float time);
void stop_mp3();
void toggle_playback_mp3();