let isSimulating = false;
let indicatorData = [];
let eventLog = [];

let lastThreatScore = -1;
let lastProcessedSecond = -1;

// DOM Elements
const droneVideo = document.getElementById('drone-video');
const thermalVideo = document.getElementById('thermal-video');
const simulateBtn = document.getElementById('simulate-btn');
const threatScoreEl = document.getElementById('threat-score');
const threatStatus = document.getElementById('threat-status');
const threatCard = document.getElementById('threat-card');

const crowdDensity = document.getElementById('crowd-density');
const stampedeRisk = document.getElementById('stampede-risk');
const noiseLevel = document.getElementById('noise-level');
const crowdAnomaly = document.getElementById('crowd-anomaly');

const eventList = document.getElementById('event-list');
const noEvents = document.getElementById('no-events');
const eventCount = document.getElementById('event-count');

// Clock
function updateClock() {
    const now = new Date();
    document.getElementById('current-date').textContent = now.toLocaleDateString();
    document.getElementById('current-time').textContent = now.toLocaleTimeString();
}
setInterval(updateClock, 1000);
updateClock();

// Load analytics (ONLY req.txt)
async function loadData() {
    const res = await fetch('req1.txt');
    const text = await res.text();

    indicatorData = text
        .trim()
        .split('\n')
        .slice(1)
        .map(line => {
            const [time, crowd, stampede, noise, anomaly] = line.split(',');
            return {
                time: parseInt(time),
                crowd: parseInt(crowd),
                stampede: parseInt(stampede),
                noise: parseInt(noise),
                anomaly: parseInt(anomaly)
            };
        });
}

// Threat aggregation (weighted sensor fusion) - YOUR LOGIC PRESERVED
function computeThreat(ind) {
    const crowd = ind.crowd;
    const stampede = ind.stampede;
    const noise = ind.noise;
    const anomaly = ind.anomaly ? 100 : 0;

    const threat = 
        0.35 * crowd +
        0.30 * stampede +
        0.20 * noise +
        0.15 * anomaly;

    return Math.min(100, Math.round(threat));
}

// Update threat UI
function updateThreatUI(score) {
    threatScoreEl.textContent = score;

    threatCard.classList.remove('border-low', 'border-medium', 'border-high');
    threatScoreEl.classList.remove('color-low', 'color-medium', 'color-high');

    if (score < 30) {
        threatStatus.textContent = 'NOMINAL';
        threatCard.classList.add('border-low');
        threatScoreEl.classList.add('color-low');
    } else if (score < 70) {
        threatStatus.textContent = 'ELEVATED';
        threatCard.classList.add('border-medium');
        threatScoreEl.classList.add('color-medium');
    } else {
        threatStatus.textContent = 'CRITICAL';
        threatCard.classList.add('border-high');
        threatScoreEl.classList.add('color-high');
    }
}

// Add event to timeline
function addEvent(threat, videoTime, indicators) {
    const now = new Date();
    const timeStr = now.toLocaleTimeString();
    
    const event = {
        time: timeStr,
        videoTime: Math.floor(videoTime),
        threat: threat,
        indicators: indicators
    };
    
    eventLog.unshift(event);
    
    // Update event count
    eventCount.textContent = eventLog.length;
    
    // Hide "no events" message
    if (noEvents) {
        noEvents.style.display = 'none';
    }
    
    // Create event item
    const eventItem = document.createElement('div');
    eventItem.className = 'event-item';
    
    if (threat < 30) {
        eventItem.classList.add('threat-low');
    } else if (threat < 70) {
        eventItem.classList.add('threat-medium');
    } else {
        eventItem.classList.add('threat-high');
    }
    
    eventItem.innerHTML = `
        <div class="event-header">
            <span class="event-time">${timeStr}</span>
            <span class="event-video-time">T+${event.videoTime}s</span>
        </div>
        <div class="event-threat">
            Threat Level: <span class="event-threat-value">${threat}</span>
        </div>
        <div class="event-details">
            Crowd: ${indicators.crowd}% | Stampede: ${indicators.stampede}% | Noise: ${indicators.noise}% | Anomaly: ${indicators.anomaly}%
        </div>
    `;
    
    // Add to top of list
    eventList.insertBefore(eventItem, eventList.firstChild);
    
    // Keep only last 20 events
    while (eventList.children.length > 20) {
        eventList.removeChild(eventList.lastChild);
    }
}

// Main update loop
function updateFromVideo() {
    if (!isSimulating || droneVideo.paused) return;

    const sec = Math.floor(droneVideo.currentTime);
    if (sec === lastProcessedSecond) return;
    lastProcessedSecond = sec;

    let latest = indicatorData[0];
    for (const row of indicatorData) {
        if (row.time <= sec) latest = row;
        else break;
    }

    // Update analytics (percentages)
    crowdDensity.textContent = `${latest.crowd}%`;
    stampedeRisk.textContent = `${latest.stampede}%`;
    noiseLevel.textContent = `${latest.noise}%`;
    crowdAnomaly.textContent = `${latest.anomaly}%`;

    // Compute threat
    const threat = computeThreat(latest);
    if (threat !== lastThreatScore) {
        updateThreatUI(threat);
        
        // Add event to timeline when threat changes
        addEvent(threat, droneVideo.currentTime, {
            crowd: latest.crowd,
            stampede: latest.stampede,
            noise: latest.noise,
            anomaly: latest.anomaly
        });
        
        lastThreatScore = threat;
    }
}

// Start simulation
simulateBtn.addEventListener("click", async () => {
    if (isSimulating) return;

    isSimulating = true;
    simulateBtn.textContent = "SYSTEM LIVE";
    simulateBtn.classList.add("active");

    try {
        await droneVideo.play();
        console.log("✅ Drone video playing");
    } catch (e) {
        console.error("❌ Drone play failed", e);
    }

    try {
        await thermalVideo.play();
        console.log("✅ Thermal video playing");
    } catch (e) {
        console.error("❌ Thermal play failed", e);
    }
});

loadData().then(() => {
    console.log("✅ req.txt loaded:", indicatorData);
});

function animationLoop() {
    updateFromVideo();
    requestAnimationFrame(animationLoop);
}
animationLoop();