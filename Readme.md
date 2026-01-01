# SHIELD 1.0 – Drone Surveillance Prototype

## Overview
SHIELD 1.0 is a functional prototype for drone-assisted crowd surveillance, designed to demonstrate real-time telemetry ingestion, sensor fusion, and threat assessment.

## System Architecture
- ESP32-CAM provides live video feed
- ESP32 sensor node provides IMU, altitude, and system health
- Backend server fuses sensor data and evaluates threat levels
- Frontend dashboard visualizes live feed and analytics

## Simulation Note
Due to indoor testing constraints:
- GPS coordinates are locked to a predefined operational zone
- Acoustic crowd noise is simulated to represent real-world conversation levels

This approach is standard in early-stage prototyping.

## Features
- Live MJPEG camera feed
- Real-time telemetry
- Threat score computation
- Sensor fusion logic
- Modular backend design

## Technologies Used
- ESP32 / ESP32-CAM
- Python (FastAPI)
- OpenCV
- JavaScript dashboard
- WiFi-based telemetry

## Disclaimer
This repository represents a **prototype and architectural demonstration**, not a production deployment.
