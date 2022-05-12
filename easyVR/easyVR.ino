#include "Arduino.h"
#if !defined(SERIAL_PORT_MONITOR)
  #error "Arduino version not supported. Please update your IDE to the latest version."
#endif

#if defined(__SAMD21G18A__)
  // Shield Jumper on HW (for Zero, use Programming Port)
  #define port SERIAL_PORT_HARDWARE
  #define pcSerial SERIAL_PORT_MONITOR
#elif defined(SERIAL_PORT_USBVIRTUAL)
  // Shield Jumper on HW (for Leonardo and Due, use Native Port)
  #define port SERIAL_PORT_HARDWARE
  #define pcSerial SERIAL_PORT_USBVIRTUAL
#else
  // Shield Jumper on SW (using pins 12/13 or 8/9 as RX/TX)
  #include "SoftwareSerial.h"
  SoftwareSerial port(12, 13);
  #define pcSerial SERIAL_PORT_MONITOR
#endif

#include "EasyVR.h"
#include "arduino_secrets.h"
#include<SPI.h>
#include<Bridge.h>
#include<ArduinoMqttClient.h>
#include <WiFiNINA.h>

char ssid[] = SECRET_SSID;       
char pass[] = SECRET_PASS; 

EasyVR easyvr(port);

const char* broker = "192.168.0.100";//"broker.mqtt-dashboard.com";
const char* acdTopicPub = "automatic_cabinet_door/controls";//publish
const char* topicShelf1 = "shelf1";//publish
const char* topicShelf2 = "shelf2";//publish
const char* topicMoodStatus = "mood/state";//publish
const char* topicMoodSection = "mood/section"; //publish
const char* topicVRCommand = "voice_control/status";//publish

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

//Groups and Commands
enum Groups
{
  GROUP_0  = 0,
  GROUP_1  = 1,
};

enum Group0 
{
  G0_WAKE_UP_KITCHEN = 0,
};

enum Group1 
{
  G1_AUTOMATIC_MODE = 0,
  G1_MANUAL_MODE = 1,
  G1_OPEN_MODE = 2,
  G1_OPEN = 3,
  G1_CLOSE = 4,
  G1_WINE = 5,
  G1_GIN = 6,
  G1_COOKIES = 7,
  G1_SLEEP_KITCHEN = 8,
  G1_LED_MOODS_ON = 9,
  G1_LED_MOODS_OFF = 10,
  G1_APPLES = 11,
  G1_PASTA = 12,
  G1_KETCHUP = 13,
  G1_WATER = 14,
  G1_RICE = 15,
  G1_HAPPY = 16,
  G1_SAD = 17,
  G1_NEUTRAL = 18,
  G1_NEUTRAL_HAPPY = 19,
  G1_NEUTRAL_SAD = 20,
  G1_MILK = 21,
  G1_BREAD = 22,
  G1_PANS = 23,
  G1_CUPS = 24,
  G1_PLATES = 25,
};

// use negative group for wordsets
int8_t group, idx;

void setup()
{
  // setup PC serial port
    // setup PC serial port
  Serial.begin(9600);
  pcSerial.begin(9600);
  Serial.println("setup");

  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, 1883)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }
  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  
bridge:
  // bridge mode?
  int mode = easyvr.bridgeRequested(pcSerial);
  switch (mode)
  {
  case EasyVR::BRIDGE_NONE:
    // setup EasyVR serial port
    port.begin(9600);
    // run normally
    pcSerial.println(F("Bridge not requested, run normally"));
    pcSerial.println(F("---"));
    break;
    
  case EasyVR::BRIDGE_NORMAL:
    // setup EasyVR serial port (low speed)
    port.begin(9600);
    // soft-connect the two serial ports (PC and EasyVR)
    easyvr.bridgeLoop(pcSerial);
    // resume normally if aborted
    pcSerial.println(F("Bridge connection aborted"));
    pcSerial.println(F("---"));
    break;
    
  case EasyVR::BRIDGE_BOOT:
    // setup EasyVR serial port (high speed)
    port.begin(115200);
    pcSerial.end();
    pcSerial.begin(115200);
    // soft-connect the two serial ports (PC and EasyVR)
    easyvr.bridgeLoop(pcSerial);
    // resume normally if aborted
    pcSerial.println(F("Bridge connection aborted"));
    pcSerial.println(F("---"));
    break;
  }

  // initialize EasyVR  
  while (!easyvr.detect())
  {
    pcSerial.println(F("EasyVR not detected!"));
    for (int i = 0; i < 10; ++i)
    {
      if (pcSerial.read() == '?')
        goto bridge;
      delay(100);
    }
  }

  pcSerial.print(F("EasyVR detected, version "));
  pcSerial.print(easyvr.getID());

  if (easyvr.getID() < EasyVR::EASYVR3)
    easyvr.setPinOutput(EasyVR::IO1, LOW); // Shield 2.0 LED off

  if (easyvr.getID() < EasyVR::EASYVR)
    pcSerial.print(F(" = VRbot module"));
  else if (easyvr.getID() < EasyVR::EASYVR2)
    pcSerial.print(F(" = EasyVR module"));
  else if (easyvr.getID() < EasyVR::EASYVR3)
    pcSerial.print(F(" = EasyVR 2 module"));
  else
    pcSerial.print(F(" = EasyVR 3 module"));
  pcSerial.print(F(", FW Rev."));
  pcSerial.println(easyvr.getID() & 7);

  easyvr.setDelay(0); // speed-up replies

  easyvr.setTimeout(5);
  easyvr.setLanguage(0); //<-- same language set on EasyVR Commander when code was generated

  group = EasyVR::TRIGGER; //<-- start group (customize)

  mqttClient.beginMessage(topicVRCommand, true);
  mqttClient.print("ON");
  mqttClient.endMessage();
}

void loop()
{
  while(!mqttClient.connect(broker, 1883)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
  }
  mqttClient.poll();
  init_easyVR();
}

void init_easyVR(){
  if (easyvr.getID() < EasyVR::EASYVR3)
    easyvr.setPinOutput(EasyVR::IO1, HIGH); // LED on (listening)

  if (group < 0) // SI wordset/grammar
  {
    pcSerial.print("Say a word in Wordset ");
    pcSerial.println(-group);
    easyvr.recognizeWord(-group);
  }
  else // SD group
  {
    pcSerial.print("Say a command in Group ");
    pcSerial.println(group);
    easyvr.recognizeCommand(group);
  }

  do
  {
    // allows Commander to request bridge on Zero (may interfere with user protocol)
    if (pcSerial.read() == '?')
    {
      setup();
      return;
    }
    // <<-- can do some processing here, while the module is busy
  }
  while (!easyvr.hasFinished());
  
  if (easyvr.getID() < EasyVR::EASYVR3)
    easyvr.setPinOutput(EasyVR::IO1, LOW); // LED off

  idx = easyvr.getWord();
  if (idx == 0 && group == EasyVR::TRIGGER)
  {
    // beep
    easyvr.playSound(0, EasyVR::VOL_FULL);
    // print debug message
    pcSerial.println("Word: ROBOT");
    // write your action code here
    // group = GROUP_X\SET_X; <-- jump to another group or wordset
    return;
  }
  else if (idx >= 0)
  {
    // beep
    easyvr.playSound(0, EasyVR::VOL_FULL);
    // print debug message
    uint8_t flags = 0, num = 0;
    char name[32];
    pcSerial.print("Word: ");
    pcSerial.print(idx);
    if (easyvr.dumpGrammar(-group, flags, num))
    {
      for (uint8_t pos = 0; pos < num; ++pos)
      {
        if (!easyvr.getNextWordLabel(name))
          break;
        if (pos != idx)
          continue;
        pcSerial.print(F(" = "));
        pcSerial.println(name);
        break;
      }
    }
    // perform some action
    action();
    return;
  }
  idx = easyvr.getCommand();
  if (idx >= 0)
  {
    // beep
    easyvr.playSound(0, EasyVR::VOL_FULL);
    // print debug message
    uint8_t train = 0;
    char name[32];
    pcSerial.print("Command: ");
    pcSerial.print(idx);
    if (easyvr.dumpCommand(group, idx, name, train))
    {
      pcSerial.print(" = ");
      pcSerial.println(name);
    }
    else
      pcSerial.println();
    // perform some action
    action();
  }
  else // errors or timeout
  {
    if (easyvr.isTimeout())
      pcSerial.println("Timed out, try again...");
    int16_t err = easyvr.getError();
    if (err >= 0)
    {
      pcSerial.print("Error ");
      pcSerial.println(err, HEX);

      switch(err){
        case '11h': pcSerial.print("Recognition Failed");
      }
    }
  }
}

void action()
{
  switch (group)
  {
  case GROUP_0:
    switch (idx)
    {
    case G0_WAKE_UP_KITCHEN:
      // write your action code here
      bool retained = true;
      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("WAKE_UP_KITCHEN");
      mqttClient.endMessage();
      pcSerial.println("Hello Uninova, how can I help?\n");
      group = GROUP_1;

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("WAKE_UP_KITCHEN");
      mqttClient.endMessage();
      break;
    }
    break;
  case GROUP_1:
    switch (idx)
    {
    case G1_AUTOMATIC_MODE:
      // write your action code here
      mqttClient.beginMessage(acdTopicPub);
      mqttClient.print("AUTOMATIC_MODE");
      mqttClient.endMessage();
      pcSerial.println("Automatic mode\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("AUTOMATIC_MODE");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_MANUAL_MODE:
      // write your action code here
      mqttClient.beginMessage(acdTopicPub);
      mqttClient.print("MANUAL_MODE");
      mqttClient.endMessage();
      pcSerial.println("Manual mode\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("MANUAL_MODE");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_OPEN_MODE:
      // write your action code here
      mqttClient.beginMessage(acdTopicPub);
      mqttClient.print("OPEN_MODE");
      mqttClient.endMessage();
      pcSerial.println("Open mode\n");
      
      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("OPEN_MODE");
      mqttClient.endMessage();
      
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_OPEN:
      // write your action code here
      mqttClient.beginMessage(acdTopicPub);
      mqttClient.print("OPEN");
      mqttClient.endMessage();
      pcSerial.println("Open\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("OPEN");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_CLOSE:
      // write your action code here
      mqttClient.beginMessage(acdTopicPub);
      mqttClient.print("CLOSE");
      mqttClient.endMessage();
      pcSerial.println("Close\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("CLOSE");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_WINE:
      // write your action code here
      mqttClient.beginMessage(topicMoodSection, true, 0);
      mqttClient.print(4);
      mqttClient.endMessage();
      
      mqttClient.beginMessage(topicShelf1, true, 0);
      mqttClient.print(1);
      mqttClient.endMessage();
      pcSerial.println("Wine\n");

      mqttClient.beginMessage(topicVRCommand, true, 0);
      mqttClient.print("Wine");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_GIN:
      // write your action code here
      mqttClient.beginMessage(topicMoodSection);
      mqttClient.print(4);
      mqttClient.endMessage();
     
      mqttClient.beginMessage(topicShelf1);
      mqttClient.print(2);
      mqttClient.endMessage();
      pcSerial.println("Gin\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("Gin");
      mqttClient.endMessage();

      pcSerial.println("Gin!!!!!!!!!!!\n");
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_COOKIES:
      // write your action code here
      mqttClient.beginMessage(topicMoodSection);
      mqttClient.print(4);
      mqttClient.endMessage();
      
      mqttClient.beginMessage(topicShelf1);
      mqttClient.print(3);
      mqttClient.endMessage();
      pcSerial.println("Cookies\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("Cookies");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
          case G1_APPLES:
      // write your action code here
      mqttClient.beginMessage(topicMoodSection);
      mqttClient.print(4);
      mqttClient.endMessage();
      
      mqttClient.beginMessage(topicShelf1);
      mqttClient.print(4);
      mqttClient.endMessage();
      pcSerial.println("Apples\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("Apples");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_PASTA:
      // write your action code here
      mqttClient.beginMessage(topicMoodSection);
      mqttClient.print(4);
      mqttClient.endMessage();
      
      mqttClient.beginMessage(topicShelf2);
      mqttClient.print(1);
      mqttClient.endMessage();
      pcSerial.println("Pasta\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("Pasta");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_KETCHUP:
      // write your action code here
      mqttClient.beginMessage(topicMoodSection);
      mqttClient.print(4);
      mqttClient.endMessage();
      
      mqttClient.beginMessage(topicShelf2);
      mqttClient.print(2);
      mqttClient.endMessage();
      pcSerial.println("Ketchup\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("Ketchup");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_RICE:
      mqttClient.beginMessage(topicMoodSection);
      mqttClient.print(4);
      mqttClient.endMessage();
    
      mqttClient.beginMessage(topicShelf2);
      mqttClient.print(3);
      mqttClient.endMessage();
      pcSerial.println("Rice\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("Rice");
      mqttClient.endMessage();
     break;
    case G1_WATER:
      // write your action code here
      mqttClient.beginMessage(topicMoodSection);
      mqttClient.print(4);
      mqttClient.endMessage();
      
      mqttClient.beginMessage(topicShelf2);
      mqttClient.print(4);
      mqttClient.endMessage();
      pcSerial.println("Water\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("Water");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_SLEEP_KITCHEN:
      // write your action code here
      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("SLEEP_KITCHEN");
      mqttClient.endMessage();
      pcSerial.println("Sleep kitchen\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("SLEEP_KITCHEN");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_LED_MOODS_ON:
      // write your action code here
      mqttClient.beginMessage(topicMoodStatus);
      mqttClient.print(1);
      mqttClient.endMessage();
      pcSerial.println("Led moods ON\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("LED_MOODS_ON");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_LED_MOODS_OFF:
      // write your action code here
      mqttClient.beginMessage(topicMoodStatus);
      mqttClient.print(0);
      mqttClient.endMessage();
      pcSerial.println("Led Moods Off\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("LED_MOODS_OFF");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_HAPPY:
      // write your action code here
      mqttClient.beginMessage(topicMoodStatus);
      mqttClient.print(6);
      mqttClient.endMessage();
      pcSerial.println("Happy mood\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("HAPPY MOOD");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_SAD:
      // write your action code here
      mqttClient.beginMessage(topicMoodStatus);
      mqttClient.print(2);
      mqttClient.endMessage();
      pcSerial.println("sad Mood\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("SAD MOOD");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_NEUTRAL:
      // write your action code here
      mqttClient.beginMessage(topicMoodStatus);
      mqttClient.print(4);
      mqttClient.endMessage();
      pcSerial.println("Neutral mood\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("NEUTRAL MOOD");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_NEUTRAL_HAPPY:
      // write your action code here
      mqttClient.beginMessage(topicMoodStatus);
      mqttClient.print(5);
      mqttClient.endMessage();
      pcSerial.println("Neutral happy\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("NEUTRAL HAPPY");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_NEUTRAL_SAD:
      // write your action code here
      mqttClient.beginMessage(topicMoodStatus);
      mqttClient.print(3);
      mqttClient.endMessage();
      pcSerial.println("Neutral sad\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("NEUTRAL SAD");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_MILK:
      // write your action code here
      mqttClient.beginMessage(topicMoodSection);
      mqttClient.print(2);
      mqttClient.endMessage();
      pcSerial.println("Milk\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("Milk");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_BREAD:
      // write your action code here
      mqttClient.beginMessage(topicMoodSection);
      mqttClient.print(1);
      mqttClient.endMessage();
      pcSerial.println("Bread\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("Bread");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_PANS:
      // write your action code here
      mqttClient.beginMessage(topicMoodSection);
      mqttClient.print(3);
      mqttClient.endMessage();
      pcSerial.println("Pans\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("Pans");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_CUPS:
      // write your action code here
      mqttClient.beginMessage(topicMoodSection);
      mqttClient.print(5);
      mqttClient.endMessage();
      pcSerial.println("Cups\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("Cups");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    case G1_PLATES:
      // write your action code here
      mqttClient.beginMessage(topicMoodSection);
      mqttClient.print(6);
      mqttClient.endMessage();
      pcSerial.println("Plates\n");

      mqttClient.beginMessage(topicVRCommand);
      mqttClient.print("Plates");
      mqttClient.endMessage();
      // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
      break;
    }
    break;
  }
}
