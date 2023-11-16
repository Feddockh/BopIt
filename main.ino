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

  String songFilename = "test.csv";
  Song song(songFilename);

  song.loadNextNBeats(8);
  printBeats(song.getChordBuffer(), song.getLyricBuffer(), song.getBeatCount());
}

void loop() {
  
}

bool printBeats(String* chordBuffer, String* lyricBuffer, int beatCount) {
  tft.fillScreen(HX8357_BLACK);
  int interval = tft.width()/beatCount;

  // Print out the line of chords
  for (int i=0; i*interval<tft.width(); i++) {
    tft.setCursor(i*interval, 0);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2); // Character Size 2: 10 x 14
    tft.print(chordBuffer[i]);
  }

  // Print of the line of lyrics
  for (int i=0; i*interval<tft.width(); i++) {
    tft.setCursor(i*interval, 16);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2); // Character Size 2: 10 x 14
    tft.print(lyricBuffer[i]);
  }
}