# Aural Alerts

A temperature crosses 80 while you are looking at another window. With **Enable Sounds** on, Serial Studio plays a tone every 5 seconds until you press **Ctrl+Shift+A**.

To hear that alarm:

1. Turn on **Preferences > Sounds > Enable Sounds**. It is off by default, so nothing plays until you do.
2. In the Project Editor, select a temperature dataset, click **Alarm Bands** in the ribbon and add a Critical band above 80 (see [Alarm bands](Widget-Reference.md#alarm-bands)). Show the dataset in a Bar, Gauge or Meter widget. Other eligible widgets are listed in [If an alarm stays silent](#if-an-alarm-stays-silent).
3. Drive the value past 80. The Warning tone plays every 5 s until you acknowledge it.

To check the volume and output device first, press **Test** in **Preferences > Sounds**. Test creates no alarm point, so it does not prove a band works; only a value crossing the band does.

Sounds for alarm bands need no Pro license. Notifications posted from scripts or the API are a Pro feature, so their alarm points need Pro or an active trial (see [Notifications](Notifications.md) and [Pro vs Free](Pro-vs-Free.md)).

## Priorities

A dataset that crosses into a Warning or Critical band, or a Warning or Critical notification, becomes an **alarm point**: one entry in the alarm list. Bands are named Info, OK, Warning and Critical. Sounds are named by priority, which is one step lower: a Critical band raises a Warning priority point, and a Warning band raises a Caution priority point.

| Priority | Fed by | Bundled sound | Repeats |
|----------|--------|---------------|---------|
| Warning  | Critical alarm band (severity 3), Critical notification (Pro) | One sustained 2.4 s tone | Default 5 s, adjustable 2.5 s to 15 s in Preferences > Sounds, until acknowledged, silenced or the condition clears |
| Caution  | Warning alarm band (severity 2), Warning notification (Pro) | Three-pulse burst | Default 10 s, adjustable 2.5 s to 30 s in Preferences > Sounds, until acknowledged, silenced or the condition clears |
| Advisory | Info notification (Pro) | Single pulse | Once, only while nothing else sounds |

### If an alarm stays silent

A band raises an alarm point only when all of these hold:

1. **Enable Sounds** is on and [Mute](#the-master-annunciator) is off.
2. The dataset has a Bar, Gauge or Meter widget with **Hide on Dashboard** off, or belongs to a Bar Panel or LED Panel group. Bands on any other dataset stay in the project file but raise no alarm point and post no notification.
3. **Suppress Alarms** (Project Editor, the dataset's Widget Settings) is off. With it on, the bands stay drawn but never raise a point, visible or not.
4. **Alarms only while visible** (also in the dataset's Widget Settings) is off, or a widget showing the dataset is on screen. Revealing the widget while the value sits in a band counts as entering the band.

See [Alarm bands](Widget-Reference.md#alarm-bands) for the dataset settings.

An **Alarm Deadband** on the dataset (default 0, in the dataset's units) holds the active band until the value has left it by that margin, so a reading hovering on a threshold raises one point rather than one per frame. A step into a more severe band is never held back.

Info and OK alarm bands never alert. The `Problems` and `System` notification channels never raise alarm points.

## Alarm points and sequences

The sequences are the ISA-18.1 annunciator sequences, and the priority names and tone patterns are modelled on IEC 60601-1-8. Serial Studio is not certified to either standard.

The default, **A-4**, suits most projects: a point appears when the value enters a band, sounds until you acknowledge it, and disappears when the value leaves. Read on only if you need alarms to wait for a manual Reset. The sequence is set in **Preferences > Sounds > Sequence (ISA-18.1)**, and a project may override it.

An alarm point is one dataset's alarm-band state, or one notification identified by its channel and title. Each point is in one of four states:

| State | Meaning | Bell | Sound |
|-------|---------|------|-------|
| **Normal** | No active condition. | Idle | Silent |
| **Alert** | Active, unacknowledged. | Flashes | The burst repeats |
| **Acknowledged** | Active, steady. | Steady | Silent |
| **Return-to-normal** | The condition cleared but the point waits for Reset. | Steady | Silent, or ringback in sequence R |

The point table holds at most 512 points; a new point raised past that limit is not added.

A sequence has a letter (A, M or R: what happens after Acknowledge) and an optional **-4** suffix (no lock-in: what happens if the condition clears before Acknowledge). The letter decides what happens when a condition returns to normal after Acknowledge:

| Sequence | Return to normal after Acknowledge |
|----------|------------------------------------|
| **A** (automatic reset) | The point disappears. |
| **M** (manual reset) | The point stays in Return-to-normal, steady and silent, until Reset. |
| **R** (ringback) | As M, and a distinct two-pulse ringback sounds at the Caution interval until Reset or Silence. |

The form decides what happens when the condition clears *before* anyone acknowledged:

| Form | Momentary alarm (clears before Acknowledge) |
|------|---------------------------------------------|
| **A-4**, **M-4**, **R-4** (ISA-18.1 option 4, no lock-in) | The audible stops the moment the value leaves the band, like an oil-pressure warning that goes quiet once the engine is running. |
| **A**, **M**, **R** (plain, lock-in) | The point keeps sounding and flashing until acknowledged, so a dip that lasted two seconds is never missed. The sequence's after-Acknowledge rule then applies. |

After a momentary alarm, A-4 drops the point; M-4 and R-4 park it in Return-to-normal until Reset.

A notification point clears when a `Resolved: <title>` Info event arrives on the same channel, which is what `notifyClear()` and `notifications.resolve` post.

A project overrides the app preference in **Project Editor > Project Summary > Settings > Alarm Sequence**. In the project file and the API it is the code string: `A-4`, `A`, `M-4`, `M`, `R-4` or `R`.

## The master annunciator

The bell at the right end of the dashboard taskbar tray is the master annunciator. Clicking the bell opens the alarm panel: the active points, newest first (click one to jump to its widget), and the Acknowledge, Silence, Reset, Clear, Test and Mute actions. Right-clicking the bell acknowledges.

While any point is active the icon takes the colour of the highest active priority and shows the number of unacknowledged points. It flashes fast while anything is unacknowledged and slowly while a ringback is pending. With **Preferences > Startup > Reduce Motion** on, it holds a steady colour instead.

| Action | What it does | Shortcut |
|--------|--------------|----------|
| Acknowledge | Every alerting point becomes Acknowledged; the audible and the flash stop. | Ctrl+Shift+A |
| Silence | The audible stops; points stay unacknowledged and keep flashing. A new point, or a point rising in priority, re-sounds (reflash). | Ctrl+Shift+H |
| Reset | Return-to-normal points go Normal (sequences M and R). | Ctrl+Shift+R |
| Clear | Drops every point from the list, acknowledged and alerting ones included. A point still inside its band comes back on the next transition. | Panel only |
| Test | Plays Advisory, Caution, Warning and Ringback in turn and walks the annunciator through each priority. No point is created. | Ctrl+Shift+T |
| Mute | Silences every alarm and event sound without touching the point table. Persists across restarts and shows on the bell. Toggled from the alarm panel or with the shortcut; it is not in Preferences, and it is separate from **Enable Sounds**. | Ctrl+Shift+U |

At most one alarm sound plays at a time: the highest priority among unsilenced alerting points. When that priority is fully acknowledged or silenced, the next lower alerting priority takes over. Ringback plays only when nothing is in Alert.

Disconnecting on purpose returns every point to Normal without ringback, and the Disconnected event sound plays instead.

An **unrequested** link drop stops the audible for the outage but holds the point table. Acknowledged points stay acknowledged. When the link recovers, a dataset still in the same alarm band picks its point back up: an unacknowledged one resumes sounding, an acknowledged one stays quiet. Unacknowledged notification points are dropped at the loss, since the script that raised them cannot clear them across the outage.

Opening a project or starting the application never sounds.

## Preferences > Sounds

| Setting | Default | Notes |
|---------|---------|-------|
| **Enable Sounds** | Off | Nothing plays until it is on. |
| **Volume** | 80 | 0 to 100. |
| **Output Device** | System default | Any output the OS lists. |
| **Sequence (ISA-18.1)** | A-4 | A-4, A, M-4, M, R-4 or R. |
| Warning repeat interval | 5 s | 2.5 s to 15 s. |
| Caution repeat interval | 10 s | 2.5 s to 30 s. |
| Button Pressed, Toggle Changed | Off | UI feedback event sounds. |

### Missing output device

If the selected output device disappears, sounds continue on the system default, the Problem Center reports the lost device, and output returns to it when it comes back.

### Silent alarms hint

While sounds are disabled and the loaded project defines alarm sounds or alarm bands that can alert, the taskbar bell dims and the Problem Center says the alarms will be silent.

### Alarm sound files

Each alarm signal (Warning, Caution, Advisory, Ringback) has a row with its WAV file (empty means the bundled sound), a browse button, a play button and a clear button.

**Test** plays the same sequence as the alarm panel Test action.

**Reset to Bundled Sounds** discards every saved sound preference (files, intervals, volume, switches) and returns the output device to the system default.

Any PCM WAV file works: 8, 16, 24 or 32-bit integer or 32-bit float, mono or stereo, 8 kHz to 96 kHz, at most 10 s. A file that fails to decode is refused with the reason and the slot keeps its previous value. Every configured sound is decoded when it is picked or when the application starts.

### Event sounds

Each application event has a row with an enable switch: Connected, Disconnected, Link Lost, Reconnected, Export Finished, Recording Started, Recording Stopped, Error Dialog Shown, Button Pressed, Toggle Changed. Toggle Changed covers switches, checkboxes, combo boxes and spin boxes. Button Pressed covers buttons and tabs.

Event sounds play on their own lane, are lowered by 12 dB while an alarm sounds, and never delay an alarm burst. Changing a preference plays only the two off-by-default feedback events.

## Project overrides

Two overrides travel with the project file:

- **Per alarm band.** In the Project Editor, select the dataset and click **Alarm Bands**; the editor has a **Sound** column: pick a WAV to play instead of the priority default when the value enters that band. In the project file this is the band's `sound` key (see [Alarm bands](Widget-Reference.md#alarm-bands)).
- **Per notification channel.** **Project Summary > Settings > Channel Sounds** maps a channel name to up to three files, one per priority. Stored under the project's `sounds` object together with the sequence override:

```json
"sounds": {
  "sequence": "M",
  "channels": {
    "Engine": { "warning": "sounds/horn.wav", "caution": "sounds/chime.wav" }
  }
}
```

A relative path resolves against the project file's folder, so a project folder can be shared with its sounds. A file that no longer exists falls back to the bundled sound and raises a Problem Center warning naming the path.

## API

These commands need the API Server, which is off by default (port 7777); see [API Reference](API-Reference.md).

| Command | Purpose |
|---------|---------|
| `alarms.state` | Sequence, mute, enable, highest priority, unacknowledged count, the sounding burst (`priority`, `file`, `burstStartedMs`, `burstCount`) and every active point (`kind`, `title`, `channel`, `priority`, `state`, `silenced`, `sinceMs`). |
| `alarms.acknowledge`, `alarms.silence`, `alarms.reset`, `alarms.clear`, `alarms.test` | The operator actions. |
| `alarms.setMuted` | Takes `muted` (boolean). |
| `alarms.getProjectSounds`, `alarms.setProjectSounds` | Read or replace the project's `sounds` object; the setter validates every path and returns the rejected ones. |

In a `--headless` session the alarm state machine still runs without an audio device.

## See also

- [Notifications](Notifications.md): the events that feed notification points.
- [Widget Reference](Widget-Reference.md#alarm-bands): alarm bands and the per-band `sound` key.
- [Problem Center](Problem-Center.md): where lost devices and unusable sound files are reported.
