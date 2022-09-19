// Public library preinstalled in Arduino IDE
#include <Ultrasonic.h> 
#include <Servo.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

// Private library, need to have them in the main program folder
#include "PubSubClient.h"
#include "info.h"
#include "WiFiManager.h"
#include "SSD1306.h"

//MQTT Communication associated variables
String inString = ""; 
int testvariable;
char payload_global[100];
boolean flag_payload;
int k = 0; // done target constant

//calculation variables
float magnitudeDD;
float magnitudeTD;
double dotProduct;
int target_x;
int target_y;
float dist_target;
int index1=0;
float VP;
float angleTurn=0;
int z;

//MQTT Setting variables
const char* mqtt_server = "155.246.62.110";
const int mqtt_port = 1883;
const char* MQusername = "jojo";
const char* MQpassword = "hereboy";

//WiFi Define
WiFiClient espClient;
info board_info;
PubSubClient client(espClient);

////////////////////////////////////////Robot Logic Variables//////////////////////////////////////////////////
//////////////////////////Students change this section for their modification//////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////// Wifi Settings variables // Remote students change to your network parameters ///////////////////////
const char* ssid = "Stevens-IoT";
const char* password = "nMN882cmg7";

////////// If on-campus, remember to add your WeMos MAC address to Stevens network ////////////////////////////

//////********CHANGE FOR EACH ARENA***********////////
const char* MQtopic = "louis_lidar1"; // Topic for Arena_1 (EAS011 - South)
//const char* MQtopic = "louis_lidar2"; // Topic for Arena_2 (EAS011 - North)

// Define the DC motor contorl signal pins
#define motorRpin D0  //GPIO pin setting for motorR
#define motorLpin D2  //GPIO pin setting for motorL

Servo motorR;        //Create servo motorR object to control a servo
Servo motorL;        //Create servo motorL object to control a servo

// Define the Ultrasonic sensor pins
Ultrasonic ultrasonic_front(D8, D5); // Ultasonic sensor, front (trig, echo)
Ultrasonic ultrasonic_right(D9, D6); // Ultasonic sensor, right (trig, echo)
Ultrasonic ultrasonic_left(D10, D7); // Ultasonic sensor, left (trig, echo)

// Define the OLED display pins D14 SDA, D15 SCL
SSD1306  display(0x3C, D14, D15); //Address set here 0x3C
                                  //Pins defined as D2 (SDA/Serial Data) and D5 (SCK/Serial Clock).

////////////////////////// Define the variables needed for your algorithm ////////////////////////////////////

int distance_left = 0;
int distance_right = 0;
int distance_front = 0;
float x;
float y;
double previous_x = -1; // The previous coordinate
double previous_y = -1; // The previous coordinate

////////////////////////////////////Robot Logic Variables End//////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Setup the wifi, Don't touch
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  randomSeed(micros());
}

// MQTT msg callback
void callback(char* topic, byte* payload, unsigned int length) {
  for (int i = 0; i < length; i++) {
    payload_global[i] = (char)payload[i];
  }
  payload_global[length] = '\0';
  flag_payload = true;
}

// MQTT reconnection setup
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQusername,MQpassword)) {
       client.subscribe(MQtopic);
    }
  }
}

/////////////////////////////// SETUP LOOP. Don't Touch ///////////////////////////////////////////

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); 
  motorR.attach(motorRpin);   //motorR is attached using the motorRpin
  motorL.attach(motorLpin);   //motorL is attached using the motorLpin
  // Display Setup
  display.init();
  display.flipScreenVertically();
  display.drawString(0, 0, "Stevens Smart Robot 1");
  display.display();
  
}

/////////////////////////////////// MAIN LOOP /////////////////////////////////////////////

void loop() { 
  //subscribe the data from mqtt server
  if (!client.connected()) {
      reconnect();
  }
  const char *msg = "target";
  char temp1[50];
  sprintf(temp1,"%d",k);
  const char *c = temp1;
  
  client.publish( msg , c);
  client.loop();
  String payload(payload_global);
 
  int testCollector[10];
  int count = 0;
  int prevIndex, delimIndex;
  
  prevIndex = payload.indexOf('[');
  while( (delimIndex = payload.indexOf(',', prevIndex +1) ) != -1){
    testCollector[count++] = payload.substring(prevIndex+1, delimIndex).toInt();
    prevIndex = delimIndex;
  }
  delimIndex = payload.indexOf(']');
  testCollector[count++] = payload.substring(prevIndex+1, delimIndex).toInt();
  
  // Robot location x,y from MQTT subscription variable testCollector
 


  x = testCollector[0];
  y = testCollector[1];


  //Setting up the target destination, xt[]={A,B,C,D ~} 
  double xt[] = {1400,650,150,2000};
  double yt[] = {150,150,150,700};




////////////////////////////////////////Robot Logic Begin//////////////////////////////////////////////////
////////////////////////Students change this section for their modification////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////// find the closest obstacle ///////////////////////////////////////

  // Read the distances from the Utrasonic sensors. Output unit is mm
  distance_left = ultrasonic_left.read(CM)*10;
  distance_front = ultrasonic_front.read(CM)*10;
  distance_right = ultrasonic_right.read(CM)*10;


//calculations logic
int target_x=xt[index1];
int target_y=yt[index1];

if(x!=previous_x||y!=previous_y&&x!=0){
  magnitudeDD=sqrt(sq(x-previous_x)+sq(y-previous_y));
  
  magnitudeTD=sqrt(sq(xt[index1]-x)+sq(yt[index1]-y));
  
  dotProduct=((x-previous_x)*(xt[index1]-x))+((y-previous_y)*(yt[index1]-y));
  
  dist_target= sqrt(sq(xt[index1]-x)+sq(yt[index1]-y));
  
  angleTurn=degrees(acos(dotProduct/(magnitudeDD*magnitudeTD)));

  VP=(x-previous_x)*(yt[index1]-y)-(y-previous_y)*(xt[index1]-x);
  if (angleTurn>0){
    angleTurn=angleTurn;
  }
  else{
    angleTurn=0;
  }
}
else{
  angleTurn=0;
}




    // Display the x,y location in the OLED
    display.clear(); // Clear the buffer
    String str_1 = "z: " + String(z); // We need to cast the x value from 'int' to 'String'
    String str_2 = "td: " + String(dist_target);
    String str_3 = "tg: " + String(xt[index1]); // Same thing for the y value
    String str_4 = "an: " + String(angleTurn);
    String str_5 = "dd: " + String(magnitudeDD);
    String str_6 = "td: " + String(magnitudeTD);
    display.drawString(0, 0, "Stevens Smart Robot 1");
    display.drawString(0, 15, str_1);
    display.drawString(0, 30, str_2);
    display.drawString(0, 45, str_3);
    display.drawString(55, 30, str_4);
    display.drawString(55, 45, str_5);
    display.display();
   
if(distance_front <= 50){
  motorR.write(70);
  motorL.write(70);
  delay(500);
  if(VP<0){
      motorR.write(70); 
      motorL.write(110); 
      delay(500);
    }
    else if(VP>0){
      motorR.write(110); 
      motorL.write(70); 
      delay(500);
    }
}
 else if(distance_right <= 50){
  motorR.write(110);
  motorL.write(70);
  delay(500);
}
else if(distance_left <= 50){
  motorR.write(70);
  motorL.write(110);
  delay(500);
}



if(dist_target<=100){
  motorR.write(90);
  motorL.write(90);
  delay(2000);
  index1++;
}
else{
    if(VP<0){
      motorR.write(70); 
      motorL.write(110); 
      z=(1000 * (angleTurn/90));
      delay(z);
      motorR.write(110); 
      motorL.write(110); 
      //delay(500);
    }
    else if(VP>0){
      motorR.write(110); 
      motorL.write(70); 
      z=(1000 * (angleTurn/90));
      delay(z);
      motorR.write(110); 
      motorL.write(110);
      //delay(500);
    }
}


 

  previous_x = x; 
  previous_y = y;

 motorR.write(110);
 motorL.write(110); 

////////////////////////////////////////Robot Logic End////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
}
