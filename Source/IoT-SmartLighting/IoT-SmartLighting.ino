const int LDR_PIN = A0;		
const int LED_PIN = 7;		
const int BUTTON_PIN = 2;

const unsigned int ldrThreshold = 950;
const unsigned int blinkDuration = 2000;
unsigned long autoModeRevertDelay = 6000;
unsigned long blink_time;
unsigned long push_time;
bool firstPush = false; 
bool previous_ldr_status = 0; //1: light mode, 0: night mode
bool autoModeSwitchByTimer = true; 
bool is_light_condition = true; 
bool is_auto_mode = true; 
bool is_push_online_button = false;  
unsigned long autoModeRevertDelay_online = 6000; 
  
// Variables to store the LED and button states
bool blinkState = false;
bool ledState = false;       // LED initially off
bool buttonState = false;    // Current state of the button
bool lastButtonState = false; // Previous state of the button

void setup(){
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);	
  pinMode(LDR_PIN, INPUT);	
}

void loop(){
  // Auto Mode
  // Day condition
  if(analogRead(LDR_PIN) < ldrThreshold){	
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
    
  // Send data to Flask
  Serial.print(autoModeRevertDelay); Serial.print(",");
  Serial.print(is_light_condition); Serial.print(",");
  Serial.print(is_auto_mode); Serial.print(",");
  Serial.print(blinkState); Serial.print(",");
  Serial.print(ledState);
  Serial.println(); // Marks end of data

  // Check for incoming data from Flask
  if(Serial.available() > 0) {
    //autoModeRevertDelay_online = Serial.parseInt();
    //is_push_online_button = Serial.parseInt();
    
    // Read the entire line of input
    String dataString = Serial.readStringUntil('\n');  // Read until newline character

    // Find the index of the comma
    int commaIndex = dataString.indexOf(',');

    if (commaIndex != -1) {
        // Extract and convert the autoModeRevertDelay_online part
        String delayString = dataString.substring(0, commaIndex);
        autoModeRevertDelay_online = atol(delayString.c_str());

        // Extract and convert the is_push_online_button part
        String pushButtonString = dataString.substring(commaIndex + 1);
        is_push_online_button = pushButtonString.toInt();  // Convert to int
  	}
  }
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
  	
  delay(50); 
}


/*
const int LDR_PIN = A0;		
const int LED_PIN = 7;		
const int BUTTON_PIN = 2;
const unsigned int blinkDuration = 2000;

void setup() {
  Serial.begin(9600);  
  pinMode(LDR_PIN, INPUT);  
  pinMode(BUTTON_PIN, INPUT);  
  pinMode(LED_PIN, OUTPUT);  
}

//UNIT TEST FOR LDR
void LDR_UnitTest(){
  int ldrValue = analogRead(LDR_PIN);

  Serial.print("LDR Value: ");
  Serial.println(ldrValue);
}

//UNIT TEST FOR PUSHBUTTON
void pushButton_UnitTest(){
  int currentState = digitalRead(BUTTON_PIN);
  
  Serial.print("Pushbutton Current State: ");
  Serial.println(currentState);
}

//UNIT TEST FOR Blinking LED
void blinkingLED_UnitTest(){
  digitalWrite(LED_PIN, (millis() % blinkDuration < (blinkDuration / 2))? HIGH : LOW);
  Serial.println("LED is currently blinking.");
}

void loop() {
  // LDR_UnitTest(); 
  // pushButton_UnitTest();
  // blinkingLED_UnitTest();

  delay(200);  
}
*/

/*
Format dữ liệu gửi từ Arduino tới Website
"<timeout>,<is_DayCondition>,<is_autoMode>,
<is_blinking>,<is_LED_ON>\n"

-------------------------------------------
Format dữ liệu gửi từ Website tới Arduino
"<timeout>,<is_pressedButton>\n"

*/