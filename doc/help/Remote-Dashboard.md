# Remote Dashboard

Remote Dashboard lets one Serial Studio instance watch another instance's dashboard over the
network. The instance connected to the hardware (the *publisher*) keeps parsing, logging, and
displaying as usual; the watching instance (the *viewer*) attaches to it and renders the same
groups, datasets, and live values. The view is read-only: nothing done on the viewer reaches
the remote device, and the viewer's own project on disk is never modified.

The link rides the same TCP API server used by scripts and external tools (port 7777), so
there is nothing extra to install. It is available in every edition.

Typical uses:

- A test stand in another room, watched from a desk.
- One operator machine wired to the hardware, several read-only observers.
- A headless data logger on an embedded PC, checked from a laptop on the same network.

## Publisher: allow incoming viewers

1. Open **Preferences** and enable **Enable API Server (Port 7777)** in the **API & Plugins**
   group of the **General** tab.
2. For viewers on the *same machine*, that is all: local connections are pre-authorized and
   need no token.
3. For viewers on *another machine*, also enable **Allow External API Connections** and copy
   the **API Access Token** shown below it. The refresh button generates a new token and
   invalidates the old one.

On a headless publisher the same provisioning is done from the command line:

```
serial-studio --headless --api-server --api-external --api-token <token>
```

`--api-token` expects at least 32 hexadecimal characters. `--api-external` is the
non-interactive equivalent of the "Allow External API Connections" confirmation: once set,
any machine that can reach port 7777 may connect after presenting the token.

## Viewer: attach

1. Run the **Remote Dashboard** command from the [command palette](Command-Palette.md). The
   **Attach to Remote Dashboard** dialog opens.
2. Fill in the connection fields:

| Field | Meaning | Default |
|-------|---------|---------|
| Recent | Previously used `host:port` endpoints; selecting one fills the fields below, including the token that was used with it | shown once an endpoint has been used |
| Host | Host name or IP address of the publisher | `127.0.0.1` |
| Port | The publisher's API server port | `7777` |
| Token | The publisher's API access token; required only for connections from another machine | empty |

3. Press **Attach**. The status box reports the link state:
   - *Attached … live, N datasets at N Hz* — values are flowing.
   - *Attached … connected, the remote is not producing data* — the link is up but the
     publisher's device is idle or disconnected.
   - *Attached … no response, the link may be down* — heartbeats stopped; the viewer keeps
     the last values and recovers on its own when the link returns.

**Detach** restores whatever the viewer was doing before the attach — its own project,
operation mode, and connection state come back exactly as they were.

A viewer cannot attach while its own device connection or a file recording is active; the
dialog says so and the **Attach** button stays disabled until the local stream is closed.

The update rate is automatic: the viewer receives values as fast as the publisher's own
display refreshes, never faster, so there is nothing to tune. Endpoints and their tokens are
remembered in the viewer's application settings; the dialog reopens with the last used
publisher filled in.

## What travels, what does not

The publisher sends the dashboard structure (groups, datasets, titles, units) and the live
values the dashboard displays. It does not send raw frames: the viewer's console stays empty,
and the viewer's CSV/MDF4 export, MQTT publisher, and Historian never see mirrored
data — recording stays the publisher's job. If the remote project changes while attached, the
viewer picks up the new layout automatically.

## Trust model

The v1 link has three limits to understand before opening it up:

- **The transport is not encrypted.** The token authorizes the connection but does not
  protect what travels over it. Anyone who can capture the traffic can read the telemetry.
- **The token is the full API credential, not a viewer-only pass.** A holder can also issue
  API commands on the publisher, exactly as any API client can. There is no read-only token.
- **Local connections skip the token.** Any process on the publisher's machine may attach.
- **Remembered tokens are stored as plain text.** A token saved with a recent endpoint sits in
  the viewer's application settings with the same protection as any other setting; anyone with
  access to the viewer's user profile can read it.

Use Remote Dashboard on a trusted network, or wrap the connection in a tunnel (SSH port
forwarding or a VPN) when it must cross an untrusted one. Keep **Allow External API
Connections** off when no remote viewer is expected.

The number of simultaneous viewers is capped at the API server's client limit (32 connections)
by default; the `API/MaxViewers` entry in the application settings lowers it (a value of `0`
refuses viewers while leaving the rest of the API available).

## See also

- [API Reference](API-Reference.md) — the API server, token handling, and external access.
- [Command Line Interface](Command-Line-Interface.md) — headless and provisioning flags.
- [Operator Deployments](Operator-Deployments.md) — locking down the watching station.
