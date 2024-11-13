#include <DRV8833.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <vector>
#include <string>

DRV8833 driver = DRV8833();

String clientName = "testTrain";
const String serialTopic = "serial_output";

// WiFi credentials
char wifi_ssid[] = "TrainControl";
char wifi_password[] = "ringousel";

char brokerUser[] = "";  // exp: myemail@mail.com
char brokerPass[] = "";
char mqtt_broker[] = "192.168.1.7";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

String light1Topic;
String light2Topic;
String light3Topic;
String speedTopic;

const int MotorPin1 = 5; //D1 https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
const int MotorPin2 = 4; //D2

const int light1Pin = 14; //D5
const int light2Pin = 12; //D6
const int light3Pin =13; //D7

void MQTTSerialPrint(String text)
{
    //client.publish((clientName + "/" + serialTopic).c_str(), String(text).c_str());
    Serial.println(text);
}

void setup() 
{
    Serial.begin(115200);

    pinMode(light1Pin, OUTPUT);
    pinMode(light2Pin, OUTPUT);
    pinMode(light3Pin, OUTPUT);
    pinMode(MotorPin1, OUTPUT);
    pinMode(MotorPin2, OUTPUT);

    
    while (connectToWiFi() == false){
      connectToWiFi();
      delay(2000);
    }
    
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
    // publish and subscribe
    client.publish("newClient", clientName.c_str());
    light1Topic = clientName + "/lights1";
    light2Topic = clientName + "/lights2";
    light3Topic = clientName + "/lights3";
    speedTopic = clientName + "/speed";

    client.subscribe(light1Topic.c_str());
    client.subscribe(light2Topic.c_str());
    client.subscribe(light3Topic.c_str());
    client.subscribe(speedTopic.c_str());
    Serial.println(speedTopic);
    
    // Attach the motors to the input pins:
    driver.attachMotorA(MotorPin1, MotorPin2);
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

void callback(char *topic, byte *payload, unsigned int length) 
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  String pwmValue_str = "";
  for (int i = 0; i < length; i++) 
  {
      Serial.print((char) payload[i]);
      pwmValue_str = pwmValue_str += payload[i];
  }

  payload[length] = '\0';
  int pwmVal = atoi((char *)payload);

  String topicString = String(topic);

  if(topicString == speedTopic)
  {
    controlSpeed(pwmVal);
  }
  else if(topicString == light1Topic)
  {
    controlLights(pwmVal, light1Pin, 1);
  }
  else if(topicString == light2Topic)
  {
    controlLights(pwmVal, light2Pin, 2);
  }
  else if(topicString == light3Topic)
  {
    controlLights(pwmVal, light3Pin, 3);
  }
}

void controlSpeed(int pwmVal)
{
   int output = int(float((float(float(pwmVal)/100)*1023)));
   Serial.println("control Speed function");
   if(output < 0)
   {
      output = abs(output);
      driver.motorAReverse(output);
   }
   else
   {
      driver.motorAForward(output);
   }
}

void controlLights(int value, int pin, int topic)
{
  Serial.println("Control Light " + String(topic) + ", Setting to " + String(value));
   if(value == 1)
   {
      digitalWrite(pin, HIGH); //d8
   }
   else if(value == 0)
   {
      digitalWrite(pin, LOW);
   }
}

void loop() 
{
  client.loop();
}
