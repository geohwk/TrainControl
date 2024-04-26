#include <DRV8833.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <vector>

DRV8833 driver = DRV8833();

const char * clientName = "testTrain"

// WiFi credentials
const char * wifi_ssid = "TrainControl";
const char * wifi_password = "ringousel";

//const int inputA1 = 5, inputA2 = 4;//d4, d5
const int inputA1 = 2, inputA2 = 14;

const char* brokerUser = "";  // exp: myemail@mail.com
const char* brokerPass = "";
const char* mqtt_broker = "192.168.1.7";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() 
{
    Serial.begin(115200);

    pinMode(2,OUTPUT);
    pinMode(14,OUTPUT);
    //pinMode(5,OUTPUT);
    //pinMode(4,OUTPUT);
    pinMode(15,OUTPUT);
    digitalWrite(15, LOW);
    connectToWiFi();
    
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
    // publish and subscribe
    client.publish("newClient", clientName);

    client.subscribe(clientName + "/lights");
    client.subscribe(clientName + "/speed");
    
    // Attach the motors to the input pins:
    driver.attachMotorA(inputA1, inputA2);
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

  if(topicString == (clientName + "/speed"))
  {
    controlSpeed(pwmVal);
  }
  else if(topicString == (clientName + "/lights"))
  {
    controlLights(pwmVal);
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

void controlLights(int value)
{
  Serial.println("control Lights function");
   if(value == 1)
   {
      digitalWrite(15, HIGH); #d8
   }
   else if(value == 0)
   {
      digitalWrite(15, LOW);
   }
}

void loop() 
{
  client.loop();
}
