import requests
import time

ESP32_IP = "192.168.137.71"   # 🔁 your ESP32 IP
URL = f"http://{ESP32_IP}/data"

def threat_score(noise, motion, temperature):
    score = 0

    if noise > 60:
        score += 30
    if motion == 1:
        score += 40
    if temperature > 35:
        score += 30

    if score >= 70:
        level = "HIGH"
    elif score >= 40:
        level = "MEDIUM"
    else:
        level = "LOW"

    return score, level

while True:
    try:
        data = requests.get(URL, timeout=2).json()

        noise = data["noise"]
        motion = data["motion"]
        temperature = data["temperature"]

        score, level = threat_score(noise, motion, temperature)

        print(
            f"Noise={noise} | Motion={motion} | Temp={temperature} "
            f"=> Threat Score={score} [{level}]"
        )

    except Exception as e:
        print("ESP32 not reachable:", e)

    time.sleep(1)
