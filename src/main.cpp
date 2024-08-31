#include <Arduino.h>

#include <Wire.h>



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









/// @brief Handle the led toggling
void ledUpdate();


#define LED_FLASHSTYLE_STROBE 0
#define LED_FLASHSTYLE_HALF 1
#define LED_FLASHSTYLE_SLOW 2


/// @brief Set the flashing style of the led.
/// @param ledFlashStyle The index of the flashing style to use.
void ledSet( uint8_t ledFlashStyle );

/// @brief The time in milliseconds that the led should stay on.
unsigned long ledTimeOn = 50;

/// @brief The time in milliseconds that the led should stay off.
unsigned long ledTimeOff = 950;







/// @brief Halt and DO NOT catch fire.
void halt() {
  while (1) { yield(); }
}






/// @brief Perform initial setup.
void setup() {
  
  // turn on the serial port
  Serial.begin( 9600 );
  
  // configure the led pin
  pinMode( LED_BUILTIN , OUTPUT );
  digitalWrite( LED_BUILTIN , LOW );


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


unsigned long screenUpdateTime = 0;


/// @brief The main execution loop.
void loop() {
  
  currentTime = millis();

  ledUpdate();

  screenUpdate();




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

  }

}














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



  // update the display from the framebuffer.
  display.display();


}




