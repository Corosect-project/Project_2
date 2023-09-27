from typing import Callable

import paho.mqtt.client as mqtt

import config.mqtt_config as config

TopicHandler = Callable[[mqtt.MQTTMessage], None]


class MqttHandler:
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
        for key in self.messageHandlers:
            client.subscribe(key)

    def onMessage(self, client: mqtt.Client, _, msg: mqtt.MQTTMessage):
        if msg.topic == "CONTROL" and msg.payload.decode() == "QUIT":
            self.exit = True
            return
        handler = self.messageHandlers.get(msg.topic)
        if handler:
            return handler(msg)

    def addMessageHandler(self, topic: str, handler: TopicHandler):
        if self.client.is_connected:
            self.client.subscribe(topic)
        self.messageHandlers[topic] = handler
        pass
