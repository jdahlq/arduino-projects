// A music player that supports multiple playlists.
//
// The SD card should be FAT32 formatted.
// The name of each WAV file should should be 8 chars long, not including the .WAV suffix. It
// should begin with a 2 digit playlist designator
//  00_00.WAV

#include "Arduino.h"
#include "MusicPlayer.h"

// Define macro to put error messages in flash memory.
#define error(msg) error_P(PSTR(msg))

namespace {

  // Print error message and halt if SD I/O error, great for debugging!
  // TODO: Make this reset rather than halt.
  /*
  void sdErrorCheck(void) {
    if (!card.errorCode()) return;
    PgmPrint("\r\nSD I/O error: ");
    Serial.print(card.errorCode(), HEX);
    PgmPrint(", ");
    Serial.println(card.errorData(), HEX);
    while(1);
  }
  */

  // Print error message and halt.
  // TODO: Make this reset rather than halt.
  void error_P(const char *str) {
    PgmPrint("Error: ");
    SerialPrint_P(str);
// sdErrorCheck();
    while(1);
  }

}  // namespace 

MusicPlayer::MusicPlayer() {
}

bool MusicPlayer::Init() {
  putstring_nl("\nWave test!");  // say we woke up!
  
  putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(FreeRam());

  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
    error("Card init. failed!");  // Something went wrong, lets print out why
    return false;
  }
  
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
  
  // Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {   // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                           // we found one, lets bail
  }
  if (part == 5) {                     // if we ended up not finding one  :(
    error("No valid FAT partition!");  // Something went wrong, lets print out why
    return false;
  }
  
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(), DEC);     // FAT16 or FAT32?
  
  // Try to open the root directory
  if (!root.openRoot(vol)) {
    error("Can't open root dir!");
    return false;
  }
  
  // Whew! We got past the tough parts.
  putstring_nl("Files found (* = fragmented):");

  // Print out all of the files in all the directories.
  root.ls(LS_R | LS_FLAG_FRAGMENTED);

  // Ensure there are playlists.
  root.rewind();
  if (!root.readDir(dirBuf)) {
    error("Root dir is empty");
    return false;
  }
  root.rewind();

  // Setup the first playlist.
  NextPlaylist();
}

void MusicPlayer::Play() {
  if (wave.isplaying) return;
  NextSong();
}

void MusicPlayer::Stop() {
  wave.stop();
}

void MusicPlayer::NextPlaylist() {
  const uint32_t start_position = root.readPosition();  
  bool first_iteration = true;
  for (;;) {
    if (first_iteration) {
      first_iteration = false;
    } else if (root.readPosition() == start_position) {
      error("Found files but no playlists");
      return;
    }    
      
    if (!root.readDir(dirBuf)) {
      root.rewind();
      continue;
    }

    // Skip it if not a subdirectory (playlist).
    if (!DIR_IS_SUBDIR(dirBuf)) continue;

    if (!playlist.open(vol, dirBuf)) {
      error("playlist.open failed");
      continue;
    }
    putstring("Playlist: ");
    printEntryName(dirBuf);
    Serial.println();

    // Ensure there are songs.
    playlist.rewind();
    if (!playlist.readDir(dirBuf)) {
      putstring("Playlist has no songs.");
      continue;
    }
    playlist.rewind();

    if (!NextSong()) continue;
    return;
  }
}

bool MusicPlayer::NextSong() {
  const uint32_t start_position = playlist.readPosition();
  bool first_iteration = true;
  // Skip files that are not WAV.
  for (;;) {
    if (first_iteration) {
      first_iteration = false;
    } else if (playlist.readPosition() == start_position) {
      putstring("Found files but no WAVs");
      return false;
    }

    if (!playlist.readDir(dirBuf)) {
      playlist.rewind();
      continue;
    }

    if (DIR_IS_SUBDIR(dirBuf) ||
        !strncmp_P((char*)&dirBuf.name[strlen((char*)dirBuf.name)-3], PSTR("WAV"), 3)) {
      continue;
    }

    if (!song.open(vol, dirBuf)) {
      putstring("song.open failed");
      continue;
    }

    putstring("Playing ");
    printEntryName(dirBuf);
    if (!wave.create(song)) {
      putstring(" Not a valid WAV");
      continue;
    }

    Serial.println();
    wave.play();

    return true;
  }
}

void MusicPlayer::SetPlaybackRate(long rate) {
  wave.setSampleRate(rate);  
}

long MusicPlayer::GetNormalPlaybackRate() {
  return wave.dwSamplesPerSec;
}
