import panel as pn
from panel.widgets import SpeechToText, GrammarList, TextAreaInput, TextInput, Button
from datetime import datetime, timedelta
import json
import os
import google.generativeai as genai
import paho.mqtt.client as mqtt
import threading
import time

# MQTT Settings
mqtt_broker = "c2585226f216455ea46db56f72e0d2a4.s1.eu.hivemq.cloud"
mqtt_port = 8883
mqtt_user = "HiTam"
mqtt_password = "HiTam2025"
mqtt_topic = "esp32/data"

# Global variables to store the latest MQTT data
# ------------------------------------------------------
is_auto_mode = False
led_state = 0
push_button = False
# ------------------------------------------------------

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
        client.subscribe(mqtt_topic)
    else:
        print(f"Failed to connect, return code {rc}")

def on_message(client, userdata, msg):
    global is_auto_mode, led_state
    try:
        payload = json.loads(msg.payload.decode('utf-8'))
        # Update the required variables
        is_auto_mode = payload.get("is_auto_mode", is_auto_mode)
        led_state = payload.get("led_state", led_state)
        print(f"Updated is_auto_mode: {is_auto_mode}, led_state: {led_state}")
    except json.JSONDecodeError as e:
        print(f"Failed to decode JSON: {e}")

# Function to send push_button data
def send_push_button_data():
    global push_button
    if push_button:  # Only send if push_button is True
        payload = json.dumps({"push_button": push_button})
        try:
            mqtt_client.publish(mqtt_topic, payload)
            print(f"Sent to MQTT: {payload}")
            push_button = False  # Reset push_button after successful send
        except Exception as e:
            print(f"Failed to send data: {e}")

# Function to start the MQTT client
def start_mqtt_client():
    global mqtt_client
    mqtt_client = mqtt.Client()
    mqtt_client.username_pw_set(mqtt_user, mqtt_password)
    mqtt_client.tls_set()  # Enable TLS for secure connection
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    mqtt_client.connect(mqtt_broker, mqtt_port, 60)
    mqtt_client.loop_start()

# Run the MQTT publish loop in a separate thread
def mqtt_publish_loop():
    while True:
        send_push_button_data()
        time.sleep(1)  # Delay between checks

mqtt_thread = threading.Thread(target=start_mqtt_client, daemon=True)
mqtt_thread.start()

publish_thread = threading.Thread(target=mqtt_publish_loop, daemon=True)
publish_thread.start()

# ----------------------------------------------------------------
API_KEY = 'AIzaSyAt2WjTo1-19j1L6PsQ4AX_LL_KEjAaCBM'
genai.configure(api_key=API_KEY)

# gemini-1.5-pro
model = genai.GenerativeModel('gemini-1.5-pro')
chat = model.start_chat(history=[])

def control_lights_gemini(is_auto_mode, led_state, user_request):
    prompt = f"""Current Mode: {'Auto' if is_auto_mode else 'Manual'} (Note that press button can just switch from Auto Mode to Manual mode but cannot from Manual to Auto Mode)\nLight State: {led_state} (0 = Off, 1 = On, 2 = Blinking and important remember that press button can turn off Blinking but cannot turn on from Blinking)\nUser Request: {user_request}. Solve in this format: \"<Should system press the button by YES or NO, also reply NO when system's led_state currently at user wanted light status>\",\"you should reply user one polite full sentence about your action by their language>\"."""
    response = chat.send_message(prompt)
    result_text = ""
    if response._result and response._result.candidates:
        for candidate in response._result.candidates:
            if candidate.content and candidate.content.parts:
                for part in candidate.content.parts:
                    if part.text:
                        result_text += part.text
    return result_text

# Kích hoạt Panel
pn.extension()

# File lưu trữ lịch sử chat
history_file = "chat_history.json"

# Tải lịch sử chat từ file nếu có
if os.path.exists(history_file):
    with open(history_file, "r") as file:
        chat_history = json.load(file)
else:
    chat_history = []

# Hiển thị lịch sử chat
chat_display = pn.widgets.TextAreaInput(
    value="\n".join([f"{speaker} {message}" for speaker, message in chat_history]),
    disabled=True,
    height=300,
    width=600
)

# Textbox nhập liệu
user_input = pn.widgets.TextInput(name='', placeholder='Nhập tin nhắn...', width=500)

# Kết nối SpeechToText
grammar_list = GrammarList()

# Định nghĩa ngữ pháp
english_grammar = "#JSGF V1.0; grammar colors; public <color> = red | green | blue | yellow | black | white | orange | purple | brown | pink;"
vietnamese_grammar = "#JSGF V1.0; grammar colors; public <color> = đỏ | xanh | vàng | đen | trắng | cam | tím | nâu | hồng;"
grammar_list.add_from_string(english_grammar, 1)
grammar_list.add_from_string(vietnamese_grammar, 1)

speech_to_text = SpeechToText(
    grammars=grammar_list,
    button_type="primary",
    button_started="Đang nghe...",
    button_not_started="Bắt đầu",
    lang="vi-VN",
    height=200
)

# Hàm lưu lịch sử chat vào file
def save_chat_history():
    with open(history_file, "w") as file:
        json.dump(chat_history, file)

# Hàm xử lý tin nhắn
def process_message(event):
    global push_button
    user_message = user_input.value
    if not user_message.strip():
        return
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    chat_history.append((f"User ({timestamp}):", user_message))
    gemini_response = control_lights_gemini(is_auto_mode, led_state, user_message)
    split_data = [s.strip().strip('"') for s in gemini_response.split(",")]
    response_status = split_data[0]
    response_message = split_data[1]
    push_button = (response_status == "YES")
    bot_response = f"Bot ({timestamp}): {response_message}\n"
    chat_history.append((bot_response, ""))
    chat_display.value = "\n".join([f"{speaker} {message}" for speaker, message in chat_history])
    save_chat_history()
    user_input.value = ""
    scroll_to_bottom()

# Hàm cuộn xuống cuối
def scroll_to_bottom():
    script = """
    const textarea = document.querySelector('textarea');
    textarea.scrollTop = textarea.scrollHeight;
    """
    pn.pane.HTML(f"<script>{script}</script>").servable()

# Cập nhật user_input từ giọng nói
def update_user_input(event):
    global push_button
    results = event.new
    if results and isinstance(results, list):
        for result in results:
            if result.get('is_final', False):
                transcript = result['alternatives'][0]['transcript']
                timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                chat_history.append((f"User ({timestamp}):", transcript))
                chat_display.value = "\n".join([f"{speaker} {message}" for speaker, message in chat_history])
                save_chat_history()
                scroll_to_bottom()
                gemini_response = control_lights_gemini(is_auto_mode, led_state, transcript)
                split_data = [s.strip().strip('"') for s in gemini_response.split(",")]
                response_status = split_data[0]
                response_message = split_data[1]
                push_button = (response_status == "YES")
                bot_response = f"Bot ({timestamp}): {response_message}\n"
                chat_history.append((bot_response, ""))
                chat_display.value = "\n".join([f"{speaker} {message}" for speaker, message in chat_history])
                save_chat_history()
                scroll_to_bottom()
                break

speech_to_text.param.watch(update_user_input, 'results')

# Nút gửi tin nhắn
send_button = Button(name='Gửi', button_type='primary', width=80)
send_button.on_click(process_message)

# Giao diện chatbot
chatbot_ui = pn.Column(
    pn.pane.Markdown("# Chatbot trợ lý ảo thông minh"),
    chat_display,
    pn.Row(user_input, send_button, align="center"),
    pn.Row(speech_to_text)
)

# Chạy ứng dụng
chatbot_ui.servable()
