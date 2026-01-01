# 🚁 SHIELD 1.0 – Drone Surveillance Prototype

## Overview

SHIELD 1.0 is a functional prototype for drone-assisted crowd surveillance, developed for real-time video streaming, IoT telemetry ingestion, sensor fusion, and algorithmic threat assessment.

---

## System Architecture

SHIELD 1.0 follows a modular, multi-layer architecture:

### Drone Layer (Edge Devices)
- ESP32-CAM for live aerial video streaming
- ESP32 sensor node for motion, altitude, and system health telemetry

### Communication Layer
- Wi-Fi–based data transmission
- MJPEG stream for low-latency video
- Structured telemetry packets for sensor data

### Backend Processing Layer
- Receives live video frames and sensor data
- Performs data fusion and threat evaluation
- Computes a real-time Threat Score
- Exposes processed data to the frontend

### Frontend Visualization Layer
- Web-based dashboard
- Live video feed display
- Telemetry visualization
- Threat level indicators

---

## Simulation Note

Due to indoor testing constraints and academic prototyping limitations:

- GPS coordinates are locked to a predefined operational zone
- Acoustic crowd noise values are simulated to represent real-world conversation levels


---

## Features & Implementation Details

### Live MJPEG Camera Feed
- ESP32-CAM streams MJPEG over HTTP
- Backend ingests frames using OpenCV
- Feed is rendered live on the dashboard

### Real-Time Telemetry
- IMU-based motion intensity
- Altitude and proximity readings
- System health parameters

### Sensor Fusion Logic
Multiple indicators are combined, including:
- Crowd density trends from video analysis
- Motion anomalies from IMU data
- Acoustic intensity variations
- Proximity changes

This fusion reduces false alerts and improves situational awareness.

### Threat Score Computation
- Rule-based weighted scoring model
- Individual indicators contribute to a normalized score
- Threat levels classified as Low, Medium, or High risk
- Model is fully explainable and tunable

### False-Color Night Vision (Software-Based)
- Video frames are converted to grayscale
- Pixel intensity values are mapped to a false-color palette
- Higher intensity regions appear warmer, lower intensity regions cooler
- Enhances visibility in low-light conditions

This is a visual augmentation technique and not a replacement for true thermal imaging hardware.

### Modular Backend Design
- Separate modules for video processing, telemetry, simulation, and threat logic
- Allows easy replacement of simulated inputs with real sensors
- Scalable for future ML-based extensions

---

## Technologies Used

- ESP32 / ESP32-CAM
- Python (FastAPI, OpenCV)
- JavaScript, HTML, CSS
- Wi-Fi–based telemetry
- MJPEG streaming

---

## Disclaimer

This repository represents a prototype and architectural demonstration intended for academic evaluation and proof-of-concept presentation.

---

## Submission Details

Team Name: Code.py  
Submission Type: Individual Submission  
Submitted By: Ishant Bhandari  
University: Graphic Era University, Dehradun
