#!/usr/bin/env python

import requests
import time
import datetime
import xml.etree.ElementTree as ET
import re
import paho.mqtt.client as paho

# -------------------------- CONFIGURATION --------------------------------

url_api = 'http://192.168.9.1/api/device/signal' # Address modem API
cycle_time = 5                                   # Pause between data frame
mqtt_topic = 'lte'                               # MQTT topic
mqtt_broker_ip = '127.0.0.1'                     # MQTT broker local IP
mqtt_broker_port = 1883                          # MQTT broker local port

# -------------------------------------------------------------------------

def get_value(marker):
    string = tree.find(marker).text
    value = re.search(r'(\-|)(\d+)(\.?)(\d*)', string).group(0)
    # print('string=', string, ' value=', value)
    return value

mqttc = paho.Client()
mqttc.connect(mqtt_broker_ip, mqtt_broker_port, 60)

while True:
    xml_data = requests.get(url_api).text
    tree = ET.XML(xml_data)

    cell = str(get_value('cell_id'))
    rsrq = int(float(get_value('rsrq')))
    rsrp = int(get_value('rsrp'))
    rssi = int(get_value('rssi'))
    sinr = int(get_value('sinr'))

    pci = int(get_value('pci'))
    mode = int(get_value('mode'))
    ulbandwidth = int(get_value('ulbandwidth'))
    dlbandwidth = int(get_value('dlbandwidth'))
    band = int(get_value('band'))
    ulfrequency = int(get_value('ulfrequency'))
    dlfrequency = int(get_value('dlfrequency'))

    print(f'{datetime.datetime.now().strftime("%H-%M-%S")} CELL={cell} RSRQ={rsrq} RSRP={rsrp} RSSI={rssi} SINR={sinr}')
    data_frame = f'{rsrq},{rsrp},{rssi},{sinr},{cell}'
    # print(frame)
    mqttc.publish(mqtt_topic, data_frame)

    time.sleep(cycle_time)
