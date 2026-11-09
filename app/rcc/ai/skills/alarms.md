# Alarms and Aural Alerts

A dataset entering a Warning or Critical alarm band, or a Warning or
Critical notification, becomes an **alarm point**. Points follow an
ISA-18.1 annunciator sequence and sound bursts modelled on IEC 60601-1-8.
Aural alerts are in every build. The bell at the right of the dashboard
taskbar is the annunciator. Full reference:
`meta.fetchHelp{path:"Aural-Alerts"}`. To add or edit bands, load
`dashboard_layout`, which owns `alarmBands`, `suppressAlarms`,
`alarmsWhileVisible` and `alarmDeadband`.

## Two vocabularies

Bands and notifications use severity words, the annunciator uses
flight-deck words, and the same word means different things:

| Source                               | Annunciator priority |
|--------------------------------------|----------------------|
| Critical band, Critical notification | Warning              |
| Warning band, Warning notification   | Caution              |
| Info notification                    | Advisory             |

Info and OK bands never alert. The `Problems` and `System` notification
channels never become points. Say which vocabulary you mean.

## Operator actions

| Command              | Tier          | Effect |
|----------------------|---------------|--------|
| `alarms.state`       | safe          | Sequence, mute, highest priority, unacknowledged count, the sounding burst, every active point. Read this first. |
| `alarms.acknowledge` | confirm       | Every alerting point becomes Acknowledged; sound and flash stop. |
| `alarms.silence`     | confirm       | Sound stops, points stay unacknowledged and keep flashing. A new point or a rise in priority sounds again. |
| `alarms.reset`       | confirm       | Return-to-normal points go Normal (sequences M and R). |
| `alarms.clear`       | alwaysConfirm | Drops every point. One still inside its band returns on the next transition. |
| `alarms.setMuted`    | confirm       | Silences every alarm and event sound, persists across restarts, leaves the points alone. |
| `alarms.test`        | confirm       | Plays each sound and walks the annunciator. Creates no point. |

These change what an operator sees during a live alarm. Only run one when
the user asks for that action, and never to tidy up after your own work.

## Why an alarm stays silent

Work down this list, starting from `alarms.state`:

1. **Enable Sounds is off by default** (Preferences > Sounds). Nothing plays
   until it is on, and the bell dims while a project with alarms would be
   silent. This is the most common cause.
2. The annunciator is muted, or the point was silenced or acknowledged.
3. The dataset has no widget that draws its bands. Bands alert only with a
   Bar, Gauge or Meter widget and **Hide on Dashboard** off, or in a Bar
   Panel or LED Panel group. A plot-only dataset stores bands and ignores
   them.
4. `suppressAlarms` is on, or `alarmsWhileVisible` is on and no widget
   showing the dataset is on screen.
5. The band is Info or OK.
6. The value already left the band. Under the default sequence a momentary
   alarm stops as soon as it clears.
7. An `alarmDeadband` is holding the active band. A step into a more severe
   band is never held back.

Opening a project or starting the application never sounds. Disconnecting
on purpose returns every point to Normal. An unrequested link drop stops
the sound but holds the points, and a dataset still in its band resumes
when the link recovers.

## Sequences

Set in Preferences > Sounds, and a project may override it (Project Summary
> Settings > Alarm Sequence, or `sounds.sequence`). Codes: `A-4` (default),
`A`, `M-4`, `M`, `R-4`, `R`.

| Sequence | After Acknowledge, when the condition clears |
|----------|----------------------------------------------|
| A        | The point disappears. |
| M        | It stays in Return-to-normal, silent, until Reset. |
| R        | As M, with a two-pulse ringback until Reset or Silence. |

The `-4` forms have no lock-in: if the condition clears before anyone
acknowledges, the sound stops at once. The plain forms keep sounding until
acknowledged, so a two-second dip is never missed. Recommend a plain form
when a missed transient matters, and `A-4` for nuisance-prone signals.

## Custom sounds

`alarms.getProjectSounds` and `alarms.setProjectSounds{sounds}` read and
replace the project's `sounds` object; the setter returns any rejected
paths. Paths are WAV files, relative to the project file:

```json
{ "sequence": "M",
  "channels": { "Engine": { "warning": "sounds/horn.wav",
                            "caution": "sounds/chime.wav" } } }
```

A single band can also play its own WAV through its `sound` key. App-wide
sounds and event sounds are set in Preferences > Sounds.
