import panel as pn
import paho.mqtt.client as mqtt
import json
import time
import threading

# Panel extension
pn.extension()

# MQTT Settings
mqtt_broker = "c2585226f216455ea46db56f72e0d2a4.s1.eu.hivemq.cloud"
mqtt_port = 8883
mqtt_user = "HiTam"
mqtt_password = "HiTam2025"
mqtt_topic = "esp32/data"

# Panel styling
ACCENT = "teal"

# Variables to store incoming data
delay_duration = 6
is_day = True
is_auto_mode = True
led_state = 0
light_sensor = 0
ldr_threshold = 3700

# Create the MQTT Client
client = mqtt.Client()
client.username_pw_set(mqtt_user, mqtt_password)
client.tls_set()

# Function to handle incoming MQTT messages
def on_message(client, userdata, msg):
    global delay_duration, is_day, is_auto_mode, led_state, light_sensor, ldr_threshold
    
    try:
        # Parse the incoming message
        message = msg.payload.decode('utf-8')
        data = json.loads(message)

        # Extract values from the incoming JSON and update variables (only store the latest data)
        is_auto_mode = data.get("is_auto_mode", True)
        led_state = data.get("led_state", 0)
        light_sensor = data.get("light_sensor", 0)

        # Set day/night condition based on light sensor value
        if light_sensor < ldr_threshold:
            is_day = True
        else:
            is_day = False

        # Recalculate colors based on the new data
        delay_duration_color = "red" if not is_auto_mode else ACCENT
        light_sensor_color = ACCENT if light_sensor <= ldr_threshold else "red"
    
        # Update panel indicators after receiving new data
        delay_duration_view.param.update(value=delay_duration, colors=[(delay_duration, delay_duration_color), (100, "red")])
        led_state_view.object = led_state_circle(led_state)
        light_sensor_view.param.update(value=light_sensor, colors=[(light_sensor, light_sensor_color), (ldr_threshold, "red")])
        ldr_threshold_view.param.update(value=ldr_threshold, colors=[(ldr_threshold, ACCENT)])

    except json.JSONDecodeError:
        print("Failed to decode message.")
    except Exception as e:
        print(f"Error handling message: {e}")


# Set MQTT message callback
client.on_message = on_message
client.connect(mqtt_broker, mqtt_port, 60)
client.subscribe(mqtt_topic)

# Start the MQTT loop in a separate thread
def mqtt_loop():
    while True:
        client.loop()  # Run MQTT loop to listen for messages
        time.sleep(0.1)  # Decrease the sleep time to speed up message processing

# Run MQTT loop in background
threading.Thread(target=mqtt_loop, daemon=True).start()

# Function to generate LED state circle
def led_state_circle(led_state):
    if led_state == 0:
        return """
        <div style='width: 60px; height: 60px; border: 5px solid #b0b0b0; border-radius: 50%; 
                    background-color: transparent; 
                    box-shadow: 0 0 15px rgba(176, 176, 176, 0.4); 
                    transition: box-shadow 1s ease-in-out, background-color 1s ease-in-out;'></div>
        """
    elif led_state == 1:
        return """
        <div style='width: 60px; height: 60px; border: 5px solid #e74c3c; background-color: #e74c3c; 
                    border-radius: 50%; animation: pulse 1.5s infinite, glow 1.5s infinite;'></div>
        <style>
        @keyframes pulse {
            0% { transform: scale(1); opacity: 1; }
            50% { transform: scale(1.1); opacity: 0.8; }
            100% { transform: scale(1); opacity: 1; }
        }
        @keyframes glow {
            0% { box-shadow: 0 0 10px #e74c3c, 0 0 20px #e74c3c; }
            50% { box-shadow: 0 0 15px #e74c3c, 0 0 30px #e74c3c; }
            100% { box-shadow: 0 0 10px #e74c3c, 0 0 20px #e74c3c; }
        }
        </style>
        """
    elif led_state == 2:
        return """
        <div style='width: 60px; height: 60px; border: 5px solid #f39c12; background-color: #f39c12; 
                    border-radius: 50%; animation: blink 0.8s step-start infinite, glow 1.5s infinite;'></div>
        <style>
        @keyframes blink {
            0% { opacity: 1; }
            50% { opacity: 0.2; }
            100% { opacity: 1; }
        }
        @keyframes glow {
            0% { box-shadow: 0 0 15px #f39c12, 0 0 30px #f39c12; }
            50% { box-shadow: 0 0 25px #f39c12, 0 0 50px #f39c12; }
            100% { box-shadow: 0 0 15px #f39c12, 0 0 30px #f39c12; }
        }
        </style>
        """

# Panel Components to display the data
delay_duration_color = "red" if not is_auto_mode else ACCENT

# Panel Component to display the Delay Duration with dynamic color change
delay_duration_view = pn.indicators.Number(
    name="Delay Duration", 
    value=delay_duration, 
    format="{value} secs", 
    colors=[(delay_duration, delay_duration_color), (100, "red")]
)

# LED State view with circle
led_state_label = pn.pane.Markdown(
    f"<h4 style='font-size: 24px; color: {ACCENT}; font-weight: normal;'>LED Status</h4>", 
    align="center"
)

led_state_view = pn.pane.HTML(led_state_circle(led_state), width=200, margin=(-10, -10, 25, 55))

light_sensor_color = ACCENT if light_sensor <= ldr_threshold else "red"
# Panel Component to display the data with dynamic color change
light_sensor_view = pn.indicators.Number(
    name="Light Sensor", 
    value=light_sensor, 
    format="{value}", 
    colors=[(light_sensor, light_sensor_color), (ldr_threshold, "red")]
)

ldr_threshold_view = pn.indicators.Number(name="LDR Threshold", value=ldr_threshold, format="{value}", colors=[(ldr_threshold, ACCENT)])

# Layout of indicators with LED Status label above the LED state circle
indicators = pn.Row(
    pn.Column(led_state_label, led_state_view, align="center", width=180, margin=(-25, 25, 7, 0)), 
    light_sensor_view,
    ldr_threshold_view,
    delay_duration_view
)

# Panel layout and template
dashboard = pn.template.FastListTemplate(
    title="Smart Lighting Control System",
)

# Adding the indicator row to the dashboard
dashboard.main.append(indicators)

# Other code for setting up and running the app
dashboard.servable()

