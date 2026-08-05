#!/usr/bin/env python

# --------------------------------------------------------------------------------------
# Automatic dependency install (first run)
#
# Creates a private virtualenv next to this script (falling back to
# ~/.serial-studio/example-venvs when the example lives in a read-only install) and
# re-executes inside it, so system Python installs (PEP 668 "externally managed") are
# never modified. Requires only the standard library.
# --------------------------------------------------------------------------------------
import importlib.util
import os
import subprocess
import sys


def _ensure_deps(mod_to_pip):
    missing = [
        pip for mod, pip in mod_to_pip.items() if importlib.util.find_spec(mod) is None
    ]
    if not missing:
        return

    if os.environ.get("SS_EXAMPLE_BOOTSTRAPPED") == "1":
        sys.stderr.write("dependency bootstrap failed: %s\n" % ", ".join(missing))
        sys.exit(1)

    base = os.path.dirname(os.path.abspath(__file__))
    candidates = [
        os.path.join(base, ".venv"),
        os.path.join(
            os.path.expanduser("~"),
            ".serial-studio",
            "example-venvs",
            os.path.basename(base),
        ),
    ]

    py = None
    for venv in candidates:
        try:
            binary = (
                os.path.join(venv, "Scripts", "python.exe")
                if os.name == "nt"
                else os.path.join(venv, "bin", "python")
            )
            if not os.path.exists(binary):
                subprocess.check_call([sys.executable, "-m", "venv", venv])

            py = binary
            break
        except (OSError, subprocess.CalledProcessError):
            continue

    if py is None:
        sys.stderr.write("dependency bootstrap failed: cannot create a virtualenv\n")
        sys.exit(1)

    subprocess.check_call(
        [py, "-m", "pip", "install", "--disable-pip-version-check"] + missing
    )

    env = dict(os.environ, SS_EXAMPLE_BOOTSTRAPPED="1")
    argv = [py, "-u", os.path.abspath(__file__)] + sys.argv[1:]
    if os.name == "nt":
        sys.exit(subprocess.call(argv, env=env))

    os.execve(py, argv, env)


_ensure_deps({"requests": "requests", "paho": "paho-mqtt"})

import sys

# Force UTF-8 console output: Windows defaults to cp1252, which cannot encode
# the Unicode characters this script prints (e.g. arrows / check marks).
for _stream in (sys.stdout, sys.stderr):
    try:
        _stream.reconfigure(encoding="utf-8")
    except (AttributeError, ValueError):
        pass

import requests
import time
import datetime
import xml.etree.ElementTree as ET
import re
import paho.mqtt.client as paho

# -------------------------- CONFIGURATION --------------------------------

url_api = "http://192.168.9.1/api/device/signal"  # Address modem API
cycle_time = 5  # Pause between data frame
mqtt_topic = "lte"  # MQTT topic
mqtt_broker_ip = "127.0.0.1"  # MQTT broker local IP
mqtt_broker_port = 1883  # MQTT broker local port

# -------------------------------------------------------------------------


def get_value(marker):
    string = tree.find(marker).text
    value = re.search(r"(\-|)(\d+)(\.?)(\d*)", string).group(0)
    # print('string=', string, ' value=', value)
    return value


mqttc = paho.Client()
mqttc.connect(mqtt_broker_ip, mqtt_broker_port, 60)

while True:
    xml_data = requests.get(url_api).text
    tree = ET.XML(xml_data)

    cell = str(get_value("cell_id"))
    rsrq = int(float(get_value("rsrq")))
    rsrp = int(get_value("rsrp"))
    rssi = int(get_value("rssi"))
    sinr = int(get_value("sinr"))

    pci = int(get_value("pci"))
    mode = int(get_value("mode"))
    ulbandwidth = int(get_value("ulbandwidth"))
    dlbandwidth = int(get_value("dlbandwidth"))
    band = int(get_value("band"))
    ulfrequency = int(get_value("ulfrequency"))
    dlfrequency = int(get_value("dlfrequency"))

    print(
        f'{datetime.datetime.now().strftime("%H-%M-%S")} CELL={cell} RSRQ={rsrq} RSRP={rsrp} RSSI={rssi} SINR={sinr}'
    )
    data_frame = f"/*{cell} ,{rsrq},{rsrp},{rssi},{sinr}*/\n"
    print(data_frame)
    mqttc.publish(mqtt_topic, data_frame)

    time.sleep(cycle_time)
