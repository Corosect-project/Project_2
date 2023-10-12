import signal
import struct

from paho.mqtt.client import MQTTMessage


from firebase import Firebase, FirebaseTimestamp as timestamp
from mqtt import MqttHandler


firebase = Firebase()

def mqttTestTopic(msg: MQTTMessage):
    print(msg.topic, msg.payload)

def mqttGY61Handler(msg: MQTTMessage):
    data = {}
    if msg.topic == "GY61-RAW":
        (x, y, z) = struct.unpack("HHH", msg.payload) # H is unsigned short (2 bytes)
        print(msg.topic, f'{x} {y} {z}')
        data = {"x": x, "y": y, "z": z}
    elif msg.topic == "GY61-CALIBRATED":
        (x, y, z) = struct.unpack("ddd", msg.payload) # d is double (8 bytes)
        print(msg.topic, f'{x:.3f} {y:.3f} {z:.3f}');
        data = {"x": x, "y": y, "z": z}
        
    firebase.push({"timestamp": timestamp, "data": data}, data_path=msg.topic.lower())


def ctrlCHandler(signum, frame):
    if signum != signal.SIGINT:
        return
    print("Ctrl + c: Shutting down mqtt and quitting...")
    mqtt.exit = True


signal.signal(signal.SIGINT, ctrlCHandler)

if __name__ == "__main__":
    with MqttHandler() as mqtt:

        mqtt.addMessageHandler("TEST", mqttTestTopic)
        mqtt.addMessageHandler("GY61-RAW", mqttGY61Handler)
        mqtt.addMessageHandler("GY61-CALIBRATED", mqttGY61Handler)
        print("Mqtt started")
        while not mqtt.exit:
            pass
