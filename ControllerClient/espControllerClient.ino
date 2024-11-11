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
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };


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

const int removePin = 14; //D5 https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
const int swapPin = 12; //D6
const int rfPin = 13; //D7
const int light1Pin = 15; //RX
const int light2Pin = 16; //D0
const int light3Pin = 0; //D3

// Potentiometer pin
const int potentiometerPin = A0;

// Initialize the WiFi and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

//Switch States
bool RFcurrentState = false;
bool lights1State = false;
bool lights2State = false;
float currentSpeed = 0;
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
    display.println(F("No Connected Trains"));
    display.display();
    delay(2000);

    // Connect to Pi Access Point
    while (connectToWiFi() == false){
      delay(2000);
    }

    // Initialize buttons as inputs
    pinMode(removePin, INPUT);
    pinMode(swapPin, INPUT);
    pinMode(rfPin, INPUT);
    pinMode(light1Pin, INPUT);
    pinMode(light2Pin, INPUT);

    //Connect to MQTT broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);

    while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), "", "")) {
          Serial.println("Public emqx mqtt broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
    }
    client.subscribe("newClient");
}

void connectToWiFi() {
  Serial.printf("Connecting to '%s'\n", wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    Serial.print("Connected. IP: ");
    Serial.println(WiFi.localIP());
    return true
  } else {
    Serial.println("Connection Failed!");
    return false
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
    Serial.println("No current clients connected, nothing to remove...");
    return;
  }
  Serial.println("Removed Client from Controller...");

  showCurrentTopic();
}

void swapEntry() {
  if (topics.size() > 1){
    currentTopicIndex++;
    if (currentTopicIndex >= topics.size()) {
      currentTopicIndex = 0;
    }
    Serial.println(("Swapped to new topic: " + topics[currentTopicIndex]).c_str());
  }
  else{
    Serial.println("Less than 2 clients connected, cannot go to next entry...");
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

void showSpeed(speed){
  display.clearDisplay();
  display.setCursor(0,0);
  display.println((speed));
  display.display();
}

void lights1On(){
  Serial.println("Train command: Lights 1 On");
  client.publish((topics[currentTopicIndex] + "/lights1").c_str(), String("1").c_str());
}

void lights1Off(){
  Serial.println("Train command: Lights 1 Off");
  client.publish((topics[currentTopicIndex] + "/lights1").c_str(), String("0").c_str());
}

void lights2On(){
  Serial.println("Train command: Lights 2 On");
  client.publish((topics[currentTopicIndex] + "/lights2").c_str(), String("1").c_str());
}

void lights2Off(){
  Serial.println("Train command: Lights 2 Off");
  client.publish((topics[currentTopicIndex] + "/lights2").c_str(), String("0").c_str());
}

void lights3On(){
  Serial.println("Train command: Lights 3 On");
  client.publish((topics[currentTopicIndex] + "/lights3").c_str(), String("1").c_str());
}

void lights3Off(){
  Serial.println("Train command: Lights 3 Off");
  client.publish((topics[currentTopicIndex] + "/lights3").c_str(), String("0").c_str());
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
  Serial.println(curvedSpeedValue);
  // Publish potentiometer value to the currently selected speed subtopic
  client.publish((topics[currentTopicIndex] + "/speed").c_str(), String(curvedSpeedValue).c_str());
  showSpeed(String(curvedSpeedValue))
}

//New Client connected
void callback(char *topic, byte *payload, unsigned int length){
  Serial.println("New Client Added to network:");
  
  //for (int i = 0; i < length; i++) {
  //    clientName = clientName += payload[i];
  //}
  payload[length] = '\0';
  String newClientName = (char *)payload;
  
  
  Serial.println(newClientName);
  //Adding client to array
  topics.push_back(newClientName);

  Serial.println("Number of connected clients:");
  Serial.println(topics.size());
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
    showCurrentTopic();
    count2=0;
  }
  count2++;
  count++;

  // Check button presses and perform corresponding actions
  if (digitalRead(removePin) == HIGH) {
    //Remove Entry
    removeEntry();
    delay(200);
  }
  if (digitalRead(swapPin) == HIGH) {
    //Swap Selected Entry
    swapEntry();
    delay(200);
  }

  //Lights
  if ((digitalRead(light1Pin) == HIGH) && (lights1State == false)) {
    lights1On();
    lights1State = true;
    delay(200);
  }
  if ((digitalRead(light1Pin) == LOW) && (lights1State == true)){
    lights1Off();
    lights1State = false;
    delay(200);
  }
  if ((digitalRead(light2Pin) == HIGH) && (lights2State == false)){
    lights2On();
    lights2State = true;
    delay(200);
  }
  if ((digitalRead(light2Pin) == LOW) && (lights2State == true)){
    lights2Off();
    lights2State = false;
    delay(200);
  }

}
