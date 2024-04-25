#include <DRV8833.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <vector>

// WiFi credentials
const char * wifi_ssid = "TrainControl";
const char * wifi_password = "ringousel";

//MQTT Broker
const char* mqtt_broker = "192.168.1.7";
const int mqtt_port = 1883;
const char* brokerUser = "";  // exp: myemail@mail.com
const char* brokerPass = "";

// MQTT topics
std::vector<String> topics;
int currentTopicIndex = 0; // Index of the currently selected topic

// GPIO pins for buttons
const int button1Pin = D1; 
const int button2Pin = D2; //Remove client from list
const int button3Pin = D3; //Change selected client
const int button4Pin = D4;
const int button5Pin = D5;
const int button6Pin = D6;

// Potentiometer pin
const int potentiometerPin = A0;

// Initialize the WiFi and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

// Function prototypes
void connectToWiFi();
void reconnect();
void button1Pressed();
void button2Pressed();
void button3Pressed();
void button4Pressed();
void button5Pressed();
void button6Pressed();
void callback();

void setup() 
{
    Serial.begin(115200);

    // Initialize buttons as inputs
    pinMode(button1Pin, INPUT_PULLUP);
    pinMode(button2Pin, INPUT_PULLUP);
    pinMode(button3Pin, INPUT_PULLUP);
    pinMode(button4Pin, INPUT_PULLUP);
    pinMode(button5Pin, INPUT_PULLUP);
    pinMode(button6Pin, INPUT_PULLUP);

    // Connect to Pi Access Point
    connectToWiFi();
    
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
  } else {
    Serial.println("Connection Failed!");
  }
}

// Button action functions

/*Button1: Adds another train topic to the list
void button1Pressed() {
  if (topics.size() < 10) { // Limit the number of topics to 10
    topics.push_back("train" + String(topics.size() + 1));
  }
}
*/
//Button2: Remove currently selected topic from the list
void button2Pressed() {
  if (!topics.empty()) {
    topics.erase(topics.begin() + currentTopicIndex);
    if (currentTopicIndex >= topics.size()) {
      currentTopicIndex = topics.size() - 1;
    }
  }

  Serial.println("Removed Client from Controller")

}

//Button3: Change selected topic
void button3Pressed() {
  currentTopicIndex++;
  if (currentTopicIndex >= topics.size()) {
    currentTopicIndex = 0;
  }

  Serial.println(("Swapped to new topic: " + topics[currentTopicIndex]).c_str())

}

void button4Pressed() {
  // Action for Button 4
}

void button5Pressed() {
  // Action for Button 5
}

void button6Pressed() {
  // Action for Button 6
}

void readSpeed(){
  // Read potentiometer value
  int potValue = analogRead(potentiometerPin);
  
  int speedValue = (potValue/1023)*100;

  // Publish potentiometer value to the currently selected speed subtopic
  client.publish((topics[currentTopicIndex] + "/speed").c_str(), String(speedValue).c_str());
}

//New Client connected
void callback(char *topic, byte *payload, unsigned int length){
  Serial.println("New Client Added to network:");
  String str;
  for (int i = 0; i < length; i++) {
      str += byteArray[i];
  }
  Serial.println(str);

  //Adding client to array
  topics.push_back(str);
}

void loop() 
{
  client.loop();

  readSpeed();

  // Check button presses and perform corresponding actions
  if (digitalRead(button1Pin) == LOW) {
    button1Pressed();
    delay(100); // Debounce delay
  }
  if (digitalRead(button2Pin) == LOW) {
    button2Pressed();
    delay(100); // Debounce delay
  }
  if (digitalRead(button3Pin) == LOW) {
    button3Pressed();
    delay(100); // Debounce delay
  }
  if (digitalRead(button4Pin) == LOW) {
    button4Pressed();
    delay(100); // Debounce delay
  }
  if (digitalRead(button5Pin) == LOW) {
    button5Pressed();
    delay(100); // Debounce delay
  }
  if (digitalRead(button6Pin) == LOW) {
    button6Pressed();
    delay(100); // Debounce delay
  }

}
