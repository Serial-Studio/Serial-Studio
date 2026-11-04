//
// Serial Studio Acknowledged Commands sketch
//
// Author: Alex Spataru
//
// Prints one CSV line per frame and answers two commands, each ending in the token Serial Studio
// chose for that press:
//
//   BLINK <token>        blink the built-in LED; always acknowledged
//   SP <value> <token>   set the setpoint; refused above 80
//
// Every line ends with the reply to the latest command, and repeats it until the next one:
//
//   setpoint,token,code,message
//
// A code of 0 acknowledges the command, anything else refuses it. A message holds no comma.
//
// Baud rate: 115200
//

const unsigned long kIntervalMs = 100;
const unsigned long kBlinkMs    = 200;
const long kSetpointLimit       = 80;

long setpoint            = 0;
long replyToken          = 0;
int replyCode            = 0;
const char* replyMsg     = "";
unsigned long blinkUntil = 0;
String line;

void reply(long token, int code, const char* message)
{
  replyToken = token;
  replyCode  = code;
  replyMsg   = message;
}

void handle(const String& command)
{
  if (command.startsWith("BLINK ")) {
    blinkUntil = millis() + kBlinkMs;
    reply(command.substring(6).toInt(), 0, "OK");
    return;
  }

  if (command.startsWith("SP ")) {
    const int split  = command.lastIndexOf(' ');
    const long value = command.substring(3, split).toInt();
    const long token = command.substring(split + 1).toInt();
    if (value > kSetpointLimit) {
      reply(token, 2, "above limit");
      return;
    }

    setpoint = value;
    reply(token, 0, "OK");
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  static unsigned long lastFrame = 0;

  while (Serial.available() > 0) {
    const char c = Serial.read();
    if (c == '\n') {
      line.trim();
      handle(line);
      line = "";
    } else if (line.length() < 40)
      line += c;
  }

  digitalWrite(LED_BUILTIN, millis() < blinkUntil ? HIGH : LOW);

  if (millis() - lastFrame < kIntervalMs)
    return;

  lastFrame = millis();
  Serial.print(setpoint);
  Serial.print(',');
  Serial.print(replyToken);
  Serial.print(',');
  Serial.print(replyCode);
  Serial.print(',');
  Serial.println(replyMsg);
}
