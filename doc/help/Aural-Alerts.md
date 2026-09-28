# Aural Alerts

Serial Studio can sound its alarms. A dataset that crosses into a Warning or Critical alarm band, or a Warning or Critical notification posted from a script or the API, becomes an **alarm point** that is heard first and read second. The behaviour follows two published standards rather than an ad-hoc scheme:

- **ISA-18.1** (Annunciator Sequences and Specifications) for the state machine, its sequences A, M and R with or without option 4 (no lock-in), and the four operator actions: Acknowledge, Silence, Reset and Test (Serial Studio adds Clear).
- **IEC 60601-1-8** for the sound signatures: a pulse burst per priority, a fundamental between 150 Hz and 1000 Hz with at least four harmonics, and a repeat interval per priority.

Priorities use the flight-deck vocabulary: **Warning**, **Caution** and **Advisory**. Aural alerts are available in every build.

## Priorities

| Priority | Fed by | Bundled sound | Repeats |
|----------|--------|---------------|---------|
| Warning  | Critical alarm band (severity 3), Critical notification | One sustained 2.4 s tone (car-cluster buzzer) | Every 5 s (2.5 s to 15 s) until acknowledged, silenced or the condition clears |
| Caution  | Warning alarm band (severity 2), Warning notification | Three-pulse burst | Every 10 s (2.5 s to 30 s) until acknowledged, silenced or the condition clears |
| Advisory | Info notification | Single pulse | Once, only while nothing else sounds |

Info and OK alarm bands never alert. The `Problems` and `System` notification channels are the application's own diagnostics and never become alarm points.

## Alarm points and sequences

An alarm point is one dataset's alarm-band state, or one notification identified by its channel and title. Each point is in one of four states: **Normal**, **Alert** (active, unacknowledged: the annunciator flashes and the burst repeats), **Acknowledged** (active, steady, silent) or **Return-to-normal** (the condition cleared but the point waits for Reset).

The sequence decides what happens when a condition returns to normal:

| Sequence | Return to normal after Acknowledge |
|----------|------------------------------------|
| **A** (automatic reset) | The point disappears. |
| **M** (manual reset) | The point stays in Return-to-normal, steady and silent, until Reset. |
| **R** (ringback) | As M, and a distinct two-pulse ringback sounds at the Caution interval until Reset or Silence. |

Each sequence comes in two forms, and the form decides what happens when the condition clears *before* anyone acknowledged:

| Form | Momentary alarm (clears before Acknowledge) |
|------|---------------------------------------------|
| **A-4**, **M-4**, **R-4** (ISA-18.1 option 4, no lock-in; **A-4** is the default) | The audible stops the moment the value leaves the band, like an oil-pressure warning that goes quiet once the engine is running. A-4 drops the point; M-4 and R-4 park it in Return-to-normal until Reset. |
| **A**, **M**, **R** (plain, lock-in) | The point keeps sounding and flashing until acknowledged, so a dip that lasted two seconds is never missed. The sequence's after-Acknowledge rule then applies. |

A notification point clears when a `Resolved: <title>` Info event arrives on the same channel, which is what `notifyClear()` and `notifications.resolve` post.

The sequence is an app preference (**Preferences > Sounds**) that a project may override (**Project Editor > Project Summary > Settings > Alarm Sequence**). In the project file and the API it is the code string: `A-4`, `A`, `M-4`, `M`, `R-4` or `R`.

## The master annunciator

A bell sits at the right end of the dashboard taskbar tray at all times. While any point is active its icon takes the colour of the highest active priority and shows the number of unacknowledged points; it flashes fast while anything is unacknowledged, slowly while a ringback is pending, and holds a steady colour when **Reduce Motion** is on. Clicking the bell opens the alarm panel: the active points, newest first (click one to jump to its widget), and the Acknowledge, Silence, Reset, Clear, Test and Mute actions. Right-clicking the bell acknowledges.

| Action | What it does | Shortcut |
|--------|--------------|----------|
| Acknowledge | Every alerting point becomes Acknowledged; the audible and the flash stop. | Ctrl+Shift+A |
| Silence | The audible stops; points stay unacknowledged and keep flashing. A new point, or a point rising in priority, re-sounds (reflash). | Ctrl+Shift+H |
| Reset | Return-to-normal points go Normal (sequences M and R). | Ctrl+Shift+R |
| Clear | Drops every point from the list, acknowledged and alerting ones included. A point still inside its band comes back on the next transition. | Panel only |
| Test | Plays Advisory, Caution, Warning and Ringback in turn and walks the annunciator through each priority. No point is created. | Ctrl+Shift+T |
| Mute | Silences every alarm and event sound without touching the point table. Persists across restarts and shows on the annunciator. | Ctrl+Shift+U |

At most one alarm sound plays at a time: the highest priority among unsilenced alerting points. When that priority is fully acknowledged or silenced, the next lower alerting priority takes over. Ringback plays only when nothing is in Alert.

Disconnecting returns every point to Normal without ringback; the Link Lost or Disconnected event sound plays instead. Opening a project or starting the application never sounds, and changing a preference only plays the (off-by-default) UI feedback events.

## Preferences > Sounds

- **Enable Sounds** (off by default; nothing plays until it is turned on), **Mute**, **Volume**, **Output Device** (system default or any output the OS lists). If the selected device disappears, sounds continue on the system default, the Problem Center reports the lost device, and output returns to it when it comes back.
- **Sequence (ISA-18.1)**: A-4 (default), A, M-4, M, R-4 or R.
- One row per alarm signal (Warning, Caution, Advisory, Ringback): the WAV file (empty means the bundled sound), a browse button, a play button and a clear button. Warning and Caution also carry their repeat interval.
- One row per application event, each with an enable switch: Connected, Disconnected, Link Lost, Reconnected, Export Finished, Recording Started, Recording Stopped, Error Dialog Shown, Button Pressed, Toggle Changed. Toggle Changed covers switches, radio buttons, checkboxes and combo box selections; Button Pressed covers buttons and the Preferences tabs. Button Pressed and Toggle Changed are off by default. Event sounds play on their own lane, are lowered by 12 dB while an alarm sounds, and never delay an alarm burst.
- **Test** and **Reset to Bundled Sounds**.

Any PCM WAV file works: 8, 16, 24 or 32-bit integer or 32-bit float, mono or stereo, 8 kHz to 96 kHz, at most 10 s. A file that fails to decode is refused with the reason and the slot keeps its previous value. Every configured sound is decoded when it is picked or when the application starts, so playing one performs no file access.

## Project overrides

Two overrides travel with the project file:

- **Per alarm band.** The Alarm Bands editor has a **Sound** column: pick a WAV to play instead of the priority default when the value enters that band. In the project file this is the band's `sound` key (see [Alarm bands](Widget-Reference.md#alarm-bands)).
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

| Command | Purpose |
|---------|---------|
| `alarms.state` | Sequence, mute, enable, highest priority, unacknowledged count, the sounding burst (`priority`, `file`, `burstStartedMs`, `burstCount`) and every active point (`kind`, `title`, `channel`, `priority`, `state`, `silenced`, `sinceMs`). |
| `alarms.acknowledge`, `alarms.silence`, `alarms.reset`, `alarms.clear`, `alarms.test` | The operator actions. |
| `alarms.setMuted` | `muted: true|false`. |
| `alarms.getProjectSounds`, `alarms.setProjectSounds` | Read or replace the project's `sounds` object; the setter validates every path and returns the rejected ones. |

A headless session (`--headless`) runs the state machine and answers these commands without an audio device.

## See also

- [Notifications](Notifications.md): the events that feed notification points.
- [Widget Reference](Widget-Reference.md#alarm-bands): alarm bands and the per-band `sound` key.
- [Problem Center](Problem-Center.md): where lost devices and unusable sound files are reported.
