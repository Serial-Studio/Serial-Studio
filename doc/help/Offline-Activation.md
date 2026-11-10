# Offline Activation

The test rig sits behind a firewall that has never met a browser, so Serial
Studio cannot check your Pro license online. You export a small file from that
machine, trade it for a license file on any computer with a browser, and import
the result. Nothing else touches the network. The Pro trial prompt cannot help
here, because registering a trial needs the internet.

Offline activation works only with lifetime and Test Stand licenses. Monthly and
yearly subscriptions use [online activation](Pro-vs-Free.md#upgrading-from-free-to-pro),
which tolerates up to 30 days offline. A machine that connects now and then does
not need this page. The Test Stand tier includes 20 offline device activations.

> **Warning:** activating an offline device permanently consumes one license
> seat. An air-gapped machine cannot report a deactivation, so the seat is never
> released and cannot be moved to another machine. A destroyed machine takes its
> seat with it. Plan your seats before activating.

## What you need

- A lifetime or Test Stand license key and the email address used to purchase
  it.
- The offline machine, with Serial Studio installed and no license active. The
  **Activate Offline…** button appears only while nothing is activated; on an
  online-activated machine, click **Deactivate** first.
- A second computer with a browser and internet access.
- A way to move small files between the two, such as a USB drive.

## Steps

Two files cross the gap: a device file goes out, a license file comes back. The
app and its error messages call the license file a certificate; it is the same
`.sslic` file.

| File | Extension | Made by | Contents |
|------|-----------|---------|----------|
| Device file | `.ssmachine` | **Save Device File…** on the offline machine | Device ID, application name and version, timestamp. No personal data. |
| License file | `.sslic` | serial-studio.com/offline-activation | Signed certificate bound to the Device ID, with an **Expires** date. |

### 1. Save the device file

On the offline machine:

1. Click **About** in the toolbar, then click **Manage License**.
2. In the Licensing dialog, click **Activate Offline…** to open the **Activate
   Offline** wizard.
3. Click **Save Device File…** and store the `.ssmachine` file.

The device file is a small JSON file with the Device ID, the application name
and version, and the time it was generated. It carries no personal information.
The Licensing dialog shows the same Device ID.

### 2. Get the license file

On a computer with internet access:

1. Go to
   [serial-studio.com/offline-activation](https://serial-studio.com/offline-activation).
   The wizard shows this address too, so you can type it on the other machine;
   ignore its **Open in Browser** button on the offline machine.
2. Upload the `.ssmachine` device file.
3. Enter the email address and license key from your purchase.
4. Download the `.sslic` license file the site returns.

The site checks your license before it issues the file.

### 3. Import the license file

Move the `.sslic` file to the offline machine, then:

1. Reopen the wizard from **About → Manage License → Activate Offline…**.
2. Click **Import License File…** and select the `.sslic` file.

Pro features unlock immediately, and the import makes no network request. The
Licensing dialog now shows **Serial Studio Pro (Offline)** (the name follows
your license variant), the Device ID, and **Expires** as a date with the days
left, for example "(30 days left)".

With the API Server enabled (it is off by default), the `licensing.activateOffline`
command imports the same file from its `path` parameter. `licensing.getStatus`
reports `offlineActivated`, `offlineDaysRemaining` and `offlineExpiresAt`. The AI
Assistant cannot call `licensing.activateOffline`; it is in the Blocked tier.

## How the license behaves

An offline Serial Studio Pro license behaves as follows.

- The license file (`.sslic`) is bound to the machine that produced the device file. It
  will not activate any other machine.
- The activation persists across restarts and needs no network.
- The certificate has its own **Expires** date, even on a lifetime license.
  With fewer than 14 days left the Licensing dialog adds "Your offline license expires soon.
  Request a new certificate to stay activated." After the date passes, the
  certificate fails verification the next time Serial Studio starts and Pro
  locks. To renew, repeat the Save, Get and Import steps from
  [Offline Activation](#steps) and import the new `.sslic` file.
- The Device ID comes from the operating system, not Serial Studio. It changes
  after an OS reinstall or a hardware replacement, and the old license file
  stops working.

## Managing seats

To remove a Serial Studio Pro offline license from a machine, open **About → Manage License**, click
**Deactivate**, and confirm **Yes** at "Remove the offline license from this
device?". This deletes only the local certificate; the seat stays consumed. (Deactivating
an online activation does free its seat.) Copy the Device ID from the Licensing
dialog before you deactivate, or read it from the `.ssmachine` file (field
`machineId`).

To restore Pro on the same machine, import the same `.sslic` file again. Keep it
somewhere safe. If an offline machine is destroyed or decommissioned, email
alex@serial-studio.com with your license key and the Device ID.

## Troubleshooting

| Message | Cause | Fix |
|---------|-------|-----|
| "This certificate was issued for a different device." | The license file came from another machine's device file. | On the machine you are activating, repeat the Offline Activation steps: save a new device file, get a new license file at serial-studio.com/offline-activation, import it. |
| "The certificate file is malformed or unreadable." | You selected the `.ssmachine` file, or the `.sslic` copy is damaged or edited. | Select the `.sslic` file the site returned, or download it again. |
| "This certificate has expired." | The certificate is past its **Expires** date. | Get a new `.sslic` file at serial-studio.com/offline-activation with a fresh device file and import it. |
| "The certificate file is too large to be valid." | The file is over 64 KB, so it is not a license file. | Select the `.sslic` file the site returned. |
| "This certificate does not grant a valid license tier." | The certificate's signed tier field is missing or not one Serial Studio recognizes, usually a corrupted or hand-edited file. | Download the `.sslic` file again; if it repeats, email alex@serial-studio.com. |
| "Could not open the certificate file." | The file is unreadable (missing, locked, or no permission). | Copy the `.sslic` file to a local disk and select it again. |

## See Also

- [Pro vs Free](Pro-vs-Free.md) - Feature comparison and online activation.
- [License Agreement](License-Agreement.md) - Full legal terms.
- [Command-Line Interface](Command-Line-Interface.md) - License activation flags (online).
