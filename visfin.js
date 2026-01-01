"use client";

import React, { useState, useEffect, useRef } from "react";
import {
  Activity,
  Radio,
  Compass,
  Mountain,
  Gauge,
  AlertTriangle,
  TrendingUp,
  MapPin
} from "lucide-react";

export default function DroneDashboard() {
  const [telemetry, setTelemetry] = useState(null);
  const [connected, setConnected] = useState(false);
  const [audioSpike, setAudioSpike] = useState(false);
  const [altitudeHistory, setAltitudeHistory] = useState([]);

  const mapRef = useRef(null);
  const leafletMapRef = useRef(null);
  const markerRef = useRef(null);

  // 🔴 USE ONE IP ONLY
  const ESP32_BASE = "http://192.168.137.112";
  const ESP32_CAM_URL = `${ESP32_BASE}:81/stream`;
  const ESP32_DATA_URL = `${ESP32_BASE}/data`;

  /* ---------------- FETCH TELEMETRY ---------------- */
  useEffect(() => {
    let timer;

    const fetchData = async () => {
      try {
        const res = await fetch(ESP32_DATA_URL, { cache: "no-store" });
        const data = await res.json();

        setTelemetry(data);
        setConnected(true);

        if (data?.audio?.spike) {
          setAudioSpike(true);
          setTimeout(() => setAudioSpike(false), 800);
        }

        if (typeof data?.altitude_cm === "number") {
          setAltitudeHistory(prev =>
            [...prev, data.altitude_cm].slice(-50)
          );
        }

        if (data?.gps?.fix && leafletMapRef.current) {
          updateMap(data.gps.lat, data.gps.lon);
        }
      } catch (e) {
        setConnected(false);
      }
    };

    fetchData();
    timer = setInterval(fetchData, 500);

    return () => clearInterval(timer);
  }, []);

  /* ---------------- LOAD LEAFLET ---------------- */
  useEffect(() => {
    if (window.L) {
      initMap();
      return;
    }

    const css = document.createElement("link");
    css.rel = "stylesheet";
    css.href = "https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.9.4/leaflet.css";
    document.head.appendChild(css);

    const script = document.createElement("script");
    script.src = "https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.9.4/leaflet.js";
    script.onload = initMap;
    document.body.appendChild(script);
  }, []);

  const initMap = () => {
    if (!mapRef.current || leafletMapRef.current || !window.L) return;

    leafletMapRef.current = window.L.map(mapRef.current).setView(
      [26.9124, 75.7873],
      13
    );

    window.L.tileLayer(
      "https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png",
      { attribution: "© OpenStreetMap" }
    ).addTo(leafletMapRef.current);
  };

  const updateMap = (lat, lon) => {
    if (!leafletMapRef.current || !window.L) return;

    if (markerRef.current) {
      markerRef.current.setLatLng([lat, lon]);
    } else {
      markerRef.current = window.L.marker([lat, lon]).addTo(
        leafletMapRef.current
      );
    }

    leafletMapRef.current.setView([lat, lon], 15);
  };

  /* ---------------- ALTITUDE NOISE ---------------- */
  const getAltitudeNoise = () => {
    if (altitudeHistory.length < 10) return 0;
    const recent = altitudeHistory.slice(-10);
    const avg = recent.reduce((a, b) => a + b, 0) / recent.length;
    return Math.sqrt(
      recent.reduce((s, v) => s + Math.pow(v - avg, 2), 0) / recent.length
    );
  };

  const noise = getAltitudeNoise();
  const isNoisy = noise > 5;

  if (!telemetry) {
    return (
      <div className="min-h-screen bg-gray-900 flex items-center justify-center text-white">
        Connecting to ESP32…
      </div>
    );
  }

  /* ---------------- UI ---------------- */
  return (
    <div className="min-h-screen bg-gray-900 text-white p-4">
      <div className="max-w-7xl mx-auto space-y-4">

        {/* HEADER */}
        <div className="flex justify-between items-center">
          <h1 className="text-3xl font-bold flex items-center gap-3">
            <Radio className={connected ? "text-green-500" : "text-red-500"} />
            ESP32 Drone Monitor
          </h1>
          <div className={`px-4 py-2 rounded ${connected ? "bg-green-500/20 text-green-400" : "bg-red-500/20 text-red-400"}`}>
            {connected ? "CONNECTED" : "DISCONNECTED"}
          </div>
        </div>

        {/* VIDEO */}
        <div className="bg-gray-800 rounded-lg overflow-hidden">
          <div className="bg-gray-700 px-4 py-2">Live Camera Feed</div>
          <img src={ESP32_CAM_URL} className="w-full aspect-video object-contain" />
        </div>

        {/* TELEMETRY */}
        <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
          <Stat title="Altitude" value={`${telemetry.altitude_cm} cm`} icon={<Mountain />} />
          <Stat title="Audio RMS" value={telemetry.audio?.rms?.toFixed(2)} icon={<Gauge />} warn={audioSpike} />
          <Stat title="Pitch" value={`${telemetry.orientation?.pitch?.toFixed(1)}°`} icon={<Compass />} />
          <Stat title="Noise σ" value={noise.toFixed(2)} icon={<AlertTriangle />} warn={isNoisy} />
        </div>

        {/* MAP */}
        <div className="bg-gray-800 rounded-lg overflow-hidden">
          <div className="bg-gray-700 px-4 py-2">Live Map</div>
          <div ref={mapRef} className="h-64 bg-black" />
        </div>

      </div>
    </div>
  );
}

/* ---------------- SMALL STAT COMPONENT ---------------- */
function Stat({ title, value, icon, warn }) {
  return (
    <div className={`bg-gray-800 p-4 rounded ${warn ? "ring-2 ring-red-500" : ""}`}>
      <div className="flex items-center gap-2 mb-1 text-gray-400">
        {icon} {title}
      </div>
      <div className="text-xl font-mono">{value}</div>
    </div>
  );
}
