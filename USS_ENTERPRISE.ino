#include <Adafruit_PWMServoDriver.h>
#include <Button.h>
#include <DFPlayerMini_Fast.h>
#include <SoftwareSerial.h>

// Define pin numbers for buttons
#define INTERIOR_LIGHT_BUTTON_PIN 2
#define SPOT_LIGHT_BUTTON_PIN 3
#define ENGINE_MODE_BUTTON_PIN 4
#define TORPEDO_BUTTON_PIN 5
#define RED_ALERT_BUTTON_PIN 6

// Define pin numbers for LEDs and other components
#define RED_ALERT_LIGHT_PIN 11
#define TORPEDO_RAMP_PIN_1 7
#define TORPEDO_FIRE_PIN_1 8
#define TORPEDO_RAMP_PIN_2 9
#define TORPEDO_FIRE_PIN_2 10

// Define pin numbers for MP3 module
#define MP3_SERIAL_TX 14  // Connect this pin to the RX pin of the MP3 module
#define MP3_SERIAL_RX 15  // Connect this pin to the TX pin of the MP3 module

// Define sound filenames
#define THRUSTER_SOUND_FILE "Thruster.mp3"
#define IMPULSE_SOUND_FILE "Impulse.mp3"
#define WARP_SOUND_FILE "Warp.mp3"
#define TORPEDO_SOUND_FILE "Torpedo.mp3"
#define RED_ALERT_SOUND_FILE "RedAlert.mp3"
#define INTRO_SOUND_FILE_1 "Intro.mp3"
#define INTRO_SOUND_FILE_2 "Intro1.mp3"
#define INTRO_SOUND_FILE_3 "Intro2.mp3"

// Initialize objects for buttons
Button interiorLightButton(INTERIOR_LIGHT_BUTTON_PIN);
Button spotLightButton(SPOT_LIGHT_BUTTON_PIN);
Button engineModeButton(ENGINE_MODE_BUTTON_PIN);
Button torpedoButton(TORPEDO_BUTTON_PIN);
Button redAlertButton(RED_ALERT_BUTTON_PIN);

// Initialize objects for sound
SoftwareSerial mp3Serial(MP3_SERIAL_RX, MP3_SERIAL_TX);  // Define a software serial port
DFPlayerMini_Fast sound;

// State variables
unsigned long lastInteriorLightPress = 0;
unsigned long lastSpotLightPress = 0;
unsigned long lastEngineModePress = 0;
unsigned long lastRedAlertPress = 0;
bool introPlaying = true;

int mode = 0; // 0: Off, 1: Thruster, 2: Impulse, 3: Warp

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);

  // Initialize DFPlayer Mini
  mp3Serial.begin(9600); // Start the software serial port for the MP3 module
  sound.begin(mp3Serial); // Pass the software serial port to the DFPlayer Mini object

  // Setup pins for LEDs and other components
  pinMode(RED_ALERT_LIGHT_PIN, OUTPUT);
  pinMode(TORPEDO_RAMP_PIN_1, OUTPUT);
  pinMode(TORPEDO_FIRE_PIN_1, OUTPUT);
  pinMode(TORPEDO_RAMP_PIN_2, OUTPUT);
  pinMode(TORPEDO_FIRE_PIN_2, OUTPUT);

  // Play intro sounds
  playIntroSounds();
}

void loop() {
  // Handle button presses and update lighting accordingly
  if (!introPlaying) {
    handleInteriorLightButton();
    handleSpotLightButton();
    handleEngineModeButton();
    handleTorpedoButton();
    handleRedAlertButton();
  } else {
    // Check for button press to stop intro sounds
    if (interiorLightButton.pressed() || spotLightButton.pressed() ||
        engineModeButton.pressed() || torpedoButton.pressed() || redAlertButton.pressed()) {
      sound.stop();
      introPlaying = false;
    }
  }
}

void handleInteriorLightButton() {
  if (interiorLightButton.pressed() && (millis() - lastInteriorLightPress > 1000)) {
    lastInteriorLightPress = millis();
    static bool interiorLightsOn = false;
    interiorLightsOn = !interiorLightsOn;
    if (interiorLightsOn) {
      // Turn on all interior lights in sequence
      for (int i = 0; i < 4; ++i) { // Assuming 4 interior lights connected
        analogWrite(i, HIGH);
        delay(100);
      }
    } else {
      // Turn off all interior lights
      for (int i = 0; i < 4; ++i) { // Assuming 4 interior lights connected
        analogWrite(i, LOW);
      }
    }
  }
}

void handleSpotLightButton() {
  if (spotLightButton.pressed() && (millis() - lastSpotLightPress > 1000)) {
    lastSpotLightPress = millis();
    static bool spotLightsOn = false;
    spotLightsOn = !spotLightsOn;
    if (spotLightsOn) {
      // Turn on all exterior spot lights in sequence
      for (int i = 4; i < 12; ++i) { // Assuming 8 spot lights connected starting from pin 4
        analogWrite(i, HIGH);
        delay(100);
      }
    } else {
      // Turn off all exterior spot lights in reverse sequence
      for (int i = 11; i >= 4; --i) { // Assuming 8 spot lights connected starting from pin 4
        analogWrite(i, LOW);
        delay(100);
      }
    }
  }
}

void handleEngineModeButton() {
  if (engineModeButton.pressed() && (millis() - lastEngineModePress > 1000)) {
    lastEngineModePress = millis();
    mode = (mode + 1) % 4; // Cycle through modes

    switch (mode) {
      case 0: // Off
        turnOffAllLights();
        break;
      case 1: // Thruster
        thrusterMode();
        break;
      case 2: // Impulse
        impulseMode();
        break;
      case 3: // Warp
        warpMode();
        break;
    }
  }
}

void thrusterMode() {
  // Play thruster sound
  playSound(THRUSTER_SOUND_FILE);
}

void impulseMode() {
  // Play impulse sound
  playSound(IMPULSE_SOUND_FILE);
}

void warpMode() {
  // Play warp sound
  playSound(WARP_SOUND_FILE);
}

void turnOffAllLights() {
  // Turn off all lights
  for (int i = 0; i < 12; ++i) { // Assuming 12 total lights
    analogWrite(i, LOW);
  }
}

void handleTorpedoButton() {
  if (torpedoButton.pressed()) {
    // Ramp up torpedo ramp pin 1
    rampUpTorpedoRampPin(TORPEDO_RAMP_PIN_1);

    // Fire torpedo pin 1 and play sound
    fireTorpedo(TORPEDO_FIRE_PIN_1);

    // Wait for a second
    delay(1000);

    // Ramp up torpedo ramp pin 2
    rampUpTorpedoRampPin(TORPEDO_RAMP_PIN_2);

    // Fire torpedo pin 2 and play sound
    fireTorpedo(TORPEDO_FIRE_PIN_2);
  }
}

void rampUpTorpedoRampPin(int pin) {
  unsigned long startTime = millis();
  while (millis() - startTime <= 1000) { // Assuming ramp duration is 1000ms
    int brightness = map(millis() - startTime, 0, 1000, 0, 255);
    analogWrite(pin, brightness);
    delay(1);
  }
}

void fireTorpedo(int pin) {
  digitalWrite(pin, HIGH);
  delay(100); // Duration of the flash
  digitalWrite(pin, LOW);
}

void handleRedAlertButton() {
  if (redAlertButton.pressed() && (millis() - lastRedAlertPress > 1000)) {
    lastRedAlertPress = millis();
    digitalWrite(RED_ALERT_LIGHT_PIN, HIGH);
    playSound(RED_ALERT_SOUND_FILE);
    unsigned long alertStartTime = millis();
    while (millis() - alertStartTime <= 5000) {
      // Wait for 5 seconds
    }
    digitalWrite(RED_ALERT_LIGHT_PIN, LOW);
  }
}

void playSound(const char* soundFile) {
  // Play sound
  sound.playFolder(1, soundFile);
}

void playIntroSounds() {
  // Play intro sounds in a loop until any button is pressed
  sound.playFolder(1, INTRO_SOUND_FILE_1);
  delay(1000); // Assuming some delay between sounds
  sound.playFolder(1, INTRO_SOUND_FILE_2);
  delay(1000);
  sound.playFolder(1, INTRO_SOUND_FILE_3);
  delay(1000);
  sound.playFolder(1, INTRO_SOUND_FILE_1);
  delay(1000);
  sound.playFolder(1, INTRO_SOUND_FILE_2);
  delay(1000);
  sound.playFolder(1, INTRO_SOUND_FILE_3);
  delay(1000);
  sound.playFolder(1, INTRO_SOUND_FILE_1);
  delay(1000);
  sound.playFolder(1, INTRO_SOUND_FILE_2);
  delay(1000);
  sound.playFolder(1, INTRO_SOUND_FILE_3);
}
