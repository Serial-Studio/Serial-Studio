# Remote Dashboard

The hardware is on a test stand in another room, and you want its dashboard on your desk. Remote Dashboard does that: the Serial Studio next to the hardware is the *publisher*, the one on your desk is the *viewer*, and the viewer shows the same groups, datasets, and live values. Remote Dashboard is in both the free and Pro editions; no license is needed on either side.

The view is read-only. Nothing you do on the viewer reaches the device, and the viewer's own project on disk is never modified. The device stays plugged into the publisher; the viewer only needs a network route to it. The link uses Serial Studio's API Server (a network port, 7777 by default), which is off until you turn it on.

## Try it on one machine

1. Open **Preferences**, **General** tab, **API & Plugins** group, and turn on **Enable API Server**.
2. Open a project so there is a dashboard to show. Do not connect a device: attaching is refused while a local device is connected.
3. Run the **Remote Dashboard** command from the [command palette](Command-Palette.md) (**Ctrl+K**, **Cmd+K** on macOS).
4. Leave Host `127.0.0.1`, Port `7777`, Token empty, and press **Attach**.

Local connections need no token. With no device connected, expect the status line "connected, the remote is not producing data": a one-machine test shows the layout, not live values. For live values, run the viewer on a second machine and follow the next two sections.

## Publisher: allow incoming viewers

Viewers on the same machine need only step 1; another machine also needs step 2.

1. Turn on **Enable API Server** (**Preferences**, **General** tab, **API & Plugins** group). It is off by default. The **API Server Port** field below it sets the port (1024-65535, default 7777).
2. Turn on **Allow External API Connections** and copy the **API Access Token** below it. A confirmation dialog, **Allow External API Connections?**, appears; press **Yes** (**No** leaves the toggle off). The token field stays disabled, and read-only, until the toggle is on. Read [Trust model](#trust-model) first: the link is not encrypted.

With **Allow External API Connections** off, the server listens on loopback only. Turning **Enable API Server** off also turns **Allow External API Connections** off, so switch it back on after re-enabling the server.

The refresh button next to the token generates a new one and invalidates the old one.

The viewer needs the publisher's LAN address, for example `192.168.1.20`, and TCP port 7777 (or your port) allowed through the publisher's firewall.

The publisher accepts at most 32 simultaneous viewers, the API server's client limit. The `API/MaxViewers` key in the application settings can only lower that cap, and `0` refuses viewers while leaving the rest of the API available. There is no UI field for it.

## Viewer: attach

Before attaching, the publisher must have **Enable API Server** on, plus **Allow External API Connections** and a token if the viewer is another machine (see [Publisher](#publisher-allow-incoming-viewers)).

1. Run the **Remote Dashboard** command from the [command palette](Command-Palette.md) (**Ctrl+K**, **Cmd+K** on macOS). The **Attach to Remote Dashboard** dialog opens.
2. Fill in the connection fields:

| Field | Meaning | Default |
|-------|---------|---------|
| Recent | The 12 most recent `host:port` endpoints; selecting one fills the fields below, including the token used with it | shown once an endpoint has been used |
| Host | Host name or IP address of the publisher | `127.0.0.1` |
| Port | The publisher's API Server Port (**Preferences**, **General**, **API & Plugins**) | `7777` |
| Token | The publisher's API Access Token; required only for connections from another machine | empty |

3. Press **Attach**.

A viewer cannot attach while its own device connection or a recording is active. The dialog shows "Disconnect the local device or close the open recording first." and **Attach** stays disabled.

The status line shows the link state:

| Status text | What to do |
|-------------|------------|
| Not attached. | Nothing yet; press **Attach**. |
| Attached to *host:port* - live, *N* datasets at *N* Hz. | Values are flowing. |
| Attached to *host:port* - connected, the remote is not producing data. | The link is up but the publisher is producing no data. Connect a device or open a project on the publisher. |
| Attached to *host:port* - no response, the link may be down. | Heartbeats stopped. Check that the publisher is running and that host, port, and token are right. The viewer keeps the last values and recovers when the link returns. |

If an attach fails, the status line shows the failure reason instead.

While attached, the toolbar's **Connect** button is replaced by a **Remote Dashboard** button. Click it to reopen the dialog and press **Detach**. Detach restores whatever the viewer was doing before: its own project, operation mode, and connection state.

The viewer asks for 60 Hz; the publisher grants the lower of that and its own dashboard refresh rate, between 1 and 60 Hz. The status line shows the negotiated rate.

Endpoints and their tokens are remembered in the viewer's application settings, and the dialog reopens with the last used publisher filled in.

## Headless publisher

Only needed for a machine with no screen, such as an embedded PC. `serial-studio` is the installed application executable (see [Command Line Interface](Command-Line-Interface.md)). Provision the publisher from the command line:

```bash
serial-studio --headless --api-server --api-external --api-token-file /etc/serial-studio/token
```

`--api-external` is the non-interactive form of the **Allow External API Connections** toggle and skips its confirmation dialog. The token must be at least 32 hexadecimal characters; `openssl rand -hex 16` makes one. `--api-port <port>` replaces 7777 and accepts 1-65535, wider than the Settings field's 1024-65535. With no token source, the publisher generates a token.

Pass the token with `--api-token-file <path>` or the `SS_API_TOKEN` environment variable. `--api-token <token>` also works but shows the token in the process list of the machine.

## What a Remote Dashboard viewer receives

The publisher sends its project document (groups, datasets, titles, units) and the live values the dashboard displays. It does not send raw frames. On the viewer, the console is empty, and CSV/MDF4 export, the MQTT publisher, and the Historian record nothing; record on the publisher. If the publisher's project changes while attached, the viewer picks up the new layout.

FFT Plot, Waterfall, and 3D Plot are unavailable on a viewer: they need every raw sample, and the link carries only displayed values. Those widgets show "Not available on a Remote Dashboard."

The project is sent whole, but the viewer strips the publisher's logic when it loads it (see [Trust model](#trust-model)).

## Trust model

On a home or lab network this is fine; across the internet, tunnel it. The link is not encrypted, so use Remote Dashboard on a trusted network, or through an SSH tunnel or a VPN when it must cross an untrusted one. Keep **Allow External API Connections** off when no remote viewer is expected.

A viewer only displays. A spoofed publisher can make it show a wrong number. It cannot make it run anything. Before the viewer loads the publisher's project it removes the frame parsers, dataset transforms and transform libraries, the Control Loop, the output widgets' transmit functions, every source's connection settings, and the MQTT publisher and InfluxDB blocks. The viewer parses nothing, writes to no device, and opens no connection of its own.

Canvas widgets (user-written drawing scripts) still draw, but their script runs on a draw-only surface that cannot call the API or reach devices, files, or processes. See [Canvas widget](Canvas-Widget.md#canvas-code-on-a-remote-dashboard).

Known exposures:

- Anyone who can capture the traffic can read the telemetry and the project document, including its scripts and connection settings. The token authorizes the connection but does not protect what travels over it.
- A token holder can also issue API commands on the publisher. There is no read-only token.
- Remembered tokens are stored in the viewer's application settings, encrypted under a machine-specific key. This is obfuscation, not a vault: anyone running as the viewer's user can still attach with them. A settings file copied to another machine yields no token.

## See also

- [API Reference](API-Reference.md) — the API server, token handling, and external access.
- [Command Line Interface](Command-Line-Interface.md) — headless and provisioning flags.
- [Operator Deployments](Operator-Deployments.md) — locking down the watching station.
