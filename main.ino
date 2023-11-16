// Hayden Feddock
// 11/3/2023

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"

#define TFT_CS 9
#define TFT_DC 10
#define TFT_RST 8

/******************** GLOBALS ********************/
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

/******************** CLASSES ********************/
class Song {
  private:
    String filename;
    unsigned long filePosition;
    String* chordBuffer;
    String* lyricBuffer;
    int beatCount;

  public:
    Song(String file) : filename(file), filePosition(0), chordBuffer(nullptr), lyricBuffer(nullptr), beatCount(0) {}

    ~Song() {
      if (chordBuffer != nullptr) {
        delete [] chordBuffer;
      }
      if (lyricBuffer != nullptr) {
        delete [] lyricBuffer;
      }
    }

    bool loadNextNBeats(int n) {

      if (!SD.begin(TFT_CS)) {
        Serial.println(F("Initialization failed!"));
        return;
      }
      Serial.println(F("Initialization done."));

      File file = SD.open(filename);

      if (!file) {
        Serial.println(F("File Error"));
        return false;
      }

      file.seek(filePosition);

      if (chordBuffer != nullptr) {
        delete [] chordBuffer;
      }
      if (lyricBuffer != nullptr) {
        delete [] lyricBuffer;
      }

      chordBuffer = new String[n];
      lyricBuffer = new String[n];
      beatCount = n;

      for (int i=0; i<n; i++) {
        if (file.available()) {
          chordBuffer[i] = file.readStringUntil(',');
          // Serial.println(chordName);
          lyricBuffer[i] = file.readStringUntil('\n');
          // Serial.println(lyric);
        } else {
          beatCount = i;
          break;
        }
      }

      file.close();
      return true;
    }

    int getBeatCount() { return beatCount; }

    String* getChordBuffer() {
      return chordBuffer;
    }

    String* getLyricBuffer() {
      return lyricBuffer;
    }
};


/******************** MAIN ********************/
void setup() {

  Serial.begin(9600);
  Serial.println(F("Testing"));

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(HX8357_BLACK);

  // TODO: Add main menu and song selection here

  String songFilename = "test.csv";
  Song song(songFilename);

  // TODO: Include metadata with bpm here
  int bpm = 67;

  int n = 8;
  int d = 60000/bpm; // delay[ms] = n[beats] * 1/(beats/min * min/sec * sec/ms)
  // while (n == 8) {
    song.loadNextNBeats(8);
    printBeats(song.getChordBuffer(), song.getLyricBuffer(), song.getBeatCount(), HX8357_WHITE);
    n = song.getBeatCount();
    for (int i=0; i<n; i++) {
      printBeat(song.getChordBuffer(), song.getLyricBuffer(), song.getBeatCount(), i, HX8357_YELLOW);
      delay(d);
    }
  // }



}

void loop() {
  
}

bool printBeats(String* chordBuffer, String* lyricBuffer, uint8_t beatCount, uint16_t color) {
  tft.fillScreen(HX8357_BLACK); // TODO: Remove with sectional covering
  for (uint8_t i=0; i<beatCount; i++) {
    printBeat(chordBuffer, lyricBuffer, beatCount, i, color);
  }
}

bool printBeat(String* chordBuffer, String* lyricBuffer, uint8_t beatCount, uint8_t beatNumber, uint16_t color) {
  //tft.fillScreen(HX8357_BLACK); // TODO: Remove with sectional covering
  uint16_t interval = tft.width()/beatCount;

  if (beatNumber >= beatCount) return false;

  // Print out the chord
  uint16_t x = beatNumber*interval;
  tft.setCursor(x, 0);
  tft.setTextColor(color);
  tft.setTextSize(2); // Character Size 2: 10 x 14
  tft.print(chordBuffer[beatNumber]);

  // Print out the lyric
  tft.setCursor(x, 16);
  tft.print(lyricBuffer[beatNumber]);
}