# ISS Tracker

## Overview

This project visualizes the real-time position of the International Space Station (ISS) in Serial Studio. A built-in control script polls a public API directly over TCP, so there is no companion program to run and no extra hardware. Serial Studio parses the response and shows the station on an interactive map alongside a live 3D view.

Real-time satellite tracking with nothing but Serial Studio and an internet connection.

> Some Serial Studio features need a paid license. See [serial-studio.com](https://serial-studio.com/) for details.

![ISS Tracker in Serial Studio](doc/screenshot.png)

## Telemetry source

ISS position is pulled from the [Open Notify](http://open-notify.org/Open-Notify-API/ISS-Location-Now/) `iss-now.json` API. The project's control script opens a TCP connection to `api.open-notify.org:80`, issues an HTTP `GET /iss-now.json` request every few seconds, and lets the frame parser read the response. Two fields are extracted:

- `latitude`: geographic latitude in degrees.
- `longitude`: geographic longitude in degrees.

## Project features

- Real-time map tracking of the ISS.
- Interactive 3D view of the station rendered in the dashboard.
- API polling driven entirely by an in-project control script, no external program.
- TCP source with a JSON frame parser, configured in Serial Studio's project editor.

## Data format

The API returns a JSON object. The project's frame delimiters isolate the
`iss_position` block, and the parser reads its two fields:

```json
{
  "iss_position": { "latitude": "29.35", "longitude": "-94.98" },
  "timestamp": 1747294302,
  "message": "success"
}
```

Serial Studio parses the position into an array:

```json
[29.35, -94.98]
```

The widgets then map to array indices:

- `Latitude`: index 1.
- `Longitude`: index 2.
- `Altitude`: index 3 (the `ISS Position` map group's altitude channel).

## How to run

1. Open Serial Studio and load `iss-tracker.ssproj` (project file included).
2. Click **Connect**.

That is all. The bundled control script opens the TCP connection to the API and polls it for you, so the map and 3D view start updating on their own. The source is preconfigured as a **Network Socket** in **TCP** mode pointing at `api.open-notify.org`, port **80**.

## Visualizations

- **Map widget.** Live ISS position by latitude and longitude.
- **ISS 3D View.** A Heavens-Above-style render of the station over a sunlit
  Earth limb, with a starfield, an inset mini-globe showing the ground track
  and heading, and a heads-up readout (location, heading, sun altitude, frames
  per second). The view is interactive: **drag** to orbit the camera, **scroll**
  to zoom, and **double-click** to reset. It animates continuously, so the scene
  stays alive between telemetry updates.

## Files

- `iss-tracker.ssproj`: Serial Studio project file (pre-configured, includes the control script).
- `README.md`: project documentation.
- `doc/screenshot.png`: visualization screenshot.

## Notes

- The API occasionally returns incomplete or missing fields. The parser skips a frame that fails to decode and waits for the next poll.
- The open-notify API exposes only latitude and longitude; altitude and orbital velocity are not part of this feed.
