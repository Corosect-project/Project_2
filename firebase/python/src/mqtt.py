from typing import Callable
from threading import Lock

import paho.mqtt.client as mqtt

import config.mqtt_config as config

TopicHandler = Callable[[mqtt.MQTTMessage], None]


class MqttHandler:
    lock = Lock()
    exit: bool = False
    client = mqtt.Client("firebase_forwarder")
    messageHandlers: dict[str, TopicHandler] = {}

    def __init__(self) -> None:
        self.client.on_connect = self.onConnect
        self.client.on_message = self.onMessage

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, type, value, traceback):
        self.disconnect()

    def connect(self):
        self.client.connect(config.SERVER_ADDRESS, int(config.SERVER_PORT))
        self.client.loop_start()

    def disconnect(self):
        self.client.loop_stop()
        self.client.disconnect()

    def onConnect(self, client: mqtt.Client, *args):
        client.subscribe("CONTROL")
        try:
            self.lock.acquire()
            for key in self.messageHandlers:
                client.subscribe(key)
        finally: 
            self.lock.release()

    def onMessage(self, client: mqtt.Client, _, msg: mqtt.MQTTMessage):
        if msg.topic == "CONTROL" and msg.payload.decode() == "QUIT":
            self.exit = True
            return
        try:
            self.lock.acquire()
            handler = self.messageHandlers.get(msg.topic)
            if handler:
                return handler(msg)
        finally:
            self.lock.release()

    def addMessageHandler(self, topic: str, handler: TopicHandler):
        try: 
            self.lock.acquire()    
            if self.client.is_connected:
                self.client.subscribe(topic)
            self.messageHandlers[topic] = handler
        finally:
            self.lock.release()
