
#include <MFRC522.h>
#include <require_cpp11.h>
#include <MFRC522Extended.h>
#include <deprecated.h>
#include <SPI.h>
#include <Keyboard.h>
#include <Adafruit_DotStar.h>

/*
 * --------------------------------------------------------------------------------------------------------------------
 * Code for a USB capable controller and an MFRC522 to read a password from
 *   an NFC card and "type" it out to the host computer.
 * --------------------------------------------------------------------------------------------------------------------
 * This code was written to run on an Adafruit Trinket M0. The following connections need to be made between the
 * Trinket M0 and the MFRC522 board.
 * 
 *    TRINKET PIN      MFRC522 PIN
 *        0                SDA
 *        1                RST
 *        2               MISO
 *        3                SCK
 *        4               MOSI
 *        3V              3.3V
 *       Gnd               GND
 * 
 * This project has been placed into the public domain.  Anyone may make one of these wedges.
 * 
 * This Code will read any of the supported card types and will then search through the data on the card looking for
 * the header string "ZPKW" from that location until the following null will be output to the USB port as though the
 * wedge was a keyboard followed by the Enter key.
 * 
 * Currently the following NFC card types are supported:
 * 
 */

// Create the UIDPassword Array
char * UIDPassword[][2] = { 
  { "DEADBEEF", "Password1"}, 
  { "UID2", "Password2"}, 
  { "UID3", "Password3"},
  { NULL, NULL }
};

// Set the header string
byte header[] = "ZPKW";

// Define the Trinket pins, these are Configurable, but the Trinket doesn't have any spare pins.
#define RST_PIN   1
#define SS_PIN    0
#define NUMPIXELS 1
#define DATAPIN   7
#define CLOCKPIN  8
#define LED_PIN   13

// Create MFRC522 instance
MFRC522 mfrc522(SS_PIN, RST_PIN);   
MFRC522::MIFARE_Key key;

long pixelHue = 0L;

// Create DotStar instance
Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

void setup() {

  // Initialiase the SPI bus
  SPI.begin();

  // Initialise the MFRC522 Card Reader
  mfrc522.PCD_Init();

  // Initialise the dotstar
  strip.begin();
  strip.setBrightness(80);
  strip.show();

}

void loop() {

  // Have the dotstar cycle through the rainbow, slowly
  pixelHue = (pixelHue + 512) % 655536;
  strip.setPixelColor(0, strip.gamma32(strip.ColorHSV(pixelHue)));
  strip.show();
  

  //Define some variables;
  byte blocks;
  byte blocklength;
  MFRC522::PICC_Type cardName; 
 
  // Look for new cards, and select one if present
  if ( mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() ) {
    // Get the card type 
    cardName = mfrc522.PICC_GetType(mfrc522.uid.sak);
    // Select the Card info
    switch (cardName)
    {
      case MFRC522::PICC_TYPE_ISO_14443_4:
        // Not yet defined
        return;
        break;
      case MFRC522::PICC_TYPE_ISO_18092:
        // Not yet defined
        return;
        break;
      case MFRC522::PICC_TYPE_MIFARE_MINI:
        // The Classic Mifare Mini has 20 blocks of 16 bytes each.
        blocks=19;
        blocklength=16;
        break;
      case MFRC522::PICC_TYPE_MIFARE_UL:
        // The Mifare Ultralight has 16 blocks of 4 bytes each.
        blocks=15;
        blocklength=4;
        break;
//      case MFRC522::PICC_TYPE_MIFARE_UL_C:
//        // The Mifare Ultralight C has 48 blocks of 4 bytes each.
//        blocks=47;
//        blocklength=4;
//        break;
      case MFRC522::PICC_TYPE_MIFARE_PLUS:
        // No information
        return;
        break;
      case MFRC522::PICC_TYPE_MIFARE_1K:
        // The Classic Mifare 1K has 64 blocks of 16 bytes each.
        blocks=63;
        blocklength=16;
        break;
      case MFRC522::PICC_TYPE_MIFARE_4K:
        // The Classic Mifare 4K has 256 blocks of 16 bytes each.
        blocks= 255;
        blocklength=16;  
        break;
      default:
        // We don't know how to handle this card type yet.
        return;
    }
    // See if the uid is in our password array
    if (! getUIDPassword(mfrc522.uid))
    {
      // Look for a password on the card
      getPassword(blocks, blocklength);
    }
  }

  // Pause for a second.  Change this value in milliseconds if you wish to change it
  delay(1000);

  // Close the card reader 
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

}

int getUIDPassword(MFRC522::Uid uid)
{
  uint8_t entry = 0;
  uint8_t pwLength;

  char hexDigits[] = "0123456789ABCDEF"; 
  // Convert the uid into a Hex String
  char UIDString[(uid.size * 2) + 1 ];
  for (uint8_t ch ; ch < uid.size ; ch ++)
  {
    UIDString[(ch * 2)] = hexDigits[uid.uidByte[ch]/16];
    UIDString[(ch *2) + 1] = hexDigits[uid.uidByte[ch]%16];
  }
  UIDString[uid.size * 2] = '\0';
  
  while ( UIDPassword[entry][0] != NULL ) 
  {
    
    if (strcmp(UIDString, UIDPassword[entry][0]) == 0)
    {
      pwLength = strlen(UIDPassword[entry][1]);
      Keyboard.begin();
      for (uint8_t i = 0; i < pwLength ; i++)  
        Keyboard.write(UIDPassword[entry][1][i]);
      Keyboard.write(KEY_RETURN);
      Keyboard.end();
      return(true);
    }
    entry++;
  }
  return(false);  
}

void getPassword(uint8_t blocks, uint8_t blockLength) {
  
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  for (uint8_t i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  //some variables we need
  uint8_t block;
  uint8_t bufferSize;
  bool password;
  MFRC522::StatusCode status;

  bufferSize = blockLength + 2;
  uint8_t buffer[bufferSize];

  password == false;

  for (block = 1; block <= blocks; block++) 
  {

    // Authenticate
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
    if (status != MFRC522::STATUS_OK)
    {
      return;
    }

    // Read block
    status = mfrc522.MIFARE_Read(block, buffer, &bufferSize);
    if (status != MFRC522::STATUS_OK) 
    {
      return;
    }

    // Check data and output the password
    for (uint8_t i = 0; i < blockLength; i++) 
    {
         
      if (buffer[i] == header[0] && buffer[i+1] == header[1] && 
        buffer[i+2] == header[2] && buffer[i+3] == header[3])
      {
        // We found the password header, get ready to start typing
        Keyboard.begin();
        password = true;
        i+=3;
      }
      else if (buffer[i] == 0 && password == true)
      {
        // We found the NULL character at the end of the password, type return.
        password = false;
        Keyboard.write(KEY_RETURN);
        Keyboard.end();
      }
      else
      {
        // Otherwise we either ignore the character (we are not in the password block) or type it (we are)
        if (password == true)
        { 
          // We are in the password block so send the password keystroke.
          Keyboard.write(buffer[i]);
        }
      }
    }
  }
}
