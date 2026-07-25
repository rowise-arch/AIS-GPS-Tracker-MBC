# Web dashboard

1. Register a **Web app** in Firebase Console → Project settings → Your apps.
2. Copy its `apiKey` and `appId` into `app.js`. The known project values are already filled in.
3. Serve this folder (for example with VS Code Live Server); do not open `index.html` using `file://`.

The map listens to `trackers/boat-001/latest`. Change `trackerPath` in `app.js` if you use a different tracker ID.
