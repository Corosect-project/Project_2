import signal
import codecs

from paho.mqtt.client import MQTTMessage


from firebase import Firebase, FirebaseTimestamp as timestamp
from mqtt import MqttHandler


firebase = Firebase()


def mqttMessageHandler(msg: MQTTMessage):
    print(msg.topic, int.from_bytes(msg.payload, 'little'))
    data = int.from_bytes(msg.payload, 'little')
    firebase.push({"timestamp": timestamp, 'data': data},
                  data_path=msg.topic.lower())


def ctrlCHandler(signum, frame):
    if signum != signal.SIGINT:
        return
    print("Ctrl + c: Shutting down mqtt and quitting...")
    mqtt.exit = True


signal.signal(signal.SIGINT, ctrlCHandler)

if __name__ == "__main__":
    with MqttHandler() as mqtt:

        mqtt.addMessageHandler("TEST", mqttMessageHandler)
        print("Mqtt started")
        while not mqtt.exit:
            pass
