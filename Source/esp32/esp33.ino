// Smart Lighting Control System using ESP32


/*-------------------------------------------------------------
-----------------------WiFi and MQTT setup---------------------
---------------------------------------------------------------*/
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Cấu hình WiFi
const char* ssid = "Trung Hieu";              // Tên WiFi
const char* password = "0974877531";                     // Mật khẩu WiFi

// Cấu hình MQTT
const char* mqtt_server = "c2585226f216455ea46db56f72e0d2a4.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;                    // TLS MQTT port
const char* mqtt_user = "HiTam";               // Tài khoản MQTT
const char* mqtt_password = "HiTam2025";       // Mật khẩu MQTT

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

// Topics
const char* topic_publish = "esp32/data";
// const char* topic_subscribe = "esp32/control";
const char* topic_subscribe = "esp32/data";

// Biến dữ liệu
unsigned long delay_duration = 6000;  // Độ trễ (ms)
bool is_day = 1;                      // Ban ngày: 1, Ban đêm: 0
bool is_auto_mode = 1;                // Chế độ tự động: 1, Thủ công: 0
bool led_state = 0;                   // Đèn bật: 1, Tắt: 0
bool blink_state = 0;                 // Nhấp nháy: 1, Không: 0
int light_sensor = 500;               // Cảm biến ánh sáng

/*-------------------------------------------------------------
---------------------------MAIN SYSTEM-------------------------
---------------------------------------------------------------*/
//Khai báo cho main system
const int LDR_PIN = 35;	
const int BUTTON_PIN = 34;	
const int LED_PIN = 14;		

unsigned int ldrThreshold = 3700;
const unsigned int blinkDuration = 2000;
unsigned long autoModeRevertDelay = 6000;
unsigned long blink_time;
unsigned long push_time;
bool firstPush = false; 
bool previous_ldr_status = 0; //1: light mode, 0: night mode
bool autoModeSwitchByTimer = true; 
bool is_light_condition = true; 
// bool is_auto_mode = true; 
bool is_push_online_button = false;  
unsigned long autoModeRevertDelay_online = 6000; 
  
// Variables to store the LED and button states
bool blinkState = false;
bool ledState = false;       // LED initially off
bool buttonState = false;    // Current state of the button
bool lastButtonState = false; // Previous state of the button

//-------------------------------------------------------------------------

// Hàm kết nối WiFi
void setup_wifi() {
  delay(10);
  Serial.println("Kết nối WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi đã kết nối.");
}

// Hàm kết nối MQTT
void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.println("Kết nối MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("Đã kết nối MQTT.");
      client.subscribe(topic_subscribe);
    } else {
      Serial.print("Thử lại sau: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

// Callback khi nhận dữ liệu
// Callback khi nhận dữ liệu từ topic MQTT
// Callback khi nhận dữ liệu từ topic MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  // Serial.print("Nhận tin nhắn từ ");
  // Serial.print(topic);
  // Serial.print(": ");
  // Serial.println(message);

  // Kiểm tra xem thông điệp có phải là JSON hợp lệ không
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, message);
  
  if (error) {
    // Nếu không phải JSON hợp lệ, bỏ qua và không xử lý
    // Serial.println("Lỗi giải mã JSON, bỏ qua dữ liệu.");
    return;
  }

  // Kiểm tra định dạng cụ thể mà bạn đã định nghĩa
  if (String(topic) == topic_subscribe) {
    // Kiểm tra xem tất cả các khóa cần thiết có tồn tại trong JSON không
    if (doc.containsKey("push_button")) {
      // Nếu tất cả các khóa đều có trong dữ liệu, xử lý dữ liệu
      is_push_online_button = doc["push_button"];
    } else {
      // Nếu thiếu bất kỳ khóa nào, bỏ qua và không xử lý
      // Serial.println("Dữ liệu không đầy đủ hoặc không đúng định dạng.");
    } 
  }
}




void setup(){
  //Main system
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);	
  pinMode(LDR_PIN, INPUT);	

  //WiFi and MQTT 
  setup_wifi();
  wifiClient.setInsecure(); // Sử dụng kết nối không cần chứng chỉ
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

unsigned long lastLoopTime = 0;
unsigned long loopInterval = 500; // 200ms
unsigned long LDR_CurrentValue = 0;

void loop(){
  if (millis() - lastLoopTime >= loopInterval) {
    lastLoopTime = millis();
    // Auto Mode
    // Day condition
    LDR_CurrentValue = analogRead(LDR_PIN);
    if(LDR_CurrentValue < ldrThreshold){	
      if(previous_ldr_status == 0){
        digitalWrite(LED_PIN, LOW);	
        ledState = false; 
          blinkState = false; 
          is_auto_mode = true;
      }
      if(autoModeSwitchByTimer && (millis() - push_time >= autoModeRevertDelay)){
        digitalWrite(LED_PIN, LOW);	
          ledState = false; 
          blinkState = false; 
          is_auto_mode = true;
      }
      is_light_condition = true; 
      previous_ldr_status = 1; 
    }
    // Night condition
    else{
      if(previous_ldr_status == 1){
        blink_time = millis(); 
        firstPush = false; 
        is_auto_mode = true;
      }
      if(firstPush == false){
        digitalWrite(LED_PIN, ((millis() - blink_time) % blinkDuration < (blinkDuration / 2))? HIGH : LOW);
        ledState = ((millis() - blink_time) % blinkDuration < (blinkDuration / 2))? true : false;
        blinkState = true; 
        is_auto_mode = true;
      }
      if(autoModeSwitchByTimer && (millis() - push_time >= autoModeRevertDelay)){
        digitalWrite(LED_PIN, ((millis() - blink_time) % blinkDuration < (blinkDuration / 2))? HIGH : LOW);
        ledState = ((millis() - blink_time) % blinkDuration < (blinkDuration / 2))? true : false;
        blinkState = true; 
        is_auto_mode = true;
      }
      is_light_condition = false; 
      previous_ldr_status = 0; 
    }
    
    // Manual Mode
    buttonState = digitalRead(BUTTON_PIN);
    if(buttonState == LOW && lastButtonState == HIGH) {
      if(blinkState == true){
        ledState = HIGH;
      }
      push_time = millis(); 
      firstPush = true; 
      blinkState = false; 
      
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW); 
      is_auto_mode = false; 
    }

    lastButtonState = buttonState;
    

    /*-------------------------------------------------------*/
    /*-------------------GỬI DATA TỪ ESP32-------------------*/
    /*-------------------------FLASK-------------------------*/  
    // // Send data to Flask
    // Serial.print(autoModeRevertDelay); Serial.print(",");
    // Serial.print(is_light_condition); Serial.print(",");
    // Serial.print(is_auto_mode); Serial.print(",");
    // Serial.print(blinkState); Serial.print(",");
    // Serial.print(ledState);
    // Serial.println(); // Marks end of data
    /*-----------------------END FLASK------------------------*/
    /*--------------------------------------------------------*/

    /*--------------------------MQTT--------------------------*/  
    DynamicJsonDocument doc(1024);
    delay_duration = autoModeRevertDelay / 1000;
    // doc["delay_duration"] = delay_duration; // (delay_duration);
    // doc["is_day"] = is_light_condition; //(is_day);
    doc["is_auto_mode"] = is_auto_mode;
    // doc["blink_state"] = blinkState; //(blink_state);
    if(blinkState){
      doc["led_state"] = 2;
    }
    else if(ledState){
      doc["led_state"] = 1;
    }
    else{
      doc["led_state"] = 0;
    }
    // doc["led_state"] = ledState; //(led_state);
    doc["light_sensor"] = LDR_CurrentValue; //(light_sensor);
    // doc["ldr_threshold"] = ldrThreshold; 

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    client.publish(topic_publish, buffer, n);
    // Serial.println(buffer);
    /*-------------------------------------------------------*/


    /*-------------------------------------------------------*/
    /*-----------------NHẬN DATA TỪ DASHBOARD----------------*/
    /*-------------------------FLASK-------------------------*/ 
    // // Check for incoming data from Flask
    // if(Serial.available() > 0) {
    //   //autoModeRevertDelay_online = Serial.parseInt();
    //   //is_push_online_button = Serial.parseInt();
      
    //   // Read the entire line of input
    //   String dataString = Serial.readStringUntil('\n');  // Read until newline character

    //   // Find the index of the comma
    //   int commaIndex = dataString.indexOf(',');

    //   if (commaIndex != -1) {
    //       // Extract and convert the autoModeRevertDelay_online part
    //       String delayString = dataString.substring(0, commaIndex);
    //       autoModeRevertDelay_online = atol(delayString.c_str());

    //       // Extract and convert the is_push_online_button part
    //       String pushButtonString = dataString.substring(commaIndex + 1);
    //       is_push_online_button = pushButtonString.toInt();  // Convert to int
    // 	}
    // }
    /*-----------------------END FLASK------------------------*/
    /*--------------------------------------------------------*/


    /*--------------------------MQTT--------------------------*/ 


    /*--------------------------------------------------------*/


    if(autoModeRevertDelay_online > 0){
      autoModeRevertDelay = autoModeRevertDelay_online; 
      autoModeSwitchByTimer = true;
    }
    else if(autoModeRevertDelay_online == 0){
      autoModeRevertDelay = 0; 
      autoModeSwitchByTimer = false; 
    }
    if(is_push_online_button == 1){
      if(blinkState == true){
        ledState = HIGH;
      }
      push_time = millis(); 
      firstPush = true; 
      blinkState = false; 
      
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW); 
      is_auto_mode = false; 
      is_push_online_button = 0; 
    }
  
  }
  // WiFi MQTT Connecting
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();
}

/*-------------------------------------------------------------
----------------------------UNIT TEST--------------------------
---------------------------------------------------------------*/
// UNIT TEST FOR LDR, PUSHBUTTON AND LED BLINKING
// const int LDR_PIN = 35;	
// const int BUTTON_PIN = 34;	
// const int LED_PIN = 14;		
// const unsigned int blinkDuration = 2000;

// void setup() {
//   Serial.begin(115200);  
//   pinMode(LDR_PIN, INPUT);  
//   pinMode(BUTTON_PIN, INPUT);  
//   pinMode(LED_PIN, OUTPUT);  
// }

// //UNIT TEST FOR LDR
// void LDR_UnitTest(){
//   int ldrValue = analogRead(LDR_PIN);

//   Serial.print("LDR Value: ");
//   Serial.println(ldrValue);
// }

// //UNIT TEST FOR PUSHBUTTON
// void pushButton_UnitTest(){
//   int currentState = digitalRead(BUTTON_PIN);
  
//   Serial.print("Pushbutton Current State: ");
//   Serial.println(currentState);
// }

// //UNIT TEST FOR Blinking LED
// void blinkingLED_UnitTest(){
//   digitalWrite(LED_PIN, (millis() % blinkDuration < (blinkDuration / 2))? HIGH : LOW);
//   Serial.println("LED is currently blinking.");
// }

// void loop() {
//   if(millis() < 10000)
//     LDR_UnitTest(); 
//   else if (millis() < 20000)
//     pushButton_UnitTest();
//   else
//     blinkingLED_UnitTest();

//   delay(200);  
// }