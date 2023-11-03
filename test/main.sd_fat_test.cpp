#include <cstdlib>
#include <cstdint>
#include <list>

// the main program is independent from the Arduino core system and libraries
// TODO: it comes in idirectly via display.h -> Adafruit_SSD1331.h

 // include the SD library:
#include <SD.h>
#include <SPI.h>

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;

bool SD_card_OK;
int SD_file_No;

void printSpaces(int num) {
  for (int i=0; i < num; i++) {
    Serial.print(" ");
  }
}

void printTime(const DateTimeFields tm) {
  const char *months[12] = {
    "January","February","March","April","May","June",
    "July","August","September","October","November","December"
  };
  if (tm.hour < 10) Serial.print('0');
  Serial.print(tm.hour);
  Serial.print(':');
  if (tm.min < 10) Serial.print('0');
  Serial.print(tm.min);
  Serial.print("  ");
  Serial.print(tm.mon < 12 ? months[tm.mon] : "???");
  Serial.print(" ");
  Serial.print(tm.mday);
  Serial.print(", ");
  Serial.print(tm.year + 1900);
}

void printDirectory(File dir, int numSpaces) {
   while(true) {
     File entry = dir.openNextFile();
     if (! entry) {
       //Serial.println("** no more files **");
       break;
     }
     printSpaces(numSpaces);
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numSpaces+2);
     } else {
       // files have sizes, directories do not
       int n = log10f(entry.size());
       if (n < 0) n = 10;
       if (n > 10) n = 10;
       printSpaces(50 - numSpaces - strlen(entry.name()) - n);
       Serial.print("  ");
       Serial.print(entry.size(), DEC);
       DateTimeFields datetime;
       if (entry.getModifyTime(datetime)) {
         printSpaces(4);
         printTime(datetime);
       }
       Serial.println();
     }
     entry.close();
   }
}

extern "C" int main(void)
{

	// Open serial communications and wait for port to open:
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect.
	}

	Serial.print("\nInitializing SD card...");
	// we'll use the initialization code from the utility libraries
	// since we're just testing if the card is working!
	if (!card.init(SPI_HALF_SPEED, BUILTIN_SDCARD)) {
		Serial.println("initialization failed. Things to check:");
		Serial.println("* is a card inserted?");
		Serial.println("* is your wiring correct?");
		Serial.println("* did you change the chipSelect pin to match your shield or module?");
		return(-1);
	} else {
		Serial.println("Wiring is correct and a card is present.");
	}

	// print the type of card
	Serial.print("\nCard type: ");
	switch(card.type()) {
		case SD_CARD_TYPE_SD1:
			Serial.println("SD1");
		break;
		case SD_CARD_TYPE_SD2:
			Serial.println("SD2");
		break;
		case SD_CARD_TYPE_SDHC:
			Serial.println("SDHC");
		break;
		default:
			Serial.println("Unknown");
	}

    // we initialize the SD card here as some modules may want to read or
    // write data during setup
    SD_card_OK = SD.begin(BUILTIN_SDCARD);
    if (SD_card_OK)
    {
	    Serial.println("SD card found.");
    }
    else
    {
	    Serial.println("SD card not found.");
    };

	// Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
	if (!volume.init(card)) {
		Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
		return(-1);
	}

	// print the type and size of the first FAT-type volume
	uint32_t volumesize;
	Serial.print("\nVolume type is FAT");
	Serial.println(volume.fatType(), DEC);
	Serial.println();

	volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
	volumesize *= volume.clusterCount();       // we'll have a lot of clusters
	if (volumesize < 8388608ul) {
		Serial.print("Volume size (bytes): ");
		Serial.println(volumesize * 512);        // SD card blocks are always 512 bytes
	}
	Serial.print("Volume size (Kbytes): ");
	volumesize /= 2;
	Serial.println(volumesize);
	Serial.print("Volume size (Mbytes): ");
	volumesize /= 1024;
	Serial.println(volumesize);

	// print the directory
	File root = SD.open("/");
	printDirectory(root, 0);

}

