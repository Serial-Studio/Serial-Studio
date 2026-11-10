# Notifications (Pro)

Your board prints a battery voltage. When it drops under 11 V you want a line on the dashboard saying so. A dataset is one value per frame, such as the battery voltage column. Its transform runs once per frame with that value. To place the code, select the dataset in the Project Editor, click **Transform** in the dataset toolbar and pick Lua or JavaScript (see [Dataset Value Transforms](Dataset-Transforms.md)). Paste one of these two versions, not both.

Lua version:

```lua
function transform(value)
  if value < 11.0 then
    notifyWarning("Power", "Battery low", string.format("%.2f V", value))
  end

  return value
end
```

JavaScript version:

```javascript
function transform(value) {
  if (value < 11.0) {
    notifyWarning("Power", "Battery low", value.toFixed(2) + " V");
  }

  return value;
}
```

Each call adds a row to the Notification Log on the dashboard and, if you opt in, raises an OS desktop notification. Open the log from **Start menu > Tools > Notifications** on the dashboard (see [The Notification Log widget](#the-notification-log-widget)).

Notifications are a Pro feature. The first time you use a Pro feature, Serial Studio offers a 14-day trial with a Yes/No prompt (it needs an internet connection); after the trial expires, the prompt offers activation or purchase instead. See [Pro vs Free Features](Pro-vs-Free.md). Without an active Pro license, a JavaScript transform throws an error and a Lua one silently does nothing; details in [Rules and limits](#rules-and-limits).

## Overview

Any caller posts an event, and the Notification Log widget lists events in time order, oldest at the top. An event has four fields:

| Field      | Type   | Purpose |
|------------|--------|---------|
| `level`    | int    | `0` = Info, `1` = Warning, `2` = Critical. |
| `channel`  | string | Free-form label such as `"Power"` or `"Engine"`. Spaces allowed. |
| `title`    | string | Short headline, shown in bold in the log. |
| `subtitle` | string | Optional detail line under the title, word-wrapped. |

[Alarm bands](Widget-Reference.md#alarm-bands) on a dataset post notifications automatically (channel `Alarms`); you do not call anything. A band-sourced row does not create a second annunciator point, because the band already drives it.

Channels are not declared up front; they exist once something posts to them. Channel names are case-sensitive.

## Where notifications come from

| Place | Where you write it | Entry point |
|-------|--------------------|-------------|
| Dataset transform | Project Editor, dataset, **Transform** | `transform(value)` |
| Frame parser | Project Editor, source, frame parser | `parse(frame)` |
| Output widget | Output widget script | `transmit(value)` |
| [Control Loop](Control-Loop.md) | Control Loop script | `setup()` and `loop()` |
| API Server | An external program | `notifications.post` |

The four script places in the table (transform, parser, output widget, Control Loop) all expose `notify`, `notifyInfo`, `notifyWarning`, `notifyCritical` and `notifyClear`, described in [The function family](#the-function-family). Use them in a parser to flag a malformed frame, in an output widget to log a clamped setpoint, or in the Control Loop to raise a communication-loss alarm.

The API Server is off by default. Enable it in **Preferences** (port 7777), then send `notifications.post` with `level`, `channel`, `title` and `subtitle`. The other notification commands are `notifications.resolve`, `notifications.list`, `notifications.listChannels`, `notifications.getUnreadCount`, `notifications.clearChannel`, `notifications.clearAll` and `notifications.markRead`. See [API Reference](API-Reference.md).

## The function family

| Function                                  | Level         | Purpose |
|-------------------------------------------|---------------|---------|
| `notify(level, channel, title, subtitle)` | caller chosen | Generic form. `level` is `0`, `1` or `2`, or the `Info`/`Warning`/`Critical` constants. |
| `notifyInfo(channel, title, subtitle)`    | `Info`        | Informational event. |
| `notifyWarning(channel, title, subtitle)` | `Warning`     | Worth attention, not critical. |
| `notifyCritical(channel, title, subtitle)`| `Critical`    | Alarm-class event. |
| `notifyClear(channel, title, subtitle)`   | `Info`        | Posts an Info event titled `Resolved: <title>` (just `Resolved` if the title is empty). The original row stays in the log. It also returns the matching alarm point on the annunciator to normal (see [Aural Alerts](Aural-Alerts.md)). |

### Argument overloads

`notifyInfo`, `notifyWarning`, `notifyCritical` and `notifyClear` accept one, two or three arguments. With one argument it is the title; with two it is title and subtitle; the channel then defaults to `"Dashboard"`. With three, the first is the channel. `notifyInfo("Ready", "System armed")` posts to `"Dashboard"` with title `"Ready"` and subtitle `"System armed"`.

The generic `notify()` starts with the level and then follows the same overloads for the remaining fields. The level constants are globals (`Info = 0`, `Warning = 1`, `Critical = 2`), so `notify(Critical, "Engine", "EGT exceeded", "1052 C")` equals `notifyCritical("Engine", "EGT exceeded", "1052 C")`.

## Examples

Paste these into the **Transform** field of a dataset (pick Lua or JavaScript in the Transform editor). The signatures are identical in frame parsers and output widgets. The battery-voltage example is in the page introduction.

### Critical alarm with latch and resolve

A transform runs on every frame. Without a latch, a value stuck above 900 would post a row on every frame. A 10 kHz transform would otherwise fill the log with the same row ten thousand times a second. Latch the state, post once on entry, then post a `Resolved` event when the condition clears:

```lua
local latched = false

function transform(value)
  if value > 900 and not latched then
    notifyCritical("Engine", "EGT critical",
                   string.format("%.1f C", value))
    latched = true
  elseif value < 870 and latched then
    notifyClear("Engine", "EGT critical",
                string.format("Back to %.1f C", value))
    latched = false
  end

  return value
end
```

```javascript
var latched = false;

function transform(value) {
  if (value > 900 && !latched) {
    notifyCritical("Engine", "EGT critical",
                   value.toFixed(1) + " C");
    latched = true;
  } else if (value < 870 && latched) {
    notifyClear("Engine", "EGT critical",
                "Back to " + value.toFixed(1) + " C");
    latched = false;
  }

  return value;
}
```

### One-argument shorthand from a frame parser

A parser drops frames with too few fields and posts a one-liner to the default `"Dashboard"` channel:

```lua
function parse(frame)
  local parts = {}
  for token in string.gmatch(frame, "([^,]+)") do
    table.insert(parts, token)
  end

  if #parts < 3 then
    notifyWarning("Frame dropped: too few fields")
    return {}
  end

  return parts
end
```

```javascript
function parse(frame) {
  var parts = frame.split(",");
  if (parts.length < 3) {
    notifyWarning("Frame dropped: too few fields");
    return [];
  }

  return parts;
}
```

A stream that stays malformed posts this row about ten times a second (the 100 ms dedup window collapses only identical rows within that span); latch it as in the EGT example if that matters.

### Output widget logging a command

An output widget is a dashboard control that sends a command to the device (see [Output Controls](Output-Controls.md)). This one formats a setpoint command and logs the outgoing value so the operator sees what was sent:

```javascript
function transmit(value) {
  notifyInfo("Setpoint", "Sent",
             "Target = " + value.toFixed(1));
  return "SET " + value.toFixed(1) + "\r\n";
}
```

## The Notification Log widget

The Notification Log is a Pro dashboard widget. Turn it on from **Start menu > Tools > Notifications** on the dashboard, or from the **Notifications** pinned shortcut on the taskbar (choose pinned shortcuts in **Preferences > Taskbar > Pinned Buttons**). The free build hides both.

| Element | Behaviour |
|---------|-----------|
| Rows | Icon, title, channel pill and timestamp, with the subtitle on a wrapped second line. Warning and Critical titles use the alarm color. The log follows new events unless you scrolled up. |
| Unread badge | Counts unread Warning and Critical events, capped at `99+`. Info events do not count. |
| **Filter by channel** | Case-sensitive substring match on the channel name, live as you type. |
| **Clear all notifications** | Wipes the in-memory history and the dedup window. |

There is one log per dashboard. It is not tied to any dataset group, and its position and size are saved with the project layout. With no events, it shows "No notifications yet".

The history is wiped on every dashboard reset: disconnect, project reload and playback stop. Events from the previous session do not bleed into the new one.

## OS desktop notifications

OS desktop notifications are a Pro feature. Open **Preferences**, select the **Notifications** tab (hidden in the free build) and turn on **System Notifications**. It is off by default.

With it on, Warning and Critical events raise a tray toast even when Serial Studio is not the foreground window. The toast shows the channel and title on one line and the subtitle on a second. Info events never produce a toast.

It needs a system tray; the condition is that the platform reports one. Where it reports none, no toast appears and the log still records the event. Some Linux desktops, stock GNOME for one, hide tray icons without an extension.

## Routing Qt log messages

Serial Studio's own critical errors appear in the log on a `System` channel, with the title `Critical` and the message as the subtitle. To see its warnings there too, turn on **Route Warnings to Notifications** on the same **Notifications** tab (title `Warning`). This setting is on the Pro-only **Notifications** tab. It is off by default because Qt and QML emit warnings often during normal operation. With **System Notifications** also on, routed events raise tray toasts like any Warning or Critical.

Events on the `System` and `Problems` channels never become alarm points on the annunciator, so a routed warning does not sound a siren.

## Rules and limits

| Rule | Value |
|------|-------|
| Ring buffer | The most recent 1024 events. Older ones drop as new ones arrive. |
| Deduplication | An event is dropped if the same `(level, channel, title, subtitle)` was posted in the last 100 ms, even with other events in between. A rising value in the subtitle (`"1049 C"`, then `"1050 C"`) is never collapsed. |
| Normalizing | Channel and title are trimmed. An event with both empty is ignored. Levels outside `0` to `2` clamp to Info or Critical. |
| Threads | `notify*` works from any engine. Off the GUI thread the event is queued to it, and Control Loop scripts reach the same bus through the `notifications.post` and `notifications.resolve` commands. |

Without an active Pro license (a GPL build, or an official binary with no license or trial):

| Engine | Result |
|--------|--------|
| JavaScript (parser, transform, output widget) | Checked when the script engine is created. The call throws `notify() requires a Pro license. See https://serial-studio.com/pricing`. |
| Lua | Checked on every call. The call does nothing, and Serial Studio logs one warning per session in the Console: `Script notify*() ignored: requires an active Serial Studio Pro license or trial`. |
| Control Loop and API Server | `notifications.post` and `notifications.resolve` are refused. |

The level constants `Info`, `Warning` and `Critical` are defined at every tier, so a Pro project still loads. Opening a project that uses `notify*` in the free edition shows the Pro-required overlay on the dashboard.

## See also

- [Dataset Value Transforms](Dataset-Transforms.md): where `notify*` is most often used.
- [Frame Parser Scripting](Frame-Parser.md): `parse(frame)` has the same notification API.
- [Aural Alerts](Aural-Alerts.md): Warning and Critical notifications become alarm points that sound and flash the master annunciator, and `notifyClear()` returns them to normal.
- [API Reference](API-Reference.md): `notifications.post`, `notifications.list` and the rest, for external drivers.
- [MQTT Publisher](MQTT-Publisher.md): the **Publish Notifications** option republishes every event on this bus to MQTT.
