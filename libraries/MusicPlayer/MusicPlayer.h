#ifndef MusicPlayer_h
#define MusicPlayer_h

#include "Arduino.h"
#include "WaveHC.h"
#include "WaveUtil.h"

class MusicPlayer {
 public:
  MusicPlayer();
  // Init must be called before using MusicPlayer! Returns false on error.
  bool Init();
  void Play();
  void Stop();
  void NextPlaylist();
  bool NextSong();
  long GetNormalPlaybackRate();
  void SetPlaybackRate(long rate);

  // Checks to see if the current song has ended, and, if so, plays the next one.
  // TODO: Need to figure out how to listen to the WaveHC timer so I don't need this dumb method.
  void KeepPlaying();

 private:
  SdReader card;    // This object holds the information for the card
  FatVolume vol;    // This holds the information for the partition on the card
  FatReader root;   // This holds the information for the volumes root directory
  FatReader playlist;  // Current playlist dir
  dir_t dirBuf;     // Buffer for directory reads
  FatReader song;   // This object represent the WAV file
  WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time
};

#endif MusicPlayer_h
