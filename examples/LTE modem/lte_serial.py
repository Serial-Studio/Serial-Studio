#!/usr/bin/env python

import requests
import time
import datetime
import xml.etree.ElementTree as ET
import re
import serial

# -------------------------- CONFIGURATION --------------------------------

url_api = 'http://192.168.9.1/api/device/signal' # Address modem API
cycle_time = 5                                   # Pause between data frame
serial_port = '/tmp/ttyV1'                       # Serial port address
serial_speed = 9600                              # Serial port baudrate

# -------------------------------------------------------------------------

def get_value(marker):
    string = tree.find(marker).text
    value = re.search(r'(\-|)(\d+)(\.?)(\d*)', string).group(0)
    # print('string=', string, ' value=', value)
    return value

serialPort = serial.Serial(port=serial_port, baudrate=serial_speed, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)

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
    data_frame = f'/*{cell} ,{rsrq},{rsrp},{rssi},{sinr}*/\n'
    # print(frame)
    serialPort.write(data_frame.encode('utf-8'))

    time.sleep(cycle_time)
