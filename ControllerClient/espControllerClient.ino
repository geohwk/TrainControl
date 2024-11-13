#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <vector>
#include <string>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Screen Setup

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000110, 0b00000000,
  0b00000001, 0b11000000,
  0b00000000, 0b11110000,
  0b11111111, 0b11111111,
  0b00000000, 0b11000000,
  0b00000011, 0b00000000,
  0b11111111, 0b11111111,
  0b00001111, 0b00000000,
  0b00000011, 0b10000000,
  0b00000000, 0b01100000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000 };



// WiFi credentials
char wifi_ssid[] = "TrainControl";
char wifi_password[] = "ringousel";

//MQTT Broker
char mqtt_broker[] = "192.168.1.7";
const int mqtt_port = 1883;
char brokerUser[] = "";  // exp: myemail@mail.com
char brokerPass[] = "";

// MQTT topics
std::vector<String> topics;
int currentTopicIndex = 0; // Index of the currently selected topic

// GPIO pins for buttons

const int removePin = 0; //D5 https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
const int swapPin = 2; //D6
const int rfPin = 3; //D7
const int light1Pin = 14; //RX
const int light2Pin = 12; //D0
const int light3Pin =13; //D3

// Potentiometer pin
const int potentiometerPin = A0;

// Initialize the WiFi and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

const String clientName = "controller1";
const String serialTopic = "serial_output";

//Switch States
bool RFcurrentState = false;
bool lights1State = false;
bool lights2State = false;
bool lights3State = false;
float currentSpeed = 0;

void MQTTSerialPrint(String text)
{
    client.publish((clientName + "/" + serialTopic).c_str(), String(text).c_str());
    Serial.println(text);
}

void setup() 
{
    Serial.begin(115200);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }

    // Clear the buffer
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println(F("Waiting to connect to AP..."));
    display.display();
    delay(2000);

    // Connect to Pi Access Point
    
    while (connectToWiFi() == false){
      connectToWiFi();
      delay(2000);
    }
    
    display.clearDisplay();
    display.println(F("Connected to AP!"));
    display.display();
    delay(2000);

    display.clearDisplay();
    display.println(F("No Connected Trains..."));
    display.display();

    // Initialize buttons as inputs
    pinMode(removePin, INPUT_PULLUP);
    pinMode(swapPin, INPUT_PULLUP);
    pinMode(rfPin, INPUT);
    pinMode(light1Pin, INPUT);
    pinMode(light2Pin, INPUT);
    pinMode(light3Pin, INPUT);

    
    //Connect to MQTT broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);

    while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.println("Attempting connection to MQTT Broker " + client_id);
      if (client.connect(client_id.c_str(), "", "")) {
          MQTTSerialPrint("Public emqx mqtt broker connected");
      } else {
          Serial.println("failed with state " + String(client.state()));
          delay(2000);
      }
    }
    client.subscribe("newClient");
    
}

bool connectToWiFi() {
  Serial.printf("Connecting to '%s'\n", wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    Serial.print("Connected. IP: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("Connection Failed!");
    return false;
  }
}

void removeEntry() {
  if (!topics.empty()) {
    topics.erase(topics.begin() + currentTopicIndex);
    if (currentTopicIndex >= topics.size()) {
      currentTopicIndex = topics.size() - 1;
    }
  }
  else{
    MQTTSerialPrint("No current clients connected, nothing to remove...");
    return;
  }
  MQTTSerialPrint("Removed Client from Controller...");

  showCurrentTopic();
}

void swapEntry() {
  if (topics.size() > 1){
    currentTopicIndex++;
    if (currentTopicIndex >= topics.size()) {
      currentTopicIndex = 0;
    }
    MQTTSerialPrint(("Swapped to new topic: " + topics[currentTopicIndex]).c_str());
  }
  else{
    MQTTSerialPrint("Less than 2 clients connected, cannot go to next entry...");
    return;
  }
  showCurrentTopic();
}

void showCurrentTopic(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.println((topics[currentTopicIndex]));
  display.display();
}

void showSpeed(float speed){
  display.clearDisplay();
  display.setCursor(0,0);
  display.println((speed));
  display.display();
}

void lightsControl(String topicText, int value){
  MQTTSerialPrint("Train command: " + topicText + " On");
  client.publish((topics[currentTopicIndex] + topicText).c_str(), String(value).c_str());
}

void readSpeed(){
  float potValue = analogRead(potentiometerPin);

  if(abs(potValue-currentSpeed)<5){
    return;
  }
  currentSpeed = potValue;
  float speedValue = (potValue/1023)*100;

  float curvedSpeedValue;
  if(speedValue <= 5){
    curvedSpeedValue = 5*speedValue;
  }
  else if(speedValue > 5){
    curvedSpeedValue = ((75*speedValue)/95) + (2000/95);
  }

  if(curvedSpeedValue < 5){
    curvedSpeedValue = 0;
  }
  
  if(digitalRead(rfPin) == HIGH){
    curvedSpeedValue = (curvedSpeedValue*-1);
  }
  MQTTSerialPrint(String(curvedSpeedValue));
  // Publish potentiometer value to the currently selected speed subtopic
  client.publish((topics[currentTopicIndex] + "/speed").c_str(), String(curvedSpeedValue).c_str());
  showSpeed(curvedSpeedValue);
}

//New Client connected
void callback(char *topic, byte *payload, unsigned int length){
  MQTTSerialPrint("New Client Added to network:");
  
  //for (int i = 0; i < length; i++) {
  //    clientName = clientName += payload[i];
  //}
  payload[length] = '\0';
  String newClientName = (char *)payload;
  
  //Adding client to array
  topics.push_back(newClientName);

  MQTTSerialPrint(newClientName);
  MQTTSerialPrint("Number of connected clients:" + String(topics.size()));

  showCurrentTopic();
}

int count=0;
int count2=0;

void loop() 
{
  client.loop();
  if(count>10000){
    if(topics.size()>0){
      readSpeed();
    }
    count=0;
  }
  if(count2>100000){
    if(topics.size()>0){
      showCurrentTopic();
    }
    count2=0;
  }
  count2++;
  count++;
  
  // Check button presses and perform corresponding actions
  if (digitalRead(removePin) == LOW) {
    //Serial.println("REMOVE");
    //Remove Entry
    removeEntry();
    delay(200);
  }
  if (digitalRead(swapPin) == LOW) {
    //Serial.println("SWAP");
    //Swap Selected Entry
    swapEntry();
    delay(200);
  }

  //Lights
  if ((digitalRead(light1Pin) == HIGH) && (lights1State == false)) {
    //Serial.println("LIGHT1 1");
    lightsControl("/lights1", 1);
    lights1State = true;
    delay(200);
  }
  if ((digitalRead(light1Pin) == LOW) && (lights1State == true)){
    //Serial.println("LIGHT1 0");
    lightsControl("/lights1", 0);
    lights1State = false;
    delay(200);
  }
  if ((digitalRead(light2Pin) == HIGH) && (lights2State == false)){
    //Serial.println("LIGHT2 1");
    lightsControl("/lights2", 1);
    lights2State = true;
    delay(200);
  }
  if ((digitalRead(light2Pin) == LOW) && (lights2State == true)){
    //Serial.println("LIGHT2 0");
    lightsControl("/lights2", 0);
    lights2State = false;
    delay(200);
  }
  if ((digitalRead(light3Pin) == HIGH) && (lights3State == false)){
    //Serial.println("LIGHT3 1");
    lightsControl("/lights3", 1);
    lights3State = true;
    delay(200);
  }
  if ((digitalRead(light3Pin) == LOW) && (lights3State == true)){
    //Serial.println("LIGHT3 0");
    lightsControl("/lights3", 0);
    lights3State = false;
    delay(200);
  }

}
