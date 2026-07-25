// Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyCGouqpCGqDhq1NWP7rXint7CVgU4LSCr0",
  authDomain: "ais-gps-tracker.firebaseapp.com",
  databaseURL: "https://ais-gps-tracker-default-rtdb.firebaseio.com",
  projectId: "ais-gps-tracker",
  storageBucket: "ais-gps-tracker.firebasestorage.app",
  messagingSenderId: "4640602545",
  appId: "1:4640602545:web:c52dac68e8ef8199df2590"
};

const trackerPath = "trackers"; // Watch the entire trackers node
const defaultCenter = [12.4, 122.3]; // Center of Romblon
const $ = (id) => document.getElementById(id);

// Map setup
const map = L.map("map", { 
  zoomControl: false,
  center: defaultCenter,
  zoom: 10
});
L.control.zoom({ position: "bottomright" }).addTo(map);

// Add OpenStreetMap tiles
L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", { 
  maxZoom: 19, 
  attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a>'
}).addTo(map);

// Store markers for each boat
const markers = {};
const boatData = {};
let selectedBoat = null;
let latestPosition = null;

// Boat colors
const boatColors = {
  'boat-001': '#087d76',  // Teal
  'boat-002': '#e74c3c',  // Red
  'boat-003': '#3498db',  // Blue
  'boat-004': '#f39c12',  // Orange
  'boat-005': '#9b59b6'   // Purple
};

// Boat names
const boatNames = {
  'boat-001': 'Boat 001',
  'boat-002': 'Boat 002', 
  'boat-003': 'Boat 003',
  'boat-004': 'Boat 004',
  'boat-005': 'Boat 005'
};

function getBoatColor(boatId) {
  return boatColors[boatId] || '#2ecc71';
}

function getBoatName(boatId) {
  return boatNames[boatId] || boatId.toUpperCase();
}

function setStatus(label, state) {
  const element = $("connectionStatus");
  element.className = `connection ${state}`;
  element.lastElementChild.textContent = label;
}

function formatTimestamp(value) {
  if (!value) return "No signal";
  const date = new Date(typeof value === "number" ? value : value);
  if (Number.isNaN(date.getTime())) return "Invalid time";
  const seconds = Math.round((Date.now() - date.getTime()) / 1000);
  if (seconds < 15) return "Just now";
  if (seconds < 60) return `${seconds}s ago`;
  if (seconds < 3600) return `${Math.floor(seconds / 60)}m ago`;
  return date.toLocaleString();
}

function signalBars(rssi) {
  const count = rssi >= -70 ? 4 : rssi >= -90 ? 3 : rssi >= -105 ? 2 : rssi ? 1 : 0;
  document.querySelectorAll(".signal-bars i").forEach((bar, index) => bar.classList.toggle("active", index < count));
}

function createBoatMarker(boatId, lat, lng) {
  const color = getBoatColor(boatId);
  const name = getBoatName(boatId);
  
  // Create custom marker with boat color
  const iconHtml = `
    <div class="tracker-marker" style="background:${color};">
      <span></span>
    </div>
  `;
  
  const icon = L.divIcon({
    className: "",
    html: iconHtml,
    iconSize: [34, 34],
    iconAnchor: [17, 33]
  });
  
  const marker = L.marker([lat, lng], { icon: icon });
  marker.bindTooltip(name, { 
    direction: "top", 
    offset: [0, -28],
    permanent: false,
    className: 'boat-tooltip'
  });
  
  // Click on marker to select this boat
  marker.on('click', function() {
    selectBoat(boatId);
  });
  
  return marker;
}

function selectBoat(boatId) {
  selectedBoat = boatId;
  const data = boatData[boatId];
  if (data) {
    updateDashboard(boatId, data);
  }
  
  // Highlight selected marker
  for (const [id, marker] of Object.entries(markers)) {
    const color = getBoatColor(id);
    const isSelected = id === boatId;
    // You could add a highlight effect here
  }
}

function updateDashboard(boatId, data) {
  const latitude = Number(data.latitude);
  const longitude = Number(data.longitude);
  if (!Number.isFinite(latitude) || !Number.isFinite(longitude)) return;
  
  latestPosition = [latitude, longitude];
  
  const name = getBoatName(boatId);
  
  $("trackerName").textContent = name;
  $("trackerId").textContent = boatId;
  $("latitude").textContent = `${latitude.toFixed(6)}°`;
  $("longitude").textContent = `${longitude.toFixed(6)}°`;
  $("lastUpdated").textContent = formatTimestamp(data.timestamp);
  $("accuracy").textContent = data.accuracy != null ? `${data.accuracy} m` : "-- m";
  $("battery").textContent = data.battery != null ? `${data.battery}%` : "--%";
  $("rssi").textContent = data.rssi != null ? `${data.rssi} dBm` : "-- dBm";
  signalBars(Number(data.rssi));
  setStatus("Live", "connected");
  
  // Update boat count
  updateBoatList();
}

function updateBoatList() {
  const boatIds = Object.keys(boatData);
  const count = boatIds.length;
  
  // Update the subtitle with boat count
  if (count > 1) {
    $("trackerId").textContent = `${count} boats active`;
  }
}

function updateAllBoats(snapshot) {
  const data = snapshot.val();
  if (!data) {
    setStatus("Waiting for GPS", "offline");
    return;
  }
  
  let hasData = false;
  let activeBoats = 0;
  
  // Iterate through all boats
  for (const [boatId, boatInfo] of Object.entries(data)) {
    if (boatInfo.latest) {
      hasData = true;
      activeBoats++;
      const latest = boatInfo.latest;
      const lat = Number(latest.latitude);
      const lng = Number(latest.longitude);
      
      if (!Number.isFinite(lat) || !Number.isFinite(lng)) continue;
      
      // Store data
      boatData[boatId] = latest;
      
      // Update or create marker
      if (markers[boatId]) {
        markers[boatId].setLatLng([lat, lng]);
      } else {
        const marker = createBoatMarker(boatId, lat, lng);
        marker.addTo(map);
        markers[boatId] = marker;
        console.log(`✅ Added marker for ${getBoatName(boatId)}`);
      }
    }
  }
  
  // If no boat is selected, select the first one
  if (hasData && !selectedBoat) {
    const firstBoat = Object.keys(boatData)[0];
    selectBoat(firstBoat);
  }
  
  // Update status
  if (activeBoats > 0) {
    setStatus(`${activeBoats} boats live`, "connected");
  }
  
  // Log active boats
  if (activeBoats > 0) {
    console.log(`📍 ${activeBoats} boats active: ${Object.keys(boatData).join(', ')}`);
  }
}

// Locate button - zooms to selected boat
$("locateButton").addEventListener("click", () => { 
  if (latestPosition) {
    map.flyTo(latestPosition, 15, { duration: 1.1 });
  } 
});

// Connect to Firebase
async function connectFirebase() {
  if (firebaseConfig.apiKey.startsWith("PASTE_")) {
    setStatus("Firebase setup needed", "offline");
    $("trackerName").textContent = "Setup required";
    $("trackerId").textContent = "Add Firebase config in app.js";
    return;
  }
  try {
    const { initializeApp } = await import("https://www.gstatic.com/firebasejs/10.12.2/firebase-app.js");
    const { getDatabase, ref, onValue } = await import("https://www.gstatic.com/firebasejs/10.12.2/firebase-database.js");
    const app = initializeApp(firebaseConfig);
    
    // Watch the entire trackers node
    onValue(ref(getDatabase(app), trackerPath), (snapshot) => {
      updateAllBoats(snapshot);
    }, (error) => {
      console.error("Firebase error:", error);
      setStatus("Firebase unavailable", "offline");
    });
    
    console.log("✅ Connected to Firebase. Listening for boats...");
  } catch (error) {
    console.error("❌ Firebase connection error:", error);
    setStatus("Firebase unavailable", "offline");
  }
}

// Start the app
connectFirebase();

// Log startup
console.log("🚤 AIS GPS Tracker - Multi-Boat Support");
console.log("📍 Tracking boats in Romblon, Philippines");