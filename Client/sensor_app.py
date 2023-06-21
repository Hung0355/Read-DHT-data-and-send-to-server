import eventlet
from flask import Flask, render_template, request
from flask_mqtt import Mqtt
from flask_socketio import SocketIO
from random import random
from threading import Lock
from datetime import datetime
#import mysql.connector


eventlet.monkey_patch()

"""
Background Thread
"""
thread = None
thread_lock = Lock()

app = Flask(__name__)
app.config['MQTT_BROKER_URL'] = 'mqtt.flespi.io'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_USERNAME'] = 'FlespiToken PZvm0KGXr6g3FqedFCW7LQJsfEG0cSJPG41uCyhTsnMWBOX4hJ1Qny8t3Ch3lQIP'
app.config['MQTT_PASSWORD'] = ''
app.config['MQTT_REFRESH_TIME'] = 1.0  # refresh time in seconds
mqtt = Mqtt(app)
socketio = SocketIO(app)

printdata1 = ""
printdata2 = ""
printdata3 = ""

"""
Get current date time
"""
def get_current_datetime():
    now = datetime.now()
    return now.strftime("%m/%d/%Y %H:%M:%S")

"""
Generate random sequence of dummy sensor values and send it to our clients
"""
def background_thread():
    print("Generating random sensor values")
    while True:
        socketio.emit(
            'updateSensorData',
            {'value1': printdata1, 'value2': printdata2, 'value3': printdata3, "date": get_current_datetime()},
        )
        socketio.sleep(1)

"""
Serve root index file
"""
@app.route('/')
def index():
    return render_template('index.html')

"""
Decorator for connect
"""
@socketio.on('connect')
def connect():
    global thread
    print('Client connected')

    with thread_lock:
        if thread is None:
            thread = socketio.start_background_task(background_thread)

"""
Decorator for disconnect
"""
@socketio.on('disconnect')
def disconnect():
    print('Client disconnected', request.sid)

@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    mqtt.subscribe('Sensor_receive_data')
    print("subscribe successful\n")
    mqtt.publish('esp32ahaha', 'hello world')
    print("public successful")



counter = 1  # Counter variable

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    global counter  # Reference the global counter variable

    data = dict(
        topic=message.topic,
        payload=message.payload.decode()
    )
    print('Received message on topic {}: {}'.format(message.topic, message.payload.decode()))

    if counter == 1:
        global printdata1
        printdata1 = data['payload']
    elif counter == 2:
        global printdata2
        printdata2 = data['payload']

    # Increment the counter
    counter += 1

    # Reset the counter to 1 if it exceeds 3
    if counter > 2:
        counter = 1

    # socketio.emit('mqtt_message', data=data)

# connect to MySQL
conn = mysql.connector.connect(
    host='MSI',
    user='nganthanhnp',
    password='123456789',
    database='your_database'
)
cursor = conn.cursor()

# Save database to table
sql = "INSERT INTO sensor_data (value) VALUES (%s)"
values = (printdata,) 
cursor.execute(sql, values)

# Save and close
conn.commit()
cursor.close()
conn.close()

if __name__ == '__main__':
    app.run()
    socketio.run(app)
