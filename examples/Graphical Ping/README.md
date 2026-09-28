# Graphical ping

## Overview

This project plots the network latency to a host in real time, the way the [gping](https://github.com/orf/gping) terminal tool does. A Python helper runs the operating system's `ping` command five times per second, parses the round-trip time from each reply, and streams it over UDP together with running statistics.

The helper starts on its own: a Control Loop in the project launches it when you click **Connect**, and Serial Studio stops it when you disconnect.

## Project features

- Live plot of the round-trip time, in milliseconds.
- Data grid with the host, its resolved address, and the last, minimum, maximum, average, jitter, and 95th percentile values.
- LED that lights when a request gets no reply, plus a running timeout count.
- Works on Windows, macOS, and Linux with the Python standard library only.

## Data format

Each UDP frame starts with `$`, ends with a newline, and contains ten comma-separated values:

`host, ip, last, min, max, avg, jitter, p95, timeouts, lost`

Where:

- `host`, `ip`: the target as typed and its resolved IPv4 address.
- `last`: latest round-trip time in ms. It is `0` when the request timed out.
- `min`, `max`, `avg`, `p95`: statistics over the last 60 seconds of replies.
- `jitter`: mean absolute difference between consecutive replies.
- `timeouts`: requests without a reply since the helper started.
- `lost`: `1` when the latest request timed out, otherwise `0`.

Example:

`$google.com,142.250.78.14,5.143,4.191,95.912,13.291,15.473,73.657,0,0`

## How to run

The script needs Python 3.6 or later and a `ping` command on the `PATH`, which every desktop operating system ships.

1. Open the `graphical-ping.ssproj` project file in Serial Studio.
2. Click **Connect**.

The input source is preconfigured as UDP on local port `9000`.

## Changing the target

Open the project editor, select **Control Loop** under **Project Scripts**, and edit the two variables at the top:

```javascript
var host = "google.com";
var intervalSeconds = 0.2;
```

Reconnect to apply the change. To run the helper yourself instead, start it before connecting:

```
python3 gping.py serial-studio.com --interval 0.2
```

Other options: `--timeout` (seconds to wait for a reply, default `1.0`), `--window` (statistics window in seconds, default `60`), `--udp-host`, and `--udp-port`.

## How it works

The helper launches one `ping` process per sample, each sending a single echo request. That avoids raw sockets, which need administrator rights on most systems. The flags differ per platform:

| Platform | Command |
|----------|---------|
| Windows | `ping -n 1 -w <milliseconds> host` |
| macOS | `ping -c 1 -W <milliseconds> host` |
| Linux | `ping -c 1 -W <seconds> host` |

The reply line is matched for its `time=12.3 ms` field, which also covers the `time<1ms` form Windows prints and localized output that keeps the `=`/`ms` pair.

## Files included

- `gping.py`: Python helper that pings the host and streams the results over UDP.
- `graphical-ping.ssproj`: Serial Studio project file with the Control Loop.
- `README.md`: project documentation.

## Notes

- A timed-out request plots as `0 ms`, so packet loss shows as a drop to the baseline along with the **Timed Out** LED.
- Some networks block ICMP. If every request times out, try another host or check the firewall.
- The helper's output (the resolved address and each timeout) appears in the Serial Studio console.
