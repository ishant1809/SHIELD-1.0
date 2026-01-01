import requests
import time

URL = "http://192.168.137.117:80/data"

session = requests.Session()
session.headers.update({"Connection": "close"})

while True:
    try:
        r = session.get(URL, timeout=(1.5, 3))
        if r.ok:
            print(r.json())
        else:
            print("HTTP", r.status_code)
    except requests.exceptions.RequestException as e:
        print("Fetch skipped:", e)
    time.sleep(0.5)
