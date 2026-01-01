# orientation_ws.py
import json
import requests
import asyncio
import websockets
import math

DEV_URL = "http://192.168.137.240/data"

def normalize_quat(q):
    n = math.sqrt(q["w"]**2 + q["x"]**2 + q["y"]**2 + q["z"]**2)
    if n == 0:
        return None
    return {
        "w": q["w"] / n,
        "x": q["x"] / n,
        "y": q["y"] / n,
        "z": q["z"] / n
    }

async def stream_orientation(ws):
    session = requests.Session()

    while True:
        try:
            r = session.get(DEV_URL, timeout=(1.5, 3))
            j = r.json()

            if "quat" not in j:
                print("WS error: quat missing")
                await asyncio.sleep(0.2)
                continue

            q = normalize_quat(j["quat"])
            if q is None:
                print("WS error: bad quaternion")
                await asyncio.sleep(0.2)
                continue

            payload = {
                "qw": q["w"],
                "qx": q["x"],
                "qy": q["y"],
                "qz": q["z"]
            }

            # 🔎 uncomment ONLY if debugging
            # print("WS →", payload)

            await ws.send(json.dumps(payload))
            await asyncio.sleep(0.03)  # ~30 FPS

        except Exception as e:
            print("WS exception:", e)
            await asyncio.sleep(0.5)

async def main():
    async with websockets.serve(stream_orientation, "0.0.0.0", 8765):
        print("Orientation WS running on :8765")
        await asyncio.Future()

asyncio.run(main())
