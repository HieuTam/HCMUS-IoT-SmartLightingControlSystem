# HCMUS IoT - Smart Lighting Control System

## Overview
This project, developed as part of the IoT Programming course at the University of Science, VNU-HCM, focuses on creating a Smart Lighting Control System that automatically adjusts LED lighting based on ambient light conditions. The system operates in two modes: **Auto Mode** (light adjusts based on day/night) and **Manual Mode** (user-controlled via a push button or web interface). The aim is to provide energy efficiency and flexible lighting control.

## Features
- **Auto Mode**: LED automatically switches off in daylight and blinks at night.
- **Manual Mode**: Allows users to toggle the LED state manually.
- **Automatic Mode Switching**: Switches back to Auto Mode after a set duration or a day/night transition.
- **Web Interface**: Users can monitor the real-time LED state, adjust mode, set a timeout for Manual Mode, and manually control LED operations by text or voice command.

## Components
- **Controller**: Kit Wifi BLE ESP32 NodeMCU-32S CH340 Ai-Thinker (ESP32) for control and data processing.
- **Sensors**: Photoresistor for light detection and a push button for mode toggling.
- **Web Service**: Local Python Panel-based web server connecting to MQTT through wireless connection WiFI, serving a simple interface to manage settings and LED control.

## System Design
Includes FSM diagrams, data flow between the controller and web service, and circuit diagrams for Arduino components.
<div align="center">
<img src="Report - LaTeX source/img/SystemDesign.jpg" alt="Smart Lighting System Diagram" width="300"/>
<img src="Report - LaTeX source/img/FSM-2.jpg" alt="FSM Diagram" width="450"/>
</div>

## Deployment
1. Code uploaded to Arduino using Arduino IDE.
2. Flask server set up for real-time data exchange between the web interface and Arduino.
<div align="center">
<img src="Report - LaTeX source/img/Data.jpg" alt="Data Communication Diagram" width="450"/>
<img src="Report - LaTeX source/img/UI.jpg" alt="UI.jpg" width="450"/>
</div>

## Testing and Results
The system was tested in simulation ([Wokwi](https://wokwi.com/projects/418228728977255425)) and real hardware setups. Key tests included component-level validation, integration testing, and FSM behavior validation to ensure alignment with design specifications.

## Instructions

### Step 1: Connect Devices to ESP32
Connect the devices to the ESP32 according to the designed circuit.

### Step 2: Upload Code to ESP32
Upload the code from `/Source/esp32/esp32.ino` to the ESP32 via WiFI Connection using the Arduino IDE. At this stage, the system will be running and send data to MQTT Topic. Web interface can get data from this time through MQTT.

### Step 3: Run Real-time Dashboard and Chatbot Application
After uploading the code, open a command prompt (CMD) in the `/Source` folder and run the following command:

```bash
panel serve app.py
```
### Step 4. Access the Web Interface
After executing the command successfully, open the URL provided in the command prompt to access the web interface and start using the system.


## Release
- **Version**: 2.0
- **Release Date**: December 30, 2024
- [**Demo Video**](https://www.youtube.com/watch?v=8Jh9oFRTNO0&list=PL49PFd0rcrSMamwSboGjRe7nyyKdWxkfa&index=1&ab_channel=T%C3%A2mTr%E1%BA%A7nHi%E1%BA%BFu)
- [**Google Drive**](https://drive.google.com/drive/u/2/folders/1Dzda6UcX8GyJiqW1w3eEMZk_fvy09f99)
