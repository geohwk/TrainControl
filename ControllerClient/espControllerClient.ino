#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <vector>
#include <string>

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
const int light1Pin = 3; //RX
const int light2Pin = 16; //D0

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

    // Connect to Pi Access Point
    connectToWiFi();

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
  } else {
    Serial.println("Connection Failed!");
  }
}

void removeEntry() {
  /*if (!topics.empty()) {
    topics.erase(topics.begin() + currentTopicIndex);
    if (currentTopicIndex >= topics.size()) {
      currentTopicIndex = topics.size() - 1;
    }
  }*/
  Serial.println("Removed Client from Controller");
}

void swapEntry() {
  /*
  currentTopicIndex++;
  if (currentTopicIndex >= topics.size()) {
    currentTopicIndex = 0;
  }
  Serial.println(("Swapped to new topic: " + topics[currentTopicIndex]).c_str());
  */
  Serial.println("Swap Entry");
}

void lights1On(){
  Serial.println("Train command: Lights 1 On");
}

void lights1Off(){
  Serial.println("Train command: Lights 1 Off");
}

void lights2On(){
  Serial.println("Train command: Lights 2 On");
}

void lights2Off(){
  Serial.println("Train command: Lights 2 Off");
}


void readSpeed(){
  int potValue = analogRead(potentiometerPin);

  if(abs(potValue-currentSpeed)<5){
    return;
  }
  currentSpeed = potValue;
  float speedValue = (potValue/1023)*100;

  if(digitalRead(rfPin) == HIGH){
    speedValue = (speedValue*-1)
  }
    
   
  Serial.println(speedValue);
  // Publish potentiometer value to the currently selected speed subtopic
  client.publish((topics[currentTopicIndex] + "/speed").c_str(), String(speedValue).c_str());
}

//New Client connected
void callback(char *topic, byte *payload, unsigned int length){
  Serial.println("New Client Added to network:");
  char clientName[length];
  for (int i = 0; i < length; i++) {
      clientName[i] = byteArray[i];
  }
  Serial.println(clientName);
  //Adding client to array
  topics.push_back(clientName);
}

int count=0;

void loop() 
{
  client.loop();
  if(count>10000){
    if(topicString.size()>0){
      readSpeed();
    }
    count=0;
  }
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
  if (digitalRead(light1Pin) == HIGH) && (lights1State == false) {
    lights1On();
    lights1State = true
    delay(200);
  }
  if (digitalRead(light1Pin) == LOW) && (lights1State == true){
    lights1Off();
    lights1State = false
    delay(200);
  }
  if (digitalRead(light2Pin) == HIGH) && (lights2State == false){
    lights2On();
    lights2State = true
    delay(200);
  }
  if (digitalRead(light2Pin) == LOW) && (lights2State == true){
    lights2Off();
    lights2State = false
    delay(200);
  }

}
