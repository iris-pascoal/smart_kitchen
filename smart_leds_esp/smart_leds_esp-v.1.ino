#include <FastLED.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <String.h>
#include <WiFi.h>
#include "secrets.h"
#include <BlockNot.h>


WiFiClient wifi_client;
PubSubClient client(wifi_client);

char ssid[] = SECRET_SSID;       
char pass[] = SECRET_PASS; 

BlockNot sectionTimer(6, SECONDS);
BlockNot shelfTimer(6,  SECONDS);

///////////////////////////////////////////////////////
//.........MAIN LED STRIP INITIALIZATION.............//
///////////////////////////////////////////////////////
#define LED_PIN     14 //main led strip pin 
#define NUM_LEDS    173 //total number of leds in the main strip
#define NUM_SEC     6 //total number of led sections

CRGB leds[NUM_LEDS];//main led strip controller
CRGB moodColor_aux;//color control for mood service
int moodColor[6][3] = {{255,255,255},{255,255,255},{255,255,255},{255,255,255},{255,255,255},{255,255,255}};//color associated withe ach state
int secColor[3] = {255,255,255};//color control for main strip sections
boolean moodFlag = false;

int main_led_map[] = {26, 30, 28, 29, 29, 31};//number of leds per section: 0-if section is off; 1- if section is on
int section = -1; // section of leds being used

///////////////////////////////////////////////////////
//.........SHELVES LED STRIP INITIALIZATION..........//
///////////////////////////////////////////////////////
#define LED_PIN_1 22
#define LED_PIN_2 33
#define NUM_STRIPS 2 //two led strips connected in series
#define NUM_LEDS_PER_STRIP 36
#define NUM_LEDS_1 74
#define NUM_SEC_1 8

CRGB leds_1[NUM_LEDS_1]; // 2 led strips
int shelfColor[NUM_SEC_1][3] = {{255,255,255}, {255,255,255},{255,255,255},{255,255,255},{255,255,255},{255,255,255},{255,255,255},{255,255,255}};
int sec_led_map[NUM_SEC_1] = {9,9,9,9,9,9,9,9};//number of leds per section: 
int section_1 = -1; // section of leds being used


//////////////////////////////////////////////////////
//....................MQTT TOPICS...................//
/////////////////////////////////////////////////////
// Update these with values suitable for your network.
const char* server = "192.168.0.100";
const char* topicMoodColor_6 = "mood/color/6";//subscribe
const char* topicMoodColor_1 = "mood/color/1";//subscribe
const char* topicMoodColor_2 = "mood/color/2";//subscribe
const char* topicMoodColor_3 = "mood/color/3";//subscribe
const char* topicMoodColor_4 = "mood/color/4";//subscribe
const char* topicMoodColor_5 = "mood/color/5";//subscribe
const char* topicMoodStatus = "mood/state";//subscribe
const char* topicMoodSection = "mood/section";//subscribe
const char* topicMoodSectionColor = "mood/section/color";//subscribe
const char* topicShelf1 = "shelf1";//subscribe
const char* topicShelf2 = "shelf2";//subscribe
const char* topicShelfColor_0= "shelf/color/0";//subscribe
const char* topicShelfColor_1= "shelf/color/1";//subscribe
const char* topicShelfColor_2= "shelf/color/2";//subscribe
const char* topicShelfColor_3= "shelf/color/3";//subscribe
const char* topicShelfColor_4= "shelf/color/4";//subscribe
const char* topicShelfColor_5= "shelf/color/5";//subscribe
const char* topicShelfColor_6= "shelf/color/6";//subscribe
const char* topicShelfColor_7= "shelf/color/7";//subscribe
const char* acdTopic = "automatic_cabinet_door/controls";//subscribe



void callback(char* topic, byte* payload, unsigned int length) {
  char payloadChar[length+1];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int i = 0;

  for (i=0;i<length;i++) {
    payloadChar[i] = (char)payload[i];
    Serial.print((char)payload[i]);
  }
  payloadChar[i] = '\0';
  Serial.println();
  
  if(strcmp(topic,topicMoodSection) == 0){//mood sections topic  
    CRGB sec_color = CRGB(secColor[0], secColor[1], secColor[2]);

    int len = strlen(payloadChar);
    client.publish("mood/section/status", payloadChar, true);

    sectionTimer.RESET;
    
    
    if (strcmp(payloadChar, "1")==0) {//1st module of leds ON
      Serial.println("1st module of leds ON");
      handle_led_sec(5, sec_color, main_led_map, NUM_LEDS, leds, 0);
    }else if(strcmp(payloadChar, "2")==0){//2nd module of led ON
      Serial.println("2nd module of leds ON"); 
      handle_led_sec(4, sec_color, main_led_map, NUM_LEDS, leds, 0);
    }else if(strcmp(payloadChar, "3")==0){
      Serial.println("3rd module of leds ON"); 
      handle_led_sec(3, sec_color, main_led_map, NUM_LEDS, leds, 0);
    }else if(strcmp(payloadChar, "4")==0){
      Serial.println("4th module of leds ON"); 
      handle_led_sec(2, sec_color, main_led_map, NUM_LEDS, leds, 0);
    }else if(strcmp(payloadChar, "5")==0){
      Serial.println("5th module of leds ON"); 
      handle_led_sec(1, sec_color, main_led_map, NUM_LEDS, leds, 0);
    }else if(strcmp(payloadChar, "6")==0){
      Serial.println("6th module of leds ON"); 
      handle_led_sec(0, sec_color, main_led_map, NUM_LEDS, leds, 0);
    }else if(strcmp(payloadChar, "0")==0){
      //off
      handle_led_sec(section,moodColor_aux , main_led_map, NUM_LEDS, leds, 0);
      section = -1;
    }  
    
  }
  if(strcmp(topic,topicMoodSectionColor) == 0){
    client.publish("mood/section/color/status", payloadChar, true);
    char * token = strtok(payloadChar, ",");
    int aux[3];
    aux[0] = atoi(token);
    for(int i = 1; i<3; i++) {
      token = strtok(NULL, ",");
      aux[i] = atoi(token);
    }
    for(i = 0; i<3; i++)
      secColor[i] = aux[i];

    
  }
  if(strcmp(topic, topicMoodStatus)==0){//led moods status
    CRGB aux_color;
    client.publish("mood/state/status", payloadChar, true);
    
    if(strcmp(payloadChar, "1")==0){ //neutral mood
      Serial.println("Mood service ON"); 
      //Serial.println("waiting for input......"); 
      aux_color = CRGB(moodColor[0][0], moodColor[0][1], moodColor[0][2]);
      handle_mood_service(aux_color);
      moodFlag=true;   
    } else if(strcmp(payloadChar, "0")==0){ //neutral mood
      Serial.println("Mood service OFF"); 
      handle_mood_service(CRGB::Black);
      moodFlag = false;
    } else if(moodFlag){
      if (strcmp(payloadChar, "2")==0) {
        aux_color = CRGB(moodColor[1][0], moodColor[1][1], moodColor[1][2]);
        Serial.println("Negative mood");  
        handle_mood_service(aux_color);
      }else if(strcmp(payloadChar, "3")==0){
        aux_color = CRGB(moodColor[2][0], moodColor[2][1], moodColor[2][2]);
        Serial.println("Medium Negative mood"); 
        handle_mood_service(aux_color);
      }else if(strcmp(payloadChar, "4")==0){ 
        aux_color = CRGB(moodColor[3][0], moodColor[3][1], moodColor[3][2]);
        Serial.println("Neutral mood");  
        handle_mood_service(aux_color);
      }else if(strcmp(payloadChar, "5")==0){ 
        aux_color = CRGB(moodColor[4][0], moodColor[4][1], moodColor[4][2]);
        Serial.println("Medium Positive mood");  
        handle_mood_service(aux_color);
      }else if(strcmp(payloadChar, "6")==0){ 
        aux_color = CRGB(moodColor[5][0], moodColor[5][1], moodColor[5][2]);
        Serial.println("Positive mood"); 
        handle_mood_service(aux_color);
      }
    } 
   }
   if(strcmp(topic, topicMoodColor_1)==0){
    client.publish("mood/color/1/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        moodColor[0][i] = aux[i];  

      
   }
   if(strcmp(topic, topicMoodColor_2)==0){
    client.publish("mood/color/2/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        moodColor[1][i] = aux[i];  
      
   }
   if(strcmp(topic, topicMoodColor_3)==0){
    client.publish("mood/color/3/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        moodColor[2][i] = aux[i]; 
       
   }
   if(strcmp(topic, topicMoodColor_4)==0){
    client.publish("mood/color/4/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        moodColor[3][i] = aux[i];  
      
   }
   if(strcmp(topic, topicMoodColor_5)==0){
    client.publish("mood/color/5/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        moodColor[4][i] = aux[i];  
      
   }
   if(strcmp(topic, topicMoodColor_6)==0){
    client.publish("mood/color/6/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        moodColor[5][i] = aux[i];  
      
   }
   if(strcmp(topic, topicShelf1)==0){//led shelves topic
    CRGB aux_color;
    shelfTimer.RESET;
    client.publish("shelf1/status", payloadChar, true);

    
      if (strcmp(payloadChar, "1")==0) {
        Serial.println("WINE");
        aux_color = CRGB(shelfColor[0][0], shelfColor[0][1], shelfColor[0][2]);
        handle_led_sec(0, aux_color, sec_led_map, NUM_LEDS_1, leds_1, 1);
      } 
      if(strcmp(payloadChar, "2")==0){
         Serial.println("GIN");
         aux_color = CRGB(shelfColor[1][0], shelfColor[1][1], shelfColor[1][2]);
        handle_led_sec(1, aux_color, sec_led_map, NUM_LEDS_1, leds_1, 1);
      }
      if(strcmp(payloadChar, "3")==0){
        Serial.println("COOKIES");
         aux_color = CRGB(shelfColor[2][0], shelfColor[2][1], shelfColor[2][2]);
        handle_led_sec(2, aux_color, sec_led_map, NUM_LEDS_1, leds_1, 1);
      } 
      if(strcmp(payloadChar, "4")==0){
        Serial.println("APPLES");
         aux_color = CRGB(shelfColor[3][0], shelfColor[3][1], shelfColor[3][2]);
        handle_led_sec(3, aux_color, sec_led_map, NUM_LEDS_1, leds_1, 1);
      }
      if(strcmp(payloadChar, "0")==0){
        Serial.println("OFF");
        aux_color = CRGB(0,0,0);
        handle_led_sec(section_1, aux_color, sec_led_map, NUM_LEDS_1, leds_1, 1);
        section_1 = -1;
      }
    }
    if(strcmp(topic, topicShelf2) == 0){
      client.publish("shelf2/status", payloadChar, true);

      CRGB aux_color;
      shelfTimer.RESET;
      if(strcmp(payloadChar, "1")==0){
        Serial.println("PASTA");
        aux_color = CRGB(shelfColor[4][0], shelfColor[4][1], shelfColor[4][2]);
        handle_led_sec(4, aux_color, sec_led_map, NUM_LEDS_1, leds_1, 1);
      } 
      if(strcmp(payloadChar, "2")==0){
        Serial.println("KETCHUP");
        aux_color = CRGB(shelfColor[5][0], shelfColor[5][1], shelfColor[5][2]);
        handle_led_sec(5, aux_color, sec_led_map, NUM_LEDS_1, leds_1, 1);
      }  
      if(strcmp(payloadChar, "3")==0){
        Serial.println("RICE");
        CRGB aux_color = CRGB(shelfColor[6][0], shelfColor[6][1], shelfColor[6][2]);
        handle_led_sec(6, aux_color, sec_led_map, NUM_LEDS_1, leds_1, 1);
      } 
      if(strcmp(payloadChar, "4")==0){
        Serial.println("WATER");
        aux_color = CRGB(shelfColor[7][0], shelfColor[7][1], shelfColor[7][2]);
        handle_led_sec(7, aux_color, sec_led_map, NUM_LEDS_1, leds_1, 1);
      } 
      if(strcmp(payloadChar, "0")==0){
        Serial.println("OFF");
        aux_color = CRGB(0,0,0);
        handle_led_sec(section_1, aux_color, sec_led_map, NUM_LEDS_1, leds_1, 1);
        section_1=-1;
      } 
    }
    if(strcmp(topic,topicShelfColor_0) == 0){
      client.publish("shelf/color/0/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        shelfColor[0][i] = aux[i];  
     
    }
    if(strcmp(topic,topicShelfColor_1) == 0){
      client.publish("shelf/color/1/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        shelfColor[1][i] = aux[i]; 

      
    }
    if(strcmp(topic,topicShelfColor_2) == 0){
      client.publish("shelf/color/2/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        shelfColor[2][i] = aux[i];  

      
    }
    if(strcmp(topic,topicShelfColor_3) == 0){
      client.publish("shelf/color/3/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        shelfColor[3][i] = aux[i];  
      
    }
    if(strcmp(topic,topicShelfColor_4) == 0){
      client.publish("shelf/color/4/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        shelfColor[4][i] = aux[i];  
      
    }
    if(strcmp(topic,topicShelfColor_5) == 0){
      client.publish("shelf/color/5/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        shelfColor[5][i] = aux[i];  
      
    }
    if(strcmp(topic,topicShelfColor_6) == 0){ 
      client.publish("shelf/color/6/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        shelfColor[6][i] = aux[i]; 
      
    }
    if(strcmp(topic,topicShelfColor_7) == 0){
      client.publish("shelf/color/7/status", payloadChar, true);
      char * token = strtok(payloadChar, ",");
      int aux[3];
      aux[0] = atoi(token);
      for(int i = 1; i<3; i++) {
        token = strtok(NULL, ",");
        aux[i] = atoi(token);
      }
      for(i = 0; i<3; i++)
        shelfColor[7][i] = aux[i];  
      
    }
}

void reconnect() {
 // Loop until we're reconnected
 //while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 String clientId = "YunClient";
 clientId += String(random(0xffff), HEX);//CHANGED ID HERE! 
 if (client.connect(clientId.c_str())) {
  Serial.println("connected");
  // Once connected, publish an announcement...
  // ... and subscribe to topic  
  client.subscribe(topicMoodStatus);
  client.subscribe(topicMoodColor_6);
  client.subscribe(topicMoodColor_5);
  client.subscribe(topicMoodColor_4);
  client.subscribe(topicMoodColor_3);
  client.subscribe(topicMoodColor_2);
  client.subscribe(topicMoodColor_1);
  client.subscribe(topicMoodSection);
  client.subscribe(topicMoodSectionColor);
  client.subscribe(topicShelf1);
  client.subscribe(topicShelf2);
  client.subscribe(topicShelfColor_0);
  client.subscribe(topicShelfColor_1);
  client.subscribe(topicShelfColor_2);
  client.subscribe(topicShelfColor_3);
  client.subscribe(topicShelfColor_4);
  client.subscribe(topicShelfColor_5);
  client.subscribe(topicShelfColor_6);
  client.subscribe(topicShelfColor_7);
  client.subscribe(acdTopic);


  
 } else {
  Serial.print("failed, rc=");
  Serial.print(client.state());
  Serial.println(" try again in 5 seconds");
  // Wait 5 seconds before retrying
  delay(5000);
  }
 //}
}


void setup() {
  Serial.begin(9600);
  delay(5000);
  //Bridge.begin();
  //initialize led strips
  
 
 FastLED.addLeds<WS2812,LED_PIN , GRB>(leds, NUM_LEDS); //main led strip
 FastLED.addLeds<WS2812, LED_PIN_1, GRB>(leds_1,0, NUM_LEDS_PER_STRIP); //shelf leds strip
 FastLED.addLeds<WS2812, LED_PIN_2, GRB>(leds_1,NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP); //shelf leds strip
  
  Serial.print("\n\r Initializing Wifi \n\r");
  
  WiFi.begin(ssid,pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(F("\n\r WiFi connected!"));
  
  Serial.println("Atempting to connect to the network........");
  client.setServer(server, 1883);
  client.setCallback(callback);

 //blinking all leds
   for(int j = 0; j<5; j++){
    for(int i = 0; i < NUM_LEDS; i ++){
      leds[i] = CRGB::Blue;
    }
    for(int i = 0; i < NUM_LEDS_1; i ++){
      leds_1[i] = CRGB::Blue;
    }
    FastLED.show();
    delay(500);
    for(int i = 0; i < NUM_LEDS; i ++){
      leds[i] = CRGB::Black;
      
    }
    for(int i = 0; i < NUM_LEDS_1; i ++){
      leds_1[i] = CRGB::Black;
    }
    FastLED.show();
    delay(500);
   }

}

void loop() {
 while (!client.connected()) {
  //WifiAuthentication();
  Serial.println("loop");
  reconnect();
 }
 client.loop();

 if(sectionTimer.TRIGGERED){
  Serial.println("timer!!!!!!");
  //turn off leds
  handle_led_sec(section,moodColor_aux , main_led_map, NUM_LEDS, leds, 0);
  Serial.println("timer 1 off");
  section = -1;

 }
 if(shelfTimer.TRIGGERED){
  handle_led_sec(section_1, CRGB::Black, sec_led_map, NUM_LEDS_1, leds_1, 1); 
  Serial.println("timer 2 off");
 }
 
}

///////////////////////////////////////////////////////
//...................LED MOOD CONTROL................//
///////////////////////////////////////////////////////

void handle_mood_service(CRGB color){//if the mood is positive leds->blue; if the mood is negative the light should transmit positive mood ->blue
  int offset = -1;
  moodColor_aux = color;
  if(section!=-1){ //section is being used 
    for(int i = 0; i< section; i++){
      offset = offset + main_led_map[i];//calculating section offset
    }
    for ( int i = 0; i<NUM_LEDS; i++){
      if(i<=offset || i>offset+main_led_map[section]){
         leds[i] = color;
      }
    }
  }else{
    for ( int i = 0; i<NUM_LEDS; i++){
         leds[i] = color;
      }
    }

 Serial.println("color changed!!!");
  FastLED.show();
  delay(500);
}
////////////////////////////////////////////////////////
//....................LED SECTION CONTROL..............//
////////////////////////////////////////////////////////
void handle_led_sec(int sec, CRGB led_color, int led_map[], int num_leds, CRGB led[], int id){
  int offset = 0;

  Serial.println("here!!!!!");
  for(int i = 0; i< sec; i++){
    offset = offset + led_map[i];//calculating section offset
  }
  for (int i = 0; i<num_leds; i++){
    if(i>=offset && i<offset+led_map[sec]){
      led[i]=led_color;
    }
    else{
      if(id==0){
        if(moodFlag){//mood service is being used
          led[i] = moodColor_aux;
        }
        else{
          led[i] = CRGB::Black;
        }
      }else led[i] = CRGB::Black;
    }
  }
  if(id == 0)
    section = sec;//updating
  else section_1 = sec;
 
  FastLED.show();
  delay(500); 
  Serial.println("finished"); 
}
