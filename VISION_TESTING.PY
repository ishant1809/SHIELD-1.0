import cv2
import time

VIDEO_URL = "http://192.168.137.240:81/stream"

cap = cv2.VideoCapture(VIDEO_URL, cv2.CAP_FFMPEG)

# Reduce internal buffering
cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)

print("Opening stream...")

last_frame_time = time.time()

while True:
    ret, frame = cap.read()

    if ret:
        last_frame_time = time.time()
        cv2.imshow("SHIELD CAM", frame)
    else:
        # Don't panic — ESP32 MJPEG stalls briefly sometimes
        if time.time() - last_frame_time > 5:
            print("⚠️ No frames for 5s, reconnecting...")
            cap.release()
            time.sleep(1)
            cap = cv2.VideoCapture(VIDEO_URL, cv2.CAP_FFMPEG)
            cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)

    if cv2.waitKey(1) & 0xFF == 27:
        break

cap.release()
cv2.destroyAllWindows()
