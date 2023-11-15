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
class Chord {
  private:
    String chordName;

  public:
    Chord() {};
    Chord(String name) : chordName(name) {}
    String getChordName() { return chordName; }
    String getFingerPositions() {
      if (chordName == "G Major") return "320003";
      if (chordName == "C Major") return "x32010";
      return "Unknown";
    }
};

class Beat {
  private:
    Chord chord;
    String lyric;

  public:
    Beat() {};
    Beat(String chordName, String text) : chord(Chord(chordName)), lyric(text) {}
    Chord getChord() { return chord; }
    String getLyric() { return lyric; }
};

class Song {
  private:
    String filename;
    unsigned long filePosition;
    Beat* beatBuffer;
    int beatCount;

  public:
    Song(String file) : filename(file), filePosition(0), beatBuffer(nullptr), beatCount(0) {}

    ~Song() {
      if (beatBuffer != nullptr) {
        delete [] beatBuffer;
      }
    }

    bool loadNextNBeats(int n) {

      if (!SD.begin(TFT_CS)) {
        Serial.println("Initialization failed!");
        return;
      }
      Serial.println("Initialization done.");

      File file = SD.open(filename);

      if (!file) {
        Serial.println("File Error");
        return false;
      }

      file.seek(filePosition);

      if (beatBuffer != nullptr) {
        delete [] beatBuffer;
      }

      beatBuffer = new Beat[n];
      beatCount = n;

      String chordName;
      String lyric;
      for (int i=0; i<n; i++) {
        if (file.available()) {
          chordName = file.readStringUntil(',');
          // Serial.println(chordName);
          lyric = file.readStringUntil('\n');
          // Serial.println(lyric);
          beatBuffer[i] = Beat(chordName, lyric);
        } else {                                  // End of file
          beatCount = i;
          break;
        }
      }

      file.close();
      return true;
    }

    int getBeatCount() { return beatCount; }

    Beat* getBeats() {
      return beatBuffer;
    }

    Beat getBeat(int n) {
      if (n < beatCount) {
        return beatBuffer[n];
      }
    }
};


/******************** MAIN ********************/
void setup() {

  Serial.begin(9600);
  Serial.println(F("Testing"));

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(HX8357_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(HX8357_WHITE); tft.setTextSize(2);
  tft.print("Hello World!");
  Serial.println(tft.getCursorX());
  Serial.println(tft.getCursorY());

  String songFilename = "test.csv";
  Song song(songFilename);

  song.loadNextNBeats(4);

  // for(uint8_t rotation=0; rotation<4; rotation++) {
  //   Serial.print("rotation = ");
  //   Serial.println(rotation);
  //   tft.setRotation(rotation);
  //   testText();
  //   int w = tft.width();
  //   Serial.print("w =");
  //   Serial.println(w);
  //   int h = tft.height();
  //   Serial.print("h = ");
  //   Serial.println(h);
  //   delay(1000);
  // }

  
  String text = song.getBeat(0).getChord().getChordName();
  Serial.println(text);
  tft.println(text);

  printBeats(song.getBeats(), song.getBeatCount());

  


  /* 
  Guitar string names:

  E High
  B
  G
  D
  A
  E Low
  */

  

}

void loop() {
  // for(uint8_t rotation=0; rotation<4; rotation++) {
  //   tft.setRotation(rotation);
  //   testText();
  //   delay(1000);
  // }
}

bool printBeats(Beat* beats, int beatCount) {
  tft.fillScreen(HX8357_BLACK);
  int interval = tft.width()/beatCount;

  // Print out the line of chords
  for (int i=0; i<tft.width(); i+=interval) {
    tft.setCursor(i, 0);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2); // Character Size 2: 10 x 14
    tft.print(beats[i].getChord().getChordName());
  }

  // Print of the line of lyrics
  for (int i=0; i<tft.width(); i+=interval) {
    tft.setCursor(i, 15);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2); // Character Size 2: 10 x 14
    tft.print(beats[i].getLyric());
  }
}








