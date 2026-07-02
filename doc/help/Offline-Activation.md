# Offline Activation

Offline activation unlocks Serial Studio Pro on a machine that never connects to
the internet. Instead of validating the license online, the machine imports a
signed license file that you obtain once from the activation website.

This is meant for air-gapped, industrial, or locked-down deployments where the
computer running Serial Studio cannot reach the internet. A machine that has
occasional internet access does not need this: normal
[online activation](Pro-vs-Free.md#upgrading-from-free-to-pro) covers brief
disconnections with a 30-day offline grace period.

Offline activation is available in the Pro (commercial) build only, and is
offered for lifetime and test-stand licenses. Monthly and yearly subscriptions
use online activation with the 30-day offline grace period instead.

## What you need

- A Serial Studio Pro lifetime or test-stand license key and the email address
  used to purchase it.
- The offline machine, running a Pro build of Serial Studio.
- A second computer with internet access (a phone, laptop, or any browser).
- A way to move a small file between the two machines, such as a USB drive.

## Steps

Offline activation is a three-step exchange: the offline machine produces a
device file, the website turns it into a license file, and the offline machine
imports that license file.

### 1. Save the device file

On the offline machine:

1. Open the **About** dialog from the toolbar, then click **Manage License**.
2. In the License Management dialog, click **Activate Offline…** to open the
   offline activation wizard.
3. Click **Save Device File…** and store the `.ssmachine` file.

The device file identifies this machine and carries no personal information. It
holds a hardware fingerprint and the application version.

### 2. Get the license file

On a computer with internet access:

1. Go to
   [serial-studio.com/offline-activation](https://serial-studio.com/offline-activation).
   The wizard also shows this address so you can type it on the other machine.
2. Upload the `.ssmachine` device file.
3. Enter the email address and license key from your purchase.
4. Download the `.sslic` license file the site returns.

The site verifies your license before it issues the file, so a valid Pro license
is required.

### 3. Import the license file

Move the `.sslic` file to the offline machine, then:

1. Reopen the wizard from **About → Manage License → Activate Offline…**.
2. Click **Import License File…** and select the `.sslic` file.

Pro features unlock immediately, and no network request is made on this machine.

## How the license behaves

- The license file is bound to the machine that produced the device file. It
  will not activate any other machine.
- Each offline activation uses one seat on your license, the same as an online
  activation.
- A lifetime license file does not expire. It keeps working on that machine with
  no further steps.
- A test-stand license file carries an expiry that follows your license term. Run
  the three steps again after a renewal to refresh it. Serial Studio shows the
  remaining days and warns before the file lapses.
- The activation persists across restarts and needs no network on the offline
  machine.

## Managing seats

Because the offline machine cannot reach the internet, it cannot release its own
seat from within Serial Studio, and a lifetime offline activation holds its seat
until it is removed. To free or move an offline seat, contact
alex@serial-studio.com.

## Troubleshooting

- **"This certificate was issued for a different device."** The license file was
  generated from another machine's device file. Export a new device file on the
  machine you are activating, then repeat the steps.
- **"This certificate has expired."** The subscription period covered by the file
  has ended. Renew the subscription and run the three steps again.
- **The import reports a malformed file.** Confirm you selected the `.sslic` file
  returned by the activation site, not the `.ssmachine` device file.

## See Also

- [Pro vs Free](Pro-vs-Free.md) - Feature comparison and online activation.
- [License Agreement](License-Agreement.md) - Full legal terms.
- [Command-Line Interface](Command-Line-Interface.md) - Headless activation flags.
