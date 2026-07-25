// 1. Replace this object with Firebase's web app config from your project settings.
// 2. The base station must write to: trackers/boat-001/latest
const firebaseConfig = {
  apiKey: "PASTE_YOUR_API_KEY",
  authDomain: "ais-gps-tracker.firebaseapp.com",
  databaseURL: "https://ais-gps-tracker-default-rtdb.firebaseio.com",
  projectId: "ais-gps-tracker",
  storageBucket: "ais-gps-tracker.firebasestorage.app",
  messagingSenderId: "4640602545",
  appId: "PASTE_YOUR_APP_ID"
};

const trackerPath = "trackers/boat-001/latest";
const defaultCenter = [12.8797, 121.7740]; // Philippines; used until a GPS update arrives.
const $ = (id) => document.getElementById(id);
const map = L.map("map", { zoomControl: false }).setView(defaultCenter, 6);
L.control.zoom({ position: "bottomright" }).addTo(map);
L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", { maxZoom: 19, attribution: "&copy; OpenStreetMap contributors" }).addTo(map);

const markerIcon = L.divIcon({ className: "", html: document.querySelector("#markerTemplate").innerHTML, iconSize: [34, 34], iconAnchor: [17, 33] });
let marker;
let latestPosition;

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

function updateDashboard(data) {
  const latitude = Number(data.latitude);
  const longitude = Number(data.longitude);
  if (!Number.isFinite(latitude) || !Number.isFinite(longitude)) throw new Error("Location needs numeric latitude and longitude.");
  latestPosition = [latitude, longitude];
  $("trackerName").textContent = data.name || "Boat 001";
  $("trackerId").textContent = data.deviceId || "boat-001";
  $("latitude").textContent = `${latitude.toFixed(6)}°`;
  $("longitude").textContent = `${longitude.toFixed(6)}°`;
  $("lastUpdated").textContent = formatTimestamp(data.timestamp);
  $("accuracy").textContent = data.accuracy != null ? `${data.accuracy} m` : "-- m";
  $("battery").textContent = data.battery != null ? `${data.battery}%` : "--%";
  $("rssi").textContent = data.rssi != null ? `${data.rssi} dBm` : "-- dBm";
  signalBars(Number(data.rssi));
  if (!marker) marker = L.marker(latestPosition, { icon: markerIcon }).addTo(map);
  else marker.setLatLng(latestPosition);
  marker.bindTooltip(data.name || "Boat 001", { direction: "top", offset: [0, -28] });
  map.flyTo(latestPosition, Math.max(map.getZoom(), 15), { duration: 1.1 });
  setStatus("Live", "connected");
}

$("locateButton").addEventListener("click", () => { if (latestPosition) map.flyTo(latestPosition, 16); });

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
    onValue(ref(getDatabase(app), trackerPath), (snapshot) => {
      if (!snapshot.exists()) { setStatus("Waiting for GPS", "offline"); return; }
      try { updateDashboard(snapshot.val()); } catch (error) { console.error(error); setStatus("Invalid GPS data", "offline"); }
    }, () => setStatus("Firebase unavailable", "offline"));
  } catch (error) {
    console.error(error);
    setStatus("Firebase unavailable", "offline");
  }
}
connectFirebase();
