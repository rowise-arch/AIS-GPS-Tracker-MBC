// Copy this object from Firebase Console > Project settings > Your apps > Web app.
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
const $ = (id) => document.getElementById(id);
const map = L.map("map", { zoomControl: false }).setView([12.8797, 121.774], 6);
L.control.zoom({ position: "bottomright" }).addTo(map);
L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", { maxZoom: 19, attribution: "&copy; OpenStreetMap" }).addTo(map);
const markerIcon = L.divIcon({ className: "", html: $("markerTemplate").innerHTML, iconSize: [34, 34], iconAnchor: [17, 33] });
let marker, latestPosition;
function status(text, type) { const e = $("connectionStatus"); e.className = `connection ${type}`; e.lastElementChild.textContent = text; }
function age(timestamp) { if (!timestamp) return "No signal"; const seconds = Math.max(0, Math.round((Date.now() - new Date(timestamp).getTime()) / 1000)); return seconds < 15 ? "Just now" : seconds < 60 ? `${seconds}s ago` : seconds < 3600 ? `${Math.floor(seconds / 60)}m ago` : new Date(timestamp).toLocaleString(); }
function bars(rssi) { const count = rssi >= -70 ? 4 : rssi >= -90 ? 3 : rssi >= -105 ? 2 : rssi ? 1 : 0; document.querySelectorAll(".signal-bars i").forEach((bar, i) => bar.classList.toggle("active", i < count)); }
function show(data) {
  const latitude = Number(data.latitude), longitude = Number(data.longitude);
  if (!Number.isFinite(latitude) || !Number.isFinite(longitude)) throw new Error("Invalid coordinates");
  latestPosition = [latitude, longitude];
  $("trackerName").textContent = data.name || "Boat 001"; $("trackerId").textContent = data.deviceId || "boat-001";
  $("latitude").textContent = `${latitude.toFixed(6)}°`; $("longitude").textContent = `${longitude.toFixed(6)}°`;
  $("lastUpdated").textContent = age(data.timestamp); $("accuracy").textContent = data.accuracy != null ? `${data.accuracy} m` : "-- m";
  $("battery").textContent = data.battery != null ? `${data.battery}%` : "--%"; $("rssi").textContent = data.rssi != null ? `${data.rssi} dBm` : "-- dBm"; bars(Number(data.rssi));
  if (!marker) marker = L.marker(latestPosition, { icon: markerIcon }).addTo(map); else marker.setLatLng(latestPosition);
  map.flyTo(latestPosition, Math.max(map.getZoom(), 15), { duration: 1 }); status("Live", "connected");
}
$("locateButton").addEventListener("click", () => latestPosition && map.flyTo(latestPosition, 16));
async function connect() {
  if (firebaseConfig.apiKey.startsWith("PASTE_") || firebaseConfig.appId.startsWith("PASTE_")) { status("Firebase setup needed", "offline"); $("trackerName").textContent = "Setup required"; return; }
  try { const { initializeApp } = await import("https://www.gstatic.com/firebasejs/10.12.2/firebase-app.js"); const { getDatabase, ref, onValue } = await import("https://www.gstatic.com/firebasejs/10.12.2/firebase-database.js");
    onValue(ref(getDatabase(initializeApp(firebaseConfig)), trackerPath), (snap) => { if (!snap.exists()) return status("Waiting for GPS", "offline"); try { show(snap.val()); } catch { status("Invalid GPS data", "offline"); } }, () => status("Firebase unavailable", "offline"));
  } catch { status("Firebase unavailable", "offline"); }
} connect();
