import signal
import codecs

from paho.mqtt.client import MQTTMessage


from firebase import Firebase, FirebaseTimestamp as timestamp
from mqtt import MqttHandler


firebase = Firebase()

def mqttTestTopic(msg: MQTTMessage):
    print(msg.topic, msg.payload)

def mqttGY61Handler(msg: MQTTMessage):
    x = int.from_bytes(msg.payload[:2], 'little')
    y = int.from_bytes(msg.payload[2:4], 'little')
    z = int.from_bytes(msg.payload[4:], 'little')
    print(msg.topic, x, y, z)
    data = {"x": x, "y": y, "z": z}
    firebase.push({"timestamp": timestamp, "data": data},
                  data_path=msg.topic.lower())


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
        print("Mqtt started")
        while not mqtt.exit:
            pass
