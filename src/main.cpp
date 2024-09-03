#include <Arduino.h>

#include <Wire.h>

#include <WiFi.h>


#include <esp_wifi.h>


// super secret codez!
#include <secrets.h>





#include <Adafruit_SSD1306.h>





#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     16 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


/// @brief The time between screen updates in milliseconds.
unsigned long screenUpdateDelay = 250;


/// @brief The periodic screen update function.
void screenUpdate();








#define LED_FLASHSTYLE_STROBE 0
#define LED_FLASHSTYLE_HALF 1
#define LED_FLASHSTYLE_SLOW 2
#define LED_FLASHSTYLE_OFF 3


/// @brief Set the flashing style of the led.
/// @param ledFlashStyle The index of the flashing style to use.
void ledSet( uint8_t ledFlashStyle );

/// @brief The time in milliseconds that the led should stay on.
unsigned long ledTimeOn = 50;

/// @brief The time in milliseconds that the led should stay off.
unsigned long ledTimeOff = 950;

/// @brief Handle the led toggling
void ledUpdate();





/// @brief Execute the sniffer setup state machine.
void doStateMachine();

/// @brief The sniffer state machine current state.
uint8_t snifferCurrentState = 0;


/// @brief Reset the wifi chip to its default state.
void wifiReset();


void wifipromiscousmode();

void sniffer(void* buf, wifi_promiscuous_pkt_type_t type);






/// @brief Halt and DO NOT catch fire.
void halt() {
  while (1) { yield(); }
}






/// @brief Perform initial setup.
void setup() {
  
  // turn on the serial port
  Serial.begin( 115200 );
  
  // configure the led pin
  pinMode( LED_BUILTIN , OUTPUT );
  digitalWrite( LED_BUILTIN , LOW );
  // turn off the led
  ledSet( LED_FLASHSTYLE_OFF );


  // configure i2c for buit in screen
  Wire.begin( 4 , 15 );

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Clear the screen
  display.clearDisplay();
  display.display();

  display.setCursor(0, 0);     // Start at top-left corner
  display.print("Starting up!");
  display.display();
  


  

}












/// @brief The current time, in milliseconds, since turn on.
unsigned long currentTime = 0;


/// @brief The target time to toggle the led.
unsigned long ledUpdateTime = 0;

/// @brief The target time to update the screen.
unsigned long screenUpdateTime = 0;



/// @brief The main execution loop.
void loop() {

  // get the current time.  
  currentTime = millis();

  // check for led update.
  ledUpdate();

  // check for screen update.
  screenUpdate();

  // poll the sniffer state machine
  doStateMachine();

}








/// @brief The number of networks found during the wifi scan.
int8_t networksfound = 0;

/// @brief The correct channel number for the target ssid.
uint8_t channelNumber = 0;

void doStateMachine() {


  switch (snifferCurrentState)
  {
  case 0:
    //initial startup.
    Serial.println( "Wifi reset.");
    wifiReset();
    networksfound = 0;
    snifferCurrentState = 1;
    break;

  case 1:
    // begin scan for SSID
    Serial.println("Scan started!");
    WiFi.scanNetworks( true );
    snifferCurrentState = 2;
    break;

  case 2:
    networksfound = WiFi.scanComplete();
    if ( networksfound < 0 ) {
      return;
    }
    snifferCurrentState = 3;
    break;

  case 3:
    Serial.print( "Scan complete: " );
    Serial.print( networksfound );
    Serial.println( " networks found." );
    
    for ( uint8_t i = 0 ; i < networksfound ; i++ ) {
      Serial.print("Checking: " );
      Serial.println( WiFi.SSID(i).c_str() );
      if ( (String)WiFi.SSID(i).c_str() == WIFI_SSID ) {
        Serial.print("Found ");
        Serial.print( WIFI_SSID );
        Serial.print( " on channel " );
        Serial.println( WiFi.channel(i) );
        channelNumber = WiFi.channel(i);
      }
    }

    snifferCurrentState = 4;
    break;

  case 4:
    // channel number is found
    Serial.print("Channel Found: " );
    Serial.print( channelNumber );
    Serial.println();
    // remove the scan results
    WiFi.scanDelete();
    // reset the wifi.
    wifiReset();
    snifferCurrentState = 5;
    break;

  case 5:
    // configure wifi as promiscous.
    Serial.println("Entering Promiscous Mode.");
    wifipromiscousmode();
    snifferCurrentState = 6;
    break;

  case 6:
    break;

  default:
    break;
  }



}






















void ledUpdate() {

  // check to see if we have reached the update time yet
  if ( currentTime < ledUpdateTime ) {
    return;
  }

  // check the state of the led.
  if ( digitalRead( LED_BUILTIN ) ) {
    // if the led is on, turn it off.
    digitalWrite( LED_BUILTIN , LOW );
    // schedule the turn on.
    ledUpdateTime = currentTime + ledTimeOff;
  }
  else {
    // the led is off, so turn it on.
    digitalWrite( LED_BUILTIN , HIGH );
    // shedule the turn off.
    ledUpdateTime = currentTime + ledTimeOn;
  }



}





void ledSet( uint8_t ledFlashStyle ) {

  switch (ledFlashStyle) {
      
    case LED_FLASHSTYLE_STROBE:
      ledTimeOn = 50;
      ledTimeOff = 950;
      break;

    case LED_FLASHSTYLE_HALF:
      ledTimeOn = 500;
      ledTimeOff = 500;
      break;

    case LED_FLASHSTYLE_SLOW:
      ledTimeOn = 750;
      ledTimeOff = 250;
      break;
    
    case LED_FLASHSTYLE_OFF:
      ledTimeOn = 0;
      ledTimeOff = 1000;
      break;

  }

}











uint8_t guiActivityIndicator = 0;


void screenUpdate() {

  // check if it is time to update yet.
  if ( currentTime < screenUpdateTime ) {
    // return early if too early.
    return;
  }

  // it is time for an update!

  // first off, housekeeping.
  
  // schedule the next update
  screenUpdateTime = currentTime + screenUpdateDelay;

  // clear the display buffer
  display.clearDisplay();

  display.setCursor( 0 , 0 );
  display.print("Ready");

  if ( guiActivityIndicator ) {
    display.print("!");
    guiActivityIndicator = 0;
  }
  else {
    guiActivityIndicator = 1;
  }


  // update the display from the framebuffer.
  display.display();

}





















void wifiReset() {

  // reset the wifi chip
  WiFi.disconnect();

  // set it to station mode
  WiFi.mode( WIFI_STA );

  delay(100);
  
}




void wifipromiscousmode() {
  wifi_promiscuous_filter_t filter = {
		.filter_mask = WIFI_PROMIS_FILTER_MASK_ALL};
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&sniffer);
}




// Callback function to process received packets
void sniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
    wifi_pkt_rx_ctrl_t rx_ctrl = pkt->rx_ctrl;

    // skip boradcast
  if ( pkt->payload[ 4 + 0 ] == 0xFF) {
    return;
  }

    Serial.print("Packet received: ");
    Serial.print("RSSI: ");
    Serial.print(rx_ctrl.rssi);
    Serial.print(", Channel: ");
    Serial.print(rx_ctrl.channel);
    Serial.print(", Length: ");
    Serial.print(rx_ctrl.sig_len);
    Serial.print(", Type: ");
    Serial.println(type);

    // Print source and destination MAC addresses
    Serial.print("Source MAC: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02x", pkt->payload[10 + i]);
        if (i < 5) Serial.print(":");
    }
    Serial.print(", Destination MAC: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02x", pkt->payload[4 + i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();

}




























