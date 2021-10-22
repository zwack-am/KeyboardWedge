
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

// Create MFRC522 instance
MFRC522 mfrc522(SS_PIN, RST_PIN);   
MFRC522::MIFARE_Key key;

// Create DotStar instance
Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

// Some strings

String password, confirm;

void setPassword() {
  // Turn the red led on
  digitalWrite(LED_BUILTIN, HIGH);
  
  // Clear the Serial buffer
  if (Serial.available() > 0)
  {
    Serial.readString();
  }
  
  // Get the password
  Serial.println("Please enter the new password");
  Serial.setTimeout(100000);
  password = Serial.readStringUntil('\n');
  Serial.setTimeout(1000);
  // Get the password again to compare it.
  Serial.println("Please enter the password again");
  Serial.setTimeout(100000);
  confirm = Serial.readStringUntil('\n');
  Serial.setTimeout(1000);

  // Compare the two
  if (password == confirm)
  {
      

  // Get them to place the chip on the antenna

  // Search the chip for space to write the password

  // Write the password to the chip

  // Read the password
  
  }
  else
  {
    Serial.println("Passwords don't match");
  }

  // Output some instructions for next time.
  Serial.println();
  Serial.println("Enter '1' in order to program a new password onto a chip.");
  Serial.println();
  
  // Turn the red led off
  digitalWrite(LED_BUILTIN, LOW);
}

int getUIDPassword(MFRC522::Uid uid) {
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
      // Set the Dotstar to Blue 
      strip.setPixelColor(0, 0, 0, 255);
      strip.show();
      pwLength = strlen(UIDPassword[entry][1]);
      Keyboard.begin();
      for (uint8_t i = 0; i < pwLength ; i++)  
        Keyboard.write(UIDPassword[entry][1][i]);
      Keyboard.write(KEY_RETURN);
      Keyboard.end();
      delay(1000);
      return(true);
    }
    entry++;
  }
  return(false);  
}

int getPassword(uint8_t blocks, uint8_t blockLength) {
  
  //some variables we need
  uint8_t block;
  uint8_t bufferSize;
  bool password;
  MFRC522::StatusCode status;

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  for (uint8_t i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }


  bufferSize = blockLength + 2;
  uint8_t buffer[bufferSize];

  password = false;
  
  for (byte count = 0; count < 3; count++)
  {
    for (block = 1; block <= blocks; block++) 
    {

      // Authenticate
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
      if (status != MFRC522::STATUS_OK)
      {
        return(false);
      }

      // Read block
      status = mfrc522.MIFARE_Read(block, buffer, &bufferSize);
      if (status != MFRC522::STATUS_OK) 
      {
        return(false);
      }  

      // Check data and output the password
      for (uint8_t i = 0; i < blockLength; i++) 
      {
         
        if (buffer[i] == header[0] && buffer[i+1] == header[1] && 
          buffer[i+2] == header[2] && buffer[i+3] == header[3])
        {
          // We found the password header
          // Set the Dotstar to Green 
          strip.setPixelColor(0, 0, 255, 0);
          strip.show();

          // Now get ready to start typing
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
          delay(1000);
          return(true);
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
  return(false);
}

int getUID(MFRC522::Uid uid) {
  uint8_t pwLength;
  char hexDigits[] = "0123456789ABCDEF"; 

  // Set the DotStar to Red
  strip.setPixelColor(0, 255, 0, 0);
  strip.show();

  Keyboard.begin();
    
  // Convert the uid into a Hex String
  for (uint8_t ch ; ch < uid.size ; ch ++)
  {
    Keyboard.write(hexDigits[uid.uidByte[ch]/16]);
    Keyboard.write(hexDigits[uid.uidByte[ch]%16]);
  }

  Keyboard.write(KEY_RETURN);
  Keyboard.end();
  delay(1000);
  return(true);
}

void setup() {

  // Initialiase the SPI bus
  SPI.begin();

  // Initialise the MFRC522 Card Reader
  mfrc522.PCD_Init();
  mfrc522.PCD_WriteRegister(MFRC522::RFCfgReg, MFRC522::RxGain_avg);

  // Initialise the dotstar
  strip.begin();
  strip.setBrightness(80);
  strip.setPixelColor(0, 255, 255, 255);
  strip.show();

  // Initialise the RED LED (for write mode)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Initialse the Serial console
  Serial.begin(9600);
  
  // Give the serial console a chance to become ready.
  delay(3000);
  
  // Output some instructions to the serialconsole.
  Serial.println();
  Serial.println("Enter '1' in order to program a new password onto a chip.");
  Serial.println();

}

char rxByte = 0;

void loop() {

  // Have the dotstar set to white
  strip.setPixelColor(0, 255, 255, 255);
  strip.show();
  

  //Define some variables;
  byte blocks;
  byte blocklength;
  MFRC522::PICC_Type cardName; 

  // Check the serial console for input
  if (Serial.available() > 0)
  {
     // if we got a 1 jump to the set password routine
     rxByte = Serial.read();

     if (rxByte == '1')
     {
        setPassword();
     }
  }
 
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
        // If this is an NTAG21[356] it will show up here, the UID will be 7 characters
        // and the first UID byte will be 04h
        if (mfrc522.uid.uidByte[0] == 04 && mfrc522.uid.size == 7)
        {

          // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
          for (uint8_t i = 0; i < 6; i++) {
            key.keyByte[i] = 0xFF;
          }
          uint8_t bufferSize = 18;
          uint8_t buffer[bufferSize];
          uint8_t block=3;
          MFRC522::StatusCode status;

          // Read block
          status = mfrc522.MIFARE_Read(block, buffer, &bufferSize);
          if (status != MFRC522::STATUS_OK) 
          {
            return;
          }

          switch (buffer[2])
          {
            case 0x12:
              // NTAG213
              blocks=43;
              break;
            case 0x3E:
              // NTAG215
              blocks=133;
              break;
            case 0x6D:
              // NTAG216
              blocks=229;
              break;
            default:
              //Unknown NXP tag
              return;
          }
        }
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
    if ( ! getUIDPassword(mfrc522.uid))
    {
      if ( ! getPassword(blocks, blocklength))
      {
        getUID(mfrc522.uid);
      }
    }
  }

  // Close the card reader 
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
  
