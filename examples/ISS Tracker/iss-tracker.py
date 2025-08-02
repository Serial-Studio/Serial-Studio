import time
import json
import socket
import requests

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
addr = ("127.0.0.1", 9000)

API_URL = "https://api.wheretheiss.at/v1/satellites/25544"

def get_iss_data():
    r = requests.get(API_URL, timeout=5)
    r.raise_for_status()
    return r.json()

while True:
    try:
        line = json.dumps(get_iss_data())
        print(line)
        sock.sendto((line + "\n").encode(), addr)

    except Exception as e:
        print("Error:", e)

    time.sleep(1)