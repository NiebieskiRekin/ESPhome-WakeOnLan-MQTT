import paho.mqtt.client as mqtt
import random
broker = ''
port = 8883
topic = "python/mqtt"
client_id = f'python-mqtt-{random.randint(0, 1000)}'
username = ''
password = ''

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Connected with result code {reason_code}")
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("test")
    client.publish("test", "1234")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

mqttc = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2,client_id=client_id)
mqttc.tls_set(tls_version=mqtt.ssl.PROTOCOL_TLS)
mqttc.username_pw_set(username, password)
mqttc.on_connect = on_connect
mqttc.on_message = on_message


mqttc.connect(broker, port)

print("something")
try:
    mqttc.loop_forever()
finally:
    mqttc.disconnect()
    print('end')