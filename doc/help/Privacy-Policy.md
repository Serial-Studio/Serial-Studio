# Privacy Policy

Last updated: June 28, 2026

This policy describes how the Serial Studio desktop application handles your
data. It covers the application you install and run on your own computer. The
Serial Studio website and online store (`serial-studio.com`,
`store.serial-studio.com`) are operated separately and handle account and
purchase data under their own terms.

## Summary

Serial Studio is a local-first desktop application. The data you stream from
your hardware, the projects you build, the sessions you record, and the files
you export all stay on your machine. Serial Studio does not run analytics, does
not collect usage telemetry, and does not upload crash reports.

The only network connections Serial Studio makes on its own are license
activation, trial registration, and an optional update check. Everything else
that leaves your machine is something you start: connecting to a device,
opening the AI Assistant, loading map tiles in the GPS widget, or browsing the
examples and extensions catalogs.

## What stays on your device

The following never leaves your computer unless you explicitly export or share
it:

- Live data received from your devices (serial, network, BLE, MQTT, CAN, and
  every other source).
- Project files, frame parsers, transforms, and control scripts.
- Recorded sessions in the SQLite session database, CSV exports, MDF4 files,
  and PDF reports.
- Application settings, themes, window layout, and workspace configuration.
- AI Assistant chat history, stored as local JSON files.
- AI provider API keys and your license token, stored encrypted (see
  [Local data security](#local-data-security)).
- Crash-recovery state. Serial Studio detects an abnormal exit with a local
  flag in application settings and never transmits any crash information.

## Information Serial Studio sends to us

Serial Studio operates one server that the application contacts directly:

- **Trial registration** (`https://cloud.serial-studio.com/trial`). When you
  start a Pro trial, the application sends a machine fingerprint, a timestamp,
  and a one-time nonce so the trial can be bound to your machine and its
  expiry tracked. No name, email, or device data is included.

There is no other reporting back to Serial Studio. No analytics, no telemetry,
no crash reporting, no usage statistics.

## Information sent to third parties

Some features rely on third-party services. When you use them, data goes
directly from your machine to that service, governed by that service's own
privacy policy.

### License activation (Lemon Squeezy)

Pro license activation, validation, and deactivation contact the Lemon Squeezy
licensing API (`https://api.lemonsqueezy.com/v1/licenses/...`). The application
sends your license key and a machine fingerprint. Lemon Squeezy is the payment
processor and reseller for Serial Studio Pro; it holds the purchase
information you provided at checkout (such as name, email, and payment
details) under its own privacy policy at <https://www.lemonsqueezy.com/privacy>.
Once activated, your license metadata is cached locally in encrypted form so
the application can work offline for up to 30 days between checks.

### AI Assistant (Pro, optional, bring-your-own-key)

The AI Assistant is off by default and requires you to supply your own API key
for a provider you choose. When you send a message, the conversation and any
project context you include are sent directly from your machine to that
provider's endpoint. Requests are not routed through any Serial Studio server.
Before a request is sent, a redaction pass removes detected secrets (such as
API keys) from the context.

Supported providers and their endpoints:

| Provider   | Endpoint host          |
|------------|------------------------|
| Anthropic  | `api.anthropic.com`    |
| OpenAI     | `api.openai.com`       |
| Google     | Gemini API endpoint    |
| DeepSeek   | `api.deepseek.com`     |
| Groq       | `api.groq.com`         |
| Mistral    | `api.mistral.ai`       |
| OpenRouter | `openrouter.ai`        |
| Local      | a host you configure   |

Each remote provider processes your messages under its own privacy policy.
Review that policy before sending sensitive data. To keep AI processing fully
offline, point the Assistant at a local OpenAI-compatible endpoint (such as
Ollama, llama.cpp, LM Studio, or vLLM); in that case no data leaves your
network. See the [AI Assistant](AI-Assistant.md) page for details.

### Software updates (GitHub)

If update checks are enabled, the application downloads a small static version
file from GitHub (`https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/master/updates.json`)
and compares it against the running version on your machine. The request
carries only what any HTTP client sends (your IP address and a user agent),
which GitHub processes under its own policy. No license, machine, or usage
data is attached. You can disable automatic update checks in the application
settings.

### In-application content (GitHub, Iconify, Esri)

Some panels fetch content on demand when you open them:

- Documentation, examples, and the extensions catalog are fetched from GitHub
  (`raw.githubusercontent.com`, `api.github.com`). These requests carry no
  personal data.
- The icon picker queries the Iconify service (`api.iconify.design`) with your
  search term to resolve icons.
- The GPS map widget fetches map tiles from Esri ArcGIS Online
  (`services.arcgisonline.com`) and a cloud overlay image. The tile requests
  include the coordinates of the area being displayed, which reveals the
  geographic region you are viewing to the tile provider.

## The machine fingerprint

Activation and trial registration use a machine fingerprint to bind a license
or trial to a specific computer. The fingerprint is a one-way hash derived from
a stable operating-system identifier (for example the OS machine ID on Linux,
the platform UUID on macOS, or the registry machine GUID on Windows). It
contains no personal files, no file contents, and no account information, and
the original identifier cannot be recovered from it. The same value is used to
derive the local encryption key described below, so encrypted data from one
machine cannot be decrypted on another.

## Local data security

API keys and license data are stored encrypted at rest in the application
settings, using a key derived from the machine fingerprint and protected with
an integrity hash. Plaintext API keys are scrubbed from memory when a key is
removed. Project files, session databases, and exports are ordinary files under
your control and are not encrypted by the application; protect them with your
operating system's own mechanisms if they contain sensitive data.

## Data retention and your choices

- **Local data** is retained until you delete it. Removing project files,
  clearing the session database, deleting chat history, or removing a stored
  API key deletes that data from your machine.
- **Purchase and license data** held by Lemon Squeezy is subject to its
  retention practices and your rights under applicable law. Contact Lemon
  Squeezy, or email us, to access or delete that data.
- **Trial records** held by Serial Studio consist of the machine fingerprint
  and trial timing described above. Email us to request deletion.

Depending on where you live, you may have rights to access, correct, or delete
personal data held about you. To exercise those rights for data Serial Studio
holds, use the contact below.

## Children

Serial Studio is a tool for engineering, education, and hobbyist use and is not
directed at children under 13. Serial Studio does not knowingly collect
personal data from children.

## Changes to this policy

This policy may be updated as features change. Material changes will be
reflected here with a new date at the top, and the current version always ships
with the application.

## Contact

For privacy questions or data requests, email alex@serial-studio.com.

## See also

- [License Agreement](License-Agreement.md): the dual-license terms and
  activation policy.
- [AI Assistant](AI-Assistant.md): how the bring-your-own-key chat panel works
  and what it sends.
- [Session Database](Session-Database.md): how recorded sessions are stored
  locally.
