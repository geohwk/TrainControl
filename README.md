# TrainAssistant
A Train control system using the mosquitto MQTT broker running on a raspberry pi and MQTT clients running on ESP-8266 devices. This can be controlled by a seperate MQTT client running on a mobile device or a dedicated controller client.

Each train on a layout will take an ESP-12F device running the train client with the accompanying PCB and electronics with a power source. The controller client acts as a manager for new clients. They register their designations to the controller client which will then be able to switch through all registered clients to control their speed, direction and lights. 
