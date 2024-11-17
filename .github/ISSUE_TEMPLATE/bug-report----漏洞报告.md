---
name: Bug report /  漏洞报告
about: Report a bug or crash
title: ''
labels: bug
assignees: alex-spataru

---

**Bug Report Template**

---

### **1. Describe the bug**

> Provide a clear and concise description of the bug you’re experiencing.  

[Your answer here]

### **2. Screenshots (if applicable)**

> Add screenshots to help illustrate the problem, if applicable.  

[Attach your screenshots here]

### **3. Upload your JSON map file**

> This is essential for reproducing any UI-related issues.  

[Attach your JSON map file here]

### **4. Provide a minimal Arduino program to replicate the issue**

> Sharing a minimal Arduino sketch helps me replicate the environment and setup that causes the bug.  

[Your Arduino sketch here]  

Here’s an example of a minimal program:  

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

### **4. Important Notes**

- Non-critical bugs will be treated with **lower priority than critical bugs**.  
- Donations are always welcome to help keep the project alive and ensure a timely response. You can support the project here: [Donate Now](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE).  
- **I am an open-source developer.** I provide this software as-is and cannot be responsible for long-term maintenance or personalized service. The source code is available freely, and if the issue is urgent, you’re encouraged to fix it yourself or hire someone (including myself) to do so.  

Thank you for understanding and for helping improve the software!

---

### **对于中国用户 / For Chinese users, please delete this part if you speak English**

### **1. 描述问题**

> 请清晰简洁地描述您遇到的问题。  

[请在这里填写您的答案]

### **2. 截图（如果适用）**

> 如果适用，请添加截图以帮助说明您的问题。  

[在此处附加您的截图]

### **3. 上传您的 JSON 地图文件**

> 这是重现任何与 UI 相关问题的必要条件。  

[在此处附加您的 JSON 地图文件]

### **4. 提供一个最小的 Arduino 程序以复现问题**

> 提供一个最小的 Arduino 程序有助于我复现导致问题的环境和设置。  

[请在此填写您的 Arduino 程序]  

程序的基本步骤包括：

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

### **5. 注意事项**

- **非关键性问题的优先级将低于关键性问题。**  
- 欢迎通过[捐赠链接](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE)支持该项目，以帮助项目持续开发并确保及时响应。  
- **我是一个开源开发者。** 我提供此软件是“按原样”的，我无法负责长期维护或个性化服务。源代码是免费提供的，如果问题很紧急，建议您自己修复或雇佣他人（包括我）进行修复。  

感谢您的理解并帮助改进此软件！
