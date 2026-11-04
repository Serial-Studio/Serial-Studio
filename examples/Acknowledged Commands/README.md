# Acknowledged Commands

**What you'll learn:** how an output control waits for the device to answer a command, and shows "acknowledged", "refused" or "unanswered" instead of assuming the command worked.

**Prerequisites:** a Pro license or trial (output controls are a Pro feature) and an Arduino-compatible board with a built-in LED.

## Files

| File | Purpose |
|------|---------|
| `AcknowledgedCommands.ino` | Arduino sketch that prints one frame every 100 ms and answers `BLINK` and `SP` commands at 115200 baud. |
| `AcknowledgedCommands.ssproj` | The project: a Device group, plus a Blink button and a Setpoint number field. |

## The convention

Each command line ends in a token that Serial Studio chooses for that press:

```
BLINK 17
SP 42 18
```

Every frame the device sends ends in the reply to the latest command: the token it saw, a result code and a short message. The device repeats the reply in every frame until the next command arrives.

```
42,18,0,OK
42,19,2,above limit
```

| Field | Meaning |
|-------|---------|
| Token | The number the command carried. Serial Studio ignores a reply whose token is not the one it sent last. |
| Code | `0` acknowledges the command. Any other number refuses it. |
| Message | Text shown beside a refusal. It holds no comma. |

The three reply fields are hidden datasets in the Device group. Both controls name them under **State Feedback** in the Project Editor (Reply Token, Result Code, Reply Message), and each transmit function receives the token as its second argument: `transmit(value, token)`.

## The two limits of the setpoint

The Setpoint field refuses a value outside 0 to 100 before anything is sent; that limit belongs to the control. The sketch refuses a value above 80 with code 2 and the message `above limit`; that limit belongs to the device. Typing 90 therefore sends the command and shows "refused: above limit".

## Running it

1. Flash `AcknowledgedCommands.ino` onto the board.
2. In Serial Studio, choose **Parse via Project File** under **Operation Mode** and open `AcknowledgedCommands.ssproj`.
3. In **Device Setup**, pick the board's port and 115200 baud, then press **Connect**.
4. Press **Blink**. The LED lights for 200 ms and the control reads "acknowledged".
5. Type `50` in the Setpoint field and press Return: acknowledged. Type `90`: refused, "above limit".
6. Unplug the board's TX line (or reset it mid-command) and press **Blink** again: after the Confirm Within time of 1000 ms the control reads "unanswered".
