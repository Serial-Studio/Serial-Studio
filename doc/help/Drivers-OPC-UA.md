# OPC UA Driver (Pro)

A PLC on the bench at 192.168.1.10 publishes `Plant.Line1.Filler.Level_pct`, and you want it on a plot. OPC UA is how PLCs, SCADA servers and gateways publish named, typed values over Ethernet, and this driver is the Serial Studio client for it. If your device prints text over a serial port (an Arduino with `Serial.println`), you want the UART driver instead.

The driver connects to an OPC UA server, so you need one: a PLC, a gateway such as KEPServerEX or Ignition, or the [bundled simulator](#try-it-with-the-simulator). You do not write a parser or script: Serial Studio generates the project for you.

OPC UA is a Pro feature. A Free install is offered the 14-day Pro trial the first time you use a Pro feature (a Yes/No prompt; registering the trial needs an internet connection, and after the trial ends the prompt offers activation or purchase instead). See [Pro vs Free](Pro-vs-Free.md).

## Try it with the simulator

The `OPC UA PLC Simulator` is a Python 3 server that models a bottling line. Its script, `opcua_plc_simulator.py`, lives in the Serial Studio GitHub repository under `examples/OPC UA PLC Simulator/`. Download that folder, or clone the repository. The **Examples** toolbar button fetches example projects, not this script.

1. Install the server library and start the simulator: `pip install asyncua`, then `python "examples/OPC UA PLC Simulator/opcua_plc_simulator.py"`.
2. In the **Device Setup** panel, set the **Connection** selector to **OPC UA**. On a Free install this shows the Pro trial prompt (Yes/No). Keep the endpoint `opc.tcp://127.0.0.1:4840` and press **Discover**.
3. Press **Browse Tags...**. In the dialog that opens, tick the `Plant` folder and press **OK**. Back in the panel, press **Generate Project**.
4. Press **Connect** in the toolbar. The generated dashboard has one group per folder the ticked tags live in. `FaultySensor` is a tag the simulator makes fail on purpose: it holds its last good value while the simulator reports the fault.

The address space mixes string and numeric node ids, engineering units and ranges, a six-element float array, string status tags and a struct-shaped Variable with child Variables.

| Simulator flag | Effect |
|----------------|--------|
| `--user NAME --password PW` | Require username/password login. |
| `--no-subscriptions` | Refuse subscriptions, which forces the polling fallback. |
| `--drop-after N` | Stop publishing after N seconds without closing the socket (the watchdog case). |
| `--secure-only` | Advertise only a secured endpoint. |

The simulator README lists the rest.

## OPC UA overview

An OPC UA server is a tree, a file explorer for variables. Under the **Objects** folder, vendors arrange folders (a plant, a line, a machine) and inside them **variable nodes**: the tags. Every node has a **node id** (`ns=2;s=Plant.Line1.Filler.Level_pct` or `ns=3;i=1017`), a display name, a data type and an access level. The client asks for values by node id and receives typed values with a **status code** (Good, or a Bad reason) and a **source timestamp** from the device's own clock.

The efficient way to get values is a **subscription**: the client tells the server which tags it wants and the server pushes every change. The alternative is a timed **read** of all tags, which Serial Studio uses as a fallback.

### Endpoints and security

A server advertises one **endpoint** per security configuration. Each endpoint pairs a **security policy** (`None`, `Basic256Sha256`, ...) with a **message security mode** (None, Sign, Sign and Encrypt). A client picks one endpoint and opens a secure channel with it.

On a local test server leave the policy at `None`. On a real plant network use `Basic256Sha256` or newer.

Serial Studio bundles its own OPC UA stack, so it needs no system crypto library. Six security policies are supported: `None`, `Basic128Rsa15`, `Basic256`, `Basic256Sha256`, `Aes128_Sha256_RsaOaep` and `Aes256_Sha256_RsaPss`. Each secure policy can be used in **Sign** (messages are authenticated) or **Sign and Encrypt** (authenticated and confidential). `Basic128Rsa15` and `Basic256` are deprecated by the OPC Foundation (SHA-1, RSA-1.5); they are labelled as deprecated wherever they appear and never chosen automatically.

Login is **Anonymous**, **Username / Password**, or an **X.509 certificate** (a digital identity file issued to you). A password is protected only under Sign and Encrypt: the Setup panel warns when it would travel unencrypted (Policy `None` or Mode Sign) and clears the warning under Sign and Encrypt.

Secure policies need a one-time certificate trust step: see [Secure connections](#secure-connections).

## Setup panel settings

The OPC UA driver (Serial Studio Pro) is configured in the **Device Setup** panel: set the **Connection** selector to **OPC UA**. One source is one session with one server; the session subscribes to every selected tag.

| Setting | Default | What it does |
|---------|---------|--------------|
| Endpoint | `opc.tcp://127.0.0.1:4840` | The server address, `opc.tcp://host:port/path`. The **Discover** button next to it fetches the server's endpoint list. |
| Poll Interval (ms) | 100 | The OPC UA publishing interval requested from the server, also the read timer and the frame rate of the source. Clamped to 10-60000 ms. |
| Security | - | Shown only after Discover returns endpoints. One row per endpoint, as `policy / mode / url`. A row is disabled when the server offers no user identity token the selected Authentication can present. |
| Policy | `None` | The policy you prefer. With no row selected, an endpoint matching this Policy and Mode wins; otherwise the most secure non-deprecated endpoint is picked. |
| Mode | `None` (becomes Sign and Encrypt when you pick a Policy) | **Sign** or **Sign and Encrypt**. Hidden while Policy is `None`. |
| Authentication | Anonymous | **Anonymous**, **Username / Password** or **X.509 Certificate**. |

The API command `setPublishingInterval` and the CLI flag `--opcua-interval` set the same value as Poll Interval.

**Endpoint.** Press **Discover** to fetch the server's endpoint list. The most secure endpoint whose user tokens match the selected authentication is picked automatically (never a deprecated policy on its own), and an explicit choice survives the next discovery. Connecting without a selected endpoint runs the same discovery first. Serial Studio always dials the **host and port you typed**, keeping the rest of the discovered description: servers routinely advertise their own hostname (`opc.tcp://PLC-01:4840`), which may be a name your laptop's DNS does not know.

**Poll Interval.** Servers revise the interval upward when it is below their minimum (PLC-embedded servers commonly floor it at 50-100 ms). The status line under the pane reports the rate in force, not the one requested.

**Authentication.** The password is stored in the same per-machine credential vault the MQTT driver uses, obfuscated in the machine's settings and keyed by host and port; it is never written to a project file. For certificate login, pick the certificate and its private key: only their **paths** are saved, never the key itself.

### Selecting tags

**Browse Tags...** opens a browse-only session and fetches exactly one level per expansion: one Browse plus one batched Read for the whole level. Nothing below a node is read until you expand it, so a gateway with a hundred thousand nodes stays responsive. Variables expand too, because PLC structs (UDTs) expose their members, and their `EngineeringUnits` and `EURange` (the min/max range the PLC declares) properties, as child Variables.

Ticking a folder marks it: every readable supported variable under it is selected, including ones fetched later when you expand it. **Select All Readable** ticks everything fetched so far. **OK** commits the selection; **Cancel** and the window's close button discard it. Tags in branches you never expanded keep their place. An edit made while a session is connected is applied when the link closes.

Up to 2048 channels can be selected; the dialog warns above 512 because a very wide frame slows the dashboard. A frame is capped at 64 KiB, so many String channels can reach the limit before 2048.

Supported tag types: Boolean, SByte, Byte, Int16, UInt16, Int32, UInt32, Int64, UInt64, Float, Double, String, and one-dimensional arrays of those. The common namespace-0 subtypes resolve too (`Duration`, `UtcTime`, `Enumeration`, `IntegerId`, `DateTime`, `Guid`, `ByteString`, `LocalizedText`); when a vendor declares a type the table does not know, the value's own type decides, and anything printable becomes a string channel. Strings are capped at 256 bytes of UTF-8, truncated on a character boundary. Variables of any other type show in the browser but cannot be ticked.

Press **Generate Project** to build a project from the selection ([Generated project](#generated-project)).

### Subscription and polling

On connect the driver asks the server for one monitored item (a server-side watch on one tag) per tag and reports progress as **Subscribing, n of N tags**. The status line then settles on one of three texts:

| Status | Meaning |
|--------|---------|
| `Subscribed, N tags, X Hz` | Every tag is subscribed. X is the rate in force. |
| `Subscribed N tags, polling M refused, X Hz` | The server refused M individual tags (an item cap, per-user access, a node id gone stale after a PLC download). Each is logged with its node id and moves to timed reads; the rest stay subscribed. |
| `Polling (server refused subscriptions), N tags, X Hz` | The server refused every item (no subscription support, or its quota is exhausted), so the driver reads all tags on a timer. |

Timed reads are batched to the server's `MaxNodesPerRead`, one read at a time. A tick that arrives while the previous read is still open is skipped and counted.

A subscription can go silent without any error when a server reloads its project but leaves the session up. After silence longer than six revised publishing intervals (3 s at minimum), the driver falls back to polling instead of freezing the dashboard.

Each publishing tick, the driver publishes the tags whose value changed since the previous tick as one frame, and the generated frame parser keeps the last value of the rest. An array tag fans out into one channel per element.

Value quality follows the OPC UA severity bits: Good and Uncertain values (a gateway serving its last usable value, a device in warm-up) are published, Bad values are dropped. A tag whose newest value is Bad keeps its last good value on the dashboard, increments the bad-status counter and appears in the API status `badTags` list.

### Timestamps

Values keep the PLC's own timestamp, so recordings and CSV exports follow the device clock, even a PLC without NTP. A stamp more than 5 s away from the offset sampled at connect, or a missing one, falls back to the receipt time and counts as unstamped. A frame stamp never goes backwards.

### Generated project

**Generate Project** writes a project with:

- One source of type OPC UA carrying the endpoint, authentication mode, username, interval and the tag list, so reopening the project reconnects without browsing again.
- One group per folder the selected tags live in, titled after the folder.
- One dataset per channel: the tag's display name as the title (`name[i]` for array elements), booleans become lights in an LED Panel group placed after the group, plotting enabled for numeric types, string tags routed to the data grid. `EngineeringUnits` becomes the dataset unit and `EURange` its widget and plot range, read from the ticked tags' properties.
- A Built-In frame parser using the **OPC UA tag frames** template. The template's schema parameter lists each channel's wire type and is regenerated with the project; edit the tag selection and generate again rather than editing the schema by hand.

The project opens in the Project Editor for customization, like any other generated project. The headless API command `io.opcua.generateProject` performs the same generation without a save dialog.

### Connection lifecycle

Connecting is asynchronous. The driver reports exactly one outcome per attempt: the session becomes connected, or the attempt fails with a reason (unreachable, access denied, unsupported authentication, rejected endpoint), at the latest after a 15 s deadline. A session that drops after it was established is reported through the normal disconnect path; pressing connect again resubscribes to the same tags.

## Command line

The command-line flags need Serial Studio Pro, like the Setup panel. A minimal run:

```bash
SerialStudio --opcua opc.tcp://192.168.1.10:4840/ \
  --opcua-tag "ns=2;s=Plant.Line1.Filler.Level_pct:f64:Level" \
  --opcua-tag "ns=2;s=Plant.Line1.Filler.Running:bool:Running" \
  --opcua-interval 100
```

| Flag | Effect |
|------|--------|
| `--opcua URL` | The endpoint to connect to. |
| `--opcua-tag SPEC` | One tag; repeat for more. |
| `--opcua-interval MS` | Publishing interval, 10-60000 ms. An invalid value is rejected with a warning. |
| `--opcua-user NAME`, `--opcua-pass PW` | Username/password login. Without `--opcua-user` the login is Anonymous; certificate login is Setup panel and API only. |
| `--project FILE` | Use this project. Without it, Serial Studio generates a project from the tag list so the frames decode. |

`--opcua-tag` takes `nodeId[:type[:name]][:n=N][:unit=U]`. The node id may itself contain colons: the suffixes are parsed from the right. `n` gives an array length and `unit` a dataset unit. The type code is one of `bool`, `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`, `f32`, `f64` (the default) or `str`.

## API

The Socket API exposes the driver under `io.opcua.*`.

- **Connection:** `getConfig`, `setEndpointUrl`, `discoverEndpoints`, `listEndpoints`, `setEndpointIndex`, `setPublishingInterval`, `getStatus`.
- **Security and login:** `setSecurityPolicy`, `setSecurityMode`, `setAuthMode`, `setIdentityType` (an alias of `setAuthMode`), `setUsername`, `setPassword`, `setUserCertificate`.
- **Client certificate:** `getCertificate`, `regenerateCertificate`, `exportCertificate`.
- **Trust:** `listTrusted`, `trustServer`, `revokeTrust`.
- **Tags:** `startBrowse`, `browse`, `stopBrowse`, `listTags`, `setTags`, `addTag`, `removeTag`, `clearTags`, `generateProject`.

Discovery and browsing are asynchronous: the command starts the request and the matching list command is polled for the result. `getStatus` returns the session state and the pulled counters: values received, bad-status count and the node ids currently bad, unstamped values, frames published, link drops, refused tags, skipped polls and the revised publishing interval. Each command, with its parameters, is listed under [OPC UA Driver Commands](API-Reference.md#opc-ua-driver-commands---pro-29).

## Secure connections

In the OPC UA driver (Serial Studio Pro), a secure policy needs the server to be trusted once, and the server to trust Serial Studio once.

### Connecting for the first time

1. Choose a **Policy** other than `None` and leave **Mode** on Sign and Encrypt (picking a policy sets it).
2. Press **Connect** in the toolbar. The first attempt stops and a trust prompt appears, because Serial Studio has never seen this server's certificate. The prompt shows the certificate's subject, issuer, validity window and **SHA-256 fingerprint**. Compare the fingerprint with the one the server operator gave you, then press **Trust This Server**.
3. Press **Connect** again. Accepting a certificate records the decision but does not reconnect for you.

The decision is stored per installation, keyed by fingerprint. A server that is re-keyed presents a different fingerprint and asks again, which is the point: a silent inheritance would defeat the check.

### Letting the server trust you

A secure channel is mutual. On the first secure connection Serial Studio generates its own client certificate and keeps it, so a server operator has to trust this installation once rather than on every launch. With a Policy other than `None`, an **Identity** row appears in the OPC UA section of the Device Setup panel:

- **Export…** is enabled once the first secure connection has generated the certificate. It writes the certificate where the server expects trusted clients (`pki/trusted/certs` on many servers). The private key is never exported.
- **Replace** generates a new certificate and key. Every server that trusted the old one has to trust the new one, so use it only when the key may have been exposed.

### Why a secure connection was refused

The four causes are reported separately, because they have four different fixes:

| Reason | What to do |
|--------|------------|
| The server certificate is not trusted | Compare the fingerprint and accept it in the prompt. |
| The server certificate has expired | Renew it on the server. Serial Studio will not accept an expired certificate. |
| The server certificate is not valid yet | Check the clock on the server and on this machine. |
| The server certificate was not issued for this host | You are dialing a name the certificate does not cover. Use the name it was issued for, or reissue it with the right SubjectAltName. |

## Common pitfalls

| Symptom | Cause | Fix |
|---------|-------|-----|
| Discover fails or hangs | Wrong URL or port, or the server is not running. | Check the URL the server prints on start, and test the port from this machine. |
| The **Security** rows are greyed out | None of the server's endpoints advertises a user identity token the selected Authentication can present (a server offering Anonymous only rejects a username session). | Switch the authentication mode, or enable a matching endpoint on the server. |
| "Access denied" | Anonymous login is disabled on the server, or the username/password is wrong. | Switch the authentication mode and check the account. If browsing fails after login, grant the account read access to the folders. |
| Discovery works, connect times out | A firewall blocks the port, or the server binds an interface this machine cannot reach. Serial Studio dials the host you typed, so the hostname the server advertises does not matter. | Check the firewall for the port in the Endpoint field (4840 by default) and the server's bind address. Connect fails after 15 s at the latest. |
| Everything polls instead of subscribing | The server hit its subscription limit. A previous session that was not closed cleanly holds its subscriptions until the server times it out. | Wait for the timeout, or raise the limit on the server. |
| A tag is visible but cannot be ticked | Its data type is not supported (a structure as a whole, a multi-dimensional array), or the account has no read access. | Expand a struct and tick its members. |
| Values update slower than the interval | The server revised the publishing interval upward; asking politely does not change its floor. | Read the rate in the status line, then ask the server for a faster minimum or accept its floor. |

## Further reading

- [OPC UA (Wikipedia)](https://en.wikipedia.org/wiki/OPC_Unified_Architecture)
- [OPC Foundation: Unified Architecture](https://opcfoundation.org/about/opc-technologies/opc-ua/)

## See also

- [Auto-Generating Projects](Auto-Generating-Projects.md): the other driver-side project generators.
- [Drivers: Modbus](Drivers-Modbus.md): the register-oriented industrial protocol.
- [Drivers: MQTT](Drivers-MQTT.md): broker-based telemetry; many gateways bridge OPC UA to MQTT.
- [Devices](Devices.md): driver capability summary across all transports.
- [Communication Protocols](Communication-Protocols.md): overview of all supported transports.
