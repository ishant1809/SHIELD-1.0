import cv2
from ultralytics import YOLO

# =========================
# Load YOLOv8 model
# =========================
# Use nano model for real-time performance
model = YOLO("yolov8n.pt")

# =========================
# Video source
# =========================
# 0 = webcam
# Or replace with drone video / stream URL
cap = cv2.VideoCapture("crowd.mp4")

# =========================
# Main loop
# =========================
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    # Run YOLO inference (only detect persons -> class 0)
    results = model(frame, classes=[0], conf=0.4)

    person_count = 0

    # Extract detections
    for box in results[0].boxes:
        x1, y1, x2, y2 = map(int, box.xyxy[0])
        confidence = float(box.conf[0])

        person_count += 1

        # Draw bounding box
        cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
        cv2.putText(
            frame,
            f"Person {confidence:.2f}",
            (x1, y1 - 8),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (0, 255, 0),
            1
        )

    # Display count
    cv2.putText(
        frame,
        f"People Count: {person_count}",
        (20, 40),
        cv2.FONT_HERSHEY_SIMPLEX,
        1,
        (0, 0, 255),
        2
    )

    cv2.imshow("SHIELD 1.0 - YOLOv8 Person Detection", frame)

    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

cap.release()
cv2.destroyAllWindows()
