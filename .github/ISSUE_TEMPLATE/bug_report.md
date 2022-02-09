---
name: Bug report
about: Create a report to help us improve
title: ''
labels: bug
assignees: alex-spataru

---

## Describe the bug

A clear and concise description of what the bug is.

### Screenshots

If applicable, add screenshots to help explain your problem.

## Upload your JSON map file*

This is important in order to reproduce any UI-related issues.

## Upload minimal Arduino program to replicate your issue

This is important so that I can replicate the environment and setup that causes the bug. 

An example program could be:

```cpp
void setup() {
    Serial.begin(115200);
    while (!Serial);
}

void loop() {
    Serial.println("/*1,2,3*/");
    delay(50);
}
```

If you need me to test with a specific microcontroller, please consider [donating](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) so that I can buy that MCU and help you!
