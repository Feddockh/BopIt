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
    uint32_t filePosition;
    String* chordBuffer;
    String* tabBuffer;
    String* lyricBuffer;
    int beatCount;

  public:
    Song(String file) : filename(file), filePosition(0), chordBuffer(nullptr), tabBuffer(nullptr), lyricBuffer(nullptr), beatCount(0) {}

    ~Song() {
      if (chordBuffer != nullptr) {
        delete [] chordBuffer;
      }
      if (lyricBuffer != nullptr) {
        delete [] lyricBuffer;
      }
    }

    bool loadNextNBeats(int n) {

      if (!SD.begin(4)) {
        Serial.println(F("Initialization failed!"));
        return;
      }
      Serial.println(F("Initialization done."));

      File file = SD.open(filename);

      if (!file) {
        Serial.println(F("File Error"));
        return false;
      }

      Serial.println(filePosition);
      file.seek(filePosition);

      if (chordBuffer != nullptr) { delete [] chordBuffer; }
      if (tabBuffer != nullptr) { delete [] tabBuffer; }
      if (lyricBuffer != nullptr) { delete [] lyricBuffer; }

      chordBuffer = new String[n];
      tabBuffer = new String[n];
      lyricBuffer = new String[n];
      beatCount = n;

      for (int i=0; i<n; i++) {
        if (file.available()) {
          chordBuffer[i] = file.readStringUntil(',');
          Serial.println(chordBuffer[i]);
          tabBuffer[i] = file.readStringUntil(',');
          Serial.println(tabBuffer[i]);
          lyricBuffer[i] = file.readStringUntil('\n');
          Serial.println(lyricBuffer[i]);
        } else {
          beatCount = i;
          break;
        }
      }

      filePosition = file.position();
      Serial.println(filePosition);
      file.close();
      return true;
    }

    int getBeatCount() { return beatCount; }

    String* getChordBuffer() { return chordBuffer; }

    String* getTabBuffer() { return tabBuffer; }

    String* getLyricBuffer() { return lyricBuffer; }
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
  uint8_t bpm = 67;


  uint8_t n0 = 6; // initial number of "beats" to show on the screen at a time
  uint8_t n = n0;
  int d = 60000/bpm; // delay[ms] = n[beats] * 1/(beats/min * min/sec * sec/ms)
  String lastTab = ""; // Keep track of the last tab to know when to update

  // Iterate through each of the sets of beats and query the next set of song data from the SD card
  while (n0 == n) {

    // Load the next n beats from the SD card to the microcontroller
    song.loadNextNBeats(6);

    // Print the n set of chords to the screen
    printBeats(song.getChordBuffer(), song.getLyricBuffer(), song.getBeatCount(), HX8357_WHITE);

    // Get the number of beats that were returned from the song
    // This value will be less than the initial number of beats if the end of the song is reached
    n = song.getBeatCount();

    // Retrieve the tab buffer
    String* tabBuffer = song.getTabBuffer();

    // Iterate through each beat in the set of retrieved n beats
    for (int i=0; i<n; i++) {

      // Print the current beat in a different color to indicate the pacing
      printBeat(song.getChordBuffer(), song.getLyricBuffer(), song.getBeatCount(), i, HX8357_YELLOW);

      // Refresh the current tab if needed
      if (tabBuffer[i] != lastTab) {
        resetTab(lastTab);
        displayTab(tabBuffer[i], HX8357_WHITE);
      }

      // Update the new last tab
      lastTab = tabBuffer[i];

      // Delay some amount determined by the bpm
      delay(d);
    }
  }
}

void loop() {}

bool displayTab(String tab, uint16_t color) {
  for (uint8_t i=0; i<tab.length(); i++) {
    char c = tab.charAt(i);
    if (c == 'x' || c == 'o') {
      tft.setCursor(15, 63 + i*40);
      tft.setTextColor(color);
      tft.setTextSize(2); // Character Size 2: 10 x 14
      tft.print(c);
    } else {
      uint8_t n = c - '0'; // Convert to int value
      tft.fillCircle(90 + 100*(n-1), 70 + 40*i, 10, color);
    }
  }
}

void resetTab(String lastTab) {

  // screen height: 320, width: 480

  // Reset the open/closed markers
  tft.fillRect(0, 60, 40, 220, HX8357_BLACK);

  // Reset the finger indicator dots from the last tab
  displayTab(lastTab, HX8357_BLACK);

  // Horizontal lines
  for (int y=70; y<=270; y+=40) { // height 200 / chords 5 = 40
    tft.drawFastHLine(40, y, 400, HX8357_WHITE); // x start: 30, end: 450, width: 400
  }

  // Vertical lines
  for (int x=40; x<440; x+=100) { // width 400 / tabs 4 = 100
    tft.drawFastVLine(x, 70, 200, HX8357_WHITE);
  }
}

bool printBeats(String* chordBuffer, String* lyricBuffer, uint8_t beatCount, uint16_t color) {
  tft.fillRect(0, 0, tft.width(), 32, HX8357_BLACK); // Clear text section

  for (uint8_t i=0; i<beatCount; i++) {
    printBeat(chordBuffer, lyricBuffer, beatCount, i, color);
  }
}

bool printBeat(String* chordBuffer, String* lyricBuffer, uint8_t beatCount, uint8_t beatNumber, uint16_t color) {
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

