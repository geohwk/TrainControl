# TrainAssistant
A Train control system using the mosquitto MQTT broker running on a raspberry pi and MQTT clients running on ESP-8266 devices. This can be controlled by a seperate MQTT client running on a mobile device or a dedicated controller client.

This repo includes the files for the custom PCBs for the controller and train client:
![IMG_20250722_220610963~2](https://github.com/user-attachments/assets/ca81b514-b088-4fcd-a980-e4f5f3e87f05)

Each train on a layout will take an ESP-12F device running the train client with the accompanying PCB and electronics with a power source:
![IMG_20250722_220949580~2](https://github.com/user-attachments/assets/d6d66581-9b0f-4776-a71e-f7a36cb46f1b)
![IMG_20250722_221028727~2](https://github.com/user-attachments/assets/5c7aace7-4a60-4c06-8e4c-d29e17ed4593)

The controller client acts as a manager for new clients. They register their designations to the controller client which will then be able to switch through all registered clients to control their speed, direction and lights. 
![IMG_20250722_220438927](https://github.com/user-attachments/assets/3a7332f0-237d-4f19-b944-6644300ba2c9)
![IMG_20250722_221146620](https://github.com/user-attachments/assets/51fbb045-1dcf-4073-98c3-9eb676511ff6)
![IMG_20250722_221153048](https://github.com/user-attachments/assets/e466bdb6-e1c4-4464-8398-42f5b8708ea2)
