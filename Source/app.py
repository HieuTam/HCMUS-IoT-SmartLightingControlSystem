from flask import Flask, render_template, request, jsonify, redirect, url_for
import threading
import serial
import time

app = Flask(__name__)

# Initialize global variables
autoModeRevertDelay = 6000
is_light_condition = 1
is_auto_mode = 1
blinkState = 0
ledState = 0
switch_mode_duration = 6000
is_push_button = 0
previous_duration = 6000

# Set up the serial connection with Arduino
try:
    ser = serial.Serial('COM5', 9600, timeout=0.5)
except Exception as e:
    print(f"Error connecting to Arduino: {e}")
    ser = None  # Set ser to None to avoid issues if Arduino isn't connected

# Function to read data from Arduino
def read_from_arduino():
    global autoModeRevertDelay, is_light_condition, is_auto_mode, blinkState, ledState
    try:
        while True:
            if ser and ser.in_waiting > 0:
                line = ser.readline().decode('utf-8').strip()
                data = line.split(',')
                if len(data) == 5:
                    autoModeRevertDelay = int(data[0])
                    is_light_condition = int(data[1])
                    is_auto_mode = int(data[2])
                    blinkState = int(data[3])
                    ledState = int(data[4])
                print(f"Arduino Data - Delay: {autoModeRevertDelay}, Light Condition: {is_light_condition}, "
                      f"Auto Mode: {is_auto_mode}, Blink State: {blinkState}, LED State: {ledState}")

    except Exception as e:
        print(f"Error reading from Arduino: {e}")

# Home route
@app.route('/')
def index():
    status = "Blinking" if blinkState else ("On" if ledState else "Off")
    mode = "Auto" if is_auto_mode else "Manual"
    condition = "Day" if is_light_condition else "Night"
    return render_template(
        'index.html',
        status=status,
        mode=mode,
        condition=condition,
        switch_mode_duration=switch_mode_duration
    )

# Toggle light route
@app.route('/toggle', methods=['POST'])
def toggle_light():
    global switch_mode_duration, is_push_button
    duration = request.form.get('duration', type=int)

    if duration is not None:
        switch_mode_duration = duration
        is_push_button = 0
    else:
        switch_mode_duration = autoModeRevertDelay
        is_push_button = 1

    send_to_arduino()
    is_push_button = 0
    return redirect(url_for('index'))

# Function to send data to Arduino
def send_to_arduino():
    global switch_mode_duration, is_push_button
    try:
        data = f"{switch_mode_duration},{is_push_button}\n"
        if ser:
            ser.write(data.encode())
            print(f"Sent to Arduino: {data}")
        else:
            print("Serial connection not available. Data not sent.")
    except Exception as e:
        print(f"Error sending to Arduino: {e}")

# Route to get current Arduino data
@app.route('/get_arduino_data')
def get_arduino_data():
    status = "Blinking" if blinkState else ("On" if ledState else "Off")
    mode = "Auto" if is_auto_mode else "Manual"
    condition = "Day" if is_light_condition else "Night"
    return jsonify({
        "status": status,
        "mode": mode,
        "condition": condition,
        "switch_mode_duration": autoModeRevertDelay
    })

# Start periodic updates
def update_data_periodically():
    while True:
        read_from_arduino()
        time.sleep(1)

thread = threading.Thread(target=update_data_periodically)
thread.daemon = True
thread.start()

if __name__ == '__main__':
    app.run(debug=True)