<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="hi_IN" sourcelanguage="en_US">
<context>
    <name>AI::AnthropicReply</name>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="164"/>
        <source>Anthropic error</source>
        <translation>Anthropic त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="310"/>
        <source>Stream parse error: %1</source>
        <translation>स्ट्रीम पार्स त्रुटि: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="359"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="362"/>
        <source>Invalid API key (%1)</source>
        <translation>अमान्य API कुंजी (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="364"/>
        <source>Rate limited: %1</source>
        <translation>रेट सीमित: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="366"/>
        <source>Anthropic %1: %2</source>
        <translation>Anthropic %1: %2</translation>
    </message>
</context>
<context>
    <name>AI::Assistant</name>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="191"/>
        <source>Allow AI Device Control?</source>
        <translation>AI डिवाइस नियंत्रण की अनुमति दें?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="192"/>
        <source>This lets the AI assistant configure devices, open and close connections, and send data to your hardware.

Every device action still requires your explicit per-call approval in the chat, even when auto-approve is enabled. Only enable this if you trust the configured AI provider with hardware access.</source>
        <translation>यह AI सहायक को डिवाइस कॉन्फ़िगर करने, कनेक्शन खोलने और बंद करने, तथा आपके हार्डवेयर को डेटा भेजने की अनुमति देता है।

ऑटो-अप्रूव सक्षम होने पर भी, प्रत्येक डिवाइस क्रिया के लिए चैट में आपकी स्पष्ट प्रति-कॉल स्वीकृति आवश्यक है। इसे केवल तभी सक्षम करें जब आप हार्डवेयर एक्सेस के लिए कॉन्फ़िगर किए गए AI प्रदाता पर भरोसा करते हों।</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="390"/>
        <source>Switch AI provider?</source>
        <translation>AI प्रदाता बदलें?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="391"/>
        <source>Switching to a different provider clears the current conversation. Do you want to continue?</source>
        <translation>किसी अन्य प्रदाता पर स्विच करने से वर्तमान वार्तालाप साफ़ हो जाएगा। क्या आप जारी रखना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="394"/>
        <source>Assistant</source>
        <translation>असिस्टेंट</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="431"/>
        <source>AI Assistant is not available in this build</source>
        <translation>AI Assistant इस बिल्ड में उपलब्ध नहीं है</translation>
    </message>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">AI असिस्टेंट के लिए Pro लाइसेंस आवश्यक है</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="436"/>
        <source>Set an API key first</source>
        <translation>पहले API key सेट करें</translation>
    </message>
</context>
<context>
    <name>AI::Conversation</name>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">AI Assistant के लिए Pro लाइसेंस आवश्यक है</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="172"/>
        <source>AI Assistant is not available in this build</source>
        <translation>AI Assistant इस बिल्ड में उपलब्ध नहीं है</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="178"/>
        <source>AI subsystem not initialized</source>
        <translation>AI सबसिस्टम इनिशियलाइज़ नहीं है</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="184"/>
        <source>Already busy with a previous request</source>
        <translation>पिछली रिक्वेस्ट में व्यस्त है</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="500"/>
        <source>Tool-call budget reached for this turn; no further tools will run.</source>
        <translation>इस बारी के लिए टूल-कॉल बजट पूरा हो गया; अब कोई और टूल नहीं चलेगा।</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1098"/>
        <source>Waiting for %1 to respond. Loading the model and processing the prompt can take a while on local hardware...</source>
        <translation>%1 के जवाब का इंतज़ार है। लोकल हार्डवेयर पर मॉडल लोड करने और प्रॉम्प्ट प्रोसेस करने में समय लग सकता है...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1956"/>
        <source>You have reached the tool-call budget for this turn. Do not request more tools. Summarize what you found so far, and if the task is incomplete, say which steps remain so the user can tell you to continue.</source>
        <translation>आपने इस बारी के लिए टूल-कॉल बजट पूरा कर लिया है। अब और टूल का अनुरोध न करें। अब तक जो मिला है उसे सारांशित करें, और यदि कार्य अधूरा है, तो बताएं कि कौन से चरण शेष हैं ताकि उपयोगकर्ता आपको जारी रखने के लिए कह सके।</translation>
    </message>
    <message>
        <source>Tool-call budget exceeded</source>
        <translation type="vanished">Tool-call बजट सीमा पार हो गई</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="945"/>
        <source>(The model returned an empty response. Try rephrasing, switching to a different model, or checking that the request is allowed by the provider's safety filters.)</source>
        <translation>(मॉडल ने खाली रिस्पॉन्स दिया। दोबारा शब्दों में बदलकर प्रयास करें, किसी अन्य मॉडल पर स्विच करें, या जांचें कि प्रोवाइडर के सेफ्टी फ़िल्टर द्वारा रिक्वेस्ट की अनुमति है।)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1102"/>
        <source>Sending request to %1...</source>
        <translation>%1 को रिक्वेस्ट भेजी जा रही है...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1114"/>
        <source>Provider returned no reply</source>
        <translation>प्रोवाइडर ने कोई रिप्लाई नहीं दी</translation>
    </message>
</context>
<context>
    <name>AI::GeminiReply</name>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="146"/>
        <source>Prompt blocked by Gemini safety filter: %1</source>
        <translation>Gemini सेफ्टी फ़िल्टर द्वारा प्रॉम्प्ट ब्लॉक किया गया: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="200"/>
        <source>Gemini stopped without producing a response: %1</source>
        <translation>Gemini बिना रिस्पॉन्स दिए रुक गया: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="262"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="265"/>
        <source>Invalid API key (%1)</source>
        <translation>अमान्य API कुंजी (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="267"/>
        <source>Rate limited: %1</source>
        <translation>रेट सीमित: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="269"/>
        <source>Invalid API key</source>
        <translation>अमान्य API कुंजी</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="271"/>
        <source>Gemini %1: %2</source>
        <translation>Gemini %1: %2</translation>
    </message>
</context>
<context>
    <name>AI::OpenAIReply</name>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="426"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="429"/>
        <source>Invalid API key (%1)</source>
        <translation>अमान्य API कुंजी (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="431"/>
        <source>Rate limited: %1</source>
        <translation>रेट सीमित: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="433"/>
        <source>%1 %2: %3</source>
        <translation>%1 %2: %3</translation>
    </message>
    <message>
        <source>OpenAI %1: %2</source>
        <translation type="vanished">OpenAI %1: %2</translation>
    </message>
</context>
<context>
    <name>API::GRPC::GRPCServer</name>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="421"/>
        <source>Export Protobuf File</source>
        <translation>Protobuf फ़ाइल एक्सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="423"/>
        <source>Protocol Buffers (*.proto)</source>
        <translation>Protocol Buffers (*.proto)</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="469"/>
        <source>Unable to start gRPC server</source>
        <translation>GRPC सर्वर प्रारंभ करने में असमर्थ</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="470"/>
        <source>Failed to bind to %1</source>
        <translation>%1 से बाइंड करने में विफल</translation>
    </message>
</context>
<context>
    <name>API::ProcessLauncher</name>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="82"/>
        <source>No program specified</source>
        <translation>कोई प्रोग्राम निर्दिष्ट नहीं</translation>
    </message>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="88"/>
        <source>Program "%1" not found in PATH</source>
        <translation>प्रोग्राम "%1" PATH में नहीं मिला</translation>
    </message>
</context>
<context>
    <name>API::Server</name>
    <message>
        <location filename="../../src/API/Server.cpp" line="434"/>
        <source>Unable to start API TCP server</source>
        <translation>API TCP सर्वर प्रारंभ करने में असमर्थ</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="478"/>
        <source>Allow External API Connections?</source>
        <translation>बाहरी API कनेक्शन की अनुमति दें?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="479"/>
        <source>Exposing the API server to external hosts allows other devices on your network to connect to Serial Studio on port 7777.

Only enable this on trusted networks. Untrusted clients may read live data or send commands to your device.</source>
        <translation>API सर्वर को बाहरी होस्ट के लिए उपलब्ध कराने से आपके नेटवर्क पर अन्य डिवाइस पोर्ट 7777 पर Serial Studio से कनेक्ट हो सकते हैं।

इसे केवल विश्वसनीय नेटवर्क पर सक्षम करें। अविश्वसनीय क्लाइंट लाइव डेटा पढ़ सकते हैं या आपके डिवाइस को कमांड भेज सकते हैं।</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="513"/>
        <source>Unable to restart API TCP server</source>
        <translation>API TCP सर्वर पुनः प्रारंभ करने में असमर्थ</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="600"/>
        <source>Allow API device control?</source>
        <translation>API डिवाइस नियंत्रण की अनुमति दें?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="601"/>
        <source>A program using Serial Studio's local API is requesting to send data to the connected device. Allow API clients to write to the device?</source>
        <translation>Serial Studio की लोकल API का उपयोग करने वाला एक प्रोग्राम कनेक्टेड डिवाइस को डेटा भेजने का अनुरोध कर रहा है। API क्लाइंट को डिवाइस पर लिखने की अनुमति दें?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="604"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1251"/>
        <source>API server</source>
        <translation>API सर्वर</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1251"/>
        <source>Invalid pending connection</source>
        <translation>अमान्य लंबित कनेक्शन</translation>
    </message>
</context>
<context>
    <name>About</name>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="39"/>
        <source>About</source>
        <translation>परिचय</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="96"/>
        <source>Version %1</source>
        <translation>संस्करण %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="106"/>
        <source>Copyright © %1 %2</source>
        <translation>कॉपीराइट © %1 %2</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="112"/>
        <source>All Rights Reserved</source>
        <translation>सर्वाधिकार सुरक्षित</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="127"/>
        <source>%1 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

%1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</source>
        <translation>%1 मुक्त सॉफ़्टवेयर है: आप इसे GNU General Public License की शर्तों के तहत पुनर्वितरित और/या संशोधित कर सकते हैं जैसा कि Free Software Foundation द्वारा प्रकाशित किया गया है; लाइसेंस के संस्करण 3, या (आपके विकल्प पर) किसी बाद के संस्करण के अनुसार।

%1 को इस आशा के साथ वितरित किया गया है कि यह उपयोगी होगा, लेकिन बिना किसी वारंटी के; यहाँ तक कि व्यापारिकता या किसी विशेष उद्देश्य के लिए उपयुक्तता की निहित वारंटी के बिना भी। अधिक जानकारी के लिए GNU General Public License देखें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="146"/>
        <source>This configuration is licensed for commercial and proprietary use. It may be used in closed-source and commercial applications, subject to the terms of the commercial license.</source>
        <translation>यह कॉन्फ़िगरेशन व्यावसायिक और स्वामित्व उपयोग के लिए लाइसेंस प्राप्त है। इसे बंद-स्रोत और व्यावसायिक अनुप्रयोगों में उपयोग किया जा सकता है, व्यावसायिक लाइसेंस की शर्तों के अधीन।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="160"/>
        <source>This configuration is for personal and evaluation purposes only. Commercial use is prohibited unless a valid commercial license is activated.</source>
        <translation>यह कॉन्फ़िगरेशन केवल व्यक्तिगत और मूल्यांकन उद्देश्यों के लिए है। व्यावसायिक उपयोग निषिद्ध है जब तक कि वैध व्यावसायिक लाइसेंस सक्रिय न हो।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="174"/>
        <source>This software is provided 'as is' without warranty of any kind, express or implied, including but not limited to the warranties of merchantability or fitness for a particular purpose. In no event shall the author be liable for any damages arising from the use of this software.</source>
        <translation>यह सॉफ़्टवेयर 'जैसा है' प्रदान किया गया है बिना किसी प्रकार की वारंटी के, व्यक्त या निहित, जिसमें व्यापारिकता या किसी विशेष उद्देश्य के लिए उपयुक्तता की वारंटी शामिल है लेकिन इन्हीं तक सीमित नहीं है। इस सॉफ़्टवेयर के उपयोग से उत्पन्न किसी भी क्षति के लिए लेखक उत्तरदायी नहीं होगा।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="195"/>
        <source>Manage License</source>
        <translation>लाइसेंस प्रबंधित करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="203"/>
        <source>Donate</source>
        <translation>दान करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="214"/>
        <source>Check for Updates</source>
        <translation>अपडेट जाँचें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="223"/>
        <source>What's New</source>
        <translation>नया क्या है</translation>
    </message>
    <message>
        <source>Tips &amp;&amp; Tricks</source>
        <translation type="vanished">टिप्स और ट्रिक्स</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="232"/>
        <source>License Agreement</source>
        <translation>लाइसेंस अनुबंध</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="241"/>
        <source>Report Bug</source>
        <translation>बग रिपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="250"/>
        <source>Acknowledgements</source>
        <translation>आभार</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="259"/>
        <source>Benchmark</source>
        <translation>बेंचमार्क</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="267"/>
        <source>Website</source>
        <translation>वेबसाइट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="283"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>Accelerometer</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="186"/>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="187"/>
        <source>Settings</source>
        <translation>सेटिंग्स</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="245"/>
        <source>G-FORCE</source>
        <translation>G-FORCE</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="283"/>
        <source>PITCH ↕</source>
        <translation>PITCH ↕</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="320"/>
        <source>ROLL ↔</source>
        <translation>ROLL ↔</translation>
    </message>
</context>
<context>
    <name>AccelerometerConfigDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="35"/>
        <source>Accelerometer Configuration</source>
        <translation>एक्सेलेरोमीटर कॉन्फ़िगरेशन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="95"/>
        <source>Configure the accelerometer display range and input units.</source>
        <translation>एक्सेलेरोमीटर डिस्प्ले रेंज और इनपुट यूनिट कॉन्फ़िगर करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="109"/>
        <source>Display Range</source>
        <translation>डिस्प्ले रेंज</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="130"/>
        <source>Max G:</source>
        <translation>अधिकतम G:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="144"/>
        <source>Enter max G value</source>
        <translation>अधिकतम G मान दर्ज करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="164"/>
        <source>Input Configuration</source>
        <translation>इनपुट कॉन्फ़िगरेशन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="184"/>
        <source>Input already in G</source>
        <translation>इनपुट पहले से G में है</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="218"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>Acknowledgements</name>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="35"/>
        <source>Acknowledgements</source>
        <translation>आभार</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="76"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="87"/>
        <source>About Qt…</source>
        <translation>QT के बारे में…</translation>
    </message>
</context>
<context>
    <name>ActionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="136"/>
        <source>Change Icon</source>
        <translation>आइकन बदलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="138"/>
        <source>Change the icon used for this action</source>
        <translation>इस एक्शन के लिए उपयोग किया जाने वाला आइकन बदलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="156"/>
        <source>Duplicate</source>
        <translation>डुप्लिकेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="160"/>
        <source>Duplicate this action with all its settings</source>
        <translation>इस एक्शन को इसकी सभी सेटिंग्स के साथ डुप्लिकेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="169"/>
        <source>Delete</source>
        <translation>डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="171"/>
        <source>Delete this action from the project</source>
        <translation>इस एक्शन को प्रोजेक्ट से डिलीट करें</translation>
    </message>
</context>
<context>
    <name>AddWidgetDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="44"/>
        <source>Add Widget</source>
        <translation>विजेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="211"/>
        <source>Available Widgets</source>
        <translation>उपलब्ध विजेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="220"/>
        <source>Click a row to add it to the workspace.</source>
        <translation>इसे वर्कस्पेस में जोड़ने के लिए पंक्ति पर क्लिक करें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="228"/>
        <source>Search</source>
        <translation>खोजें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="247"/>
        <source>Widget</source>
        <translation>विजेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="248"/>
        <source>Group</source>
        <translation>समूह</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="249"/>
        <source>Name</source>
        <translation>नाम</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="316"/>
        <source>(entire group)</source>
        <translation>(संपूर्ण समूह)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="351"/>
        <source>Already in workspace</source>
        <translation>कार्यस्थान में पहले से मौजूद</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="352"/>
        <source>Add to workspace</source>
        <translation>कार्यस्थान में जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="381"/>
        <source>No widgets available.</source>
        <translation>कोई विजेट उपलब्ध नहीं।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="382"/>
        <source>No widgets match.</source>
        <translation>कोई विजेट मेल नहीं खाता।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="399"/>
        <source>%1 widgets</source>
        <translation>%1 विजेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="400"/>
        <source>%1 of %2 widgets</source>
        <translation>%2 में से %1 विजेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="404"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>AlarmBandsEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="35"/>
        <source>Alarm Bands</source>
        <translation>अलार्म बैंड्स</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="71"/>
        <source>Info</source>
        <translation>जानकारी</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="72"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="152"/>
        <source>OK</source>
        <translation>ठीक है</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="73"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="153"/>
        <source>Warning</source>
        <translation>चेतावनी</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="74"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="154"/>
        <source>Critical</source>
        <translation>गंभीर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="83"/>
        <source>Tachometer</source>
        <translation>टैकोमीटर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="85"/>
        <source>Idle</source>
        <translation>निष्क्रिय</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="86"/>
        <source>Operating</source>
        <translation>संचालन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="87"/>
        <source>Caution</source>
        <translation>सावधानी</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="88"/>
        <source>Redline</source>
        <translation>रेडलाइन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="92"/>
        <source>Speedometer</source>
        <translation>स्पीडोमीटर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="94"/>
        <source>Cruise</source>
        <translation>क्रूज़</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="95"/>
        <source>Fast</source>
        <translation>तेज़</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="96"/>
        <source>Top Speed</source>
        <translation>टॉप स्पीड</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="100"/>
        <source>Engine Temperature</source>
        <translation>इंजन तापमान</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="102"/>
        <source>Cold</source>
        <translation>ठंडा</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="103"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="112"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="144"/>
        <source>Normal</source>
        <translation>सामान्य</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="104"/>
        <source>Warm</source>
        <translation>गर्म</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="105"/>
        <source>Overheat</source>
        <translation>अधिक ताप</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="109"/>
        <source>Pressure</source>
        <translation>दाब</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="111"/>
        <source>Vacuum</source>
        <translation>निर्वात</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="113"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="122"/>
        <source>High</source>
        <translation>उच्च</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="114"/>
        <source>Burst</source>
        <translation>बर्स्ट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="118"/>
        <source>Battery Voltage</source>
        <translation>बैटरी वोल्टेज</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="120"/>
        <source>Low</source>
        <translation>निम्न</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="121"/>
        <source>Nominal</source>
        <translation>नॉमिनल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="126"/>
        <source>Fuel Level</source>
        <translation>ईंधन स्तर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="128"/>
        <source>Empty</source>
        <translation>खाली</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="129"/>
        <source>Reserve</source>
        <translation>रिज़र्व</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="134"/>
        <source>Signal Strength</source>
        <translation>सिग्नल शक्ति</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="136"/>
        <source>No Signal</source>
        <translation>कोई सिग्नल नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="137"/>
        <source>Weak</source>
        <translation>कमज़ोर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="138"/>
        <source>Good</source>
        <translation>अच्छा</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="142"/>
        <source>CPU / System Load</source>
        <translation>CPU / सिस्टम लोड</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="145"/>
        <source>Busy</source>
        <translation>व्यस्त</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="146"/>
        <source>Overload</source>
        <translation>ओवरलोड</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="150"/>
        <source>OK / Warning / Critical</source>
        <translation>ठीक / चेतावनी / गंभीर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="158"/>
        <source>Indicator (On / Off)</source>
        <translation>इंडिकेटर (चालू / बंद)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="160"/>
        <source>On</source>
        <translation>चालू</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="164"/>
        <source>Fault Indicator</source>
        <translation>फॉल्ट इंडिकेटर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="166"/>
        <source>Fault</source>
        <translation>फॉल्ट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="287"/>
        <source>Choose Band Color</source>
        <translation>बैंड रंग चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="314"/>
        <source>Presets</source>
        <translation>प्रीसेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="337"/>
        <source>Preset</source>
        <translation>प्रीसेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="352"/>
        <source>Choose preset…</source>
        <translation>प्रीसेट चुनें…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="462"/>
        <source>Blink</source>
        <translation>ब्लिंक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="582"/>
        <source>Reset to severity default</source>
        <translation>गंभीरता डिफ़ॉल्ट पर रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="596"/>
        <source>Click to choose a color. Right-click to reset to severity default.</source>
        <translation>रंग चुनने के लिए क्लिक करें। गंभीरता डिफ़ॉल्ट पर रीसेट करने के लिए राइट-क्लिक करें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="597"/>
        <source>Click to choose a custom color.</source>
        <translation>कस्टम रंग चुनने के लिए क्लिक करें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="628"/>
        <source>Flash the LED while the value sits in this band.</source>
        <translation>जब मान इस बैंड में हो तो LED को फ्लैश करें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="702"/>
        <source>No bands defined. Pick a preset above or add a band to get started.</source>
        <translation>कोई बैंड परिभाषित नहीं। आरंभ करने के लिए ऊपर से प्रीसेट चुनें या बैंड जोड़ें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="826"/>
        <source>Apply</source>
        <translation>लागू करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="829"/>
        <source>Apply changes to the dataset.</source>
        <translation>डेटासेट में परिवर्तन लागू करें।</translation>
    </message>
    <message>
        <source>Apply Preset</source>
        <translation type="vanished">प्रीसेट लागू करें</translation>
    </message>
    <message>
        <source>Replace the current bands with the selected preset, scaled to this dataset's range.</source>
        <translation type="vanished">वर्तमान बैंड्स को चयनित प्रीसेट से बदलें, इस डेटासेट की रेंज के अनुसार स्केल किया गया।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="363"/>
        <source>Range</source>
        <translation>रेंज</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="391"/>
        <source>Bands</source>
        <translation>बैंड्स</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="402"/>
        <source>Add Band</source>
        <translation>बैंड जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="406"/>
        <source>Add a new band continuing from the last one.</source>
        <translation>अंतिम बैंड से जारी रखते हुए नया बैंड जोड़ें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="437"/>
        <source>Min</source>
        <translation>न्यूनतम</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="443"/>
        <source>Max</source>
        <translation>अधिकतम</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="449"/>
        <source>Severity</source>
        <translation>गंभीरता</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="455"/>
        <source>Color</source>
        <translation>रंग</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="469"/>
        <source>Label</source>
        <translation>लेबल</translation>
    </message>
    <message>
        <source>Choose a custom color.</source>
        <translation type="vanished">कस्टम रंग चुनें।</translation>
    </message>
    <message>
        <source>auto</source>
        <translation type="vanished">स्वतः</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="639"/>
        <source>(optional)</source>
        <translation>(वैकल्पिक)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="656"/>
        <source>Move up.</source>
        <translation>ऊपर ले जाएं।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="675"/>
        <source>Move down.</source>
        <translation>नीचे ले जाएं।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="688"/>
        <source>Remove this band.</source>
        <translation>यह बैंड हटाएं।</translation>
    </message>
    <message>
        <source>No bands defined. Apply a preset or add a band to get started.</source>
        <translation type="vanished">कोई बैंड परिभाषित नहीं। आरंभ करने के लिए प्रीसेट लागू करें या बैंड जोड़ें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="719"/>
        <source>Preview</source>
        <translation>पूर्वावलोकन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="815"/>
        <source>Cancel</source>
        <translation>रद्द करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="817"/>
        <source>Discard changes.</source>
        <translation>परिवर्तन त्यागें।</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="vanished">सहेजें</translation>
    </message>
    <message>
        <source>Save changes to the dataset.</source>
        <translation type="vanished">डेटासेट में परिवर्तन सहेजें।</translation>
    </message>
</context>
<context>
    <name>AssistantPanel</name>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="31"/>
        <source>Assistant</source>
        <translation>असिस्टेंट</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="129"/>
        <source>CSV vs MDF4 export - what is the difference?</source>
        <translation>CSV बनाम MDF4 एक्सपोर्ट - क्या अंतर है?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="132"/>
        <source>Plot, Bar, and Gauge - when to use each?</source>
        <translation>Plot, Bar, और Gauge - प्रत्येक का उपयोग कब करें?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="204"/>
        <source>How can I help with your project?</source>
        <translation>मैं आपके प्रोजेक्ट में कैसे मदद कर सकता हूँ?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="205"/>
        <source>Set up your API key to get started</source>
        <translation>शुरू करने के लिए अपनी API Key सेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="217"/>
        <source>Describe what you would like to build, and I will configure the sources, groups, datasets, frame parsers, and transforms for you.</source>
        <translation>बताएं कि आप क्या बनाना चाहते हैं, और मैं आपके लिए सोर्स, ग्रुप, डेटासेट, फ़्रेम पार्सर और ट्रांसफ़ॉर्म कॉन्फ़िगर कर दूंगा।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="220"/>
        <source>To start chatting, paste an API key for the selected provider. Keys are encrypted on this machine and never leave your computer except to talk to the provider you choose.</source>
        <translation>चैट शुरू करने के लिए, चयनित प्रोवाइडर के लिए API Key पेस्ट करें। Key इस मशीन पर एन्क्रिप्ट की जाती हैं और आपके कंप्यूटर से बाहर नहीं जातीं, सिवाय आपके चुने हुए प्रोवाइडर से बात करने के।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="241"/>
        <source>Open API Key Setup</source>
        <translation>API Key सेटअप खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="251"/>
        <source>Get a key from %1</source>
        <translation>%1 से Key प्राप्त करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="125"/>
        <source>List the sources in this project</source>
        <translation>इस प्रोजेक्ट में सोर्स की सूची दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="122"/>
        <source>Help me discover Serial Studio's features</source>
        <translation>Serial Studio की सुविधाओं को जानने में मेरी मदद करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="123"/>
        <source>What can this app do for my telemetry?</source>
        <translation>यह ऐप मेरी टेलीमेट्री के लिए क्या कर सकता है?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="124"/>
        <source>Walk me through what this project already contains</source>
        <translation>मुझे बताएं कि इस प्रोजेक्ट में पहले से क्या है</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="128"/>
        <source>What is a session database, and why would I use one?</source>
        <translation>सेशन डेटाबेस क्या है, और मैं इसका उपयोग क्यों करूं?</translation>
    </message>
    <message>
        <source>CSV vs MDF4 export — what is the difference?</source>
        <translation type="vanished">CSV बनाम MDF4 एक्सपोर्ट — क्या अंतर है?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="130"/>
        <source>What is a frame parser, and when do I need one?</source>
        <translation>फ्रेम पार्सर क्या है, और मुझे इसकी कब आवश्यकता है?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="131"/>
        <source>When should I use Lua vs JavaScript for the parser?</source>
        <translation>पार्सर के लिए Lua बनाम JavaScript का उपयोग कब करना चाहिए?</translation>
    </message>
    <message>
        <source>Plot, Bar, and Gauge — when to use each?</source>
        <translation type="vanished">Plot, Bar, और Gauge — प्रत्येक का उपयोग कब करें?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="133"/>
        <source>What is the difference between a transform and a frame parser?</source>
        <translation>ट्रांसफॉर्म और फ्रेम पार्सर में क्या अंतर है?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="136"/>
        <source>Add a UART source for an Arduino</source>
        <translation>Arduino के लिए UART सोर्स जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="137"/>
        <source>Set up an IMU project from scratch</source>
        <translation>शुरू से IMU प्रोजेक्ट सेट अप करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="138"/>
        <source>Configure an MQTT subscriber</source>
        <translation>MQTT Subscriber कॉन्फ़िगर करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="139"/>
        <source>Add a CAN bus source</source>
        <translation>CAN Bus स्रोत जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="140"/>
        <source>Set up a Modbus poller</source>
        <translation>Modbus पोलर सेट अप करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="141"/>
        <source>Add a network (TCP/UDP) source</source>
        <translation>नेटवर्क (TCP/UDP) स्रोत जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="142"/>
        <source>Write a CSV frame parser for me</source>
        <translation>मेरे लिए CSV फ़्रेम पार्सर लिखें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="143"/>
        <source>Help me parse a JSON frame</source>
        <translation>JSON फ़्रेम पार्स करने में मदद करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="144"/>
        <source>Add an EMA smoothing transform to a dataset</source>
        <translation>डेटासेट में EMA स्मूथिंग ट्रांसफ़ॉर्म जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="145"/>
        <source>Decode hexadecimal frames</source>
        <translation>हेक्साडेसिमल फ़्रेम डिकोड करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="146"/>
        <source>Calibrate a sensor with a linear transform</source>
        <translation>लीनियर ट्रांसफ़ॉर्म से सेंसर कैलिब्रेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="149"/>
        <source>Suggest dashboard widgets for my data</source>
        <translation>मेरे डेटा के लिए डैशबोर्ड विजेट सुझाएं</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="150"/>
        <source>Build an executive overview workspace</source>
        <translation>एक्ज़ीक्यूटिव ओवरव्यू वर्कस्पेस बनाएं</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="151"/>
        <source>Add a painter widget for a custom visualization</source>
        <translation>कस्टम विज़ुअलाइज़ेशन के लिए पेंटर विजेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="152"/>
        <source>Show Plot, FFT, and Waterfall for one dataset</source>
        <translation>एक डेटासेट के लिए Plot, FFT और Waterfall दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="153"/>
        <source>Group my datasets into useful workspaces</source>
        <translation>मेरे डेटासेट को उपयोगी Workspace में समूहित करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="432"/>
        <source>Drop files or folders to let the assistant read them</source>
        <translation>फ़ाइलें या फ़ोल्डर छोड़ें ताकि सहायक उन्हें पढ़ सके</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="477"/>
        <source>Added folder "%1" - readable this session</source>
        <translation>फ़ोल्डर "%1" जोड़ा गया - इस सत्र में पठनीय</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="478"/>
        <source>Added "%1" - readable this session</source>
        <translation>"%1" जोड़ा गया - इस सत्र में पठनीय</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="562"/>
        <source>Ask Serial Studio anything…</source>
        <translation>Serial Studio से कुछ भी पूछें…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="582"/>
        <source>Clear conversation</source>
        <translation>वार्तालाप साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="626"/>
        <source>Stop generating</source>
        <translation>जनरेट करना रोकें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="627"/>
        <source>Send message (Enter)</source>
        <translation>संदेश भेजें (Enter)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="669"/>
        <source>Provider</source>
        <translation>प्रदाता</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="702"/>
        <source>Model selection</source>
        <translation>मॉडल चयन</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="748"/>
        <source>Run editing actions without asking each time. Blocked actions stay blocked.</source>
        <translation>हर बार पूछे बिना संपादन क्रियाएँ चलाएँ। ब्लॉक की गई क्रियाएँ ब्लॉक रहेंगी।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="750"/>
        <source>Auto-approve edits</source>
        <translation>संपादन स्वतः स्वीकृत करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="766"/>
        <source>Let the AI configure devices, connect/disconnect and send data. Each action still asks for your approval.</source>
        <translation>AI को डिवाइस कॉन्फ़िगर करने, कनेक्ट/डिस्कनेक्ट करने और डेटा भेजने दें। प्रत्येक क्रिया अभी भी आपकी स्वीकृति मांगती है।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="768"/>
        <source>Allow device control</source>
        <translation>डिवाइस नियंत्रण की अनुमति दें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="794"/>
        <source>Manage API keys</source>
        <translation>API कुंजियाँ प्रबंधित करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="815"/>
        <source>Working</source>
        <translation>कार्य जारी है</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="816"/>
        <source>Ready</source>
        <translation>तैयार</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="817"/>
        <source>  •  cache %1k tok</source>
        <translation>•  कैश %1k tok</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="818"/>
        <source>  •  cache write %1k tok</source>
        <translation>कैश लिखें %1k टोक</translation>
    </message>
</context>
<context>
    <name>Audio</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="89"/>
        <source>No Microphone Detected</source>
        <translation>कोई माइक्रोफ़ोन नहीं मिला</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="98"/>
        <source>Connect a mic or check your settings</source>
        <translation>माइक या अपनी सेटिंग्स जाँचें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="123"/>
        <source>Input Device</source>
        <translation>इनपुट डिवाइस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="142"/>
        <source>Sample Rate</source>
        <translation>सैम्पल रेट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="230"/>
        <source>Sample Format</source>
        <translation>सैम्पल फ़ॉर्मेट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="180"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="249"/>
        <source>Channels</source>
        <translation>चैनल</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="211"/>
        <source>Output Device</source>
        <translation>आउटपुट डिवाइस</translation>
    </message>
</context>
<context>
    <name>AuthenticateDialog</name>
    <message>
        <source>Dialog</source>
        <translation type="vanished">संवाद</translation>
    </message>
    <message>
        <source>Please provide the user name and password for the download location.</source>
        <translation type="vanished">डाउनलोड स्थान के लिए उपयोगकर्ता नाम और पासवर्ड प्रदान करें।</translation>
    </message>
    <message>
        <source>&amp;User name:</source>
        <translation type="vanished">उपयोगकर्ता नाम (&amp;U):</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="vanished">पासवर्ड (&amp;P):</translation>
    </message>
</context>
<context>
    <name>AxisRangeDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="65"/>
        <source>Axis Range Configuration</source>
        <translation>अक्ष रेंज कॉन्फ़िगरेशन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="179"/>
        <source>Configure the visible range for the plot axes. Values update in real-time as you type.</source>
        <translation>प्लॉट अक्षों के लिए दृश्य रेंज कॉन्फ़िगर करें। टाइप करते समय मान रीयल-टाइम में अपडेट होते हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="187"/>
        <source>X Axis</source>
        <translation>X अक्ष</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="212"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="283"/>
        <source>Minimum:</source>
        <translation>न्यूनतम:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="224"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="295"/>
        <source>Enter min value</source>
        <translation>न्यूनतम मान दर्ज करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="233"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="304"/>
        <source>Maximum:</source>
        <translation>अधिकतम:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="245"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="316"/>
        <source>Enter max value</source>
        <translation>अधिकतम मान दर्ज करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="260"/>
        <source>Y Axis</source>
        <translation>Y अक्ष</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="335"/>
        <source>Reset</source>
        <translation>रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="345"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>BackupRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="31"/>
        <source>Recover Backup</source>
        <translation>बैकअप पुनर्प्राप्त करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="94"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="165"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="166"/>
        <source>Untitled</source>
        <translation>शीर्षकहीन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="97"/>
        <source>Project Loaded</source>
        <translation>प्रोजेक्ट लोड हुआ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="98"/>
        <source>Auto-save</source>
        <translation>स्वतः-सहेजें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="99"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="119"/>
        <source>Before Restore</source>
        <translation>पुनर्स्थापना से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="100"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="106"/>
        <source>Before Delete Dataset</source>
        <translation>डेटासेट डिलीट करने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="101"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="107"/>
        <source>Before Delete Group</source>
        <translation>ग्रुप डिलीट करने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="102"/>
        <source>Before New Project</source>
        <translation>नया प्रोजेक्ट बनाने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="103"/>
        <source>Before Open Project</source>
        <translation>प्रोजेक्ट खोलने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="104"/>
        <source>Before Load JSON</source>
        <translation>JSON लोड करने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="105"/>
        <source>Before Apply Template</source>
        <translation>टेम्पलेट लागू करने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="108"/>
        <source>Before Delete Action</source>
        <translation>एक्शन डिलीट करने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="109"/>
        <source>Before Delete Output Widget</source>
        <translation>आउटपुट विजेट डिलीट करने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="110"/>
        <source>Before Move Dataset</source>
        <translation>डेटासेट मूव करने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="111"/>
        <source>Before Move Group</source>
        <translation>समूह मूव करने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="112"/>
        <source>Before Delete Workspace</source>
        <translation>वर्कस्पेस डिलीट करने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="113"/>
        <source>Before Clear All Workspaces</source>
        <translation>सभी वर्कस्पेस साफ़ करने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="114"/>
        <source>Before Remove Widget</source>
        <translation>विजेट हटाने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="115"/>
        <source>Before Reorder Workspaces</source>
        <translation>वर्कस्पेस पुनः क्रमित करने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="116"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="117"/>
        <source>Before Batch Operation</source>
        <translation>बैच ऑपरेशन से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="118"/>
        <source>Before Add Tile</source>
        <translation>टाइल जोड़ने से पहले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="142"/>
        <source>%1 (and %2 more)</source>
        <translation>%1 (और %2 अधिक)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="159"/>
        <source> The frame parser code also differs and will be replaced.</source>
        <translation>फ्रेम पार्सर कोड भी भिन्न है और इसे बदल दिया जाएगा।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="164"/>
        <source>Title changes from “%1” to “%2”. Group structure unchanged.</source>
        <translation>शीर्षक "%1" से "%2" में बदला। समूह संरचना अपरिवर्तित।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="169"/>
        <source>Same groups and datasets, but the frame parser code differs — restoring will replace it.</source>
        <translation>समान ग्रुप और डेटासेट, लेकिन फ्रेम पार्सर कोड भिन्न है — रिस्टोर करने पर इसे बदल दिया जाएगा।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="171"/>
        <source>Same groups and datasets as your current project. Restoring may still revert field-level edits.</source>
        <translation>आपके वर्तमान प्रोजेक्ट के समान ग्रुप और डेटासेट। पुनर्स्थापना से फ़ील्ड-स्तरीय संपादन वापस हो सकते हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="178"/>
        <source>Restoring removes %1 and brings back %2.</source>
        <translation>पुनर्स्थापना %1 को हटाती है और %2 को वापस लाती है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="181"/>
        <source>Restoring removes %1.</source>
        <translation>पुनर्स्थापना %1 को हटाती है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="183"/>
        <source>Restoring brings back %1.</source>
        <translation>पुनर्स्थापना %1 को वापस लाती है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="209"/>
        <source>Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.</source>
        <translation>पुनर्स्थापित करने के लिए बैकअप चुनें। वर्तमान प्रोजेक्ट पहले स्वचालित रूप से सहेजा जाता है, इसलिए पुनर्स्थापना प्रतिवर्ती है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="292"/>
        <source>No backups for this project yet. Edit or save the project to start the rolling backup.</source>
        <translation>इस प्रोजेक्ट के लिए अभी तक कोई बैकअप नहीं। रोलिंग बैकअप शुरू करने के लिए प्रोजेक्ट संपादित या सहेजें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="320"/>
        <source>Open Folder</source>
        <translation>फ़ोल्डर खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="328"/>
        <source>Cancel</source>
        <translation>रद्द करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="334"/>
        <source>Restore</source>
        <translation>पुनर्स्थापित करें</translation>
    </message>
</context>
<context>
    <name>Benchmark</name>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="32"/>
        <source>Benchmark</source>
        <translation>बेंचमार्क</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="79"/>
        <source>%1 frames/s</source>
        <translation>%1 फ़्रेम/सेकंड</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="83"/>
        <source>%1 s</source>
        <translation>%1 सेकंड</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="88"/>
        <source>n/a</source>
        <translation>उपलब्ध नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Pass</source>
        <translation>पास</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Fail</source>
        <translation>फेल</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="111"/>
        <source>Hotpath Benchmark</source>
        <translation>हॉटपाथ बेंचमार्क</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="122"/>
        <source>Measures how fast this computer can extract, parse, and visualize frames through Serial Studio's data pipeline.</source>
        <translation>यह मापता है कि यह कंप्यूटर Serial Studio के डेटा पाइपलाइन के माध्यम से फ़्रेम को कितनी तेज़ी से निकाल, पार्स और विज़ुअलाइज़ कर सकता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="168"/>
        <source>The interface will freeze while running</source>
        <translation>चलते समय इंटरफ़ेस फ्रीज़ हो जाएगा</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="179"/>
        <source>Each phase runs flat-out on the main thread, so the window stops responding until it finishes. Your current project is reloaded automatically when the benchmark ends.</source>
        <translation>प्रत्येक चरण मुख्य थ्रेड पर पूर्ण गति से चलता है, इसलिए विंडो समाप्त होने तक प्रतिसाद देना बंद कर देती है। बेंचमार्क समाप्त होने पर आपका वर्तमान प्रोजेक्ट स्वचालित रूप से पुनः लोड हो जाता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="236"/>
        <source>Frames per phase:</source>
        <translation>प्रति चरण फ़्रेम:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="250"/>
        <source>Minimum duration:</source>
        <translation>न्यूनतम अवधि:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="279"/>
        <source>Stages</source>
        <translation>चरण</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="287"/>
        <source>Parsers</source>
        <translation>पार्सर</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="296"/>
        <source>Data export</source>
        <translation>डेटा एक्सपोर्ट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="304"/>
        <source>Dashboard</source>
        <translation>डैशबोर्ड</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="316"/>
        <source>Data</source>
        <translation>डेटा</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="326"/>
        <source>Numeric only</source>
        <translation>केवल संख्यात्मक</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="335"/>
        <source>Mixed (numeric + text)</source>
        <translation>मिश्रित (संख्यात्मक + टेक्स्ट)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="351"/>
        <source>Select at least one stage and one data type to run a benchmark.</source>
        <translation>बेंचमार्क चलाने के लिए कम से कम एक स्टेज और एक डेटा प्रकार चुनें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="210"/>
        <source>Running %1...</source>
        <translation>%1 चल रहा है...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="211"/>
        <source>Preparing...</source>
        <translation>तैयारी हो रही है...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="383"/>
        <source>Pipeline</source>
        <translation>पाइपलाइन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="395"/>
        <source>Throughput</source>
        <translation>थ्रूपुट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="407"/>
        <source>Time</source>
        <translation>समय</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="419"/>
        <source>Result</source>
        <translation>परिणाम</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="520"/>
        <source>Run a test to see results</source>
        <translation>परिणाम देखने के लिए टेस्ट चलाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="539"/>
        <source>Pass/Fail applies to the data-pipeline and parser stages (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard stages are informational.</source>
        <translation>पास/फेल केवल डेटा-पाइपलाइन और पार्सर चरणों पर लागू होता है (डेटा पाइपलाइन और Built-in न्यूमेरिक 1024 K फ़्रेम/s; Built-in मिक्स्ड 512 K; Lua न्यूमेरिक 256 K; JavaScript न्यूमेरिक और Lua मिक्स्ड 128 K; JavaScript मिक्स्ड 64 K)। एक्सपोर्ट और डैशबोर्ड चरण सूचनात्मक हैं।</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">पास/फेल केवल डेटा-पाइपलाइन और पार्सर चरणों पर लागू होता है (डेटा पाइपलाइन और Built-in न्यूमेरिक 1024 K फ़्रेम/s; Built-in मिक्स्ड 512 K; Lua न्यूमेरिक 256 K; JavaScript न्यूमेरिक और Lua मिश्रित 128 K; JavaScript मिश्रित 64 K)। एक्सपोर्ट और डैशबोर्ड चरण सूचनात्मक हैं।</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Native numeric 1024 K frames/s; Native mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Pass/Fail डेटा-पाइपलाइन और पार्सर चरणों पर लागू होता है (डेटा पाइपलाइन और Native न्यूमेरिक 1024 K फ्रेम्स/s; Native मिक्स्ड 512 K; Lua न्यूमेरिक 256 K; JavaScript न्यूमेरिक और Lua मिक्स्ड 128 K; JavaScript मिक्स्ड 64 K)। एक्सपोर्ट और डैशबोर्ड चरण सूचनात्मक हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="554"/>
        <source>Copy</source>
        <translation>कॉपी करें</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline 1024 K frames/s; numeric: Lua 256 K, JavaScript 128 K; mixed: Lua 128 K, JavaScript 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">पास/फेल डेटा-पाइपलाइन और पार्सर चरणों पर लागू होता है (डेटा पाइपलाइन 1024 K फ़्रेम/s; संख्यात्मक: Lua 256 K, JavaScript 128 K; मिश्रित: Lua 128 K, JavaScript 64 K)। एक्सपोर्ट और डैशबोर्ड चरण सूचनात्मक हैं।</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the parser phases only (Lua target 256 K frames/s, JavaScript 128 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">पास/फेल केवल पार्सर चरणों पर लागू होता है (Lua लक्ष्य 256 K फ़्रेम/s, JavaScript 128 K)। एक्सपोर्ट और डैशबोर्ड चरण सूचनात्मक हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="561"/>
        <source>Clear</source>
        <translation>साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="570"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Running...</source>
        <translation>चल रहा है...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Run Benchmark</source>
        <translation>बेंचमार्क चलाएं</translation>
    </message>
</context>
<context>
    <name>BenchmarkRunner</name>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="255"/>
        <source>Data pipeline</source>
        <translation>डेटा पाइपलाइन</translation>
    </message>
    <message>
        <source>Lua parser</source>
        <translation type="vanished">Lua पार्सर</translation>
    </message>
    <message>
        <source>JavaScript parser</source>
        <translation type="vanished">JavaScript पार्सर</translation>
    </message>
    <message>
        <source>Lua + data export</source>
        <translation type="vanished">Lua + डेटा एक्सपोर्ट</translation>
    </message>
    <message>
        <source>Lua + dashboard</source>
        <translation type="vanished">Lua + डैशबोर्ड</translation>
    </message>
    <message>
        <source>Native parser (numeric)</source>
        <translation type="vanished">Native पार्सर (न्यूमेरिक)</translation>
    </message>
    <message>
        <source>Native parser (mixed)</source>
        <translation type="vanished">Native पार्सर (मिक्स्ड)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="279"/>
        <source>Built-in parser (numeric)</source>
        <translation>Built-in पार्सर (न्यूमेरिक)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="297"/>
        <source>Built-in parser (mixed)</source>
        <translation>Built-in पार्सर (मिक्स्ड)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="281"/>
        <source>Lua parser (numeric)</source>
        <translation>Lua पार्सर (संख्यात्मक)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="288"/>
        <source>JavaScript parser (numeric)</source>
        <translation>JavaScript पार्सर (संख्यात्मक)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="299"/>
        <source>Lua parser (mixed)</source>
        <translation>Lua पार्सर (मिश्रित)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="306"/>
        <source>JavaScript parser (mixed)</source>
        <translation>JavaScript पार्सर (मिश्रित)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="322"/>
        <source>Built-in + data export (numeric)</source>
        <translation>Built-in + डेटा एक्सपोर्ट (न्यूमेरिक)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="329"/>
        <source>Lua + data export (numeric)</source>
        <translation>Lua + डेटा एक्सपोर्ट (संख्यात्मक)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="336"/>
        <source>JavaScript + data export (numeric)</source>
        <translation>JavaScript + डेटा एक्सपोर्ट (संख्यात्मक)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="345"/>
        <source>Built-in + data export (mixed)</source>
        <translation>Built-in + डेटा एक्सपोर्ट (मिक्स्ड)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="347"/>
        <source>Lua + data export (mixed)</source>
        <translation>Lua + डेटा एक्सपोर्ट (मिश्रित)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="354"/>
        <source>JavaScript + data export (mixed)</source>
        <translation>JavaScript + डेटा एक्सपोर्ट (मिश्रित)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="370"/>
        <source>Built-in + dashboard (numeric)</source>
        <translation>Built-in + डैशबोर्ड (न्यूमेरिक)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="372"/>
        <source>Lua + dashboard (numeric)</source>
        <translation>Lua + डैशबोर्ड (संख्यात्मक)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>100 K frames</source>
        <translation>100 K फ़्रेम</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>250 K frames</source>
        <translation>250 K फ़्रेम</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>500 K frames</source>
        <translation>500 K फ़्रेम</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>1 M frames</source>
        <translation>1 M फ़्रेम</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>1 second</source>
        <translation>1 सेकंड</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>2 seconds</source>
        <translation>2 सेकंड</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>5 seconds</source>
        <translation>5 सेकंड</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>10 seconds</source>
        <translation>10 सेकंड</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="200"/>
        <source>Serial Studio %1 - Hotpath Benchmark</source>
        <translation>Serial Studio %1 - Hotpath बेंचमार्क</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="202"/>
        <source>%1 (%2), workload: %3 frames minimum, %4 s minimum</source>
        <translation>%1 (%2), वर्कलोड: न्यूनतम %3 फ्रेम्स, न्यूनतम %4 s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Pipeline</source>
        <translation>पाइपलाइन</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Throughput</source>
        <translation>थ्रूपुट</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Target</source>
        <translation>लक्ष्य</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Time</source>
        <translation>समय</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Result</source>
        <translation>परिणाम</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="225"/>
        <source>%1 frames/s</source>
        <translation>%1 फ़्रेम/सेकंड</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>n/a</source>
        <translation>उपलब्ध नहीं</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Pass</source>
        <translation>पास</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Fail</source>
        <translation>फेल</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="227"/>
        <source>%1 s</source>
        <translation>%1 सेकंड</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="379"/>
        <source>JavaScript + dashboard (numeric)</source>
        <translation>JavaScript + डैशबोर्ड (संख्यात्मक)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="388"/>
        <source>Built-in + dashboard (mixed)</source>
        <translation>बिल्ट-इन + डैशबोर्ड (मिश्रित)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="390"/>
        <source>Lua + dashboard (mixed)</source>
        <translation>Lua + डैशबोर्ड (मिश्रित)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="397"/>
        <source>JavaScript + dashboard (mixed)</source>
        <translation>JavaScript + डैशबोर्ड (मिश्रित)</translation>
    </message>
    <message>
        <source>Showing dashboard...</source>
        <translation type="vanished">डैशबोर्ड दिखाया जा रहा है...</translation>
    </message>
</context>
<context>
    <name>BluetoothLE</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="54"/>
        <source>Device</source>
        <translation>डिवाइस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="106"/>
        <source>Service</source>
        <translation>सेवा</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="142"/>
        <source>Characteristic</source>
        <translation>विशेषता</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="200"/>
        <source>Scanning…</source>
        <translation>स्कैन हो रहा है…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="236"/>
        <source>No Bluetooth Adapter Detected</source>
        <translation>कोई Bluetooth एडाप्टर नहीं मिला</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="247"/>
        <source>Connect a Bluetooth adapter or check your system settings</source>
        <translation>Bluetooth एडाप्टर कनेक्ट करें या अपनी सिस्टम सेटिंग्स जाँचें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="274"/>
        <source>This OS is not Supported Yet.</source>
        <translation>यह OS अभी तक समर्थित नहीं है।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="285"/>
        <source>We'll update Serial Studio to work with this operating system as soon as Qt officially supports it</source>
        <translation>जैसे ही QT आधिकारिक रूप से इसे समर्थन देगा, हम Serial Studio को इस ऑपरेटिंग सिस्टम के साथ काम करने के लिए अपडेट करेंगे</translation>
    </message>
</context>
<context>
    <name>CANBus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="57"/>
        <source>No CAN Drivers Found</source>
        <translation>कोई CAN ड्राइवर नहीं मिला</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="70"/>
        <source>Install CAN hardware drivers for your system</source>
        <translation>अपने सिस्टम के लिए CAN हार्डवेयर ड्राइवर इंस्टॉल करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="97"/>
        <source>CAN Driver</source>
        <translation>CAN ड्राइवर</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="140"/>
        <source>Interface</source>
        <translation>इंटरफ़ेस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="169"/>
        <source>Bitrate</source>
        <translation>बिटरेट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="223"/>
        <source>Flexible Data-Rate</source>
        <translation>फ्लेक्सिबल डेटा-रेट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="243"/>
        <source>Loopback</source>
        <translation>लूपबैक</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="263"/>
        <source>Listen-Only</source>
        <translation>केवल-सुनने योग्य</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="283"/>
        <source>DBC Database</source>
        <translation>DBC डेटाबेस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="287"/>
        <source>Import DBC File…</source>
        <translation>DBC फ़ाइल आयात करें…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="320"/>
        <source>No CAN Interfaces Found</source>
        <translation>कोई CAN इंटरफ़ेस नहीं मिला</translation>
    </message>
</context>
<context>
    <name>CSV::Player</name>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="206"/>
        <source>Select CSV file</source>
        <translation>CSV फ़ाइल चुनें</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="208"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV फ़ाइलें (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="366"/>
        <source>Device Connection Active</source>
        <translation>डिवाइस कनेक्शन सक्रिय</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="367"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>इस सुविधा का उपयोग करने के लिए, डिवाइस से डिस्कनेक्ट करना आवश्यक है। क्या आप आगे बढ़ना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Check file permissions and location</source>
        <translation>फ़ाइल अनुमतियाँ और स्थान जाँचें</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="411"/>
        <source>Insufficient Data in CSV File</source>
        <translation>CSV फ़ाइल में अपर्याप्त डेटा</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Cannot read CSV file</source>
        <translation>CSV फ़ाइल नहीं पढ़ी जा सकती</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="412"/>
        <source>The CSV file must contain at least one data row to proceed. Check the file and try again.</source>
        <translation>CSV फ़ाइल में आगे बढ़ने के लिए कम से कम एक डेटा पंक्ति होनी चाहिए। फ़ाइल जाँचें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="642"/>
        <source>Invalid CSV</source>
        <translation>अमान्य CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="643"/>
        <source>The CSV file does not contain any data or headers.</source>
        <translation>CSV फ़ाइल में कोई डेटा या हेडर नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <source>Select a date/time column</source>
        <translation>दिनांक/समय कॉलम चुनें</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <location filename="../../src/CSV/Player.cpp" line="664"/>
        <source>Set interval manually</source>
        <translation>अंतराल मैन्युअल रूप से सेट करें</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="654"/>
        <source>CSV Date/Time Selection</source>
        <translation>CSV दिनांक/समय चयन</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="655"/>
        <source>Choose how to handle the date/time data:</source>
        <translation>दिनांक/समय डेटा को कैसे संभालना है चुनें:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="667"/>
        <source>Set Interval</source>
        <translation>अंतराल सेट करें</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="668"/>
        <source>Please enter the interval between rows in milliseconds:</source>
        <translation>कृपया पंक्तियों के बीच अंतराल मिलीसेकंड में दर्ज करें:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="684"/>
        <source>Select Date/Time Column</source>
        <translation>दिनांक/समय कॉलम चुनें</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="685"/>
        <source>Please select the column that contains the date/time data:</source>
        <translation>कृपया वह कॉलम चुनें जिसमें दिनांक/समय डेटा है:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>Invalid Selection</source>
        <translation>अमान्य चयन</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>The selected column is not valid.</source>
        <translation>चयनित कॉलम मान्य नहीं है।</translation>
    </message>
</context>
<context>
    <name>Console</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Console.qml" line="32"/>
        <source>Console</source>
        <translation>कंसोल</translation>
    </message>
</context>
<context>
    <name>Console::Export</name>
    <message>
        <location filename="../../src/Console/Export.cpp" line="331"/>
        <source>Console Export is a Pro feature.</source>
        <translation>कंसोल एक्सपोर्ट एक Pro फीचर है।</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="332"/>
        <source>This feature requires a license. Please purchase one to enable console export.</source>
        <translation>इस फीचर के लिए लाइसेंस आवश्यक है। कंसोल एक्सपोर्ट सक्षम करने के लिए कृपया एक खरीदें।</translation>
    </message>
</context>
<context>
    <name>Console::Handler</name>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="243"/>
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="244"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="254"/>
        <source>No Line Ending</source>
        <translation>कोई लाइन एंडिंग नहीं</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="255"/>
        <source>New Line</source>
        <translation>नई लाइन</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="256"/>
        <source>Carriage Return</source>
        <translation>कैरिज रिटर्न</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="257"/>
        <source>CR + NL</source>
        <translation>CR + NL</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="267"/>
        <source>Plain Text</source>
        <translation>सादा टेक्स्ट</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="268"/>
        <source>Hexadecimal</source>
        <translation>हेक्साडेसिमल</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="290"/>
        <source>No Checksum</source>
        <translation>कोई चेकसम नहीं</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="926"/>
        <source>Device %1</source>
        <translation>डिवाइस %1</translation>
    </message>
</context>
<context>
    <name>ConstantsLibraryDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="44"/>
        <source>Insert Constant</source>
        <translation>स्थिरांक डालें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Fundamental</source>
        <translation>मौलिक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <source>Speed of light in vacuum</source>
        <translation>निर्वात में प्रकाश की गति</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <source>Planck constant</source>
        <translation>प्लैंक नियतांक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <source>Elementary charge</source>
        <translation>मूल आवेश</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <source>Avogadro constant</source>
        <translation>आवोगाद्रो नियतांक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <source>Boltzmann constant</source>
        <translation>बोल्ट्ज़मान नियतांक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Stefan-Boltzmann constant</source>
        <translation>स्टीफ़न-बोल्ट्ज़मान नियतांक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Mechanics</source>
        <translation>यांत्रिकी</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <source>Standard gravity</source>
        <translation>मानक गुरुत्वाकर्षण</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Gravitational constant</source>
        <translation>गुरुत्वाकर्षण नियतांक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Pressure</source>
        <translation>दाब</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <source>Standard atmosphere</source>
        <translation>मानक वायुमंडल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Sea-level barometric pressure</source>
        <translation>समुद्र-स्तर पर वायुमंडलीय दाब</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Temperature</source>
        <translation>तापमान</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <source>Absolute zero (Celsius)</source>
        <translation>परम शून्य (सेल्सियस)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <source>Water freezing point</source>
        <translation>जल हिमांक बिंदु</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Water boiling point (1 atm)</source>
        <translation>जल क्वथनांक बिंदु (1 atm)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Gases &amp; Fluids</source>
        <translation>गैसें और तरल पदार्थ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <source>Universal gas constant</source>
        <translation>सार्वभौमिक गैस स्थिरांक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <source>Specific gas constant (dry air)</source>
        <translation>विशिष्ट गैस स्थिरांक (शुष्क वायु)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <source>Specific gas constant (water vapor)</source>
        <translation>विशिष्ट गैस स्थिरांक (जल वाष्प)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <source>Air density (sea level, 15°C)</source>
        <translation>वायु घनत्व (समुद्र-स्तर, 15°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <source>Water density (4°C)</source>
        <translation>जल घनत्व (4°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <source>Speed of sound in air (20°C)</source>
        <translation>हवा में ध्वनि की गति (20°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Heat capacity ratio (dry air)</source>
        <translation>ऊष्मा क्षमता अनुपात (शुष्क हवा)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Electromagnetism</source>
        <translation>विद्युत चुंबकत्व</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <source>Vacuum permittivity</source>
        <translation>निर्वात परावैद्युतता</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Vacuum permeability</source>
        <translation>निर्वात पारगम्यता</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Math</source>
        <translation>गणित</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <source>Pi</source>
        <translation>पाई</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <source>Euler's number</source>
        <translation>ऑयलर संख्या</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Golden ratio</source>
        <translation>स्वर्णिम अनुपात</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="212"/>
        <source>Physics Constants</source>
        <translation>भौतिकी स्थिरांक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="221"/>
        <source>SI-unit preset values. Click a row to insert it into %1.</source>
        <translation>SI-इकाई पूर्वनिर्धारित मान। %1 में सम्मिलित करने के लिए किसी पंक्ति पर क्लिक करें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="231"/>
        <source>Search</source>
        <translation>खोजें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="250"/>
        <source>Symbol</source>
        <translation>प्रतीक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="251"/>
        <source>Name</source>
        <translation>नाम</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="252"/>
        <source>Value</source>
        <translation>मान</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="253"/>
        <source>Category</source>
        <translation>श्रेणी</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="357"/>
        <source>No constants match.</source>
        <translation>कोई स्थिरांक मेल नहीं खाते।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="378"/>
        <source>%1 constants</source>
        <translation>%1 स्थिरांक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="379"/>
        <source>%1 of %2 constants</source>
        <translation>%2 में से %1 स्थिरांक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="383"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>ControlScriptView</name>
    <message>
        <source>Control Script</source>
        <translation type="vanished">कंट्रोल स्क्रिप्ट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="33"/>
        <source>Control Loop</source>
        <translation>नियंत्रण लूप</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="45"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="168"/>
        <source>Undo</source>
        <translation>पूर्ववत करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="52"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="179"/>
        <source>Redo</source>
        <translation>पुनः करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="60"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="198"/>
        <source>Cut</source>
        <translation>काटें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="61"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="208"/>
        <source>Copy</source>
        <translation>कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="62"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="218"/>
        <source>Paste</source>
        <translation>पेस्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="67"/>
        <source>Select All</source>
        <translation>सभी चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="77"/>
        <source>Format Document</source>
        <translation>दस्तावेज़ फ़ॉर्मेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="84"/>
        <source>Format Selection</source>
        <translation>चयन फ़ॉर्मेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="148"/>
        <source>Reset</source>
        <translation>रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="153"/>
        <source>Reset to the default control loop</source>
        <translation>डिफ़ॉल्ट नियंत्रण लूप पर रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="163"/>
        <source>Import a control loop file</source>
        <translation>नियंत्रण लूप फ़ाइल आयात करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="241"/>
        <source>Open the control loop documentation</source>
        <translation>नियंत्रण लूप दस्तावेज़ीकरण खोलें</translation>
    </message>
    <message>
        <source>Reset to the default control script</source>
        <translation type="vanished">डिफ़ॉल्ट नियंत्रण स्क्रिप्ट पर रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="158"/>
        <source>Open</source>
        <translation>खोलें</translation>
    </message>
    <message>
        <source>Import a control script file</source>
        <translation type="vanished">नियंत्रण स्क्रिप्ट फ़ाइल आयात करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="173"/>
        <source>Undo the last code edit</source>
        <translation>अंतिम कोड संपादन पूर्ववत करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="185"/>
        <source>Redo the previously undone edit</source>
        <translation>पहले पूर्ववत किया गया संपादन फिर से करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="203"/>
        <source>Cut selected code to clipboard</source>
        <translation>चयनित कोड को क्लिपबोर्ड पर कट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="213"/>
        <source>Copy selected code to clipboard</source>
        <translation>चयनित कोड को क्लिपबोर्ड पर कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="222"/>
        <source>Paste code from clipboard</source>
        <translation>क्लिपबोर्ड से कोड पेस्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="236"/>
        <source>Help</source>
        <translation>सहायता</translation>
    </message>
    <message>
        <source>Open the control script documentation</source>
        <translation type="vanished">नियंत्रण स्क्रिप्ट दस्तावेज़ीकरण खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="251"/>
        <source>Validate</source>
        <translation>सत्यापित करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="255"/>
        <source>Verify that the script compiles correctly</source>
        <translation>सत्यापित करें कि स्क्रिप्ट सही ढंग से कंपाइल होती है</translation>
    </message>
</context>
<context>
    <name>CrashRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="31"/>
        <source>Recovery Options</source>
        <translation>रिकवरी विकल्प</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="57"/>
        <source>Serial Studio has closed unexpectedly several times in a row.</source>
        <translation>Serial Studio कई बार अप्रत्याशित रूप से बंद हो गया है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="82"/>
        <source>Consecutive crashes: %1</source>
        <translation>लगातार क्रैश: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="90"/>
        <source>Last reported stage: %1</source>
        <translation>अंतिम रिपोर्ट किया गया चरण: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="98"/>
        <source>Detected at: %1</source>
        <translation>पता चला समय: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="113"/>
        <source>Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.</source>
        <translation>कोई रिकवरी कार्रवाई चुनें। इसे लागू करने के बाद Serial Studio बंद हो जाएगा ताकि अगली बार लॉन्च साफ़ रूप से शुरू हो सके।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="127"/>
        <source>Reset Rendering Backend to Default</source>
        <translation>रेंडरिंग बैकएंड को डिफ़ॉल्ट पर रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="135"/>
        <source>Skip Restoring the Last Opened Project</source>
        <translation>अंतिम खुले प्रोजेक्ट को पुनर्स्थापित करना छोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="142"/>
        <source>Reset all Preferences</source>
        <translation>सभी प्राथमिकताएँ रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="160"/>
        <source>Continue Anyway</source>
        <translation>फिर भी जारी रखें</translation>
    </message>
</context>
<context>
    <name>CsvPlayer</name>
    <message>
        <location filename="../../qml/Dialogs/CsvPlayer.qml" line="36"/>
        <source>CSV Player</source>
        <translation>CSV प्लेयर</translation>
    </message>
</context>
<context>
    <name>DBCPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="44"/>
        <source>DBC File Preview</source>
        <translation>DBC फ़ाइल पूर्वावलोकन</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="169"/>
        <source>DBC File: %1</source>
        <translation>DBC फ़ाइल: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="177"/>
        <source>Review the CAN messages and signals to import into a new Serial Studio project.</source>
        <translation>नए Serial Studio प्रोजेक्ट में आयात करने के लिए CAN संदेशों और सिग्नल की समीक्षा करें।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="185"/>
        <source>Messages</source>
        <translation>संदेश</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="219"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="229"/>
        <source>Message Name</source>
        <translation>संदेश नाम</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="235"/>
        <source>CAN ID</source>
        <translation>CAN ID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="242"/>
        <source>Signals</source>
        <translation>सिग्नल</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="323"/>
        <source>No messages found in DBC file.</source>
        <translation>DBC फ़ाइल में कोई संदेश नहीं मिला।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="341"/>
        <source>Total: %1 messages, %2 signals</source>
        <translation>कुल: %1 संदेश, %2 सिग्नल</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="348"/>
        <source>Cancel</source>
        <translation>रद्द करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="359"/>
        <source>Create Project</source>
        <translation>प्रोजेक्ट बनाएँ</translation>
    </message>
</context>
<context>
    <name>Dashboard</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard.qml" line="262"/>
        <source>Dashboard %1</source>
        <translation>डैशबोर्ड %1</translation>
    </message>
</context>
<context>
    <name>DashboardButton</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="40"/>
        <source>Send</source>
        <translation>भेजें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="64"/>
        <source>No transmit function defined</source>
        <translation>कोई ट्रांसमिट फ़ंक्शन परिभाषित नहीं है</translation>
    </message>
</context>
<context>
    <name>DashboardCanvas</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="326"/>
        <source>Set Wallpaper…</source>
        <translation>वॉलपेपर सेट करें…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="332"/>
        <source>Clear Wallpaper</source>
        <translation>वॉलपेपर हटाएँ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="342"/>
        <source>Tile Windows</source>
        <translation>विंडो टाइल करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="361"/>
        <source>Pro features detected in this project.</source>
        <translation>इस प्रोजेक्ट में Pro सुविधाएँ पाई गईं।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="363"/>
        <source>Fallback widgets are active. Purchase a license for full functionality.</source>
        <translation>फ़ॉलबैक विजेट सक्रिय हैं। पूर्ण कार्यक्षमता के लिए लाइसेंस खरीदें।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="504"/>
        <source>Empty Workspace</source>
        <translation>खाली कार्यस्थान</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="518"/>
        <source>Use the search bar to find and add widgets, or right-click a widget in another workspace to add it here.</source>
        <translation>विजेट खोजने और जोड़ने के लिए खोज बार का उपयोग करें, या किसी अन्य कार्यस्थान में विजेट पर राइट-क्लिक करके इसे यहाँ जोड़ें।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="533"/>
        <source>Search Widgets</source>
        <translation>विजेट खोजें</translation>
    </message>
</context>
<context>
    <name>DashboardLayout</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="37"/>
        <source>Dashboard</source>
        <translation>डैशबोर्ड</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="215"/>
        <source>API Server Active (%1)</source>
        <translation>API सर्वर सक्रिय (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="216"/>
        <source>API Server Ready</source>
        <translation>API सर्वर तैयार</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="217"/>
        <source>API Server Off</source>
        <translation>API सर्वर बंद</translation>
    </message>
</context>
<context>
    <name>DashboardOutputPanel</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="155"/>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="307"/>
        <source>Send</source>
        <translation>भेजें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="295"/>
        <source>Enter command…</source>
        <translation>कमांड दर्ज करें…</translation>
    </message>
</context>
<context>
    <name>DashboardSlider</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardSlider.qml" line="90"/>
        <source>No transmit function defined</source>
        <translation>कोई ट्रांसमिट फ़ंक्शन परिभाषित नहीं है</translation>
    </message>
</context>
<context>
    <name>DashboardTextField</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="47"/>
        <source>Enter command…</source>
        <translation>कमांड दर्ज करें…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="57"/>
        <source>Send</source>
        <translation>भेजें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="76"/>
        <source>No transmit function defined</source>
        <translation>कोई ट्रांसमिट फ़ंक्शन परिभाषित नहीं है</translation>
    </message>
</context>
<context>
    <name>DashboardToggle</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="57"/>
        <source>ON</source>
        <translation>ON</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="59"/>
        <source>OFF</source>
        <translation>OFF</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="70"/>
        <source>No transmit function defined</source>
        <translation>कोई ट्रांसमिट फ़ंक्शन परिभाषित नहीं है</translation>
    </message>
</context>
<context>
    <name>DataGrid</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Pause</source>
        <translation>रोकें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Resume</source>
        <translation>फिर से शुरू करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="313"/>
        <source>Awaiting data…</source>
        <translation>डेटा की प्रतीक्षा में…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="368"/>
        <source>Open %1 in a separate window</source>
        <translation>%1 को अलग विंडो में खोलें</translation>
    </message>
</context>
<context>
    <name>DataModel::ControlScriptEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="289"/>
        <source>Select Javascript file to import</source>
        <translation>आयात करने के लिए Javascript फ़ाइल चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="357"/>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="368"/>
        <source>Code Validation Failed</source>
        <translation>कोड सत्यापन विफल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="358"/>
        <source>Line %1: %2</source>
        <translation>पंक्ति %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="369"/>
        <source>The script must define a setup() and/or loop() function.</source>
        <translation>स्क्रिप्ट में setup() और/या loop() फ़ंक्शन परिभाषित होना चाहिए।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="374"/>
        <source>Code Validation Successful</source>
        <translation>कोड सत्यापन सफल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="375"/>
        <source>No syntax errors detected in the control loop.</source>
        <translation>नियंत्रण लूप में कोई सिंटैक्स त्रुटि नहीं मिली।</translation>
    </message>
    <message>
        <source>No syntax errors detected in the control script.</source>
        <translation type="vanished">नियंत्रण स्क्रिप्ट में कोई सिंटैक्स त्रुटि नहीं मिली।</translation>
    </message>
</context>
<context>
    <name>DataModel::DBCImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>Import DBC File</source>
        <translation>DBC फ़ाइल इम्पोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>DBC Files (*.dbc);;All Files (*)</source>
        <translation>DBC फ़ाइलें (*.DBC);;सभी फ़ाइलें (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="160"/>
        <source>Failed to parse DBC file: %1</source>
        <translation>DBC फ़ाइल पार्स करने में विफल: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="161"/>
        <source>Verify the file format and try again.</source>
        <translation>फ़ाइल फ़ॉर्मेट सत्यापित करें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="163"/>
        <source>DBC Import Error</source>
        <translation>DBC इम्पोर्ट त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="171"/>
        <source>DBC file contains no messages</source>
        <translation>DBC फ़ाइल में कोई मैसेज नहीं है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="172"/>
        <source>The selected file does not contain any CAN message definitions.</source>
        <translation>चयनित फ़ाइल में कोई CAN मैसेज परिभाषा नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="174"/>
        <source>DBC Import Warning</source>
        <translation>DBC इम्पोर्ट चेतावनी</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="269"/>
        <source>Overview</source>
        <translation>अवलोकन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="326"/>
        <source>Active</source>
        <translation>सक्रिय</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">अस्थायी प्रोजेक्ट फ़ाइल बनाने में विफल</translation>
    </message>
    <message>
        <source>Check if the application has write permissions to the temporary directory.</source>
        <translation type="vanished">जाँचें कि एप्लिकेशन के पास अस्थायी डायरेक्टरी में लिखने की अनुमति है या नहीं।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="225"/>
        <source>Successfully imported DBC file with %1 messages and %2 signals.</source>
        <translation>DBC फ़ाइल सफलतापूर्वक आयात की गई: %1 संदेश और %2 सिग्नल।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="218"/>
        <source>The project editor is now open for customization.</source>
        <translation>प्रोजेक्ट एडिटर अब कस्टमाइज़ेशन के लिए खुला है।</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">आयातित प्रोजेक्ट लोड करने में विफल</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">जेनरेट किया गया प्रोजेक्ट JSON लोड नहीं हो सका।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="220"/>
        <source> Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); only simple multiplexing is supported.</source>
        <translation>%1 सिग्नल छोड़े गए जो विस्तारित मल्टीप्लेक्सिंग (SG_MUL_VAL_) का उपयोग करते हैं; केवल सरल मल्टीप्लेक्सिंग समर्थित है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="230"/>
        <source>DBC Import Complete</source>
        <translation>DBC इम्पोर्ट पूर्ण</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="254"/>
        <source>CAN Bus</source>
        <translation>CAN Bus</translation>
    </message>
</context>
<context>
    <name>DataModel::DatasetTransformEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="60"/>
        <source>Dataset Value Transform</source>
        <translation>डेटासेट मान ट्रांसफ़ॉर्म</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="97"/>
        <source>Lua</source>
        <translation>Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="97"/>
        <source>JavaScript</source>
        <translation>JavaScript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="121"/>
        <source>Language:</source>
        <translation>भाषा:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="124"/>
        <source>Template:</source>
        <translation>टेम्पलेट:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="103"/>
        <source>Enter raw value (e.g., 1024)</source>
        <translation>कच्चा मान दर्ज करें (उदा., 1024)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="108"/>
        <source>Test</source>
        <translation>परीक्षण करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="109"/>
        <source>Clear</source>
        <translation>साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="135"/>
        <source>Input:</source>
        <translation>इनपुट:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="138"/>
        <source>Output:</source>
        <translation>आउटपुट:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="111"/>
        <source>Apply</source>
        <translation>लागू करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="112"/>
        <source>Cancel</source>
        <translation>रद्द करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="216"/>
        <source>Transform — %1</source>
        <translation>ट्रांसफ़ॉर्म — %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="298"/>
        <source>Enter a value</source>
        <translation>मान दर्ज करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="305"/>
        <source>Invalid number</source>
        <translation>अमान्य संख्या</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="374"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>दस्तावेज़ फ़ॉर्मेट करें	Ctrl+Shift+I</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="375"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>चयन फ़ॉर्मेट करें	Ctrl+I</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="486"/>
        <source>--
-- Define a transform(value) function that receives the live
-- dataset reading and returns a transformed number. If no
-- transform() function is defined, the raw value is kept
-- and nothing is saved with the project.
--
-- Example:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- Globals declared at the top level persist between frames,
-- which is useful for filters, accumulators, and any other
-- stateful transform, just like a frame parser script:
--
--    local alpha = 0.1
--    local ema
--
--    function transform(value)
--      ema = ema or value
--      ema = alpha * value + (1 - alpha) * ema
--      return ema
--    end
--
-- Use the Template dropdown for ready-made examples, or
-- click Test to try your function.
--
</source>
        <translation>--
-- एक transform(value) फ़ंक्शन परिभाषित करें जो लाइव
-- डेटासेट रीडिंग प्राप्त करता है और एक ट्रांसफ़ॉर्म की गई संख्या
-- लौटाता है। यदि कोई transform() फ़ंक्शन परिभाषित नहीं है,
-- तो रॉ मान रखा जाता है और प्रोजेक्ट के साथ कुछ भी सेव नहीं होता।
--
-- उदाहरण:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- शीर्ष स्तर पर घोषित ग्लोबल्स फ़्रेम के बीच बने रहते हैं,
-- जो फ़िल्टर, एक्युमुलेटर और किसी भी अन्य स्टेटफ़ुल
-- ट्रांसफ़ॉर्म के लिए उपयोगी है, बिल्कुल फ़्रेम पार्सर
-- स्क्रिप्ट की तरह:
--
--    local alpha = 0.1
--    local ema
--
--    function transform(value)
--      ema = ema or value
--      ema = alpha * value + (1 - alpha) * ema
--      return ema
--    end
--
-- तैयार उदाहरणों के लिए Template ड्रॉपडाउन का उपयोग करें,
-- या अपने फ़ंक्शन को आज़माने के लिए परीक्षण करें पर क्लिक करें।
--</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="514"/>
        <source>/*
 * Define a transform(value) function that receives the live
 * dataset reading and returns a transformed number. If no
 * transform() function is defined, the raw value is kept
 * and nothing is saved with the project.
 *
 * Example:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Globals declared at the top level persist between frames,
 * which is useful for filters, accumulators, and any other
 * stateful transform -- just like a frame parser script:
 *
 *   var alpha = 0.1;
 *   var ema;
 *
 *   function transform(value) {
 *     if (ema === undefined) ema = value;
 *     ema = alpha * value + (1 - alpha) * ema;
 *     return ema;
 *   }
 *
 * Use the Template dropdown for ready-made examples, or
 * click Test to try your function.
 */</source>
        <translation>/*
 * एक transform(value) फ़ंक्शन परिभाषित करें जो लाइव
 * डेटासेट रीडिंग प्राप्त करता है और एक रूपांतरित संख्या लौटाता है। यदि कोई
 * transform() फ़ंक्शन परिभाषित नहीं है, तो कच्चा मान रखा जाता है
 * और प्रोजेक्ट के साथ कुछ भी सहेजा नहीं जाता।
 *
 * उदाहरण:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * शीर्ष स्तर पर घोषित ग्लोबल्स फ़्रेम के बीच बने रहते हैं,
 * जो फ़िल्टर, संचयक और किसी भी अन्य
 * स्टेटफुल रूपांतरण के लिए उपयोगी है -- बिल्कुल फ़्रेम पार्सर स्क्रिप्ट की तरह:
 *
 *   var alpha = 0.1;
 *   var ema;
 *
 *   function transform(value) {
 *     if (ema === undefined) ema = value;
 *     ema = alpha * value + (1 - alpha) * ema;
 *     return ema;
 *   }
 *
 * तैयार उदाहरणों के लिए टेम्पलेट ड्रॉपडाउन का उपयोग करें, या
 * अपने फ़ंक्शन को आज़माने के लिए टेस्ट पर क्लिक करें।
 */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="757"/>
        <source>Select Template…</source>
        <translation>टेम्पलेट चुनें…</translation>
    </message>
    <message>
        <source>/*
 * Define a transform(value) function that receives the live
 * dataset reading and returns a transformed number. If no
 * transform() function is defined, the raw value is kept
 * and nothing is saved with the project.
 *
 * Example:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Globals declared at the top level persist between frames,
 * which is useful for filters, accumulators, and any other
 * stateful transform — just like a frame parser script:
 *
 *   var alpha = 0.1;
 *   var ema;
 *
 *   function transform(value) {
 *     if (ema === undefined) ema = value;
 *     ema = alpha * value + (1 - alpha) * ema;
 *     return ema;
 *   }
 *
 * Use the Template dropdown for ready-made examples, or
 * click Test to try your function.
 */</source>
        <translation type="vanished">/*
 * एक transform(value) फ़ंक्शन परिभाषित करें जो लाइव
 * डेटासेट रीडिंग प्राप्त करता है और एक रूपांतरित संख्या
 * लौटाता है। यदि कोई transform() फ़ंक्शन परिभाषित नहीं
 * है, तो कच्चा मान रखा जाता है और प्रोजेक्ट के साथ कुछ
 * भी सहेजा नहीं जाता।
 *
 * उदाहरण:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * शीर्ष स्तर पर घोषित ग्लोबल्स फ़्रेम के बीच बने रहते हैं,
 * जो फ़िल्टर, एक्युमुलेटर और किसी भी अन्य स्टेटफुल
 * ट्रांसफ़ॉर्म के लिए उपयोगी है — बिल्कुल फ़्रेम पार्सर
 * स्क्रिप्ट की तरह:
 *
 *   var alpha = 0.1;
 *   var ema;
 *
 *   function transform(value) {
 *     if (ema === undefined) ema = value;
 *     ema = alpha * value + (1 - alpha) * ema;
 *     return ema;
 *   }
 *
 * तैयार उदाहरणों के लिए टेम्पलेट ड्रॉपडाउन का उपयोग करें,</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="648"/>
        <source>Engine error</source>
        <translation>इंजन त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="671"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="684"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="701"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="712"/>
        <source>Error: %1</source>
        <translation>त्रुटि: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="677"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="705"/>
        <source>Error: transform() not defined</source>
        <translation>त्रुटि: transform() परिभाषित नहीं है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="689"/>
        <source>Error: transform() must return a number</source>
        <translation>त्रुटि: transform() को एक संख्या लौटानी चाहिए</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameBuilder</name>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1622"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1736"/>
        <source>Channel %1</source>
        <translation>चैनल %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1747"/>
        <source>Audio Input</source>
        <translation>ऑडियो इनपुट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1631"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1752"/>
        <source>Quick Plot</source>
        <translation>क्विक प्लॉट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1348"/>
        <source>JavaScript transform exceeded budget</source>
        <translation>JavaScript ट्रांसफॉर्म बजट सीमा पार हो गई</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1349"/>
        <source>A dataset transform took longer than %1 ms; remaining datasets in the frame fell back to raw values until the next frame. Profile or simplify the transform code.</source>
        <translation>एक डेटासेट ट्रांसफॉर्म को %1 मि.से. से अधिक समय लगा; फ्रेम के बाकी डेटासेट अगले फ्रेम तक कच्चे मानों पर लौट आए। ट्रांसफॉर्म कोड को प्रोफाइल या सरल करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="199"/>
        <source>Frame pool exhausted</source>
        <translation>फ्रेम पूल समाप्त हो गया</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="201"/>
        <source>A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API subscriber) is not draining frames fast enough. Serial Studio is falling back to per-frame allocations until the backlog clears. Disable a heavy consumer or reduce the data rate.</source>
        <translation>एक डाउनस्ट्रीम उपभोक्ता (डैशबोर्ड, CSV/MDF4 निर्यात, सत्र DB, या API सब्सक्राइबर) फ्रेम को पर्याप्त तेजी से नहीं निकाल रहा है। Serial Studio अब बैकलॉग साफ़ होने तक प्रति-फ्रेम आवंटन पर वापस जा रहा है। भारी उपभोक्ता को अक्षम करें या डेटा दर कम करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1583"/>
        <source>Device A</source>
        <translation>डिवाइस A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1638"/>
        <source>Quick Plot Data</source>
        <translation>क्विक प्लॉट डेटा</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1650"/>
        <source>Multiple Plots</source>
        <translation>मल्टीपल प्लॉट</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserModel</name>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Plain text (UTF-8)</source>
        <translation>सादा टेक्स्ट (UTF-8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Hexadecimal</source>
        <translation>हेक्साडेसिमल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Binary (raw bytes)</source>
        <translation>बाइनरी (रॉ बाइट्स)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="265"/>
        <source>End delimiter only</source>
        <translation>केवल अंत डिलीमिटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="266"/>
        <source>Start + end delimiters</source>
        <translation>प्रारंभ + अंत डिलीमिटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="267"/>
        <source>Start delimiter only</source>
        <translation>केवल प्रारंभ डिलीमिटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="268"/>
        <source>No delimiters (whole chunk)</source>
        <translation>कोई डिलीमिटर नहीं (पूरा चंक)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="279"/>
        <source>No Checksum</source>
        <translation>कोई चेकसम नहीं</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="310"/>
        <source>Select Frame Parser Template</source>
        <translation>फ़्रेम पार्सर टेम्पलेट चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="311"/>
        <source>Choose a template to load:</source>
        <translation>लोड करने के लिए टेम्पलेट चुनें:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="494"/>
        <source>Invalid hexadecimal input.</source>
        <translation>अमान्य हेक्साडेसिमल इनपुट।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="521"/>
        <source>No template selected.</source>
        <translation>कोई टेम्पलेट चयनित नहीं।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="561"/>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation>%1 फ़्रेम एक्सट्रैक्ट किए गए | %2 बाइट उपयोग किए गए | %3 बाइट बफ़र किए गए | %4 छोड़े गए</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="632"/>
        <source>Invalid JSON: %1</source>
        <translation>अमान्य JSON: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="728"/>
        <source>Parameters</source>
        <translation>पैरामीटर</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserTestDialog</name>
    <message>
        <source>None</source>
        <translation type="vanished">कोई नहीं</translation>
    </message>
    <message>
        <source>Invalid Hex Input</source>
        <translation type="vanished">अमान्य Hex इनपुट</translation>
    </message>
    <message>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation type="vanished">कृपया मान्य हेक्साडेसिमल बाइट्स दर्ज करें।

मान्य प्रारूप: 01 A2 FF 3C</translation>
    </message>
    <message>
        <source>(no frames)</source>
        <translation type="vanished">(कोई फ़्रेम नहीं)</translation>
    </message>
    <message>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation type="vanished">एक्सट्रैक्शन से पूर्ण फ़्रेम नहीं बना। स्टार्ट / एंड डिलिमिटर और डिटेक्शन मोड की जाँच करें।</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 फ़्रेम एक्सट्रैक्ट किए गए | %2 बाइट उपयोग किए गए | %3 बाइट बफ़र किए गए | %4 छोड़े गए</translation>
    </message>
    <message>
        <source>Pipeline Configuration</source>
        <translation type="vanished">पाइपलाइन कॉन्फ़िगरेशन</translation>
    </message>
    <message>
        <source>Pipeline Results</source>
        <translation type="vanished">पाइपलाइन परिणाम</translation>
    </message>
    <message>
        <source>Detection</source>
        <translation type="vanished">डिटेक्शन</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">डिकोडर</translation>
    </message>
    <message>
        <source>Checksum</source>
        <translation type="vanished">चेकसम</translation>
    </message>
    <message>
        <source>Start Delimiter</source>
        <translation type="vanished">स्टार्ट डिलीमिटर</translation>
    </message>
    <message>
        <source>End Delimiter</source>
        <translation type="vanished">एंड डिलीमिटर</translation>
    </message>
    <message>
        <source>Hex Delimiters</source>
        <translation type="vanished">हेक्स डिलीमिटर</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">केवल अंत डिलीमिटर</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">प्रारंभ + अंत डिलीमिटर</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">केवल प्रारंभ डिलीमिटर</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">कोई डिलीमिटर नहीं (पूरा चंक)</translation>
    </message>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">सादा टेक्स्ट (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">हेक्साडेसिमल</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">बाइनरी (रॉ बाइट्स)</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">साफ़ करें</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">मूल्यांकन करें</translation>
    </message>
    <message>
        <source>Enter raw stream bytes here...</source>
        <translation type="vanished">यहाँ रॉ स्ट्रीम बाइट्स दर्ज करें...</translation>
    </message>
    <message>
        <source>Stage</source>
        <translation type="vanished">स्टेज</translation>
    </message>
    <message>
        <source>Frame Data Input</source>
        <translation type="vanished">फ़्रेम डेटा इनपुट</translation>
    </message>
    <message>
        <source>Frame Parser Results</source>
        <translation type="vanished">फ़्रेम पार्सर परिणाम</translation>
    </message>
    <message>
        <source>Enter frame data here…</source>
        <translation type="vanished">यहाँ फ़्रेम डेटा दर्ज करें…</translation>
    </message>
    <message>
        <source>Dataset Index</source>
        <translation type="vanished">डेटासेट इंडेक्स</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="vanished">मान</translation>
    </message>
    <message>
        <source>Enter frame data above, enable HEX mode if needed, then click "Evaluate" to run the frame parser.

Example (Text): a,b,c,d,e,f
Example (HEX):  48 65 6C 6C 6F</source>
        <translation type="vanished">ऊपर फ़्रेम डेटा दर्ज करें, आवश्यकता हो तो HEX मोड सक्षम करें, फिर फ़्रेम पार्सर चलाने के लिए "मूल्यांकन करें" पर क्लिक करें।

उदाहरण (Text): a,b,c,d,e,f
उदाहरण (HEX):  48 65 6C 6C 6F</translation>
    </message>
    <message>
        <source>Test Frame Parser</source>
        <translation type="vanished">फ़्रेम पार्सर परीक्षण करें</translation>
    </message>
    <message>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation type="vanished">हेक्स बाइट्स दर्ज करें (उदा., 01 A2 FF)</translation>
    </message>
    <message>
        <source>(empty)</source>
        <translation type="vanished">(खाली)</translation>
    </message>
    <message>
        <source>No data returned</source>
        <translation type="vanished">कोई डेटा नहीं मिला</translation>
    </message>
</context>
<context>
    <name>DataModel::JsCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="219"/>
        <source>Change Scripting Language?</source>
        <translation>स्क्रिप्टिंग भाषा बदलें?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="220"/>
        <source>Switching the scripting language replaces the current parser code with the equivalent template in the new language.

Any unsaved changes are lost. Continue?</source>
        <translation>स्क्रिप्टिंग भाषा बदलने से वर्तमान पार्सर कोड नई भाषा के समकक्ष टेम्पलेट से बदल जाता है।

कोई भी असहेजित परिवर्तन खो जाएगा। जारी रखें?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Javascript file to import</source>
        <translation>आयात करने के लिए Javascript फ़ाइल चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Lua file to import</source>
        <translation>आयात करने के लिए Lua फ़ाइल चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="414"/>
        <source>Code Validation Successful</source>
        <translation>कोड सत्यापन सफल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="415"/>
        <source>No syntax errors detected in the parser code.</source>
        <translation>पार्सर कोड में कोई सिंटैक्स त्रुटि नहीं मिली।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="525"/>
        <source>Select Frame Parser Template</source>
        <translation>फ़्रेम पार्सर टेम्पलेट चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="526"/>
        <source>Choose a template to load:</source>
        <translation>लोड करने के लिए टेम्पलेट चुनें:</translation>
    </message>
</context>
<context>
    <name>DataModel::ModbusMapImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="299"/>
        <source>Import Modbus Register Map</source>
        <translation>Modbus रजिस्टर मैप आयात करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="303"/>
        <source>Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files (*.xml);;JSON Files (*.json);;All Files (*)</source>
        <translation>Modbus रजिस्टर मैप (*.CSV *.XML *.JSON);;CSV फ़ाइलें (*.CSV);;XML फ़ाइलें (*.XML);;JSON फ़ाइलें (*.JSON);;सभी फ़ाइलें (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="340"/>
        <source>No registers found</source>
        <translation>कोई रजिस्टर नहीं मिला</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="341"/>
        <source>The file could not be parsed or contains no register definitions.</source>
        <translation>फ़ाइल पार्स नहीं की जा सकी या इसमें कोई रजिस्टर परिभाषा नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="343"/>
        <source>Modbus Import</source>
        <translation>Modbus आयात</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="787"/>
        <source>Overview</source>
        <translation>अवलोकन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="813"/>
        <source>On</source>
        <translation>चालू</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">आयातित प्रोजेक्ट लोड करने में विफल</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">जेनरेट किया गया प्रोजेक्ट JSON लोड नहीं हो सका।</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">अस्थायी प्रोजेक्ट फ़ाइल बनाने में विफल</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">अस्थायी डायरेक्टरी में लिखने की अनुमति जाँचें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="388"/>
        <source>Successfully imported %1 registers in %2 groups.</source>
        <translation>%2 समूहों में %1 रजिस्टर सफलतापूर्वक आयात किए गए।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="390"/>
        <source>The project editor is now open for customization.</source>
        <translation>प्रोजेक्ट एडिटर अब अनुकूलन के लिए खुला है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="392"/>
        <source>Modbus Import Complete</source>
        <translation>Modbus आयात पूर्ण</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="728"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
</context>
<context>
    <name>DataModel::NativeParserEditor</name>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">सादा टेक्स्ट (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">हेक्साडेसिमल</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">बाइनरी (रॉ बाइट्स)</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">केवल अंत डिलीमिटर</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">प्रारंभ + अंत डिलीमिटर</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">केवल प्रारंभ डिलीमिटर</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">कोई डिलीमिटर नहीं (पूरा चंक)</translation>
    </message>
    <message>
        <source>No Checksum</source>
        <translation type="vanished">कोई चेकसम नहीं</translation>
    </message>
    <message>
        <source>Invalid hexadecimal input.</source>
        <translation type="vanished">अमान्य हेक्साडेसिमल इनपुट।</translation>
    </message>
    <message>
        <source>No template selected.</source>
        <translation type="vanished">कोई टेम्पलेट चयनित नहीं।</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 फ़्रेम एक्सट्रैक्ट किए गए | %2 बाइट उपयोग किए गए | %3 बाइट बफ़र किए गए | %4 छोड़े गए</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation type="vanished">पैरामीटर</translation>
    </message>
</context>
<context>
    <name>DataModel::OutputCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="288"/>
        <source>Select Javascript file to import</source>
        <translation>आयात करने के लिए Javascript फ़ाइल चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="346"/>
        <source>Select Output Widget Template</source>
        <translation>आउटपुट विजेट टेम्पलेट चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="347"/>
        <source>Choose a template to load:</source>
        <translation>लोड करने के लिए टेम्पलेट चुनें:</translation>
    </message>
</context>
<context>
    <name>DataModel::PainterCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="300"/>
        <source>Select Javascript file to import</source>
        <translation>आयात करने के लिए Javascript फ़ाइल चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="387"/>
        <source>Select Painter Widget Template</source>
        <translation>पेंटर विजेट टेम्पलेट चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="388"/>
        <source>Choose a template to load:</source>
        <translation>लोड करने के लिए टेम्पलेट चुनें:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="432"/>
        <source>Add datasets for this template?</source>
        <translation>इस टेम्पलेट के लिए डेटासेट जोड़ें?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="433"/>
        <source>"%1" expects %2 dataset(s); the current group has %3.

Add %4 dataset(s) using the template's defaults?</source>
        <translation>"%1" को %2 डेटासेट की आवश्यकता है; वर्तमान समूह में %3 हैं।

टेम्पलेट के डिफ़ॉल्ट का उपयोग करके %4 डेटासेट जोड़ें?</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectEditor</name>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2187"/>
        <source>Project Information</source>
        <translation>प्रोजेक्ट जानकारी</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2197"/>
        <source>Project Title</source>
        <translation>प्रोजेक्ट शीर्षक</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2198"/>
        <source>Untitled Project</source>
        <translation>अनाम प्रोजेक्ट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2199"/>
        <source>Name or description of the project</source>
        <translation>प्रोजेक्ट का नाम या विवरण</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2330"/>
        <source>Time</source>
        <translation>समय</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2330"/>
        <source>Samples</source>
        <translation>सैंपल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2342"/>
        <source>Plot every curve against time or against the sample number</source>
        <translation>प्रत्येक वक्र को समय या सैंपल संख्या के विरुद्ध प्लॉट करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2358"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2360"/>
        <source>Web address to load in this widget</source>
        <translation>इस विजेट में लोड करने के लिए वेब पता</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2468"/>
        <source>Frame Detection</source>
        <translation>फ्रेम डिटेक्शन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2482"/>
        <source>Frame Detection Method</source>
        <translation>फ्रेम डिटेक्शन विधि</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2483"/>
        <source>Select how incoming data frames are identified</source>
        <translation>आने वाले डेटा फ्रेम की पहचान कैसे की जाती है चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2493"/>
        <source>Hexadecimal Delimiters</source>
        <translation>हेक्साडेसिमल डिलीमिटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2494"/>
        <source>Enter frame start/end sequences as hexadecimal values</source>
        <translation>फ्रेम स्टार्ट/एंड सीक्वेंस हेक्साडेसिमल वैल्यू के रूप में दर्ज करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2510"/>
        <source>Frame Start Delimiter</source>
        <translation>फ्रेम स्टार्ट डिलीमिटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2511"/>
        <source>e.g. /*</source>
        <translation>उदा. /*</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2512"/>
        <source>Sequence that marks the beginning of a data frame</source>
        <translation>डेटा फ्रेम की शुरुआत को चिह्नित करने वाला सीक्वेंस</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2524"/>
        <source>Frame End Delimiter</source>
        <translation>फ़्रेम समाप्ति डिलिमिटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2525"/>
        <source>e.g. */</source>
        <translation>उदा. */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2526"/>
        <source>Sequence that marks the end of a data frame</source>
        <translation>डेटा फ़्रेम के अंत को चिह्नित करने वाला अनुक्रम</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2532"/>
        <source>Payload Processing &amp; Validation</source>
        <translation>पेलोड प्रोसेसिंग और सत्यापन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2543"/>
        <source>Data Conversion Method</source>
        <translation>डेटा रूपांतरण विधि</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2544"/>
        <source>Select how incoming binary data is decoded before parsing</source>
        <translation>आने वाले बाइनरी डेटा को पार्स करने से पहले कैसे डिकोड किया जाता है चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2560"/>
        <source>Checksum Algorithm</source>
        <translation>चेकसम एल्गोरिदम</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2561"/>
        <source>Select the checksum algorithm used to validate frames</source>
        <translation>फ़्रेम सत्यापित करने के लिए उपयोग किया जाने वाला चेकसम एल्गोरिदम चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2217"/>
        <source>Group Information</source>
        <translation>समूह जानकारी</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2227"/>
        <source>Group Title</source>
        <translation>समूह शीर्षक</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2228"/>
        <source>Untitled Group</source>
        <translation>शीर्षकहीन समूह</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2229"/>
        <source>Title or description of this dataset group</source>
        <translation>इस डेटासेट समूह का शीर्षक या विवरण</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2400"/>
        <source>Composite Widget</source>
        <translation>कंपोजिट विजेट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2401"/>
        <source>Select how this group of datasets should be visualized (optional)</source>
        <translation>चुनें कि डेटासेट के इस समूह को कैसे विज़ुअलाइज़ किया जाना चाहिए (वैकल्पिक)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2279"/>
        <source>Image Configuration</source>
        <translation>इमेज कॉन्फ़िगरेशन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3140"/>
        <source>Virtual Dataset</source>
        <translation>वर्चुअल डेटासेट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3141"/>
        <source>Virtual datasets compute their value from transforms and data tables, they do not require a frame index</source>
        <translation>वर्चुअल डेटासेट अपना मान ट्रांसफ़ॉर्म और डेटा टेबल से कंप्यूट करते हैं, उन्हें फ़्रेम इंडेक्स की आवश्यकता नहीं होती</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3544"/>
        <source>Fixed decimal places for the value display; overrides the format (-1 = auto)</source>
        <translation>मान प्रदर्शन के लिए निश्चित दशमलव स्थान; प्रारूप को ओवरराइड करता है (-1 = स्वतः)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3673"/>
        <source>Auto-detect</source>
        <translation>स्वतः पहचानें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3673"/>
        <source>Manual Delimiters</source>
        <translation>मैनुअल डिलिमिटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2292"/>
        <source>Detection Mode</source>
        <translation>पहचान मोड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1307"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1310"/>
        <source>Frame Parser</source>
        <translation>फ़्रेम पार्सर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1449"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1450"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1492"/>
        <source>Groups</source>
        <translation>ग्रुप्स</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1522"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1535"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1536"/>
        <source>Shared Memory</source>
        <translation>शेयर्ड मेमोरी</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1522"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1541"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1542"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5059"/>
        <source>Dataset Values</source>
        <translation>डेटासेट वैल्यूज़</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1584"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1597"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1598"/>
        <source>Workspaces</source>
        <translation>वर्कस्पेसेज़</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1636"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1639"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1640"/>
        <source>MQTT Publisher</source>
        <translation>MQTT प्रकाशक</translation>
    </message>
    <message>
        <source>Control Script</source>
        <translation type="vanished">नियंत्रण स्क्रिप्ट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1713"/>
        <source>Publishing</source>
        <translation>प्रकाशन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1723"/>
        <source>Enable Publishing</source>
        <translation>प्रकाशन सक्षम करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1724"/>
        <source>Broadcast frames, raw bytes and notifications to the broker</source>
        <translation>फ़्रेम, रॉ बाइट्स और सूचनाएं ब्रोकर पर प्रसारित करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1735"/>
        <source>Payload</source>
        <translation>पेलोड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1736"/>
        <source>Selects what gets published: parsed dashboard data or raw RX bytes</source>
        <translation>क्या प्रकाशित होगा यह चुनें: पार्स किया गया डैशबोर्ड डेटा या रॉ RX बाइट्स</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1746"/>
        <source>Publish Rate (Hz)</source>
        <translation>प्रकाशन दर (Hz)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1747"/>
        <source>How many times per second to publish (1-30 Hz). Higher rates increase broker load; dashboard data is rate-limited so a slow broker never blocks frame parsing.</source>
        <translation>प्रति सेकंड कितनी बार प्रकाशित करना है (1-30 Hz)। उच्च दर से ब्रोकर लोड बढ़ता है; डैशबोर्ड डेटा रेट-लिमिटेड है इसलिए धीमा ब्रोकर कभी फ़्रेम पार्सिंग को ब्लॉक नहीं करता।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1759"/>
        <source>Topic Base</source>
        <translation>टॉपिक बेस</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1760"/>
        <source>serial-studio/device</source>
        <translation>serial-studio/device</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1761"/>
        <source>Base topic used for frame and raw-byte publishing</source>
        <translation>फ़्रेम और रॉ-बाइट प्रकाशन के लिए उपयोग किया जाने वाला बेस टॉपिक</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1771"/>
        <source>Script Topic</source>
        <translation>स्क्रिप्ट टॉपिक</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1772"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1796"/>
        <source>Defaults to Topic Base when empty</source>
        <translation>खाली होने पर टॉपिक बेस पर डिफ़ॉल्ट होता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1773"/>
        <source>Topic the user script publishes to</source>
        <translation>टॉपिक जिस पर यूज़र स्क्रिप्ट प्रकाशित करती है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1783"/>
        <source>Publish Notifications</source>
        <translation>नोटिफ़िकेशन प्रकाशित करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1784"/>
        <source>Mirror dashboard notifications to a dedicated topic</source>
        <translation>डैशबोर्ड नोटिफ़िकेशन को एक समर्पित टॉपिक पर मिरर करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1795"/>
        <source>Notification Topic</source>
        <translation>नोटिफ़िकेशन टॉपिक</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1797"/>
        <source>Topic where dashboard notifications are mirrored</source>
        <translation>टॉपिक जहाँ डैशबोर्ड नोटिफ़िकेशन मिरर किए जाते हैं</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1810"/>
        <source>Broker</source>
        <translation>Broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1820"/>
        <source>Hostname</source>
        <translation>होस्टनेम</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1821"/>
        <source>broker.hivemq.com</source>
        <translation>broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1822"/>
        <source>Hostname or IP address of the MQTT broker</source>
        <translation>MQTT ब्रोकर का होस्टनेम या IP एड्रेस</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1831"/>
        <source>Port</source>
        <translation>पोर्ट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1832"/>
        <source>TCP port exposed by the broker (1883 plain, 8883 TLS)</source>
        <translation>ब्रोकर द्वारा एक्सपोज़ किया गया TCP पोर्ट (1883 प्लेन, 8883 TLS)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1842"/>
        <source>Custom Client ID</source>
        <translation>कस्टम क्लाइंट ID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1844"/>
        <source>Off: a fresh random id is generated on every project load. On: use the id below.</source>
        <translation>बंद: हर प्रोजेक्ट लोड पर एक नया रैंडम ID जनरेट होता है। चालू: नीचे दिया गया ID उपयोग करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1855"/>
        <source>Client ID</source>
        <translation>क्लाइंट ID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1856"/>
        <source>Identifier sent to the broker on CONNECT</source>
        <translation>CONNECT पर ब्रोकर को भेजा जाने वाला आइडेंटिफ़ायर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1869"/>
        <source>Protocol Version</source>
        <translation>प्रोटोकॉल संस्करण</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1870"/>
        <source>MQTT protocol revision used on CONNECT</source>
        <translation>CONNECT पर उपयोग किया गया MQTT प्रोटोकॉल संशोधन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1879"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1880"/>
        <source>Seconds between PINGREQ packets when idle</source>
        <translation>निष्क्रिय होने पर PINGREQ पैकेट के बीच सेकंड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1889"/>
        <source>Clean Session</source>
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1890"/>
        <source>Discard any persistent session state on CONNECT</source>
        <translation>CONNECT पर किसी भी स्थायी सत्र स्थिति को त्यागें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1905"/>
        <source>Username</source>
        <translation>उपयोगकर्ता नाम</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1906"/>
        <source>Username for broker authentication (leave empty for anonymous)</source>
        <translation>ब्रोकर प्रमाणीकरण के लिए उपयोगकर्ता नाम (अनाम के लिए खाली छोड़ें)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1916"/>
        <source>Password</source>
        <translation>पासवर्ड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1917"/>
        <source>Password for broker authentication</source>
        <translation>ब्रोकर प्रमाणीकरण के लिए पासवर्ड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1928"/>
        <source>SSL / TLS</source>
        <translation>SSL / TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1938"/>
        <source>Use SSL/TLS</source>
        <translation>SSL/TLS उपयोग करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1939"/>
        <source>Tunnel the broker connection over TLS</source>
        <translation>TLS के माध्यम से ब्रोकर कनेक्शन टनल करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1952"/>
        <source>Protocol</source>
        <translation>प्रोटोकॉल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1953"/>
        <source>Negotiated TLS protocol family</source>
        <translation>नेगोशिएटेड TLS प्रोटोकॉल फैमिली</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1963"/>
        <source>Peer Verify</source>
        <translation>पीयर सत्यापन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1964"/>
        <source>How strictly the broker's certificate chain is validated</source>
        <translation>ब्रोकर की सर्टिफिकेट चेन को कितनी सख्ती से मान्य किया जाता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1974"/>
        <source>Verify Depth</source>
        <translation>सत्यापन गहराई</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1975"/>
        <source>Maximum certificate chain length accepted (0 = unlimited)</source>
        <translation>स्वीकृत अधिकतम सर्टिफिकेट चेन लंबाई (0 = असीमित)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2244"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2878"/>
        <source>Device %1</source>
        <translation>डिवाइस %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2262"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2427"/>
        <source>Input Device</source>
        <translation>इनपुट डिवाइस</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2263"/>
        <source>Select which connected device provides data for this group</source>
        <translation>चुनें कि कौन सा कनेक्टेड डिवाइस इस ग्रुप के लिए डेटा प्रदान करता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2294"/>
        <source>Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences</source>
        <translation>स्वतः-पहचान JPEG/PNG मैजिक बाइट्स पढ़ता है; मैनुअल स्पष्ट प्रारंभ/समाप्ति अनुक्रम उपयोग करता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2304"/>
        <source>Start Sequence (Hex)</source>
        <translation>स्टार्ट सीक्वेंस (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2305"/>
        <source>e.g. FF D8 FF</source>
        <translation>उदा. FF D8 FF</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2306"/>
        <source>Hex bytes marking the start of an image frame</source>
        <translation>इमेज फ्रेम की शुरुआत को चिह्नित करने वाले Hex बाइट्स</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2315"/>
        <source>End Sequence (Hex)</source>
        <translation>समाप्ति अनुक्रम (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2316"/>
        <source>e.g. FF D9</source>
        <translation>उदा. FF D9</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2317"/>
        <source>Hex bytes marking the end of an image frame</source>
        <translation>इमेज फ्रेम के अंत को चिह्नित करने वाले Hex बाइट्स</translation>
    </message>
    <message>
        <source>Identity</source>
        <translation type="vanished">पहचान</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2437"/>
        <source>Device Name</source>
        <translation>डिवाइस का नाम</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2438"/>
        <source>Device 1</source>
        <translation>डिवाइस 1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2439"/>
        <source>Human-readable name for this input device</source>
        <translation>इस इनपुट डिवाइस के लिए पठनीय नाम</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2448"/>
        <source>Bus Type</source>
        <translation>बस प्रकार</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2449"/>
        <source>Select the hardware interface for this input device</source>
        <translation>इस इनपुट डिवाइस के लिए हार्डवेयर इंटरफ़ेस चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2451"/>
        <source>Serial Port</source>
        <translation>सीरियल पोर्ट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2451"/>
        <source>Network Socket</source>
        <translation>नेटवर्क सॉकेट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2451"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2453"/>
        <source>Audio Input</source>
        <translation>ऑडियो इनपुट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2453"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2453"/>
        <source>CAN Bus</source>
        <translation>CAN Bus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2453"/>
        <source>Raw USB</source>
        <translation>Raw USB</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2454"/>
        <source>HID Device</source>
        <translation>HID डिवाइस</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2454"/>
        <source>Process</source>
        <translation>प्रोसेस</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2454"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT Subscriber</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2609"/>
        <source>Connection Settings</source>
        <translation>कनेक्शन सेटिंग्स</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2846"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3116"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4755"/>
        <source>General Information</source>
        <translation>सामान्य जानकारी</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2855"/>
        <source>Action Title</source>
        <translation>एक्शन शीर्षक</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2857"/>
        <source>Untitled Action</source>
        <translation>अनाम एक्शन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2858"/>
        <source>Name or description of this action</source>
        <translation>इस एक्शन का नाम या विवरण</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2867"/>
        <source>Action Icon</source>
        <translation>एक्शन आइकन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2868"/>
        <source>Default Icon</source>
        <translation>डिफ़ॉल्ट आइकन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2869"/>
        <source>Icon displayed for this action in the dashboard</source>
        <translation>डैशबोर्ड में इस एक्शन के लिए प्रदर्शित आइकन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2896"/>
        <source>Target Device</source>
        <translation>लक्ष्य डिवाइस</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2897"/>
        <source>Select which connected device this action sends data to</source>
        <translation>चुनें कि यह एक्शन किस कनेक्टेड डिवाइस को डेटा भेजता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2909"/>
        <source>Data Payload</source>
        <translation>डेटा पेलोड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2920"/>
        <source>Send as Binary</source>
        <translation>बाइनरी के रूप में भेजें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2921"/>
        <source>Send raw binary data when this action is triggered</source>
        <translation>जब यह एक्शन ट्रिगर हो तो रॉ बाइनरी डेटा भेजें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2932"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2944"/>
        <source>Command</source>
        <translation>कमांड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2933"/>
        <source>Transmit Data (Hex)</source>
        <translation>डेटा ट्रांसमिट करें (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2934"/>
        <source>Hexadecimal payload to send when the action is triggered</source>
        <translation>हेक्साडेसिमल पेलोड जो एक्शन ट्रिगर होने पर भेजा जाएगा</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2945"/>
        <source>Transmit Data</source>
        <translation>डेटा ट्रांसमिट करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2946"/>
        <source>Text payload to send when the action is triggered</source>
        <translation>टेक्स्ट पेलोड जो एक्शन ट्रिगर होने पर भेजा जाएगा</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2957"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4814"/>
        <source>Text Encoding</source>
        <translation>टेक्स्ट एन्कोडिंग</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2958"/>
        <source>Character encoding used to serialize the text payload</source>
        <translation>टेक्स्ट पेलोड को सीरियलाइज़ करने के लिए उपयोग की जाने वाली कैरेक्टर एन्कोडिंग</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2982"/>
        <source>End-of-Line Sequence</source>
        <translation>एंड-ऑफ-लाइन सीक्वेंस</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2983"/>
        <source>EOL characters to append to the message (e.g. \n, \r\n)</source>
        <translation>संदेश में जोड़े जाने वाले EOL कैरेक्टर (जैसे </translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2995"/>
        <source>Execution Behavior</source>
        <translation>निष्पादन व्यवहार</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3006"/>
        <source>Auto-Execute on Connect</source>
        <translation>कनेक्ट पर ऑटो-निष्पादित करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3007"/>
        <source>Automatically trigger this action when the device connects</source>
        <translation>डिवाइस कनेक्ट होने पर इस एक्शन को स्वचालित रूप से ट्रिगर करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3013"/>
        <source>Timer Behavior</source>
        <translation>टाइमर व्यवहार</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3022"/>
        <source>Timer Mode</source>
        <translation>टाइमर मोड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3025"/>
        <source>Choose when and how this action should repeat automatically</source>
        <translation>चुनें कि यह एक्शन कब और कैसे स्वचालित रूप से दोहराया जाना चाहिए</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3032"/>
        <source>Interval (ms)</source>
        <translation>अंतराल (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3036"/>
        <source>Timer Interval (ms)</source>
        <translation>टाइमर अंतराल (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3037"/>
        <source>Milliseconds between each repeated trigger of this action</source>
        <translation>इस एक्शन के प्रत्येक बार-बार ट्रिगर के बीच मिलीसेकंड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3044"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3048"/>
        <source>Repeat Count</source>
        <translation>रिपीट काउंट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3049"/>
        <source>Number of times to send the command on each trigger</source>
        <translation>प्रत्येक ट्रिगर पर कमांड भेजने की संख्या</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3126"/>
        <source>Untitled Dataset</source>
        <translation>शीर्षकहीन डेटासेट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3127"/>
        <source>Dataset Title</source>
        <translation>डेटासेट शीर्षक</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3128"/>
        <source>Name of the dataset, used for labeling and identification</source>
        <translation>डेटासेट का नाम, लेबलिंग और पहचान के लिए उपयोग किया जाता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3157"/>
        <source>Hide on Dashboard</source>
        <translation>डैशबोर्ड पर छिपाएँ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3158"/>
        <source>Suppress this dataset's standalone dashboard tile; the painter widget can still read its values</source>
        <translation>इस डेटासेट की स्वतंत्र डैशबोर्ड टाइल को दबाएँ; पेंटर विजेट अभी भी इसके मान पढ़ सकता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3204"/>
        <source>Lower bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>डेटासेट वैल्यू रेंज की निचली सीमा; विजेट और FFT इस पर फॉलबैक करते हैं जब उनकी अपनी रेंज सेट नहीं होती</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3217"/>
        <source>Upper bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>डेटासेट वैल्यू रेंज की ऊपरी सीमा; विजेट और FFT इस पर फॉलबैक करते हैं जब उनकी अपनी रेंज सेट नहीं होती</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3275"/>
        <source>Choose Time or a dataset to drive the X-Axis in plots</source>
        <translation>प्लॉट में X-Axis चलाने के लिए Time या डेटासेट चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3288"/>
        <source>Frequency Analysis</source>
        <translation>फ्रीक्वेंसी विश्लेषण</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3335"/>
        <source>Choose Time (default) or any dataset whose value drives the Y axis -- produces a Campbell diagram when bound to e.g. RPM</source>
        <translation>समय (डिफ़ॉल्ट) या कोई भी डेटासेट चुनें जिसका मान Y अक्ष को संचालित करता है -- उदाहरण के लिए RPM से बाइंड करने पर कैंपबेल आरेख उत्पन्न करता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3385"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3474"/>
        <source>Minimum Value (optional)</source>
        <translation>न्यूनतम मान (वैकल्पिक)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3386"/>
        <source>Lower bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>डेटा सामान्यीकरण के लिए निचली सीमा; अनसेट छोड़ने पर डेटासेट मान रेंज पर वापस आता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3398"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3487"/>
        <source>Maximum Value (optional)</source>
        <translation>अधिकतम मान (वैकल्पिक)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3399"/>
        <source>Upper bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>डेटा सामान्यीकरण के लिए ऊपरी सीमा; अनसेट छोड़ने पर डेटासेट मान रेंज पर वापस आता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3475"/>
        <source>Lower bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>गेज या बार रेंज की निचली सीमा; अनसेट छोड़ने पर डेटासेट मान रेंज पर वापस आता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3488"/>
        <source>Upper bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>गेज या बार रेंज की ऊपरी सीमा; अनसेट छोड़ने पर डेटासेट मान रेंज पर वापस आता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3540"/>
        <source>Decimal Points</source>
        <translation>दशमलव बिंदु</translation>
    </message>
    <message>
        <source>Decimal places shown in the data grid value column (-1 = auto)</source>
        <translation type="vanished">डेटा ग्रिड मान कॉलम में दिखाए गए दशमलव स्थान (-1 = स्वतः)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3587"/>
        <source>On</source>
        <translation>चालू</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3631"/>
        <source>LED lights up when value meets or exceeds this threshold; define alarm bands for multi-state colors</source>
        <translation>LED इस थ्रेशोल्ड को पूरा करने या पार करने पर प्रकाशित होता है; मल्टी-स्टेट रंगों के लिए अलार्म बैंड परिभाषित करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3688"/>
        <source>Painter Widget</source>
        <translation>पेंटर विजेट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3689"/>
        <source>Web View</source>
        <translation>वेब व्यू</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5060"/>
        <source>Raw and transformed values for every dataset (read-only)</source>
        <translation>प्रत्येक डेटासेट के लिए कच्चे और रूपांतरित मान (केवल-पढ़ने योग्य)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5069"/>
        <source>Shared table defined in this project</source>
        <translation>इस प्रोजेक्ट में परिभाषित साझा टेबल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5428"/>
        <source>Remove 1 widget reference whose target group or dataset no longer exists?</source>
        <translation>1 विजेट संदर्भ हटाएं जिसका लक्ष्य ग्रुप या डेटासेट अब मौजूद नहीं है?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5429"/>
        <source>Remove %1 widget references whose target groups or datasets no longer exist?</source>
        <translation>%1 विजेट संदर्भ हटाएं जिनके लक्ष्य ग्रुप या डेटासेट अब मौजूद नहीं हैं?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5434"/>
        <source>This will only affect workspace tile placement; no groups, datasets, or data are deleted.</source>
        <translation>यह केवल वर्कस्पेस टाइल प्लेसमेंट को प्रभावित करेगा; कोई ग्रुप, डेटासेट या डेटा डिलीट नहीं होगा।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5437"/>
        <source>Clean Up Workspaces</source>
        <translation>वर्कस्पेसेज़ साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3171"/>
        <source>Frame Index</source>
        <translation>फ़्रेम इंडेक्स</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1660"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1663"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1664"/>
        <source>Control Loop</source>
        <translation>नियंत्रण लूप</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3172"/>
        <source>Frame position used for aligning datasets in time</source>
        <translation>समय में डेटासेट को संरेखित करने के लिए उपयोग की जाने वाली फ़्रेम स्थिति</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3181"/>
        <source>Measurement Unit</source>
        <translation>माप इकाई</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3182"/>
        <source>Volts, Amps, etc.</source>
        <translation>वोल्ट, एम्पीयर, आदि।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3183"/>
        <source>Unit of measurement, such as volts or amps (optional)</source>
        <translation>माप की इकाई, जैसे वोल्ट या एम्पीयर (वैकल्पिक)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3230"/>
        <source>Plot Settings</source>
        <translation>प्लॉट सेटिंग्स</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3253"/>
        <source>Enable Plot Widget</source>
        <translation>प्लॉट विजेट सक्षम करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3255"/>
        <source>Plot data in real-time</source>
        <translation>रियल-टाइम में डेटा प्लॉट करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2341"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3274"/>
        <source>X-Axis Source</source>
        <translation>X-अक्ष स्रोत</translation>
    </message>
    <message>
        <source>Choose which dataset to use for the X-Axis in plots</source>
        <translation type="vanished">प्लॉट में X-अक्ष के लिए कौन सा डेटासेट उपयोग करना है चुनें</translation>
    </message>
    <message>
        <source>Minimum Plot Value (optional)</source>
        <translation type="vanished">न्यूनतम प्लॉट मान (वैकल्पिक)</translation>
    </message>
    <message>
        <source>Lower bound for plot display range</source>
        <translation type="vanished">प्लॉट डिस्प्ले रेंज के लिए निचली सीमा</translation>
    </message>
    <message>
        <source>Maximum Plot Value (optional)</source>
        <translation type="vanished">अधिकतम प्लॉट मान (वैकल्पिक)</translation>
    </message>
    <message>
        <source>Upper bound for plot display range</source>
        <translation type="vanished">प्लॉट डिस्प्ले रेंज के लिए ऊपरी सीमा</translation>
    </message>
    <message>
        <source>FFT Configuration</source>
        <translation type="vanished">FFT कॉन्फ़िगरेशन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3299"/>
        <source>Enable FFT Analysis</source>
        <translation>FFT विश्लेषण सक्षम करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3300"/>
        <source>Perform frequency-domain analysis of the dataset</source>
        <translation>डेटासेट का फ़्रीक्वेंसी-डोमेन विश्लेषण करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3310"/>
        <source>Enable Waterfall Plot</source>
        <translation>वॉटरफॉल प्लॉट सक्षम करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3311"/>
        <source>Show a scrolling spectrogram of frequency content over time (Pro)</source>
        <translation>समय के साथ फ्रीक्वेंसी सामग्री का स्क्रॉलिंग स्पेक्ट्रोग्राम दिखाएं (Pro)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3334"/>
        <source>Waterfall Y Axis</source>
        <translation>वॉटरफॉल Y अक्ष</translation>
    </message>
    <message>
        <source>Choose Time (default) or any dataset whose value drives the Y axis — produces a Campbell diagram when bound to e.g. RPM</source>
        <translation type="obsolete">समय (डिफ़ॉल्ट) या कोई भी डेटासेट चुनें जिसका मान Y अक्ष को संचालित करता है — उदाहरण के लिए RPM से बाइंड करने पर कैंपबेल आरेख उत्पन्न करता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3362"/>
        <source>FFT Window Size</source>
        <translation>FFT विंडो आकार</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3363"/>
        <source>Number of samples used for each FFT calculation window</source>
        <translation>प्रत्येक FFT गणना विंडो के लिए उपयोग किए गए नमूनों की संख्या</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3374"/>
        <source>FFT Sampling Rate (Hz, required)</source>
        <translation>FFT सैंपलिंग रेट (Hz, आवश्यक)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3375"/>
        <source>Sampling frequency used for FFT (in Hz)</source>
        <translation>FFT के लिए उपयोग की गई सैंपलिंग फ़्रीक्वेंसी (Hz में)</translation>
    </message>
    <message>
        <source>Minimum Value (recommended)</source>
        <translation type="vanished">न्यूनतम मान (अनुशंसित)</translation>
    </message>
    <message>
        <source>Lower bound for data normalization</source>
        <translation type="vanished">डेटा नॉर्मलाइज़ेशन के लिए निचली सीमा</translation>
    </message>
    <message>
        <source>Maximum Value (recommended)</source>
        <translation type="vanished">अधिकतम मान (अनुशंसित)</translation>
    </message>
    <message>
        <source>Upper bound for data normalization</source>
        <translation type="vanished">डेटा नॉर्मलाइज़ेशन के लिए ऊपरी सीमा</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3424"/>
        <source>Widget Settings</source>
        <translation>विजेट सेटिंग्स</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3447"/>
        <source>Widget</source>
        <translation>विजेट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3448"/>
        <source>Select the visual widget used to display this dataset</source>
        <translation>इस डेटासेट को डिस्प्ले करने के लिए उपयोग किया जाने वाला विज़ुअल विजेट चुनें</translation>
    </message>
    <message>
        <source>Minimum Display Value (required)</source>
        <translation type="vanished">न्यूनतम डिस्प्ले मान (आवश्यक)</translation>
    </message>
    <message>
        <source>Lower bound of the gauge or bar display range</source>
        <translation type="vanished">गेज या बार डिस्प्ले रेंज की निचली सीमा</translation>
    </message>
    <message>
        <source>Maximum Display Value (required)</source>
        <translation type="vanished">अधिकतम डिस्प्ले मान (आवश्यक)</translation>
    </message>
    <message>
        <source>Upper bound of the gauge or bar display range</source>
        <translation type="vanished">गेज या बार डिस्प्ले रेंज की ऊपरी सीमा</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3504"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3539"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3700"/>
        <source>Auto</source>
        <translation>स्वतः</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3505"/>
        <source>Tick Count</source>
        <translation>टिक संख्या</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3509"/>
        <source>Major-tick count on the dial scale (0 = auto-fit to widget size)</source>
        <translation>डायल स्केल पर मुख्य-टिक संख्या (0 = विजेट आकार में स्वतः फ़िट)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3528"/>
        <source>Label Format</source>
        <translation>लेबल फ़ॉर्मेट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3529"/>
        <source>Decimal places or notation used on tick labels and the value display</source>
        <translation>टिक लेबल और मान प्रदर्शन में उपयोग किए गए दशमलव स्थान या संकेतन</translation>
    </message>
    <message>
        <source>Show Value Indicator</source>
        <translation type="vanished">मान संकेतक दिखाएँ</translation>
    </message>
    <message>
        <source>Toggle the boxed numeric readout that sits below or beside the widget</source>
        <translation type="vanished">विजेट के नीचे या बगल में स्थित बॉक्सयुक्त संख्यात्मक रीडआउट टॉगल करें</translation>
    </message>
    <message>
        <source>Alarm Settings</source>
        <translation type="vanished">अलार्म सेटिंग्स</translation>
    </message>
    <message>
        <source>Enable Alarms</source>
        <translation type="vanished">अलार्म सक्षम करें</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds alarm thresholds</source>
        <translation type="vanished">मान अलार्म थ्रेशोल्ड से अधिक होने पर विज़ुअल अलार्म ट्रिगर करता है</translation>
    </message>
    <message>
        <source>Low Threshold</source>
        <translation type="vanished">निम्न थ्रेशोल्ड</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value drops below this threshold</source>
        <translation type="vanished">मान इस थ्रेशोल्ड से नीचे गिरने पर विज़ुअल अलार्म ट्रिगर करता है</translation>
    </message>
    <message>
        <source>High Threshold</source>
        <translation type="vanished">उच्च थ्रेशोल्ड</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds this threshold</source>
        <translation type="vanished">मान इस थ्रेशोल्ड से अधिक होने पर विज़ुअल अलार्म ट्रिगर करता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3604"/>
        <source>LED Display Settings</source>
        <translation>LED डिस्प्ले सेटिंग्स</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3615"/>
        <source>Show in LED Panel</source>
        <translation>LED पैनल में दिखाएं</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3616"/>
        <source>Enable visual status monitoring using an LED display</source>
        <translation>LED डिस्प्ले का उपयोग करके विज़ुअल स्टेटस मॉनिटरिंग सक्षम करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3630"/>
        <source>LED On Threshold (required)</source>
        <translation>LED ऑन थ्रेशोल्ड (आवश्यक)</translation>
    </message>
    <message>
        <source>LED lights up when value meets or exceeds this threshold</source>
        <translation type="vanished">LED इस थ्रेशोल्ड को पूरा करने या पार करने पर प्रकाशित होता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3651"/>
        <source>Off</source>
        <translation>बंद</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3651"/>
        <source>Auto Start</source>
        <translation>स्वतः प्रारंभ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3651"/>
        <source>Start on Trigger</source>
        <translation>ट्रिगर पर प्रारंभ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3651"/>
        <source>Toggle on Trigger</source>
        <translation>ट्रिगर पर टॉगल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3652"/>
        <source>Repeat N Times</source>
        <translation>N बार दोहराएं</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3655"/>
        <source>Plain Text (UTF8)</source>
        <translation>सादा टेक्स्ट (UTF8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3655"/>
        <source>Hexadecimal</source>
        <translation>हेक्साडेसिमल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3655"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3656"/>
        <source>Binary (Direct)</source>
        <translation>बाइनरी (प्रत्यक्ष)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3661"/>
        <source>No Checksum</source>
        <translation>कोई चेकसम नहीं</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3665"/>
        <source>End Delimiter Only</source>
        <translation>केवल अंत डिलीमिटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3665"/>
        <source>Start Delimiter Only</source>
        <translation>केवल प्रारंभ डिलीमिटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3666"/>
        <source>Start + End Delimiter</source>
        <translation>प्रारंभ + अंत डिलीमिटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3666"/>
        <source>No Delimiters</source>
        <translation>कोई डिलीमिटर नहीं</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3676"/>
        <source>Button</source>
        <translation>बटन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3676"/>
        <source>Slider</source>
        <translation>स्लाइडर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3676"/>
        <source>Toggle</source>
        <translation>टॉगल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3676"/>
        <source>Text Field</source>
        <translation>टेक्स्ट फ़ील्ड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3677"/>
        <source>Knob</source>
        <translation>नॉब</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3681"/>
        <source>Data Grid</source>
        <translation>डेटा ग्रिड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3682"/>
        <source>GPS Map</source>
        <translation>GPS मैप</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3683"/>
        <source>Gyroscope</source>
        <translation>जायरोस्कोप</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3684"/>
        <source>Multiple Plot</source>
        <translation>मल्टीपल प्लॉट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3685"/>
        <source>Accelerometer</source>
        <translation>एक्सेलेरोमीटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3686"/>
        <source>3D Plot</source>
        <translation>3D प्लॉट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3687"/>
        <source>Image View</source>
        <translation>इमेज व्यू</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3690"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3693"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3708"/>
        <source>None</source>
        <translation>कोई नहीं</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3694"/>
        <source>Bar</source>
        <translation>बार</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3695"/>
        <source>Gauge</source>
        <translation>गेज</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3696"/>
        <source>Compass</source>
        <translation>कंपास</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3697"/>
        <source>Meter</source>
        <translation>मीटर</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">थर्मामीटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3701"/>
        <source>Integer (0 decimals)</source>
        <translation>पूर्णांक (0 दशमलव)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3702"/>
        <source>1 decimal</source>
        <translation>1 दशमलव</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3703"/>
        <source>2 decimals</source>
        <translation>2 दशमलव</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3704"/>
        <source>3 decimals</source>
        <translation>3 दशमलव</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3705"/>
        <source>Scientific</source>
        <translation>वैज्ञानिक</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3709"/>
        <source>New Line (\n)</source>
        <translation>नई लाइन (</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3710"/>
        <source>Carriage Return (\r)</source>
        <translation>कैरिज रिटर्न (\r)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3711"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3714"/>
        <source>No</source>
        <translation>नहीं</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3715"/>
        <source>Yes</source>
        <translation>हाँ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4765"/>
        <source>Label</source>
        <translation>लेबल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4766"/>
        <source>Display label</source>
        <translation>डिस्प्ले लेबल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4776"/>
        <source>Button Icon</source>
        <translation>बटन आइकन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4785"/>
        <source>Colorize Icon</source>
        <translation>आइकन रंगीन करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4786"/>
        <source>Tint the icon with the button color</source>
        <translation>बटन रंग से आइकन को टिंट करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4803"/>
        <source>Initial Value</source>
        <translation>प्रारंभिक मान</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4815"/>
        <source>Character encoding used when transmit() returns a string value</source>
        <translation>कैरेक्टर एन्कोडिंग जब transmit() स्ट्रिंग मान लौटाता है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4833"/>
        <source>Value Range</source>
        <translation>मान रेंज</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3203"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4843"/>
        <source>Minimum Value</source>
        <translation>न्यूनतम मान</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3216"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4852"/>
        <source>Maximum Value</source>
        <translation>अधिकतम मान</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4861"/>
        <source>Step Size</source>
        <translation>स्टेप साइज़</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectModel</name>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="539"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="548"/>
        <source>Lock Project</source>
        <translation>प्रोजेक्ट लॉक करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="540"/>
        <source>Choose a password to lock the project:</source>
        <translation>प्रोजेक्ट लॉक करने के लिए पासवर्ड चुनें:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="548"/>
        <source>Confirm the password:</source>
        <translation>पासवर्ड की पुष्टि करें:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="553"/>
        <source>Passwords do not match</source>
        <translation>पासवर्ड मेल नहीं खाते</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="554"/>
        <source>The two passwords you entered do not match. The project was not locked.</source>
        <translation>दर्ज किए गए दोनों पासवर्ड मेल नहीं खाते। प्रोजेक्ट लॉक नहीं किया गया।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="588"/>
        <source>Unlock Project</source>
        <translation>प्रोजेक्ट अनलॉक करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="589"/>
        <source>Enter the project password:</source>
        <translation>प्रोजेक्ट पासवर्ड दर्ज करें:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="599"/>
        <source>Incorrect password</source>
        <translation>गलत पासवर्ड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="600"/>
        <source>The password you entered does not match the one stored in the project file.</source>
        <translation>दर्ज किया गया पासवर्ड प्रोजेक्ट फ़ाइल में संग्रहीत पासवर्ड से मेल नहीं खाता।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="631"/>
        <source>New Project</source>
        <translation>नया प्रोजेक्ट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="831"/>
        <source>Samples</source>
        <translation>सैंपल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1340"/>
        <source>Multiple data sources require a Pro license</source>
        <translation>एकाधिक डेटा स्रोतों के लिए Pro लाइसेंस आवश्यक है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1341"/>
        <source>Serial Studio Pro allows connecting to multiple devices simultaneously. Please upgrade to unlock this feature.</source>
        <translation>Serial Studio Pro एक साथ कई डिवाइस से कनेक्ट करने की अनुमति देता है। कृपया इस सुविधा को अनलॉक करने के लिए अपग्रेड करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1353"/>
        <source>Device %1</source>
        <translation>डिवाइस %1</translation>
    </message>
    <message>
        <source> (Copy)</source>
        <translation type="vanished">(कॉपी)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1623"/>
        <source>Do you want to save your changes?</source>
        <translation>क्या आप अपने परिवर्तन सहेजना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1624"/>
        <source>You have unsaved modifications in this project!</source>
        <translation>इस प्रोजेक्ट में आपके पास असहेजे संशोधन हैं!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="411"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="420"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="433"/>
        <source>Project error</source>
        <translation>प्रोजेक्ट त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="411"/>
        <source>Project title cannot be empty!</source>
        <translation>प्रोजेक्ट शीर्षक खाली नहीं हो सकता!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="420"/>
        <source>You need to add at least one group!</source>
        <translation>आपको कम से कम एक ग्रुप जोड़ना होगा!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="433"/>
        <source>You need to add at least one dataset!</source>
        <translation>आपको कम से कम एक डेटासेट जोड़ना होगा!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="480"/>
        <source>Your project needs a title</source>
        <translation>आपके प्रोजेक्ट को एक शीर्षक चाहिए</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="482"/>
        <source>Add a group to get started</source>
        <translation>शुरू करने के लिए एक समूह जोड़ें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="484"/>
        <source>Add a dataset to a group</source>
        <translation>किसी समूह में एक डेटासेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="498"/>
        <source>Open the Project view at the top of the tree and enter a name. You can rename the project at any time.</source>
        <translation>ट्री के शीर्ष पर प्रोजेक्ट व्यू खोलें और एक नाम दर्ज करें। आप किसी भी समय प्रोजेक्ट का नाम बदल सकते हैं।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="501"/>
        <source>Groups organize datasets into dashboard widgets. Use the Group button in the toolbar above to create one, then add datasets to it.</source>
        <translation>समूह डेटासेट को डैशबोर्ड विजेट में व्यवस्थित करते हैं। एक बनाने के लिए ऊपर टूलबार में समूह बटन का उपयोग करें, फिर उसमें डेटासेट जोड़ें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="505"/>
        <source>Datasets are the values that appear on the dashboard. Select a group in the tree and use the Dataset button in the toolbar to add one.</source>
        <translation>डेटासेट वे मान हैं जो डैशबोर्ड पर दिखाई देते हैं। ट्री में एक समूह चुनें और एक जोड़ने के लिए टूलबार में डेटासेट बटन का उपयोग करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="830"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="879"/>
        <source>Time</source>
        <translation>समय</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1385"/>
        <source>Do you want to delete data source "%1"?</source>
        <translation>क्या आप डेटा स्रोत "%1" को डिलीट करना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1386"/>
        <source>Groups using this source will move to the default source. This action cannot be undone.</source>
        <translation>इस स्रोत का उपयोग करने वाले समूह डिफ़ॉल्ट स्रोत में चले जाएंगे। यह एक्शन पूर्ववत नहीं किया जा सकता।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1663"/>
        <source>Save Serial Studio Project</source>
        <translation>Serial Studio प्रोजेक्ट सहेजें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1665"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2287"/>
        <source>Serial Studio Project Files (*.ssproj)</source>
        <translation>Serial Studio प्रोजेक्ट फ़ाइलें (*.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1686"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1905"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2278"/>
        <source>Untitled Project</source>
        <translation>अनाम प्रोजेक्ट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1917"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2433"/>
        <source>Device A</source>
        <translation>डिवाइस A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2116"/>
        <source>Select Project File</source>
        <translation>प्रोजेक्ट फ़ाइल चुनें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2118"/>
        <source>Project Files (*.json *.ssproj)</source>
        <translation>प्रोजेक्ट फ़ाइलें (*.json *.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2162"/>
        <source>JSON validation error</source>
        <translation>JSON सत्यापन त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2252"/>
        <source>Project upgraded from an earlier file format</source>
        <translation>प्रोजेक्ट को पुराने फ़ाइल फ़ॉर्मेट से अपग्रेड किया गया</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2253"/>
        <source>This project was saved with schema version %1; the current version is %2. Defaults have been applied to any new fields. Save the project to lock in the upgrade.</source>
        <translation>यह प्रोजेक्ट स्कीमा संस्करण %1 के साथ सेव किया गया था; वर्तमान संस्करण %2 है। किसी भी नए फ़ील्ड के लिए डिफ़ॉल्ट लागू किए गए हैं। अपग्रेड लॉक करने के लिए प्रोजेक्ट सेव करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2285"/>
        <source>Save Imported Project</source>
        <translation>आयातित प्रोजेक्ट सहेजें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2477"/>
        <source>Multi-source projects require a Pro license</source>
        <translation>मल्टी-सोर्स प्रोजेक्ट के लिए Pro लाइसेंस आवश्यक है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2478"/>
        <source>This project contains multiple data sources. Only the first source has been loaded. A Serial Studio Pro license is required to use multi-source projects.</source>
        <translation>इस प्रोजेक्ट में एकाधिक डेटा स्रोत हैं। केवल पहला स्रोत लोड किया गया है। मल्टी-सोर्स प्रोजेक्ट उपयोग करने के लिए Serial Studio Pro लाइसेंस आवश्यक है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2711"/>
        <source>Workspace IDs remapped on load</source>
        <translation>लोड करते समय वर्कस्पेस ID फिर से मैप किए गए</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2712"/>
        <source>%1 custom workspace ID(s) overlapped the new reserved auto range and were moved into the user range. Save the project to make the remap permanent.</source>
        <translation>%1 कस्टम वर्कस्पेस ID नई आरक्षित ऑटो रेंज से ओवरलैप हो गईं और यूज़र रेंज में स्थानांतरित कर दी गईं। रीमैप को स्थायी बनाने के लिए प्रोजेक्ट सहेजें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2854"/>
        <source>Legacy frame parser function updated</source>
        <translation>लीगेसी फ़्रेम पार्सर फ़ंक्शन अपडेट किया गया</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2855"/>
        <source>Your project used a legacy frame parser function with a 'separator' argument. It has been automatically migrated to the new format.</source>
        <translation>आपके प्रोजेक्ट में 'separator' आर्गुमेंट के साथ एक लीगेसी फ़्रेम पार्सर फ़ंक्शन का उपयोग किया गया था। इसे स्वचालित रूप से नए फ़ॉर्मेट में माइग्रेट कर दिया गया है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3053"/>
        <source>Do you want to delete group "%1"?</source>
        <translation>क्या आप ग्रुप "%1" को डिलीट करना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3054"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3099"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3131"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3869"/>
        <source>This action cannot be undone. Do you wish to proceed?</source>
        <translation>यह क्रिया पूर्ववत नहीं की जा सकती। क्या आप आगे बढ़ना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3098"/>
        <source>Do you want to delete action "%1"?</source>
        <translation>क्या आप एक्शन "%1" को डिलीट करना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3130"/>
        <source>Do you want to delete dataset "%1"?</source>
        <translation>क्या आप डेटासेट "%1" को डिलीट करना चाहते हैं?</translation>
    </message>
    <message>
        <source>%1 (Copy)</source>
        <translation type="vanished">%1 (कॉपी)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3781"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3817"/>
        <source>Output Controls</source>
        <translation>आउटपुट कंट्रोल्स</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3829"/>
        <source>New Button</source>
        <translation>नया बटन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3832"/>
        <source>New Slider</source>
        <translation>नया स्लाइडर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3835"/>
        <source>New Toggle</source>
        <translation>नया टॉगल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3838"/>
        <source>New Text Field</source>
        <translation>नया टेक्स्ट फ़ील्ड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3841"/>
        <source>New Knob</source>
        <translation>नया नॉब</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3868"/>
        <source>Do you want to delete output widget "%1"?</source>
        <translation>क्या आप आउटपुट विजेट "%1" को हटाना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4033"/>
        <source>Group</source>
        <translation>समूह</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4051"/>
        <source>New Dataset</source>
        <translation>नया डेटासेट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4054"/>
        <source>New Plot</source>
        <translation>नया प्लॉट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4058"/>
        <source>New FFT Plot</source>
        <translation>नया FFT प्लॉट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4062"/>
        <source>New Level Indicator</source>
        <translation>नया स्तर संकेतक</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4066"/>
        <source>New Gauge</source>
        <translation>नया गेज</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4070"/>
        <source>New Compass</source>
        <translation>नया कंपास</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4076"/>
        <source>New Meter</source>
        <translation>नया मीटर</translation>
    </message>
    <message>
        <source>New Thermometer</source>
        <translation type="vanished">नया थर्मामीटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4080"/>
        <source>New LED Indicator</source>
        <translation>नया LED इंडिकेटर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4084"/>
        <source>New Waterfall</source>
        <translation>नया वॉटरफॉल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4152"/>
        <source>Channel %1</source>
        <translation>चैनल %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4223"/>
        <source>New Action</source>
        <translation>नया एक्शन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4361"/>
        <source>Are you sure you want to change the group-level widget?</source>
        <translation>क्या आप वाकई ग्रुप-स्तरीय विजेट बदलना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4363"/>
        <source>Existing datasets for this group are deleted</source>
        <translation>इस ग्रुप के मौजूदा डेटासेट हटा दिए जाएंगे</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4431"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4432"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4433"/>
        <source>Accelerometer %1</source>
        <translation>एक्सेलेरोमीटर %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4448"/>
        <source>Gyro %1</source>
        <translation>जायरो %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4463"/>
        <source>Latitude</source>
        <translation>अक्षांश</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4463"/>
        <source>Longitude</source>
        <translation>देशांतर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4463"/>
        <source>Altitude</source>
        <translation>ऊंचाई</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4478"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4492"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4478"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4492"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4478"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4492"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4750"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5632"/>
        <source>Workspace</source>
        <translation>कार्यस्थान</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4915"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5122"/>
        <source>Shared Table</source>
        <translation>साझा टेबल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4993"/>
        <source>register</source>
        <translation>रजिस्टर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5122"/>
        <source>New Shared Table</source>
        <translation>नई साझा टेबल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5122"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5139"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5158"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5182"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5209"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5228"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5250"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5272"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5632"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5653"/>
        <source>Name:</source>
        <translation>नाम:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5139"/>
        <source>Rename Table</source>
        <translation>टेबल का नाम बदलें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5158"/>
        <source>Rename Group</source>
        <translation>समूह का नाम बदलें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5182"/>
        <source>Rename Dataset</source>
        <translation>डेटासेट का नाम बदलें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5209"/>
        <source>Rename Data Source</source>
        <translation>डेटा स्रोत का नाम बदलें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5228"/>
        <source>Rename Action</source>
        <translation>एक्शन का नाम बदलें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5249"/>
        <source>New Register</source>
        <translation>नया रजिस्टर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5272"/>
        <source>Rename Register</source>
        <translation>रजिस्टर का नाम बदलें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5309"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5334"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6155"/>
        <source>This action cannot be undone.</source>
        <translation>यह क्रिया पूर्ववत नहीं की जा सकती।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5310"/>
        <source>This removes %1 register(s) along with the table. This action cannot be undone.</source>
        <translation>यह टेबल के साथ %1 रजिस्टर हटा देगा। यह क्रिया पूर्ववत नहीं की जा सकती।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5313"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5333"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6154"/>
        <source>Delete "%1"?</source>
        <translation>"%1" डिलीट करें?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5316"/>
        <source>Delete Table</source>
        <translation>टेबल डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5336"/>
        <source>Delete Register</source>
        <translation>रजिस्टर डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5359"/>
        <source>Export Table</source>
        <translation>टेबल एक्सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5361"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5403"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV फ़ाइलें (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5401"/>
        <source>Import Table</source>
        <translation>टेबल इम्पोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5632"/>
        <source>New Workspace</source>
        <translation>नया कार्यस्थान</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5653"/>
        <source>Rename Workspace</source>
        <translation>वर्कस्पेस का नाम बदलें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5736"/>
        <source>Overview</source>
        <translation>अवलोकन</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5745"/>
        <source>All Data</source>
        <translation>सभी डेटा</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5923"/>
        <source>Discard workspace customisations?</source>
        <translation>वर्कस्पेस कस्टमाइज़ेशन को छोड़ें?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5924"/>
        <source>Switching off Customize discards your edits and rebuilds the workspace list from the project's groups.</source>
        <translation>कस्टमाइज़ बंद करने से आपके संपादन छोड़ दिए जाएंगे और प्रोजेक्ट के ग्रुप्स से वर्कस्पेस सूची पुनर्निर्मित होगी।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5927"/>
        <source>Customize Workspaces</source>
        <translation>वर्कस्पेसेज़ कस्टमाइज़ करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6157"/>
        <source>Delete Workspace</source>
        <translation>वर्कस्पेस डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6538"/>
        <source>Project file removed from disk</source>
        <translation>प्रोजेक्ट फ़ाइल डिस्क से हटा दी गई</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6539"/>
        <source>%1 was deleted or renamed by another program. Save the project to recreate it.</source>
        <translation>%1 को किसी अन्य प्रोग्राम द्वारा डिलीट या रीनेम किया गया। इसे फिर से बनाने के लिए प्रोजेक्ट सेव करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6560"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6581"/>
        <source>Project file changed on disk</source>
        <translation>प्रोजेक्ट फ़ाइल डिस्क पर बदल गई</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6561"/>
        <source>%1 was modified by another program. The in-memory project was kept; reopen the file to load the external changes.</source>
        <translation>%1 को किसी अन्य प्रोग्राम द्वारा संशोधित किया गया। मेमोरी में मौजूद प्रोजेक्ट रखा गया; बाहरी बदलाव लोड करने के लिए फ़ाइल फिर से खोलें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6577"/>
        <source>The project file was modified by another program.

Reload it and discard your unsaved changes?</source>
        <translation>प्रोजेक्ट फ़ाइल को किसी अन्य प्रोग्राम द्वारा संशोधित किया गया।

इसे रीलोड करें और अपने अनसेव्ड बदलाव डिस्कार्ड करें?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6579"/>
        <source>The project file was modified by another program.

Reload it?</source>
        <translation>प्रोजेक्ट फ़ाइल को किसी अन्य प्रोग्राम द्वारा संशोधित किया गया।

इसे रीलोड करें?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6610"/>
        <source>File save error</source>
        <translation>फ़ाइल सहेजने में त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2321"/>
        <source>File open error</source>
        <translation>फ़ाइल खोलने में त्रुटि</translation>
    </message>
</context>
<context>
    <name>DataModel::ProtoImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="902"/>
        <source>Import Protocol Buffers File</source>
        <translation>Protocol Buffers फ़ाइल इम्पोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="904"/>
        <source>Proto Files (*.proto);;All Files (*)</source>
        <translation>Proto फ़ाइलें (*.proto);;सभी फ़ाइलें (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="938"/>
        <source>Failed to open proto file: %1</source>
        <translation>Proto फ़ाइल खोलने में विफल: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="939"/>
        <source>Verify the file path and read permissions, then try again.</source>
        <translation>फ़ाइल पथ और पढ़ने की अनुमतियाँ सत्यापित करें, फिर पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="941"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="950"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="968"/>
        <source>Protobuf Import Error</source>
        <translation>Protobuf इम्पोर्ट त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="947"/>
        <source>Proto file is too large (the limit is 10 MB).</source>
        <translation>Proto फ़ाइल बहुत बड़ी है (सीमा 10 MB है)।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="948"/>
        <source>Verify you selected the correct .proto definition file.</source>
        <translation>सत्यापित करें कि आपने सही .proto परिभाषा फ़ाइल चुनी है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="965"/>
        <source>Failed to parse proto file at line %1: %2</source>
        <translation>पंक्ति %1 पर proto फ़ाइल पार्स करने में विफल: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="966"/>
        <source>Only proto3 syntax is supported. Verify the file format and try again.</source>
        <translation>केवल proto3 सिंटैक्स समर्थित है। फ़ाइल फ़ॉर्मेट सत्यापित करें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="973"/>
        <source>Proto file contains no message definitions</source>
        <translation>Proto फ़ाइल में कोई message परिभाषा नहीं है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="974"/>
        <source>The selected file has no `message` blocks to import.</source>
        <translation>चयनित फ़ाइल में इम्पोर्ट करने के लिए कोई `message` ब्लॉक नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="976"/>
        <source>Protobuf Import Warning</source>
        <translation>Protobuf इम्पोर्ट चेतावनी</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">इम्पोर्ट किया गया प्रोजेक्ट लोड करने में विफल</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">जनरेट किया गया प्रोजेक्ट JSON लोड नहीं हो सका।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1014"/>
        <source>Successfully imported %1 message(s) and %2 field(s) from the proto file.</source>
        <translation>proto फ़ाइल से %1 message(s) और %2 field(s) सफलतापूर्वक इम्पोर्ट किए गए।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1017"/>
        <source>The project editor is now open for customization.</source>
        <translation>प्रोजेक्ट एडिटर अब अनुकूलन के लिए खुला है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1019"/>
        <source>Protobuf Import Complete</source>
        <translation>Protobuf इम्पोर्ट पूर्ण</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1055"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
</context>
<context>
    <name>DataModel::TransmitTestDialog</name>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="154"/>
        <source>Invalid Hex Input</source>
        <translation>अमान्य Hex इनपुट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="155"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>कृपया मान्य हेक्साडेसिमल बाइट्स दर्ज करें।

मान्य प्रारूप: 01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="160"/>
        <source>No transmit function code to evaluate.</source>
        <translation>मूल्यांकन के लिए कोई ट्रांसमिट फ़ंक्शन कोड नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="177"/>
        <source>transmit function is not callable</source>
        <translation>ट्रांसमिट फ़ंक्शन कॉल करने योग्य नहीं है</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="238"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="239"/>
        <source>Clear</source>
        <translation>साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="240"/>
        <source>Evaluate</source>
        <translation>मूल्यांकन करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="241"/>
        <source>Input Value</source>
        <translation>इनपुट मान</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="242"/>
        <source>Transmit Function Output</source>
        <translation>ट्रांसमिट फ़ंक्शन आउटपुट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="243"/>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="267"/>
        <source>Enter value to transmit…</source>
        <translation>ट्रांसमिट करने के लिए मान दर्ज करें…</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="244"/>
        <source>Raw string output appears here</source>
        <translation>रॉ स्ट्रिंग आउटपुट यहाँ दिखाई देगा</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="245"/>
        <source>Hex byte output appears here</source>
        <translation>हेक्स बाइट आउटपुट यहाँ दिखाई देगा</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="248"/>
        <source>Test Transmit Function</source>
        <translation>ट्रांसमिट फ़ंक्शन का परीक्षण करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="261"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>हेक्स बाइट्स दर्ज करें (उदा., 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="367"/>
        <source>(empty) No data returned</source>
        <translation>(खाली) कोई डेटा नहीं मिला</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="369"/>
        <source>0 bytes</source>
        <translation>0 बाइट्स</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="408"/>
        <source>%1 byte(s)</source>
        <translation>%1 बाइट(s)</translation>
    </message>
</context>
<context>
    <name>DataTablesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="33"/>
        <source>Shared Memory</source>
        <translation>शेयर्ड मेमोरी</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="149"/>
        <source>Add Shared Table</source>
        <translation>साझा टेबल जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="151"/>
        <source>Add shared table</source>
        <translation>साझा टेबल जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="160"/>
        <source>Help</source>
        <translation>सहायता</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="165"/>
        <source>Open help documentation for shared memory</source>
        <translation>शेयर्ड मेमोरी के लिए सहायता दस्तावेज़ खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="174"/>
        <source>Name</source>
        <translation>नाम</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="175"/>
        <source>Description</source>
        <translation>विवरण</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="176"/>
        <source>Entries</source>
        <translation>प्रविष्टियाँ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="274"/>
        <source>No shared tables.</source>
        <translation>कोई शेयर्ड टेबल नहीं।</translation>
    </message>
</context>
<context>
    <name>DatabaseExplorer</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="35"/>
        <source>Sessions</source>
        <translation>सेशन</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="218"/>
        <source>Open</source>
        <translation>खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="220"/>
        <source>Open a session file</source>
        <translation>सेशन फ़ाइल खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="226"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="229"/>
        <source>Close session file</source>
        <translation>सेशन फ़ाइल बंद करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="242"/>
        <source>Replay</source>
        <translation>रीप्ले करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="246"/>
        <source>Replay selected session on the dashboard</source>
        <translation>चयनित सेशन को डैशबोर्ड पर रीप्ले करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="252"/>
        <source>Delete</source>
        <translation>डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="258"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>सेशन हटाने के लिए सेशन फ़ाइल अनलॉक करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="259"/>
        <source>Delete the selected session</source>
        <translation>चयनित सेशन को डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="276"/>
        <source>Unlock</source>
        <translation>अनलॉक करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="277"/>
        <source>Lock</source>
        <translation>लॉक करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="282"/>
        <source>Unlock the session file to allow deletions</source>
        <translation>हटाने की अनुमति देने के लिए सेशन फ़ाइल अनलॉक करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="283"/>
        <source>Set a password to prevent session deletions</source>
        <translation>सेशन हटाने से रोकने के लिए पासवर्ड सेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="298"/>
        <source>Export CSV</source>
        <translation>CSV एक्सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="303"/>
        <source>Export selected session to CSV</source>
        <translation>चयनित सेशन को CSV में एक्सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="310"/>
        <source>Export PDF</source>
        <translation>PDF एक्सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="315"/>
        <source>Generate a PDF report for the selected session</source>
        <translation>चयनित सेशन के लिए PDF रिपोर्ट जेनरेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="329"/>
        <source>Restore Project</source>
        <translation>प्रोजेक्ट रिस्टोर करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="333"/>
        <source>Restore the project file from this session file</source>
        <translation>इस सेशन फ़ाइल से प्रोजेक्ट फ़ाइल रिस्टोर करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="402"/>
        <source>Loading session…</source>
        <translation>सत्र लोड हो रहा है…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="403"/>
        <source>Working…</source>
        <translation>कार्य जारी है…</translation>
    </message>
</context>
<context>
    <name>DatasetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="109"/>
        <source>Pro features detected in this project.</source>
        <translation>इस प्रोजेक्ट में Pro सुविधाएँ पाई गईं।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="111"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>फ़ॉलबैक विजेट उपयोग में हैं। पूर्ण कार्यक्षमता अनलॉक करने के लिए लाइसेंस खरीदें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="139"/>
        <source>Plots</source>
        <translation>प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="144"/>
        <source>Plot</source>
        <translation>प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="148"/>
        <source>Toggle 2D plot visualization for this dataset</source>
        <translation>इस डेटासेट के लिए 2D प्लॉट विज़ुअलाइज़ेशन टॉगल करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="160"/>
        <source>FFT Plot</source>
        <translation>FFT प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="163"/>
        <source>Toggle FFT plot to visualize frequency content</source>
        <translation>आवृत्ति सामग्री को विज़ुअलाइज़ करने के लिए FFT प्लॉट टॉगल करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="175"/>
        <source>Waterfall</source>
        <translation>वॉटरफॉल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="179"/>
        <source>Toggle waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>वॉटरफॉल (स्पेक्ट्रोग्राम) प्लॉट टॉगल करें — FFT सेटिंग्स का उपयोग करता है</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="196"/>
        <source>Widgets</source>
        <translation>विजेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="202"/>
        <source>Bar/Level</source>
        <translation>बार/लेवल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="206"/>
        <source>Toggle bar/level indicator for this dataset</source>
        <translation>इस डेटासेट के लिए बार/लेवल इंडिकेटर टॉगल करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="217"/>
        <source>Gauge</source>
        <translation>गेज</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="222"/>
        <source>Toggle gauge widget for analog-style display</source>
        <translation>एनालॉग-स्टाइल डिस्प्ले के लिए गेज विजेट टॉगल करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="234"/>
        <source>Compass</source>
        <translation>कंपास</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="238"/>
        <source>Toggle compass widget for directional data</source>
        <translation>दिशात्मक डेटा के लिए कंपास विजेट टॉगल करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="249"/>
        <source>Meter</source>
        <translation>मीटर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="254"/>
        <source>Toggle analog meter (half-arc) widget</source>
        <translation>एनालॉग मीटर (अर्ध-चाप) विजेट टॉगल करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="305"/>
        <source>Define colored value ranges with severity tiers for this dataset's gauge or LED.</source>
        <translation>इस डेटासेट के गेज या LED के लिए गंभीरता स्तरों के साथ रंगीन मान श्रेणियाँ परिभाषित करें।</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">थर्मामीटर</translation>
    </message>
    <message>
        <source>Toggle thermometer widget for temperature data</source>
        <translation type="vanished">तापमान डेटा के लिए थर्मामीटर विजेट टॉगल करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="265"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="270"/>
        <source>Toggle LED indicator for binary or thresholded values</source>
        <translation>बाइनरी या थ्रेशोल्ड मानों के लिए LED इंडिकेटर टॉगल करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="290"/>
        <source>Behavior</source>
        <translation>व्यवहार</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="295"/>
        <source>Alarm Bands</source>
        <translation>अलार्म बैंड्स</translation>
    </message>
    <message>
        <source>Define colored value ranges with severity tiers for this dataset's gauge.</source>
        <translation type="vanished">इस डेटासेट के गेज के लिए गंभीरता स्तरों के साथ रंगीन मान श्रेणियाँ परिभाषित करें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="311"/>
        <source>Transform</source>
        <translation>ट्रांसफ़ॉर्म</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="315"/>
        <source>Edit a value transform expression for calibration, filtering, or unit conversion</source>
        <translation>कैलिब्रेशन, फ़िल्टरिंग या यूनिट रूपांतरण के लिए मान ट्रांसफ़ॉर्म एक्सप्रेशन संपादित करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="328"/>
        <source>Duplicate</source>
        <translation>डुप्लिकेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="333"/>
        <source>Duplicate this dataset with the same configuration</source>
        <translation>इस डेटासेट को समान कॉन्फ़िगरेशन के साथ डुप्लिकेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="338"/>
        <source>Delete</source>
        <translation>डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="341"/>
        <source>Delete this dataset from the group</source>
        <translation>इस डेटासेट को समूह से डिलीट करें</translation>
    </message>
</context>
<context>
    <name>Donate</name>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="37"/>
        <source>Support Serial Studio</source>
        <translation>Serial Studio को सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="86"/>
        <source>Support the development of %1!</source>
        <translation>%1 के विकास को सपोर्ट करें!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="97"/>
        <source>Serial Studio is free &amp; open-source software supported by volunteers. Consider donating or obtaining a Pro license to support development efforts :)</source>
        <translation>Serial Studio एक फ़्री और ओपन-सोर्स सॉफ़्टवेयर है जिसे स्वयंसेवकों द्वारा सपोर्ट किया जाता है। विकास प्रयासों को सपोर्ट करने के लिए दान करने या Pro लाइसेंस प्राप्त करने पर विचार करें :)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="110"/>
        <source>You can also support this project by sharing it, reporting bugs and proposing new features!</source>
        <translation>आप इस प्रोजेक्ट को शेयर करके, बग रिपोर्ट करके और नई सुविधाएँ प्रस्तावित करके भी सपोर्ट कर सकते हैं!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="124"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="135"/>
        <source>Donate</source>
        <translation>दान करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="150"/>
        <source>Get Serial Studio Pro</source>
        <translation>Serial Studio Pro प्राप्त करें</translation>
    </message>
</context>
<context>
    <name>Downloader</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="142"/>
        <source>Stop</source>
        <translation>रोकें</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="143"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="377"/>
        <source>Downloading updates</source>
        <translation>अपडेट डाउनलोड हो रहे हैं</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="144"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="470"/>
        <source>Time remaining</source>
        <translation>शेष समय</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="144"/>
        <source>unknown</source>
        <translation>अज्ञात</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="243"/>
        <source>Error</source>
        <translation>त्रुटि</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="243"/>
        <source>Cannot find downloaded update!</source>
        <translation>डाउनलोड किया गया अपडेट नहीं मिला!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="259"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="260"/>
        <source>Download complete!</source>
        <translation>डाउनलोड पूर्ण!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="268"/>
        <source>Click "OK" to begin installing the update</source>
        <translation>अपडेट इंस्टॉल करना शुरू करने के लिए "OK" पर क्लिक करें</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="270"/>
        <source>In order to install the update, you may need to quit the application.</source>
        <translation>अपडेट इंस्टॉल करने के लिए, एप्लिकेशन बंद करना आवश्यक हो सकता है।</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="261"/>
        <source>The installer opens separately</source>
        <translation>इंस्टॉलर अलग से खुलता है</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="274"/>
        <source>In order to install the update, you may need to quit the application. This is a mandatory update, exiting now will close the application.</source>
        <translation>अपडेट इंस्टॉल करने के लिए, एप्लिकेशन बंद करना आवश्यक हो सकता है। यह अनिवार्य अपडेट है, अभी बाहर निकलने से एप्लिकेशन बंद हो जाएगा।</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="290"/>
        <source>Click the "Open" button to apply the update</source>
        <translation>अपडेट लागू करने के लिए "Open" बटन पर क्लिक करें</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="303"/>
        <source>Updater</source>
        <translation>अपडेटर</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="307"/>
        <source>Are you sure you want to cancel the download?</source>
        <translation>क्या आप वाकई डाउनलोड रद्द करना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="309"/>
        <source>Are you sure you want to cancel the download? This is a mandatory update, exiting now will close the application</source>
        <translation>क्या आप वाकई डाउनलोड रद्द करना चाहते हैं? यह अनिवार्य अपडेट है, अभी बाहर निकलने से एप्लिकेशन बंद हो जाएगा।</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="364"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="371"/>
        <source>%1 bytes</source>
        <translation>%1 बाइट्स</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="366"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="373"/>
        <source>%1 KB</source>
        <translation>%1 KB</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="368"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="375"/>
        <source>%1 MB</source>
        <translation>%1 MB</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="377"/>
        <source>of</source>
        <translation>का</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="421"/>
        <source>Downloading Updates</source>
        <translation>अपडेट डाउनलोड हो रहे हैं</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="422"/>
        <source>Time Remaining</source>
        <translation>शेष समय</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="422"/>
        <source>Unknown</source>
        <translation>अज्ञात</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="446"/>
        <source>about %1 hours</source>
        <translation>लगभग %1 घंटे</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="448"/>
        <source>about one hour</source>
        <translation>लगभग एक घंटा</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="456"/>
        <source>%1 minutes</source>
        <translation>%1 मिनट</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="458"/>
        <source>1 minute</source>
        <translation>1 मिनट</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="465"/>
        <source>%1 seconds</source>
        <translation>%1 सेकंड</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="467"/>
        <source>1 second</source>
        <translation>1 सेकंड</translation>
    </message>
    <message>
        <source>Time remaining: 0 minutes</source>
        <translation type="vanished">शेष समय: 0 मिनट</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">खोलें</translation>
    </message>
</context>
<context>
    <name>ExamplesBrowser</name>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="34"/>
        <source>Examples Browser</source>
        <translation>उदाहरण ब्राउज़र</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="120"/>
        <source>Back</source>
        <translation>वापस</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="152"/>
        <source>Pro</source>
        <translation>Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="173"/>
        <source>Download &amp;&amp; Open</source>
        <translation>डाउनलोड करें &amp;&amp; खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="188"/>
        <source>View on GitHub</source>
        <translation>GitHub पर देखें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="91"/>
        <source>Search in Examples…</source>
        <translation>उदाहरणों में खोजें…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="245"/>
        <source>Fetching examples…</source>
        <translation>उदाहरण प्राप्त किए जा रहे हैं…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="568"/>
        <source>Loading...</source>
        <translation>लोड हो रहा है...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="569"/>
        <source>No README available.</source>
        <translation>कोई README उपलब्ध नहीं है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="609"/>
        <source>Copied to Clipboard</source>
        <translation>क्लिपबोर्ड पर कॉपी किया गया</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="672"/>
        <source>No screenshot available</source>
        <translation>कोई स्क्रीनशॉट उपलब्ध नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="704"/>
        <source>Details</source>
        <translation>विवरण</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="733"/>
        <source>Info</source>
        <translation>जानकारी</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="756"/>
        <source>Category:</source>
        <translation>श्रेणी:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="769"/>
        <source>Difficulty:</source>
        <translation>कठिनाई:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="787"/>
        <source>Project:</source>
        <translation>प्रोजेक्ट:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="829"/>
        <source>No Results Found</source>
        <translation>कोई परिणाम नहीं मिला</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="840"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>वर्तनी जाँचें या कोई अन्य खोज शब्द आज़माएँ।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="855"/>
        <source>%1 examples</source>
        <translation>%1 उदाहरण</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="864"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>ExtensionManager</name>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="32"/>
        <source>Extension Manager</source>
        <translation>एक्सटेंशन मैनेजर</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="119"/>
        <source>Refresh</source>
        <translation>रिफ्रेश करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="133"/>
        <source>Repos</source>
        <translation>रिपॉजिटरी</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="163"/>
        <source>Repository Settings</source>
        <translation>रिपॉजिटरी सेटिंग्स</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="175"/>
        <source>Back</source>
        <translation>वापस जाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="216"/>
        <source>Install</source>
        <translation>इंस्टॉल करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="233"/>
        <source>Uninstall</source>
        <translation>अनइंस्टॉल करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="260"/>
        <source>Run</source>
        <translation>चलाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="284"/>
        <source>Stop</source>
        <translation>रोकें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="318"/>
        <source>Reset</source>
        <translation>रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="78"/>
        <source>Search extensions…</source>
        <translation>एक्सटेंशन खोजें…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="368"/>
        <source>Fetching extensions…</source>
        <translation>एक्सटेंशन प्राप्त किए जा रहे हैं…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="605"/>
        <source>Running</source>
        <translation>चल रहा है</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Update</source>
        <translation>अपडेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Installed</source>
        <translation>इंस्टॉल किया गया</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="644"/>
        <source>Unavailable</source>
        <translation>अनुपलब्ध</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="823"/>
        <source>No description available.</source>
        <translation>कोई विवरण उपलब्ध नहीं है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="864"/>
        <source>Details</source>
        <translation>विवरण</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="885"/>
        <source>Type:</source>
        <translation>प्रकार:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="898"/>
        <source>Author:</source>
        <translation>लेखक:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="910"/>
        <source>Version:</source>
        <translation>संस्करण:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="922"/>
        <source>License:</source>
        <translation>लाइसेंस:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="983"/>
        <source>No preview</source>
        <translation>कोई पूर्वावलोकन नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1012"/>
        <source>  PLUGIN OUTPUT</source>
        <translation>प्लगइन आउटपुट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1042"/>
        <source>No output yet. Run the plugin to see its log here.</source>
        <translation>अभी तक कोई आउटपुट नहीं। इसका लॉग यहाँ देखने के लिए प्लगइन चलाएं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1077"/>
        <source>No preview available</source>
        <translation>कोई पूर्वावलोकन उपलब्ध नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1121"/>
        <source>Repositories</source>
        <translation>रिपॉजिटरी</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1134"/>
        <source>Add URLs to remote repositories or local folder paths.</source>
        <translation>रिमोट रिपॉजिटरी या स्थानीय फ़ोल्डर पथ के URL जोड़ें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1171"/>
        <source>LOCAL</source>
        <translation>स्थानीय</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1228"/>
        <source>URL or local path…</source>
        <translation>URL या स्थानीय पथ…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1259"/>
        <source>Browse…</source>
        <translation>ब्राउज़ करें…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1236"/>
        <source>Add</source>
        <translation>जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1296"/>
        <source>No Results Found</source>
        <translation>कोई परिणाम नहीं मिला</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1307"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>वर्तनी जांचें या कोई अन्य खोज शब्द आज़माएं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1331"/>
        <source>No Extensions Available</source>
        <translation>कोई एक्सटेंशन उपलब्ध नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1342"/>
        <source>Add a repository URL or local path in the Repos settings, then refresh.</source>
        <translation>रिपॉज़िटरी सेटिंग्स में URL या स्थानीय पथ जोड़ें, फिर रिफ़्रेश करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1357"/>
        <source>%1 extensions</source>
        <translation>%1 एक्सटेंशन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1366"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>FFTPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="167"/>
        <source>Interpolation: %1</source>
        <translation>इंटरपोलेशन: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="195"/>
        <source>Show Area Under Plot</source>
        <translation>प्लॉट के नीचे क्षेत्र दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="213"/>
        <source>Show X Axis Label</source>
        <translation>X अक्ष लेबल दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="225"/>
        <source>Show Y Axis Label</source>
        <translation>Y अक्ष लेबल दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="243"/>
        <source>Show Crosshair</source>
        <translation>क्रॉसहेयर दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Pause</source>
        <translation>रोकें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Resume</source>
        <translation>फिर से शुरू करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="269"/>
        <source>Reset View</source>
        <translation>व्यू रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="275"/>
        <source>Axis Range Settings</source>
        <translation>अक्ष रेंज सेटिंग्स</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="304"/>
        <source>Magnitude (dB)</source>
        <translation>मैग्निट्यूड (dB)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="305"/>
        <source>Frequency (Hz)</source>
        <translation>फ्रीक्वेंसी (Hz)</translation>
    </message>
</context>
<context>
    <name>FileDropArea</name>
    <message>
        <location filename="../../qml/Widgets/FileDropArea.qml" line="130"/>
        <source>Drop Projects and CSV files here</source>
        <translation>प्रोजेक्ट और CSV फ़ाइलें यहां ड्रॉप करें</translation>
    </message>
</context>
<context>
    <name>FileTransmission</name>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="34"/>
        <source>File Transmission</source>
        <translation>फ़ाइल ट्रांसमिशन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="102"/>
        <source>Transfer Protocol:</source>
        <translation>ट्रांसफर प्रोटोकॉल:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="135"/>
        <source>File Selection:</source>
        <translation>फ़ाइल चयन:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="152"/>
        <source>Select File…</source>
        <translation>फ़ाइल चुनें…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="170"/>
        <source>Transmission Interval:</source>
        <translation>ट्रांसमिशन अंतराल:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="196"/>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="272"/>
        <source>msecs</source>
        <translation>msecs</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="206"/>
        <source>Block Size:</source>
        <translation>ब्लॉक आकार:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="234"/>
        <source>bytes</source>
        <translation>bytes</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="244"/>
        <source>Timeout:</source>
        <translation>टाइमआउट:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="282"/>
        <source>Max Retries:</source>
        <translation>अधिकतम पुनः प्रयास:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="340"/>
        <source>Progress: %1%</source>
        <translation>प्रगति: %1%</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="373"/>
        <source>%1 / %2 bytes</source>
        <translation>%1 / %2 bytes</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="381"/>
        <source>Errors: %1</source>
        <translation>त्रुटियाँ: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="461"/>
        <source>Activity Log</source>
        <translation>गतिविधि लॉग</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="465"/>
        <source>Clear</source>
        <translation>साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="419"/>
        <source>Pause Transmission</source>
        <translation>ट्रांसमिशन रोकें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="420"/>
        <source>Resume Transmission</source>
        <translation>ट्रांसमिशन फिर से शुरू करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="423"/>
        <source>Stop Transmission</source>
        <translation>ट्रांसमिशन बंद करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="424"/>
        <source>Begin Transmission</source>
        <translation>ट्रांसमिशन शुरू करें</translation>
    </message>
</context>
<context>
    <name>FlowDiagram</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="159"/>
        <source>Frame Parser</source>
        <translation>फ़्रेम पार्सर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="166"/>
        <source>Device %1</source>
        <translation>डिवाइस %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="399"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1446"/>
        <source>Output Panel</source>
        <translation>आउटपुट पैनल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="443"/>
        <source>Control</source>
        <translation>नियंत्रण</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="466"/>
        <source>Table</source>
        <translation>टेबल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="482"/>
        <source>%1 regs</source>
        <translation>%1 रजिस्टर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="483"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="505"/>
        <source>empty</source>
        <translation>खाली</translation>
    </message>
    <message>
        <source>Control Script</source>
        <translation type="vanished">कंट्रोल स्क्रिप्ट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="528"/>
        <source>MQTT Publisher</source>
        <translation>MQTT Publisher</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="931"/>
        <source>Open the transform code editor for this dataset.</source>
        <translation>इस डेटासेट के लिए ट्रांसफ़ॉर्म कोड एडिटर खोलें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1216"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1219"/>
        <source>Dataset Container</source>
        <translation>डेटासेट कंटेनर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1228"/>
        <source>Multi-Plot</source>
        <translation>मल्टी-प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1231"/>
        <source>Multiple Plot</source>
        <translation>मल्टीपल प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1240"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1243"/>
        <source>Accelerometer</source>
        <translation>एक्सेलेरोमीटर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1252"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1255"/>
        <source>Gyroscope</source>
        <translation>जायरोस्कोप</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1264"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1267"/>
        <source>GPS Map</source>
        <translation>GPS मैप</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1275"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1278"/>
        <source>3D Plot</source>
        <translation>3D प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1286"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1289"/>
        <source>Image View</source>
        <translation>इमेज व्यू</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1298"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1301"/>
        <source>Painter Widget</source>
        <translation>पेंटर विजेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1310"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1313"/>
        <source>Web View</source>
        <translation>वेब व्यू</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1322"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1325"/>
        <source>Data Grid</source>
        <translation>डेटा ग्रिड</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1338"/>
        <source>Generic</source>
        <translation>जेनेरिक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1351"/>
        <source>Plot</source>
        <translation>प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1364"/>
        <source>FFT Plot</source>
        <translation>FFT प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1377"/>
        <source>Gauge</source>
        <translation>गेज</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1390"/>
        <source>Level Indicator</source>
        <translation>लेवल इंडिकेटर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1403"/>
        <source>Compass</source>
        <translation>कंपास</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1416"/>
        <source>Meter</source>
        <translation>मीटर</translation>
    </message>
    <message>
        <source>Edit Control Script…</source>
        <translation type="vanished">कंट्रोल स्क्रिप्ट संपादित करें…</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">थर्मामीटर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="503"/>
        <source>Control Loop</source>
        <translation>नियंत्रण लूप</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1429"/>
        <source>LED Indicator</source>
        <translation>LED इंडिकेटर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1458"/>
        <source>Slider</source>
        <translation>स्लाइडर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1471"/>
        <source>Toggle</source>
        <translation>टॉगल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1484"/>
        <source>Knob</source>
        <translation>नॉब</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1497"/>
        <source>Text Field</source>
        <translation>टेक्स्ट फ़ील्ड</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1510"/>
        <source>Button</source>
        <translation>बटन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1534"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1610"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1698"/>
        <source>Add Group</source>
        <translation>ग्रुप जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1550"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1626"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1714"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1759"/>
        <source>Add Dataset</source>
        <translation>डेटासेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1564"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1640"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1728"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1773"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1980"/>
        <source>Add Output</source>
        <translation>आउटपुट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1580"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1653"/>
        <source>Add Action</source>
        <translation>एक्शन जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1589"/>
        <source>Add Data Source</source>
        <translation>डेटा सोर्स जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1596"/>
        <source>Add Data Table</source>
        <translation>डेटा टेबल जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1664"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1800"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1867"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1995"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2029"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2085"/>
        <source>Rename…</source>
        <translation>नाम बदलें…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1672"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1830"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1900"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1952"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2003"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2059"/>
        <source>Duplicate</source>
        <translation>डुप्लिकेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1683"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1841"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1912"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1964"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2014"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2070"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2096"/>
        <source>Delete…</source>
        <translation>डिलीट करें…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1744"/>
        <source>Edit Frame Parser…</source>
        <translation>फ्रेम पार्सर एडिट करें…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1786"/>
        <source>Edit Painter Code…</source>
        <translation>पेंटर कोड एडिट करें…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1808"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1876"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1928"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2037"/>
        <source>Move Up</source>
        <translation>ऊपर ले जाएं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1819"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1888"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1940"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2048"/>
        <source>Move Down</source>
        <translation>नीचे ले जाएं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1856"/>
        <source>Edit Transform Code…</source>
        <translation>ट्रांसफ़ॉर्म कोड एडिट करें…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2111"/>
        <source>Edit Code…</source>
        <translation>कोड एडिट करें…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2126"/>
        <source>Edit Control Loop…</source>
        <translation>नियंत्रण लूप संपादित करें…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="222"/>
        <source>Group</source>
        <translation>समूह</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="349"/>
        <source>Action</source>
        <translation>एक्शन</translation>
    </message>
    <message>
        <source>No groups defined yet</source>
        <translation type="vanished">अभी तक कोई समूह परिभाषित नहीं है</translation>
    </message>
</context>
<context>
    <name>FrameParserTest</name>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="39"/>
        <source>Test Frame Parser</source>
        <translation>फ़्रेम पार्सर परीक्षण करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="96"/>
        <source>Frame %1</source>
        <translation>फ्रेम %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="98"/>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="209"/>
        <source>Decoder</source>
        <translation>डिकोडर</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="100"/>
        <source>Rows</source>
        <translation>पंक्तियाँ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="101"/>
        <source>%1 row(s)</source>
        <translation>%1 पंक्ति(याँ)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="105"/>
        <source>Row %1</source>
        <translation>पंक्ति %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="164"/>
        <source>Pipeline Configuration</source>
        <translation>पाइपलाइन कॉन्फ़िगरेशन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="186"/>
        <source>Detection</source>
        <translation>डिटेक्शन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="232"/>
        <source>Start</source>
        <translation>प्रारंभ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="253"/>
        <source>End</source>
        <translation>समाप्त</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="274"/>
        <source>Checksum</source>
        <translation>चेकसम</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="299"/>
        <source>Hex Delimiters</source>
        <translation>हेक्स डिलीमिटर</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="315"/>
        <source>Frame Data Input</source>
        <translation>फ्रेम डेटा इनपुट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="342"/>
        <source>Enter hex bytes (e.g. 01 A2 FF)</source>
        <translation>हेक्स बाइट्स दर्ज करें (जैसे 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="343"/>
        <source>Enter raw stream bytes here...</source>
        <translation>यहाँ रॉ स्ट्रीम बाइट्स दर्ज करें...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="362"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="387"/>
        <source>The sample does not contain the configured frame delimiters, so no frame will be extracted. Type them into the sample (e.g. 
 for a newline) or adjust the detection mode.</source>
        <translation>सैंपल में कॉन्फ़िगर किए गए फ्रेम डिलीमिटर नहीं हैं, इसलिए कोई फ्रेम निकाला नहीं जाएगा। उन्हें सैंपल में टाइप करें (जैसे 
 न्यूलाइन के लिए) या डिटेक्शन मोड समायोजित करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="407"/>
        <source>Pipeline Results</source>
        <translation>पाइपलाइन परिणाम</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="480"/>
        <source>Stage</source>
        <translation>स्टेज</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="487"/>
        <source>Value</source>
        <translation>मान</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="530"/>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation>निष्कर्षण से पूर्ण फ़्रेम नहीं बना। प्रारंभ / समाप्ति सीमांकक और पहचान मोड की जाँच करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="532"/>
        <source>Enter sample data above and press Evaluate to preview the parsed output</source>
        <translation>ऊपर नमूना डेटा दर्ज करें और पार्स किए गए आउटपुट का पूर्वावलोकन करने के लिए मूल्यांकन करें दबाएँ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="614"/>
        <source>Clear</source>
        <translation>साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="625"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="632"/>
        <source>Evaluate</source>
        <translation>मूल्यांकन करें</translation>
    </message>
</context>
<context>
    <name>FrameParserView</name>
    <message>
        <source>Undo</source>
        <translation type="vanished">पूर्ववत करें</translation>
    </message>
    <message>
        <source>Redo</source>
        <translation type="vanished">पुनः करें</translation>
    </message>
    <message>
        <source>Cut</source>
        <translation type="vanished">काटें</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="vanished">कॉपी करें</translation>
    </message>
    <message>
        <source>Paste</source>
        <translation type="vanished">पेस्ट करें</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation type="vanished">सभी चुनें</translation>
    </message>
    <message>
        <source>Format Document</source>
        <translation type="vanished">दस्तावेज़ फ़ॉर्मेट करें</translation>
    </message>
    <message>
        <source>Format Selection</source>
        <translation type="vanished">चयन फ़ॉर्मेट करें</translation>
    </message>
    <message>
        <source>Reset</source>
        <translation type="vanished">रीसेट करें</translation>
    </message>
    <message>
        <source>Reset to the default parsing script</source>
        <translation type="vanished">डिफ़ॉल्ट पार्सिंग स्क्रिप्ट पर रीसेट करें</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">खोलें</translation>
    </message>
    <message>
        <source>Import a script file for data parsing</source>
        <translation type="vanished">डेटा पार्सिंग के लिए स्क्रिप्ट फ़ाइल आयात करें</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">डेटा पार्सिंग के लिए सहायता दस्तावेज़ खोलें</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">भाषा:</translation>
    </message>
    <message>
        <source>Select Template…</source>
        <translation type="vanished">टेम्पलेट चुनें…</translation>
    </message>
    <message>
        <source>Undo the last code edit</source>
        <translation type="vanished">अंतिम कोड संपादन पूर्ववत करें</translation>
    </message>
    <message>
        <source>Redo the previously undone edit</source>
        <translation type="vanished">पहले पूर्ववत किया गया संपादन फिर से करें</translation>
    </message>
    <message>
        <source>Cut selected code to clipboard</source>
        <translation type="vanished">चयनित कोड को क्लिपबोर्ड पर कट करें</translation>
    </message>
    <message>
        <source>Copy selected code to clipboard</source>
        <translation type="vanished">चयनित कोड को क्लिपबोर्ड पर कॉपी करें</translation>
    </message>
    <message>
        <source>Paste code from clipboard</source>
        <translation type="vanished">क्लिपबोर्ड से कोड पेस्ट करें</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="vanished">सहायता</translation>
    </message>
    <message>
        <source>Test With Sample Data</source>
        <translation type="vanished">सैंपल डेटा से परीक्षण करें</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">मूल्यांकन करें</translation>
    </message>
</context>
<context>
    <name>GPS</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="105"/>
        <source>Auto Center</source>
        <translation>स्वतः केंद्रित करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="121"/>
        <source>Plot Trajectory</source>
        <translation>ट्रैजेक्टरी प्लॉट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="138"/>
        <source>Zoom In</source>
        <translation>ज़ूम इन करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="149"/>
        <source>Zoom Out</source>
        <translation>ज़ूम आउट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="173"/>
        <source>Show Weather</source>
        <translation>मौसम दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="191"/>
        <source>NASA Weather Overlay</source>
        <translation>NASA मौसम ओवरले</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="223"/>
        <source>Base Map: %1</source>
        <translation>बेस मैप: %1</translation>
    </message>
</context>
<context>
    <name>GroupView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="97"/>
        <source>Pro features detected in this project.</source>
        <translation>इस प्रोजेक्ट में Pro सुविधाएँ पाई गईं।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="99"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>फ़ॉलबैक विजेट्स का उपयोग किया जा रहा है। पूर्ण कार्यक्षमता अनलॉक करने के लिए लाइसेंस खरीदें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="129"/>
        <source>Add Dataset</source>
        <translation>डेटासेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="136"/>
        <source>Dataset</source>
        <translation>डेटासेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="139"/>
        <source>Add a generic dataset to the current group</source>
        <translation>वर्तमान ग्रुप में एक सामान्य डेटासेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="146"/>
        <source>Plot</source>
        <translation>प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="150"/>
        <source>Add a 2D plot to visualize numeric data</source>
        <translation>संख्यात्मक डेटा को विज़ुअलाइज़ करने के लिए 2D प्लॉट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="158"/>
        <source>FFT Plot</source>
        <translation>FFT प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="163"/>
        <source>Add an FFT plot for frequency domain visualization</source>
        <translation>फ़्रीक्वेंसी डोमेन विज़ुअलाइज़ेशन के लिए FFT प्लॉट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="169"/>
        <source>Waterfall</source>
        <translation>वॉटरफॉल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="174"/>
        <source>Add a waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>वॉटरफॉल (स्पेक्ट्रोग्राम) प्लॉट जोड़ें — FFT सेटिंग्स का उपयोग करता है</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="180"/>
        <source>Bar/Level</source>
        <translation>बार/लेवल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="184"/>
        <source>Add a bar or level indicator for scaled values</source>
        <translation>स्केल किए गए मानों के लिए बार या लेवल इंडिकेटर जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="190"/>
        <source>Gauge</source>
        <translation>गेज</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="195"/>
        <source>Add a gauge widget for analog-style visualization</source>
        <translation>एनालॉग-स्टाइल विज़ुअलाइज़ेशन के लिए गेज विजेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="202"/>
        <source>Compass</source>
        <translation>कंपास</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="206"/>
        <source>Add a compass to display directional or angular data</source>
        <translation>दिशात्मक या कोणीय डेटा प्रदर्शित करने के लिए कंपास जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="212"/>
        <source>Meter</source>
        <translation>मीटर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="216"/>
        <source>Add an analog meter (half-arc) widget</source>
        <translation>एनालॉग मीटर (अर्ध-चाप) विजेट जोड़ें</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">थर्मामीटर</translation>
    </message>
    <message>
        <source>Add a thermometer widget for temperature data</source>
        <translation type="vanished">तापमान डेटा के लिए थर्मामीटर विजेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="223"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="228"/>
        <source>Add an LED indicator for binary status signals</source>
        <translation>बाइनरी स्टेटस सिग्नल के लिए LED इंडिकेटर जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="241"/>
        <source>Add Control</source>
        <translation>नियंत्रण जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="247"/>
        <source>Button</source>
        <translation>बटन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="251"/>
        <source>Add a button that sends a command on click</source>
        <translation>क्लिक पर कमांड भेजने वाला बटन जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="257"/>
        <source>Slider</source>
        <translation>स्लाइडर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="261"/>
        <source>Add a slider for sending scaled numeric values</source>
        <translation>स्केल किए गए संख्यात्मक मान भेजने के लिए स्लाइडर जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="267"/>
        <source>Toggle</source>
        <translation>टॉगल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="271"/>
        <source>Add a toggle switch for on/off commands</source>
        <translation>ऑन/ऑफ कमांड के लिए टॉगल स्विच जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="278"/>
        <source>Text Field</source>
        <translation>टेक्स्ट फ़ील्ड</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="281"/>
        <source>Add a text field for typing and sending commands</source>
        <translation>कमांड टाइप करने और भेजने के लिए टेक्स्ट फ़ील्ड जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="287"/>
        <source>Knob</source>
        <translation>नॉब</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="291"/>
        <source>Add a rotary knob for setpoint control</source>
        <translation>सेटपॉइंट नियंत्रण के लिए रोटरी नॉब जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="310"/>
        <source>Edit Code</source>
        <translation>कोड एडिट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="314"/>
        <source>Edit the JavaScript that draws this painter widget</source>
        <translation>JavaScript एडिट करें जो इस पेंटर विजेट को ड्रॉ करता है</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="327"/>
        <source>Duplicate</source>
        <translation>डुप्लिकेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="331"/>
        <source>Duplicate the current group and its contents</source>
        <translation>वर्तमान ग्रुप और उसकी सामग्री को डुप्लिकेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="336"/>
        <source>Delete</source>
        <translation>डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="341"/>
        <source>Delete the current group and all contained datasets</source>
        <translation>वर्तमान समूह और सभी समाहित डेटासेट हटाएं</translation>
    </message>
</context>
<context>
    <name>Gyroscope</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="410"/>
        <source>ROLL ↔</source>
        <translation>ROLL ↔</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="440"/>
        <source>YAW ↻</source>
        <translation>YAW ↻</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="470"/>
        <source>PITCH ↕</source>
        <translation>PITCH ↕</translation>
    </message>
</context>
<context>
    <name>HID</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="50"/>
        <source>HID Device</source>
        <translation>HID डिवाइस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="80"/>
        <source>Usage Page</source>
        <translation>उपयोग पृष्ठ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="96"/>
        <source>Usage</source>
        <translation>उपयोग</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="137"/>
        <source>Connect gamepads, joysticks, steering wheels, flight controllers, and other HID-class USB devices.</source>
        <translation>गेमपैड, जॉयस्टिक, स्टीयरिंग व्हील, फ्लाइट कंट्रोलर और अन्य HID-क्लास USB डिवाइस कनेक्ट करें।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="145"/>
        <source>HID Usage Tables (USB.org)</source>
        <translation>HID उपयोग तालिकाएं (USB.org)</translation>
    </message>
</context>
<context>
    <name>HelpCenter</name>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="33"/>
        <source>Help Center</source>
        <translation>सहायता केंद्र</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="95"/>
        <source>Fetching help pages…</source>
        <translation>सहायता पृष्ठ लाए जा रहे हैं…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="129"/>
        <source>Search…</source>
        <translation>खोजें…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="247"/>
        <source>Loading…</source>
        <translation>लोड हो रहा है…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="291"/>
        <source>Select a page from the sidebar</source>
        <translation>साइडबार से एक पृष्ठ चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="321"/>
        <source>Copied to Clipboard</source>
        <translation>क्लिपबोर्ड पर कॉपी किया गया</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="353"/>
        <source>View Online</source>
        <translation>ऑनलाइन देखें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="372"/>
        <source>%1 pages</source>
        <translation>%1 पृष्ठ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="381"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>IO::ConnectionManager</name>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="269"/>
        <source>UART/COM</source>
        <translation>UART/COM</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="270"/>
        <source>Network Socket</source>
        <translation>नेटवर्क सॉकेट</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="271"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="273"/>
        <source>Audio</source>
        <translation>ऑडियो</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="274"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="275"/>
        <source>CAN Bus</source>
        <translation>CAN Bus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="276"/>
        <source>USB Device</source>
        <translation>USB डिवाइस</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="277"/>
        <source>HID Device</source>
        <translation>HID डिवाइस</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="278"/>
        <source>Process</source>
        <translation>प्रोसेस</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="279"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT सब्सक्राइबर</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="685"/>
        <source>Your trial period has ended.</source>
        <translation>आपकी ट्रायल अवधि समाप्त हो गई है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="686"/>
        <source>To continue using Serial Studio, please activate your license.</source>
        <translation>Serial Studio का उपयोग जारी रखने के लिए, कृपया अपना लाइसेंस सक्रिय करें।</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Audio</name>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="752"/>
        <source>channels</source>
        <translation>चैनल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="805"/>
        <source> channels</source>
        <translation>चैनल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="953"/>
        <source>Unsigned 8-bit</source>
        <translation>Unsigned 8-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="954"/>
        <source>Signed 16-bit</source>
        <translation>Signed 16-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="955"/>
        <source>Signed 24-bit</source>
        <translation>Signed 24-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="956"/>
        <source>Signed 32-bit</source>
        <translation>Signed 32-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="957"/>
        <source>Float 32-bit</source>
        <translation>Float 32-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="960"/>
        <source>Mono</source>
        <translation>मोनो</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="961"/>
        <source>Stereo</source>
        <translation>स्टीरियो</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1342"/>
        <source>Input Device</source>
        <translation>इनपुट डिवाइस</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1350"/>
        <source>Sample Rate</source>
        <translation>सैम्पल रेट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1358"/>
        <source>Sample Format</source>
        <translation>सैम्पल फ़ॉर्मेट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1366"/>
        <source>Channels</source>
        <translation>चैनल</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::BluetoothLE</name>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="456"/>
        <source>BLE I/O Module Error</source>
        <translation>BLE I/O मॉड्यूल त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="751"/>
        <source>Select Device</source>
        <translation>डिवाइस चुनें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="762"/>
        <source>Select Service</source>
        <translation>सेवा चुनें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="773"/>
        <source>Select Characteristic</source>
        <translation>विशेषता चुनें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="972"/>
        <source>Error while configuring BLE service</source>
        <translation>BLE सेवा कॉन्फ़िगर करते समय त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1190"/>
        <source>Operation error</source>
        <translation>ऑपरेशन त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1193"/>
        <source>Characteristic write error</source>
        <translation>विशेषता लिखने में त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1196"/>
        <source>Descriptor write error</source>
        <translation>डिस्क्रिप्टर लिखने में त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1199"/>
        <source>Unknown error</source>
        <translation>अज्ञात त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1202"/>
        <source>Characteristic read error</source>
        <translation>विशेषता पढ़ने में त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1205"/>
        <source>Descriptor read error</source>
        <translation>डिस्क्रिप्टर पढ़ने में त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1440"/>
        <source>BLE Device</source>
        <translation>BLE डिवाइस</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1448"/>
        <source>Service</source>
        <translation>सेवा</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1455"/>
        <source>Notify Characteristic</source>
        <translation>विशेषता को सूचित करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1462"/>
        <source>Characteristic</source>
        <translation>विशेषता</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::CANBus</name>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="318"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="324"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="330"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="335"/>
        <source>CAN Bus Not Available</source>
        <translation>CAN Bus उपलब्ध नहीं</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="331"/>
        <source>No CAN bus plugins found on this system.

CAN bus support on macOS is limited and may require third-party hardware drivers.</source>
        <translation>इस सिस्टम पर कोई CAN bus प्लगइन नहीं मिला।

macOS पर CAN bus समर्थन सीमित है और तृतीय-पक्ष हार्डवेयर ड्राइवर की आवश्यकता हो सकती है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="336"/>
        <source>No CAN bus plugins are available on this platform.</source>
        <translation>इस प्लेटफ़ॉर्म पर कोई CAN bus प्लगइन उपलब्ध नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="348"/>
        <source>Invalid CAN Configuration</source>
        <translation>अमान्य CAN कॉन्फ़िगरेशन</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="356"/>
        <source>Invalid Selection</source>
        <translation>अमान्य चयन</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="365"/>
        <source>No Devices Available</source>
        <translation>कोई डिवाइस उपलब्ध नहीं</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="274"/>
        <source>CAN Device Creation Failed</source>
        <translation>CAN डिवाइस निर्माण विफल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="301"/>
        <source>CAN Connection Failed</source>
        <translation>CAN कनेक्शन विफल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="319"/>
        <source>No CAN bus plugins found on this system.

On Linux, ensure SocketCAN kernel modules are loaded.</source>
        <translation>इस सिस्टम पर कोई CAN बस प्लगइन नहीं मिला।

Linux पर, सुनिश्चित करें कि SOCKETCAN कर्नेल मॉड्यूल लोड हैं।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="325"/>
        <source>No CAN bus plugins found on this system.

On Windows, install CAN hardware drivers (PEAK, Vector, etc.).</source>
        <translation>इस सिस्टम पर कोई CAN बस प्लगइन नहीं मिला।

Windows पर, CAN हार्डवेयर ड्राइवर इंस्टॉल करें (PEAK, VECTOR, आदि)।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="349"/>
        <source>The CAN bus configuration is incomplete. Select a valid plugin and interface.</source>
        <translation>CAN बस कॉन्फ़िगरेशन अधूरा है। मान्य प्लगइन और इंटरफ़ेस चुनें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="357"/>
        <source>The selected plugin or interface is no longer available. Refresh the lists and try again.</source>
        <translation>चयनित प्लगइन या इंटरफ़ेस अब उपलब्ध नहीं है। सूची रिफ़्रेश करें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="366"/>
        <source>The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected.</source>
        <translation>प्लगइन या इंटरफ़ेस सूची खाली है। सूची रिफ़्रेश करें और सुनिश्चित करें कि आपका CAN हार्डवेयर कनेक्ट है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="275"/>
        <source>Unable to create CAN bus device. Check your hardware and drivers.</source>
        <translation>CAN बस डिवाइस बनाने में असमर्थ। अपना हार्डवेयर और ड्राइवर जाँचें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="299"/>
        <source>Unable to connect to CAN bus device. Check your hardware connection and settings.</source>
        <translation>CAN Bus डिवाइस से कनेक्ट करने में असमर्थ। अपने हार्डवेयर कनेक्शन और सेटिंग्स जाँचें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="695"/>
        <source>CAN Bus Error</source>
        <translation>CAN Bus त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="696"/>
        <source>An error occurred but the CAN device is no longer available.</source>
        <translation>एक त्रुटि हुई लेकिन CAN डिवाइस अब उपलब्ध नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="703"/>
        <source>Error code: %1</source>
        <translation>त्रुटि कोड: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="706"/>
        <source>CAN Bus Communication Error</source>
        <translation>CAN Bus संचार त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="783"/>
        <source>No CAN driver selected</source>
        <translation>कोई CAN ड्राइवर चयनित नहीं</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="725"/>
        <source>Load SocketCAN kernel modules first</source>
        <translation>पहले SOCKETCAN कर्नेल मॉड्यूल लोड करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="721"/>
        <source>Connect a %1 adapter, then refresh</source>
        <translation>%1 एडाप्टर कनेक्ट करें, फिर रिफ़्रेश करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="728"/>
        <source>Set up a virtual CAN interface first</source>
        <translation>पहले एक वर्चुअल CAN इंटरफ़ेस सेट करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="730"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="750"/>
        <source>No interfaces found for %1</source>
        <translation>%1 के लिए कोई इंटरफ़ेस नहीं मिला</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="734"/>
        <source>Install &lt;a href='https://www.peak-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN drivers&lt;/a&gt;</source>
        <translation>&lt;a href='https://www.PEAK-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN ड्राइवर&lt;/a&gt; इंस्टॉल करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="738"/>
        <source>Install &lt;a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'&gt;Vector CAN drivers&lt;/a&gt;</source>
        <translation>&lt;a href='https://www.VECTOR.com/us/en/products/products-a-z/libraries-drivers/'&gt;VECTOR CAN ड्राइवर&lt;/a&gt; इंस्टॉल करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="742"/>
        <source>Install &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN drivers&lt;/a&gt;</source>
        <translation>&lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN ड्राइवर&lt;/a&gt; इंस्टॉल करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="745"/>
        <source>Install %1 drivers</source>
        <translation>%1 ड्राइवर इंस्टॉल करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="748"/>
        <source>Install %1 drivers for macOS</source>
        <translation>macOS के लिए %1 ड्राइवर इंस्टॉल करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="762"/>
        <source>

If the interface is down, bring it up first:
sudo ip link set %1 up type can bitrate %2</source>
        <translation>यदि इंटरफ़ेस डाउन है, तो पहले इसे अप करें:
sudo ip link set %1 up type can bitrate %2

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="864"/>
        <source>Plugin</source>
        <translation>प्लगइन</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="872"/>
        <source>Interface</source>
        <translation>इंटरफ़ेस</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="880"/>
        <source>Bitrate</source>
        <translation>बिटरेट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="889"/>
        <source>CAN FD</source>
        <translation>CAN FD</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="896"/>
        <source>Loopback</source>
        <translation>लूपबैक</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="903"/>
        <source>Listen-Only</source>
        <translation>केवल-सुनने योग्य</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::GsUsbCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="382"/>
        <source>Failed to initialize libusb for the CANable adapter.</source>
        <translation>CANable एडाप्टर के लिए libusb को आरंभ करने में विफल।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="410"/>
        <source>Unable to enumerate USB devices.</source>
        <translation>USB डिवाइस की गणना करने में असमर्थ।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="430"/>
        <source>The selected CANable adapter is no longer connected, or another application has it open. On Windows the device must use the WinUSB driver (candleLight installs it automatically).</source>
        <translation>चयनित CANable एडाप्टर अब कनेक्ट नहीं है, या कोई अन्य एप्लिकेशन इसे खोले हुए है। Windows पर डिवाइस को WinUSB ड्राइवर का उपयोग करना चाहिए (candleLight इसे स्वचालित रूप से इंस्टॉल करता है)।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="443"/>
        <source>Could not claim the CANable USB interface.</source>
        <translation>CANable USB इंटरफ़ेस को क्लेम नहीं किया जा सका।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="510"/>
        <source>CANable adapter is not open for writing.</source>
        <translation>CANable एडाप्टर लिखने के लिए खुला नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="545"/>
        <source>Failed to transmit CAN frame to the adapter.</source>
        <translation>CAN फ़्रेम को एडाप्टर पर ट्रांसमिट करने में विफल।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="561"/>
        <source>CAN bus error reported by the CANable adapter.</source>
        <translation>CANable एडाप्टर द्वारा CAN Bus त्रुटि रिपोर्ट की गई।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="616"/>
        <source>A CAN frame was not acknowledged on the bus.</source>
        <translation>CAN फ़्रेम को बस पर स्वीकार नहीं किया गया।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="715"/>
        <source>CANable adapter rejected the host-format handshake.</source>
        <translation>CANable एडाप्टर ने होस्ट-फ़ॉर्मेट हैंडशेक को अस्वीकार कर दिया।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="722"/>
        <source>Could not read CANable timing constants.</source>
        <translation>CANable टाइमिंग कॉन्स्टेंट्स नहीं पढ़े जा सके।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="728"/>
        <source>The bitrate %1 bps is not supported by this CANable adapter.</source>
        <translation>बिटरेट %1 bps इस CANable एडाप्टर द्वारा समर्थित नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="734"/>
        <source>CANable adapter rejected the requested bitrate.</source>
        <translation>CANable एडाप्टर ने अनुरोधित बिटरेट को अस्वीकार कर दिया।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="748"/>
        <source>Could not start the CANable channel.</source>
        <translation>CANable चैनल प्रारंभ नहीं हो सका।</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::HID</name>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="173"/>
        <source>Unknown error</source>
        <translation>अज्ञात त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="176"/>
        <source>

Check that your user is in the 'plugdev' group or that a udev rule grants access to this device.</source>
        <translation>जांचें कि आपका उपयोगकर्ता 'plugdev' समूह में है या कोई udev नियम इस डिवाइस तक पहुंच प्रदान करता है।

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="180"/>
        <source>Failed to open "%1"</source>
        <translation>"%1" खोलने में विफल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="285"/>
        <source>HID Device Error</source>
        <translation>HID डिवाइस त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="286"/>
        <source>The HID device was disconnected or encountered a fatal read error.</source>
        <translation>HID डिवाइस डिस्कनेक्ट हो गया या गंभीर रीड त्रुटि आई।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="395"/>
        <source>Select Device</source>
        <translation>डिवाइस चुनें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="537"/>
        <source>HID Device</source>
        <translation>HID डिवाइस</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::MQTT</name>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="55"/>
        <source>MQTT 3.1</source>
        <translation>MQTT 3.1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="56"/>
        <source>MQTT 3.1.1</source>
        <translation>MQTT 3.1.1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="57"/>
        <source>MQTT 5.0</source>
        <translation>MQTT 5.0</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="59"/>
        <source>TLS 1.2</source>
        <translation>TLS 1.2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="60"/>
        <source>TLS 1.3</source>
        <translation>TLS 1.3</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="61"/>
        <source>TLS 1.3 or Later</source>
        <translation>TLS 1.3 या बाद का</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="62"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 या बाद का</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="63"/>
        <source>Any Protocol</source>
        <translation>कोई भी प्रोटोकॉल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="64"/>
        <source>Secure Protocols Only</source>
        <translation>केवल सुरक्षित प्रोटोकॉल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="66"/>
        <source>None</source>
        <translation>कोई नहीं</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="67"/>
        <source>Query Peer</source>
        <translation>पीयर से पूछें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="68"/>
        <source>Verify Peer</source>
        <translation>पीयर सत्यापित करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="69"/>
        <source>Auto Verify Peer</source>
        <translation>स्वतः पीयर सत्यापित करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="167"/>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation>MQTT सुविधा के लिए व्यावसायिक लाइसेंस आवश्यक है</translation>
    </message>
    <message>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio commercial license (Hobbyist tier or above).</source>
        <translation type="vanished">MQTT ब्रोकर की सदस्यता केवल वैध Serial Studio व्यावसायिक लाइसेंस (Hobbyist टियर या उससे ऊपर) के साथ उपलब्ध है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="168"/>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio license or an active trial.</source>
        <translation>MQTT ब्रोकर की सदस्यता केवल वैध Serial Studio लाइसेंस या सक्रिय ट्रायल के साथ उपलब्ध है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="393"/>
        <source>Use System Database</source>
        <translation>सिस्टम डेटाबेस उपयोग करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="394"/>
        <source>Load From Folder…</source>
        <translation>फ़ोल्डर से लोड करें…</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="427"/>
        <source>Select PEM Certificates Directory</source>
        <translation>PEM प्रमाणपत्र निर्देशिका चुनें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="682"/>
        <source>Hostname</source>
        <translation>होस्टनेम</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="689"/>
        <source>Port</source>
        <translation>पोर्ट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="698"/>
        <source>Topic Filter</source>
        <translation>टॉपिक फ़िल्टर</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="705"/>
        <source>Client ID</source>
        <translation>क्लाइंट ID</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="712"/>
        <source>Username</source>
        <translation>उपयोगकर्ता नाम</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="719"/>
        <source>Password</source>
        <translation>पासवर्ड</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="726"/>
        <source>MQTT Version</source>
        <translation>MQTT संस्करण</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="734"/>
        <source>Clean Session</source>
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="741"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="750"/>
        <source>Auto Keep Alive</source>
        <translation>Auto Keep Alive</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="767"/>
        <source>SSL/TLS Enabled</source>
        <translation>SSL/TLS सक्षम</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="777"/>
        <source>SSL Protocol</source>
        <translation>SSL प्रोटोकॉल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="785"/>
        <source>Peer Verify Mode</source>
        <translation>पीयर सत्यापन मोड</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="793"/>
        <source>Peer Verify Depth</source>
        <translation>पीयर सत्यापन गहराई</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="894"/>
        <source>MQTT Subscription Error</source>
        <translation>MQTT सदस्यता त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="895"/>
        <source>Failed to subscribe to topic "%1".</source>
        <translation>Topic "%1" की सदस्यता लेने में विफल।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="922"/>
        <source>Invalid MQTT Protocol Version</source>
        <translation>अमान्य MQTT प्रोटोकॉल संस्करण</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="923"/>
        <source>The broker rejected the configured MQTT protocol version.</source>
        <translation>ब्रोकर ने कॉन्फ़िगर किए गए MQTT प्रोटोकॉल संस्करण को अस्वीकार कर दिया।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="926"/>
        <source>Client ID Rejected</source>
        <translation>Client ID अस्वीकृत</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="927"/>
        <source>The broker rejected the client ID. Try a different identifier.</source>
        <translation>ब्रोकर ने Client ID को अस्वीकार कर दिया। एक अलग पहचानकर्ता आज़माएं।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="930"/>
        <source>MQTT Server Unavailable</source>
        <translation>MQTT सर्वर अनुपलब्ध</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="931"/>
        <source>The broker is currently unavailable. Retry later.</source>
        <translation>ब्रोकर वर्तमान में अनुपलब्ध है। बाद में पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="934"/>
        <source>Authentication Error</source>
        <translation>प्रमाणीकरण त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="935"/>
        <source>The credentials provided were rejected by the broker.</source>
        <translation>प्रदान की गई क्रेडेंशियल्स को ब्रोकर द्वारा अस्वीकार कर दिया गया।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="938"/>
        <source>Authorization Error</source>
        <translation>प्राधिकरण त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="939"/>
        <source>Account lacks permission for this operation.</source>
        <translation>खाते में इस ऑपरेशन के लिए अनुमति नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="942"/>
        <source>Network or Transport Error</source>
        <translation>नेटवर्क या ट्रांसपोर्ट त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="943"/>
        <source>Network/transport layer issue while connecting to the broker.</source>
        <translation>ब्रोकर से कनेक्ट करते समय नेटवर्क/ट्रांसपोर्ट लेयर समस्या।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="946"/>
        <source>MQTT Protocol Violation</source>
        <translation>MQTT प्रोटोकॉल उल्लंघन</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="947"/>
        <source>The broker reported a protocol violation and closed the connection.</source>
        <translation>ब्रोकर ने प्रोटोकॉल उल्लंघन की रिपोर्ट की और कनेक्शन बंद कर दिया।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="950"/>
        <source>MQTT 5 Error</source>
        <translation>MQTT 5 त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="951"/>
        <source>An MQTT 5 protocol-level error occurred.</source>
        <translation>एक MQTT 5 प्रोटोकॉल-स्तरीय त्रुटि हुई।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="954"/>
        <source>MQTT Error</source>
        <translation>MQTT त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="955"/>
        <source>An unexpected MQTT error occurred.</source>
        <translation>एक अप्रत्याशित MQTT त्रुटि हुई।</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Modbus</name>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="371"/>
        <source>Invalid Serial Port</source>
        <translation>अमान्य सीरियल पोर्ट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="416"/>
        <source>Modbus Initialization Failed</source>
        <translation>Modbus आरंभीकरण विफल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="442"/>
        <source>Modbus Connection Failed</source>
        <translation>Modbus कनेक्शन विफल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="372"/>
        <source>The selected serial port "%1" is no longer available. Refresh the port list and try again.</source>
        <translation>चयनित सीरियल पोर्ट "%1" अब उपलब्ध नहीं है। पोर्ट सूची रिफ्रेश करें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="417"/>
        <source>Unable to create Modbus device. Check your system configuration and try again.</source>
        <translation>Modbus डिवाइस बनाने में असमर्थ। अपना सिस्टम कॉन्फ़िगरेशन जांचें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="444"/>
        <source>Unable to connect to "%1". Check your connection settings.</source>
        <translation>"%1" से कनेक्ट नहीं हो सका। अपनी कनेक्शन सेटिंग्स जाँचें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="445"/>
        <source>"%1": %2</source>
        <translation>"%1": %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="609"/>
        <source>None</source>
        <translation>कोई नहीं</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="610"/>
        <source>Even</source>
        <translation>सम</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="611"/>
        <source>Odd</source>
        <translation>विषम</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="612"/>
        <source>Space</source>
        <translation>स्पेस</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="613"/>
        <source>Mark</source>
        <translation>मार्क</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="665"/>
        <source>Holding Registers (0x03)</source>
        <translation>होल्डिंग रजिस्टर (0x03)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="666"/>
        <source>Input Registers (0x04)</source>
        <translation>इनपुट रजिस्टर (0x04)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="667"/>
        <source>Coils (0x01)</source>
        <translation>कॉइल (0x01)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="668"/>
        <source>Discrete Inputs (0x02)</source>
        <translation>Discrete Inputs (0x02)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="850"/>
        <source>No register groups configured</source>
        <translation>कोई रजिस्टर समूह कॉन्फ़िगर नहीं है</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="851"/>
        <source>Add at least one register group before generating a project.</source>
        <translation>प्रोजेक्ट जेनरेट करने से पहले कम से कम एक रजिस्टर समूह जोड़ें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="853"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="865"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="890"/>
        <source>Modbus Project Generator</source>
        <translation>Modbus प्रोजेक्ट जेनरेटर</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">अस्थायी प्रोजेक्ट फ़ाइल बनाने में विफल</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">अस्थायी डायरेक्टरी में लिखने की अनुमति जाँचें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="862"/>
        <source>Failed to load generated project</source>
        <translation>जेनरेट किया गया प्रोजेक्ट लोड नहीं हो सका</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="863"/>
        <source>The generated project JSON could not be loaded.</source>
        <translation>जेनरेट किया गया प्रोजेक्ट JSON लोड नहीं हो सका।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="885"/>
        <source>Successfully generated project with %1 groups and %2 datasets.</source>
        <translation>%1 समूहों और %2 डेटासेट के साथ प्रोजेक्ट सफलतापूर्वक जेनरेट किया गया।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="888"/>
        <source>The project editor is now open for customization.</source>
        <translation>प्रोजेक्ट एडिटर अब अनुकूलन के लिए खुला है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="903"/>
        <source>Modbus Project</source>
        <translation>Modbus प्रोजेक्ट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="908"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="928"/>
        <source>Holding Registers</source>
        <translation>होल्डिंग रजिस्टर</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="929"/>
        <source>Input Registers</source>
        <translation>इनपुट रजिस्टर</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="930"/>
        <source>Coils</source>
        <translation>कॉइल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="931"/>
        <source>Discrete Inputs</source>
        <translation>डिस्क्रीट इनपुट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="945"/>
        <source>Unknown</source>
        <translation>अज्ञात</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="958"/>
        <source>Register %1</source>
        <translation>रजिस्टर %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Coil %1</source>
        <translation>कॉइल %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Discrete %1</source>
        <translation>डिस्क्रीट %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1372"/>
        <source>Error code: %1</source>
        <translation>त्रुटि कोड: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1375"/>
        <source>Modbus Communication Error</source>
        <translation>Modbus संचार त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1387"/>
        <source>Select Port</source>
        <translation>पोर्ट चुनें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1540"/>
        <source>Protocol</source>
        <translation>प्रोटोकॉल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1548"/>
        <source>Slave Address</source>
        <translation>स्लेव एड्रेस</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1557"/>
        <source>Poll Interval (ms)</source>
        <translation>पोल अंतराल (ms)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1597"/>
        <source>Host / IP</source>
        <translation>होस्ट / IP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1604"/>
        <source>Port</source>
        <translation>पोर्ट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1619"/>
        <source>Serial Port</source>
        <translation>सीरियल पोर्ट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1627"/>
        <source>Baud Rate</source>
        <translation>बॉड रेट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1635"/>
        <source>Parity</source>
        <translation>पैरिटी</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1643"/>
        <source>Data Bits</source>
        <translation>डेटा बिट्स</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1651"/>
        <source>Stop Bits</source>
        <translation>स्टॉप बिट्स</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Network</name>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="533"/>
        <source>Network socket error</source>
        <translation>नेटवर्क सॉकेट त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="549"/>
        <source>Socket Type</source>
        <translation>सॉकेट प्रकार</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="557"/>
        <source>Remote Address</source>
        <translation>रिमोट पता</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="565"/>
        <source>TCP Port</source>
        <translation>TCP पोर्ट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="574"/>
        <source>UDP Local Port</source>
        <translation>UDP लोकल पोर्ट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="583"/>
        <source>UDP Remote Port</source>
        <translation>UDP रिमोट पोर्ट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="592"/>
        <source>UDP Multicast</source>
        <translation>UDP मल्टीकास्ट</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Process</name>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="183"/>
        <location filename="../../src/IO/Drivers/Process.cpp" line="224"/>
        <source>Failed to start process</source>
        <translation>प्रोसेस प्रारंभ करने में विफल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="184"/>
        <source>Executable "%1" not found in PATH.</source>
        <translation>निष्पादन योग्य "%1" PATH में नहीं मिला।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="368"/>
        <source>Select Executable</source>
        <translation>एक्ज़ीक्यूटेबल चुनें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="392"/>
        <source>Select Working Directory</source>
        <translation>वर्किंग डायरेक्टरी चुनें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="417"/>
        <source>Select Named Pipe / FIFO</source>
        <translation>नेम्ड पाइप / FIFO चुनें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="514"/>
        <source>The process crashed.</source>
        <translation>प्रोसेस क्रैश हो गया।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="515"/>
        <source>Exit code: %1</source>
        <translation>एग्ज़िट कोड: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="518"/>
        <source>Process "%1" stopped</source>
        <translation>प्रोसेस "%1" रुक गया</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="536"/>
        <source>Unknown error</source>
        <translation>अज्ञात त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="537"/>
        <source>Process Error</source>
        <translation>प्रोसेस त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Pipe Error</source>
        <translation>पाइप त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Could not open named pipe: %1</source>
        <translation>नेम्ड पाइप खोल नहीं सका: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="763"/>
        <source>Mode</source>
        <translation>मोड</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Launch Process</source>
        <translation>प्रोसेस लॉन्च करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Named Pipe</source>
        <translation>नेम्ड पाइप</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="771"/>
        <source>Executable</source>
        <translation>एक्ज़ीक्यूटेबल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="778"/>
        <source>Arguments</source>
        <translation>आर्ग्युमेंट्स</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="785"/>
        <source>Working Directory</source>
        <translation>वर्किंग डायरेक्टरी</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="792"/>
        <source>Pipe Path</source>
        <translation>पाइप पाथ</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SeeedCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="206"/>
        <source>The bitrate %1 bps is not supported by the USB-CAN Analyzer.</source>
        <translation>बिटरेट %1 bps USB-CAN Analyzer द्वारा समर्थित नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="216"/>
        <source>Could not open serial port %1: %2</source>
        <translation>सीरियल पोर्ट %1 खोल नहीं सका: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="227"/>
        <source>Failed to initialize the USB-CAN Analyzer.</source>
        <translation>USB-CAN Analyzer प्रारंभ करने में विफल।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="264"/>
        <source>USB-CAN Analyzer is not open for writing.</source>
        <translation>USB-CAN Analyzer लिखने के लिए खुला नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="310"/>
        <source>CAN bus error reported by the USB-CAN Analyzer.</source>
        <translation>USB-CAN Analyzer द्वारा CAN Bus त्रुटि रिपोर्ट की गई।</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SlcanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="167"/>
        <source>The bitrate %1 bps is not a standard slcan rate.</source>
        <translation>बिटरेट %1 bps मानक slcan दर नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="177"/>
        <source>Could not open serial port %1: %2</source>
        <translation>सीरियल पोर्ट %1 खोल नहीं सका: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="190"/>
        <source>Failed to open the slcan channel.</source>
        <translation>slcan चैनल खोलने में विफल।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="229"/>
        <source>slcan adapter is not open for writing.</source>
        <translation>slcan एडाप्टर लिखने के लिए खुला नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="267"/>
        <source>CAN bus error reported by the slcan adapter.</source>
        <translation>slcan एडाप्टर द्वारा CAN Bus त्रुटि रिपोर्ट की गई।</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::UART</name>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="69"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="70"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="391"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="430"/>
        <source>None</source>
        <translation>कोई नहीं</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="349"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="733"/>
        <source>Select Port</source>
        <translation>पोर्ट चुनें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="392"/>
        <source>Even</source>
        <translation>सम</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="393"/>
        <source>Odd</source>
        <translation>विषम</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="394"/>
        <source>Space</source>
        <translation>स्पेस</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="395"/>
        <source>Mark</source>
        <translation>मार्क</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="431"/>
        <source>RTS/CTS</source>
        <translation>RTS/CTS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="432"/>
        <source>XON/XOFF</source>
        <translation>XON/XOFF</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="562"/>
        <source>"%1" is not a valid path</source>
        <translation>"%1" मान्य पथ नहीं है</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="563"/>
        <source>Please type another path to register a custom serial device</source>
        <translation>कस्टम सीरियल डिवाइस रजिस्टर करने के लिए कोई अन्य पथ टाइप करें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="823"/>
        <source>The specified device could not be found. Check the connection and try again.</source>
        <translation>निर्दिष्ट डिवाइस नहीं मिला। कनेक्शन जाँचें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="830"/>
        <source>An unknown error occurred. Check the device and try again.</source>
        <translation>अज्ञात त्रुटि हुई। डिवाइस जाँचें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="832"/>
        <source>The device is not open. Open the device before attempting this operation.</source>
        <translation>डिवाइस खुला नहीं है। यह ऑपरेशन करने से पहले डिवाइस खोलें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="261"/>
        <source>Failed to connect to serial port "%1"</source>
        <translation>सीरियल पोर्ट "%1" से कनेक्ट करने में विफल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="798"/>
        <source>Unknown</source>
        <translation>अज्ञात</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="799"/>
        <source>Critical error on serial port "%1"</source>
        <translation>सीरियल पोर्ट "%1" पर गंभीर त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="800"/>
        <source>Unknown error</source>
        <translation>अज्ञात त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="822"/>
        <source>No error occurred.</source>
        <translation>कोई त्रुटि नहीं हुई।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="824"/>
        <source>Permission denied. Ensure the application has the necessary access rights to the device.</source>
        <translation>अनुमति अस्वीकृत। सुनिश्चित करें कि एप्लिकेशन के पास डिवाइस तक पहुँचने के लिए आवश्यक अधिकार हैं।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="825"/>
        <source>Failed to open the device. It may already be in use or unavailable.</source>
        <translation>डिवाइस खोलने में विफल। यह पहले से उपयोग में हो सकता है या अनुपलब्ध हो सकता है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="826"/>
        <source>An error occurred while writing data to the device.</source>
        <translation>डिवाइस पर डेटा लिखते समय त्रुटि हुई।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="827"/>
        <source>An error occurred while reading data from the device.</source>
        <translation>डिवाइस से डेटा पढ़ते समय त्रुटि हुई।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="828"/>
        <source>A critical resource error occurred. The device may have been disconnected or is no longer accessible.</source>
        <translation>एक गंभीर संसाधन त्रुटि हुई। डिवाइस डिस्कनेक्ट हो गया होगा या अब पहुँच योग्य नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="829"/>
        <source>The requested operation is not supported on this device.</source>
        <translation>अनुरोधित ऑपरेशन इस डिवाइस पर समर्थित नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="831"/>
        <source>The operation timed out. The device may not be responding.</source>
        <translation>ऑपरेशन टाइम आउट हो गया। डिवाइस प्रतिक्रिया नहीं दे रहा है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="994"/>
        <source>Serial Port</source>
        <translation>सीरियल पोर्ट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1002"/>
        <source>Baud Rate</source>
        <translation>बॉड रेट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1010"/>
        <source>Parity</source>
        <translation>पैरिटी</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1018"/>
        <source>Data Bits</source>
        <translation>डेटा बिट्स</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1026"/>
        <source>Stop Bits</source>
        <translation>स्टॉप बिट्स</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1034"/>
        <source>Flow Control</source>
        <translation>फ्लो कंट्रोल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1042"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1049"/>
        <source>Auto-Reconnect</source>
        <translation>स्वतः पुनः कनेक्ट</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::USB</name>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="156"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="164"/>
        <source>USB Error</source>
        <translation>USB त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="157"/>
        <source>Failed to initialize the USB subsystem. Check that libusb is available on your system.</source>
        <translation>USB सबसिस्टम को आरंभ करने में विफल। जाँचें कि आपके सिस्टम पर libusb उपलब्ध है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="199"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="214"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="572"/>
        <source>USB Device Error</source>
        <translation>USB डिवाइस त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="177"/>
        <source>Could not open the USB device: %1.

On Linux, ensure you have read/write permission on the device node (add a udev rule or run as root). On macOS, the kernel driver may need to be detached first.</source>
        <translation>USB डिवाइस नहीं खोला जा सका: %1.

Linux पर, सुनिश्चित करें कि आपके पास डिवाइस नोड पर read/write अनुमति है (udev नियम जोड़ें या root के रूप में चलाएं)। macOS पर, कर्नेल ड्राइवर को पहले detach करना आवश्यक हो सकता है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="165"/>
        <source>No USB device selected. Select a device and try again.</source>
        <translation>कोई USB डिवाइस नहीं चुना गया। एक डिवाइस चुनें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="171"/>
        <source>Unknown Device</source>
        <translation>अज्ञात डिवाइस</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="176"/>
        <source>Failed to open "%1"</source>
        <translation>"%1" खोलने में विफल</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="215"/>
        <source>Could not claim interface %1 on the USB device.

Another driver or application may already have it open. On Linux, try unloading the kernel driver (e.g. cdc_acm) or adding a udev rule.</source>
        <translation>USB डिवाइस पर इंटरफ़ेस %1 claim नहीं किया जा सका।

कोई अन्य ड्राइवर या एप्लिकेशन इसे पहले से खोल सकता है। Linux पर, कर्नेल ड्राइवर (जैसे cdc_acm) unload करने या udev नियम जोड़ने का प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="360"/>
        <source>Select Device</source>
        <translation>डिवाइस चुनें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="379"/>
        <source>Select IN Endpoint</source>
        <translation>IN एंडपॉइंट चुनें</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="390"/>
        <source>None (Read-only)</source>
        <translation>कोई नहीं (केवल-पढ़ने योग्य)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="464"/>
        <source>Enable Advanced USB Control Transfers?</source>
        <translation>उन्नत USB नियंत्रण स्थानांतरण सक्षम करें?</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="465"/>
        <source>This enables control transfers in addition to bulk transfers. Sending incorrect control requests can crash or damage connected hardware. Only enable this if you know what you are doing.</source>
        <translation>यह बल्क स्थानांतरण के अतिरिक्त नियंत्रण स्थानांतरण सक्षम करता है। गलत नियंत्रण अनुरोध भेजने से कनेक्टेड हार्डवेयर क्रैश या क्षतिग्रस्त हो सकता है। केवल तभी सक्षम करें जब आप जानते हों कि आप क्या कर रहे हैं।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="469"/>
        <source>Advanced USB Mode</source>
        <translation>उन्नत USB मोड</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="573"/>
        <source>The USB device was disconnected or encountered a fatal read error.</source>
        <translation>USB डिवाइस डिस्कनेक्ट हो गया या गंभीर रीड त्रुटि आई।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="710"/>
        <source>No isochronous IN endpoint was found on this device, but bulk endpoints are available.

Switch the Transfer Mode to "Bulk Stream" and try again.</source>
        <translation>इस डिवाइस पर कोई isochronous IN एंडपॉइंट नहीं मिला, लेकिन बल्क एंडपॉइंट उपलब्ध हैं।

ट्रांसफर मोड को "बल्क स्ट्रीम" पर स्विच करें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="715"/>
        <source>No bulk IN endpoint was found on this device, but isochronous endpoints are available.

Switch the Transfer Mode to "Isochronous" and try again.</source>
        <translation>इस डिवाइस पर कोई बल्क IN एंडपॉइंट नहीं मिला, लेकिन isochronous एंडपॉइंट उपलब्ध हैं।

ट्रांसफर मोड को "Isochronous" पर स्विच करें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="719"/>
        <source>No usable IN endpoint was found on this device.

The device may not expose data endpoints in its active configuration, or it may require a specific driver.</source>
        <translation>इस डिवाइस पर कोई उपयोग योग्य IN एंडपॉइंट नहीं मिला।

डिवाइस अपने सक्रिय कॉन्फ़िगरेशन में डेटा एंडपॉइंट उजागर नहीं कर सकता, या इसे किसी विशिष्ट ड्राइवर की आवश्यकता हो सकती है।</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1211"/>
        <source>USB Device</source>
        <translation>USB डिवाइस</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1219"/>
        <source>Transfer Mode</source>
        <translation>ट्रांसफर मोड</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Bulk Stream</source>
        <translation>बल्क स्ट्रीम</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Advanced Control</source>
        <translation>उन्नत नियंत्रण</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Isochronous</source>
        <translation>आइसोक्रोनस</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1227"/>
        <source>IN Endpoint</source>
        <translation>IN एंडपॉइंट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1235"/>
        <source>OUT Endpoint</source>
        <translation>OUT एंडपॉइंट</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1243"/>
        <source>ISO Packet Size</source>
        <translation>ISO पैकेट आकार</translation>
    </message>
</context>
<context>
    <name>IO::FileTransmission</name>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="211"/>
        <source>No file selected…</source>
        <translation>कोई फ़ाइल चयनित नहीं…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="246"/>
        <source>Plain Text</source>
        <translation>सादा टेक्स्ट</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="247"/>
        <source>Raw Binary</source>
        <translation>Raw बाइनरी</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="248"/>
        <source>XMODEM</source>
        <translation>XMODEM</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="249"/>
        <source>XMODEM-1K</source>
        <translation>XMODEM-1K</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="250"/>
        <source>YMODEM</source>
        <translation>YMODEM</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="251"/>
        <source>ZMODEM</source>
        <translation>ZMODEM</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="265"/>
        <source>Select file to transmit</source>
        <translation>ट्रांसमिट करने के लिए फ़ाइल चुनें</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="291"/>
        <source>File selected: %1 (%2 bytes)</source>
        <translation>फ़ाइल चुनी गई: %1 (%2 बाइट्स)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="294"/>
        <source>Error opening file: %1</source>
        <translation>फ़ाइल खोलने में त्रुटि: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="380"/>
        <source>Starting %1 transfer…</source>
        <translation>%1 ट्रांसफर शुरू हो रहा है…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="608"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="628"/>
        <source>Transmission complete</source>
        <translation>ट्रांसमिशन पूर्ण</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="610"/>
        <source>Plain text transmission complete</source>
        <translation>सादा टेक्स्ट ट्रांसमिशन पूर्ण</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="630"/>
        <source>Raw binary transmission complete (%1 bytes)</source>
        <translation>रॉ बाइनरी ट्रांसमिशन पूर्ण (%1 बाइट्स)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="654"/>
        <source>Transfer complete</source>
        <translation>ट्रांसफर पूर्ण</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="655"/>
        <source>Transfer completed successfully (%1 bytes)</source>
        <translation>ट्रांसफर सफलतापूर्वक पूर्ण हुआ (%1 बाइट्स)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="657"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="658"/>
        <source>Transfer failed: %1</source>
        <translation>ट्रांसफर विफल: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="742"/>
        <source>%1 B/s</source>
        <translation>%1 B/s</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="744"/>
        <source>%1 KB/s</source>
        <translation>%1 KB/s</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="746"/>
        <source>%1 MB/s</source>
        <translation>%1 MB/s</translation>
    </message>
</context>
<context>
    <name>IO::FrameReader</name>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="350"/>
        <source>Frames dropped</source>
        <translation>फ्रेम छोड़े गए</translation>
    </message>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="352"/>
        <source>Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) have been dropped. Reduce the data rate or disable a heavy consumer.</source>
        <translation>आने वाला डेटा Serial Studio द्वारा प्रोसेस किए जाने की तुलना में तेज़ी से आ रहा है; %1 फ्रेम छोड़े गए हैं। डेटा दर कम करें या किसी भारी उपभोक्ता को अक्षम करें।</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::XMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="83"/>
        <source>Cannot open file: %1</source>
        <translation>फ़ाइल नहीं खोली जा सकती: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="112"/>
        <source>Transfer cancelled</source>
        <translation>ट्रांसफर रद्द किया गया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="113"/>
        <source>Transfer cancelled by user</source>
        <translation>उपयोगकर्ता द्वारा ट्रांसफर रद्द किया गया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="93"/>
        <source>Waiting for receiver…</source>
        <translation>रिसीवर की प्रतीक्षा में…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="207"/>
        <source>Receiver ready (CRC mode), sending data…</source>
        <translation>रिसीवर तैयार (CRC मोड), डेटा भेजा जा रहा है…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="142"/>
        <source>Too many retries, transfer aborted</source>
        <translation>बहुत अधिक पुनः प्रयास, ट्रांसफर रद्द किया गया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="143"/>
        <source>Maximum retries exceeded</source>
        <translation>अधिकतम पुनः प्रयास पार हो गए</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="147"/>
        <source>NAK received, retrying block %1 (%2/%3)</source>
        <translation>NAK प्राप्त हुआ, ब्लॉक %1 पुनः प्रयास किया जा रहा है (%2/%3)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="155"/>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="384"/>
        <source>Failed to seek in file</source>
        <translation>फ़ाइल में सीक करने में विफल</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="165"/>
        <source>Transfer cancelled by receiver</source>
        <translation>रिसीवर द्वारा ट्रांसफर रद्द किया गया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="166"/>
        <source>Receiver cancelled the transfer</source>
        <translation>रिसीवर ने ट्रांसफर रद्द कर दिया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="180"/>
        <source>Transfer complete</source>
        <translation>ट्रांसफर पूर्ण</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="302"/>
        <source>File read returned more data than requested</source>
        <translation>फ़ाइल रीड ने अनुरोधित से अधिक डेटा लौटाया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="316"/>
        <source>Sending block %1 (%2 bytes)</source>
        <translation>ब्लॉक %1 भेजा जा रहा है (%2 बाइट्स)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="329"/>
        <source>Sending EOT…</source>
        <translation>EOT भेजा जा रहा है…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="375"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>टाइमआउट, पुनः प्रयास (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="370"/>
        <source>Transfer timed out</source>
        <translation>ट्रांसफर टाइमआउट हो गया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="371"/>
        <source>Timeout: no response from receiver</source>
        <translation>टाइमआउट: रिसीवर से कोई प्रतिक्रिया नहीं</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::YMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="62"/>
        <source>Cannot open file: %1</source>
        <translation>फ़ाइल नहीं खोली जा सकती: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="96"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="173"/>
        <source>Transfer cancelled by receiver</source>
        <translation>रिसीवर द्वारा ट्रांसफर रद्द किया गया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="97"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="174"/>
        <source>Receiver cancelled the transfer</source>
        <translation>रिसीवर ने ट्रांसफर रद्द कर दिया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="74"/>
        <source>Waiting for receiver…</source>
        <translation>रिसीवर की प्रतीक्षा में…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="133"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="316"/>
        <source>Sending first EOT…</source>
        <translation>पहला EOT भेजा जा रहा है…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="151"/>
        <source>Too many retries, transfer aborted</source>
        <translation>बहुत अधिक पुनः प्रयास, ट्रांसफर निरस्त किया गया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="152"/>
        <source>Maximum retries exceeded</source>
        <translation>अधिकतम पुनः प्रयास सीमा पार हो गई</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="156"/>
        <source>NAK received, retrying block %1</source>
        <translation>NAK प्राप्त हुआ, ब्लॉक %1 पुनः प्रयास किया जा रहा है</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="161"/>
        <source>Failed to seek in file</source>
        <translation>फ़ाइल में सीक करने में विफल</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="220"/>
        <source>Sending second EOT…</source>
        <translation>दूसरा EOT भेजा जा रहा है…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="299"/>
        <source>Sending end-of-batch marker…</source>
        <translation>बैच-समाप्ति मार्कर भेजा जा रहा है…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="242"/>
        <source>Transfer complete</source>
        <translation>स्थानांतरण पूर्ण</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="284"/>
        <source>Sending file header: %1 (%2 bytes)</source>
        <translation>फ़ाइल हेडर भेजा जा रहा है: %1 (%2 बाइट्स)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="330"/>
        <source>Sending block %1 (%2/%3 bytes)</source>
        <translation>ब्लॉक %1 भेजा जा रहा है (%2/%3 बाइट्स)</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::ZMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="86"/>
        <source>Cannot open file: %1</source>
        <translation>फ़ाइल नहीं खोली जा सकती: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="103"/>
        <source>File is too large for ZMODEM (%1 bytes, limit 4 GiB).</source>
        <translation>फ़ाइल ZMODEM के लिए बहुत बड़ी है (%1 बाइट्स, सीमा 4 GiB)।</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="128"/>
        <source>Transfer cancelled</source>
        <translation>स्थानांतरण रद्द किया गया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="129"/>
        <source>Transfer cancelled by user</source>
        <translation>उपयोगकर्ता द्वारा स्थानांतरण रद्द किया गया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="269"/>
        <source>Hex header CRC mismatch, dropping frame</source>
        <translation>हेक्स हेडर CRC बेमेल, फ्रेम छोड़ा जा रहा है</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="444"/>
        <source>Sending file info: %1 (%2 bytes)</source>
        <translation>फ़ाइल जानकारी भेजी जा रही है: %1 (%2 बाइट्स)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="459"/>
        <source>Failed to seek to offset %1</source>
        <translation>ऑफसेट %1 पर सीक करने में विफल</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="488"/>
        <source>File read returned more data than requested</source>
        <translation>फ़ाइल रीड ने अनुरोधित से अधिक डेटा लौटाया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="571"/>
        <source>Receiver requests data from offset %1</source>
        <translation>रिसीवर ऑफसेट %1 से डेटा का अनुरोध कर रहा है</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="579"/>
        <source>Receiver skipped the file</source>
        <translation>रिसीवर ने फ़ाइल छोड़ दी</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="591"/>
        <source>Too many errors, transfer aborted</source>
        <translation>बहुत अधिक त्रुटियाँ, स्थानांतरण निरस्त किया गया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="592"/>
        <source>Maximum retries exceeded</source>
        <translation>अधिकतम पुनः प्रयास सीमा पार हो गई</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="420"/>
        <source>Sending ZRQINIT, waiting for receiver…</source>
        <translation>ZRQINIT भेजा जा रहा है, रिसीवर की प्रतीक्षा में…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="514"/>
        <source>File data sent, waiting for confirmation…</source>
        <translation>फ़ाइल डेटा भेजा गया, पुष्टि की प्रतीक्षा में…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="525"/>
        <source>Sending ZFIN…</source>
        <translation>ZFIN भेजा जा रहा है…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="561"/>
        <source>Receiver ready, sending file info…</source>
        <translation>रिसीवर तैयार, फ़ाइल जानकारी भेजी जा रही है…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="596"/>
        <source>NAK received, retrying (%1/%2)…</source>
        <translation>NAK प्राप्त हुआ, पुनः प्रयास (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="617"/>
        <source>Transfer complete</source>
        <translation>ट्रांसफर पूर्ण</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="627"/>
        <source>Transfer cancelled by receiver</source>
        <translation>रिसीवर द्वारा ट्रांसफर रद्द किया गया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="628"/>
        <source>Receiver cancelled the transfer</source>
        <translation>रिसीवर ने ट्रांसफर रद्द किया</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="636"/>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="637"/>
        <source>Receiver reported a file error</source>
        <translation>रिसीवर ने फ़ाइल त्रुटि रिपोर्ट की</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="835"/>
        <source>Transfer timed out</source>
        <translation>ट्रांसफर टाइमआउट</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="836"/>
        <source>Timeout: no response from receiver</source>
        <translation>टाइमआउट: रिसीवर से कोई प्रतिक्रिया नहीं</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="840"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>टाइमआउट, पुनः प्रयास (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="42"/>
        <source>Select Icon</source>
        <translation>आइकन चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="119"/>
        <source>Search Online…</source>
        <translation>ऑनलाइन खोजें…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="132"/>
        <source>OK</source>
        <translation>ठीक है</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="144"/>
        <source>Cancel</source>
        <translation>रद्द करें</translation>
    </message>
</context>
<context>
    <name>ImageView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="68"/>
        <source>Normal</source>
        <translation>सामान्य</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="69"/>
        <source>Grayscale</source>
        <translation>ग्रेस्केल</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="70"/>
        <source>High Contrast</source>
        <translation>उच्च कंट्रास्ट</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="71"/>
        <source>Vivid</source>
        <translation>चमकीला</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="72"/>
        <source>Night Vision</source>
        <translation>नाइट विज़न</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="73"/>
        <source>Infrared</source>
        <translation>इन्फ्रारेड</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="74"/>
        <source>Deep Blue</source>
        <translation>डीप ब्लू</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="75"/>
        <source>Amber</source>
        <translation>एम्बर</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="172"/>
        <source>Export Images</source>
        <translation>इमेज एक्सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="182"/>
        <source>Open Export Folder</source>
        <translation>एक्सपोर्ट फ़ोल्डर खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="198"/>
        <source>Zoom In</source>
        <translation>ज़ूम इन करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="211"/>
        <source>Zoom Out</source>
        <translation>ज़ूम आउट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="231"/>
        <source>Show Crosshair</source>
        <translation>क्रॉसहेयर दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="556"/>
        <source>Waiting for Image…</source>
        <translation>इमेज का इंतज़ार है…</translation>
    </message>
</context>
<context>
    <name>KeyManagerDialog</name>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="23"/>
        <source>API Keys</source>
        <translation>API कुंजियाँ</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="47"/>
        <source>Anthropic Claude. The default is Claude Haiku 4.5 ($1 input / $5 output per million tokens). Sonnet 4.6 and Opus 4.7 are also available. Supports streaming, tool use, extended thinking, and prompt caching.</source>
        <translation>Anthropic Claude। डिफ़ॉल्ट Claude Haiku 4.5 है ($1 इनपुट / $5 आउटपुट प्रति मिलियन टोकन)। Sonnet 4.6 और OPUS 4.7 भी उपलब्ध हैं। स्ट्रीमिंग, टूल उपयोग, विस्तारित थिंकिंग और प्रॉम्प्ट कैशिंग समर्थित है।</translation>
    </message>
    <message>
        <source>OpenAI Chat Completions. The default is GPT-4o mini ($0.15 input / $0.60 output per million tokens). GPT-4o, GPT-4 Turbo, and o1-mini are also available.</source>
        <translation type="vanished">OpenAI Chat Completions। डिफ़ॉल्ट GPT-4o mini है ($0.15 इनपुट / $0.60 आउटपुट प्रति मिलियन टोकन)। GPT-4o, GPT-4 Turbo और o1-mini भी उपलब्ध हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="57"/>
        <source>Google Gemini. The default is Gemini 2.0 Flash, which has a generous free tier (subject to rate limits). Gemini 1.5 Pro and Gemini 1.5 Flash are also available.</source>
        <translation>Google Gemini। डिफ़ॉल्ट Gemini 2.0 Flash है, जिसमें उदार फ्री टियर है (रेट लिमिट के अधीन)। Gemini 1.5 Pro और Gemini 1.5 Flash भी उपलब्ध हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="100"/>
        <source>Bring your own API keys. They are encrypted at rest with a per-machine key and never leave your computer except to communicate with the provider you select.</source>
        <translation>अपनी API कुंजियाँ लाएं। वे प्रति-मशीन कुंजी के साथ एन्क्रिप्टेड रहती हैं और आपके चुने गए प्रदाता से संचार के अलावा कभी आपके कंप्यूटर से बाहर नहीं जाती हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="166"/>
        <source>Key set</source>
        <translation>कुंजी सेट की गई</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="167"/>
        <source>No key</source>
        <translation>कोई कुंजी नहीं</translation>
    </message>
    <message>
        <source>A key is on file — paste a new one to replace it</source>
        <translation type="vanished">एक कुंजी फ़ाइल में है — इसे बदलने के लिए नई कुंजी पेस्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="52"/>
        <source>OpenAI Chat Completions. The default is GPT-5 mini for fast, cost-conscious agentic work. GPT-5.2 is the stronger general-purpose option, and GPT-5.2 Chat tracks the model currently used in ChatGPT.</source>
        <translation>OpenAI Chat Completions। डिफ़ॉल्ट GPT-5 mini है जो तेज़, लागत-कुशल एजेंटिक कार्य के लिए है। GPT-5.2 मजबूत सामान्य-उद्देश्य विकल्प है, और GPT-5.2 Chat वर्तमान में ChatGPT में उपयोग किए जाने वाले मॉडल को ट्रैक करता है।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="61"/>
        <source>DeepSeek. OpenAI-compatible API. The default is deepseek-chat (V3). deepseek-reasoner (R1) is also available. Often the cheapest cloud option for tool use.</source>
        <translation>DeepSeek। OpenAI-संगत API। डिफ़ॉल्ट deepseek-chat (V3) है। deepseek-reasoner (R1) भी उपलब्ध है। टूल उपयोग के लिए अक्सर सबसे सस्ता क्लाउड विकल्प।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="65"/>
        <source>OpenRouter. One key, ~200 models from Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier models (suffixed :free) are rate-limited but require no additional billing. Pay-as-you-go for the rest.</source>
        <translation>OpenRouter। एक कुंजी, Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen और अन्य से ~200 मॉडल। फ्री-टियर मॉडल (:free प्रत्यय के साथ) रेट-लिमिटेड हैं लेकिन अतिरिक्त बिलिंग की आवश्यकता नहीं है। शेष के लिए pay-as-you-go।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="70"/>
        <source>Groq. Hardware-accelerated inference (LPUs) for very fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen models. Generous free tier with daily token limits.</source>
        <translation>Groq। बहुत तेज़ Llama, Mixtral, Gemma, DeepSeek-R1 distill और Qwen मॉडल के लिए हार्डवेयर-एक्सेलरेटेड इन्फरेंस (LPU)। दैनिक टोकन सीमा के साथ उदार फ्री टियर।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="74"/>
        <source>Mistral. The default is Mistral Large. Codestral targets code completion, Pixtral handles vision, and the Ministral models are tuned for edge / low-latency use.</source>
        <translation>Mistral। डिफ़ॉल्ट Mistral Large है। Codestral कोड कम्प्लीशन के लिए, Pixtral विज़न को हैंडल करता है, और Ministral मॉडल एज / लो-लेटेंसी उपयोग के लिए ट्यून किए गए हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="78"/>
        <source>Local model server. Works with any OpenAI-compatible endpoint -- Ollama, llama.cpp's llama-server, LM Studio, or vLLM. Nothing leaves your machine. The model list is queried live from the server.</source>
        <translation>लोकल मॉडल सर्वर। किसी भी OpenAI-संगत एंडपॉइंट के साथ काम करता है -- Ollama, llama.cpp का llama-server, LM Studio, या vLLM। कुछ भी आपकी मशीन से बाहर नहीं जाता। मॉडल सूची सर्वर से लाइव क्वेरी की जाती है।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="204"/>
        <source>A key is on file -- paste a new one to replace it</source>
        <translation>एक कुंजी फ़ाइल में है -- इसे बदलने के लिए नई कुंजी पेस्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="205"/>
        <source>Paste your API key here</source>
        <translation>अपनी API कुंजी यहाँ पेस्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="212"/>
        <source>Hide key</source>
        <translation>कुंजी छिपाएँ</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="213"/>
        <source>Show key while typing</source>
        <translation>टाइप करते समय कुंजी दिखाएँ</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="224"/>
        <source>Get key</source>
        <translation>कुंजी प्राप्त करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="225"/>
        <source>Open the provider's console to create a new key</source>
        <translation>नई कुंजी बनाने के लिए प्रदाता का कंसोल खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="236"/>
        <source>Save</source>
        <translation>सहेजें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="254"/>
        <source>Remove the stored key for %1</source>
        <translation>%1 के लिए संग्रहीत कुंजी हटाएँ</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="278"/>
        <source>http://localhost:11434/v1</source>
        <translation>http://localhost:11434/v1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="282"/>
        <source>Install Ollama</source>
        <translation>Ollama इंस्टॉल करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="283"/>
        <source>Open the Ollama download page</source>
        <translation>Ollama डाउनलोड पेज खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="294"/>
        <source>Apply</source>
        <translation>लागू करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="309"/>
        <source>Re-query the model list</source>
        <translation>मॉडल सूची पुनः क्वेरी करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="357"/>
        <source>No API keys configured yet. Add a key to get started.</source>
        <translation>अभी तक कोई API कुंजी कॉन्फ़िगर नहीं की गई है। शुरू करने के लिए एक कुंजी जोड़ें।</translation>
    </message>
    <message>
        <source>No API keys configured yet. Add at least one above to get started.</source>
        <translation type="vanished">अभी तक कोई API कुंजी कॉन्फ़िगर नहीं की गई है। शुरू करने के लिए ऊपर कम से कम एक जोड़ें।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="360"/>
        <source>One provider is ready.</source>
        <translation>एक प्रदाता तैयार है।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="362"/>
        <source>%1 providers are ready.</source>
        <translation>%1 प्रदाता तैयार हैं।</translation>
    </message>
</context>
<context>
    <name>LicenseManagement</name>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="37"/>
        <source>Licensing</source>
        <translation>लाइसेंसिंग</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="84"/>
        <source>Please wait…</source>
        <translation>कृपया प्रतीक्षा करें…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="124"/>
        <source>Activate Serial Studio Pro</source>
        <translation>Serial Studio Pro सक्रिय करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="131"/>
        <source>Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.</source>
        <translation>MQTT, 3D प्लॉटिंग और अन्य Pro फीचर्स अनलॉक करने के लिए अपनी लाइसेंस की नीचे पेस्ट करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="138"/>
        <source>Your license includes 5 device activations.
Plans include Monthly, Yearly, and Lifetime options.</source>
        <translation>आपके लाइसेंस में 5 डिवाइस एक्टिवेशन शामिल हैं।
प्लान में Monthly, Yearly और Lifetime विकल्प उपलब्ध हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="151"/>
        <source>Paste your license key here…</source>
        <translation>अपनी लाइसेंस की यहाँ पेस्ट करें…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="170"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="332"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="381"/>
        <source>Copy</source>
        <translation>कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="176"/>
        <source>Paste</source>
        <translation>पेस्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="182"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="338"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="387"/>
        <source>Select All</source>
        <translation>सभी चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="234"/>
        <source>Product</source>
        <translation>प्रोडक्ट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="241"/>
        <source>Serial Studio %1</source>
        <translation>Serial Studio %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="252"/>
        <source>Licensee</source>
        <translation>लाइसेंसधारक</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="271"/>
        <source>Licensee E-Mail</source>
        <translation>लाइसेंसधारक ई-मेल</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="288"/>
        <source>Device Usage</source>
        <translation>डिवाइस उपयोग</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="296"/>
        <source>%1 devices in use (Unlimited plan)</source>
        <translation>%1 डिवाइस उपयोग में (असीमित योजना)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="297"/>
        <source>%1 of %2 devices used</source>
        <translation>%2 में से %1 डिवाइस उपयोग किए गए</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="307"/>
        <source>Device ID</source>
        <translation>डिवाइस ID</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="354"/>
        <source>License Key</source>
        <translation>लाइसेंस कुंजी</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="409"/>
        <source>Customer Portal</source>
        <translation>ग्राहक पोर्टल</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="420"/>
        <source>Buy License</source>
        <translation>लाइसेंस खरीदें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="427"/>
        <source>Activate</source>
        <translation>सक्रिय करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="437"/>
        <source>Deactivate</source>
        <translation>निष्क्रिय करें</translation>
    </message>
</context>
<context>
    <name>Licensing::LemonSqueezy</name>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="517"/>
        <source>There was an issue validating your license.</source>
        <translation>आपके लाइसेंस को मान्य करने में समस्या आई।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="535"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="716"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="835"/>
        <source>The license key you provided does not belong to Serial Studio.</source>
        <translation>आपके द्वारा प्रदान की गई लाइसेंस कुंजी Serial Studio से संबंधित नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="536"/>
        <source>Please double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>कृपया दोबारा जाँचें कि आपने अपना लाइसेंस आधिकारिक Serial Studio स्टोर से खरीदा है।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="547"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="725"/>
        <source>This license key was activated on a different device.</source>
        <translation>यह लाइसेंस कुंजी किसी अन्य डिवाइस पर सक्रिय की गई थी।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="548"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="726"/>
        <source>Deactivate it there first or contact support for help.</source>
        <translation>पहले उसे वहाँ निष्क्रिय करें या सहायता के लिए समर्थन से संपर्क करें।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="559"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="736"/>
        <source>This license is not currently active.</source>
        <translation>यह लाइसेंस वर्तमान में सक्रिय नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="560"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="737"/>
        <source>It may have expired or been deactivated (status: %1).</source>
        <translation>यह समाप्त हो गया होगा या निष्क्रिय कर दिया गया होगा (स्थिति: %1)।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="570"/>
        <source>Something went wrong on the server.</source>
        <translation>सर्वर पर कुछ गड़बड़ हुई।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="571"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="747"/>
        <source>No activation ID was returned.</source>
        <translation>कोई सक्रियण ID नहीं मिला।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="581"/>
        <source>Could not validate your license at this time.</source>
        <translation>इस समय आपका लाइसेंस सत्यापित नहीं हो सका।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="582"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="756"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="845"/>
        <source>Try again later.</source>
        <translation>बाद में पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="717"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="836"/>
        <source>Double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>दोबारा जाँचें कि आपने अपना लाइसेंस आधिकारिक Serial Studio स्टोर से खरीदा है।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="746"/>
        <source>Something went wrong on the server…</source>
        <translation>सर्वर पर कुछ गड़बड़ हुई…</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="600"/>
        <source>%1 %2</source>
        <translation>%1 %2</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="602"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="653"/>
        <source>Your license has been successfully activated.</source>
        <translation>आपका लाइसेंस सफलतापूर्वक सक्रिय हो गया है।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="654"/>
        <source>Thank you for supporting Serial Studio!
You now have access to all premium features.</source>
        <translation>Serial Studio का समर्थन करने के लिए धन्यवाद!
अब आपके पास सभी प्रीमियम सुविधाओं तक पहुंच है।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="708"/>
        <source>There was an issue activating your license.</source>
        <translation>आपका लाइसेंस सक्रिय करने में समस्या हुई।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="755"/>
        <source>Could not activate your license at this time.</source>
        <translation>इस समय आपका लाइसेंस सक्रिय नहीं हो सका।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="827"/>
        <source>There was an issue deactivating your license.</source>
        <translation>आपका लाइसेंस निष्क्रिय करने में समस्या आई।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="844"/>
        <source>Could not deactivate your license at this time.</source>
        <translation>इस समय आपका लाइसेंस निष्क्रिय नहीं किया जा सका।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="854"/>
        <source>Your license has been deactivated.</source>
        <translation>आपका लाइसेंस निष्क्रिय कर दिया गया है।</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="855"/>
        <source>Access to Pro features has been removed.
Thank you again for supporting Serial Studio!</source>
        <translation>Pro फीचर्स की एक्सेस हटा दी गई है।
Serial Studio को सपोर्ट करने के लिए पुनः धन्यवाद!</translation>
    </message>
</context>
<context>
    <name>MDF4::Export</name>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="629"/>
        <source>MDF4 Export is a Pro feature.</source>
        <translation>MDF4 एक्सपोर्ट एक Pro फीचर है।</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="630"/>
        <source>This feature requires a license. Please purchase one to enable MDF4 export.</source>
        <translation>इस फीचर के लिए लाइसेंस आवश्यक है। MDF4 एक्सपोर्ट सक्षम करने के लिए कृपया एक खरीदें।</translation>
    </message>
</context>
<context>
    <name>MDF4::Player</name>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="399"/>
        <source>Select MDF4 file</source>
        <translation>MDF4 फ़ाइल चुनें</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="401"/>
        <source>MDF4 files (*.mf4 *.dat)</source>
        <translation>MDF4 फ़ाइलें (*.mf4 *.dat)</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="423"/>
        <source>Disconnect from device?</source>
        <translation>डिवाइस से डिस्कनेक्ट करें?</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="424"/>
        <source>You must disconnect from the current device before opening a MDF4 file.</source>
        <translation>MDF4 फ़ाइल खोलने से पहले आपको वर्तमान डिवाइस से डिस्कनेक्ट करना होगा।</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="440"/>
        <source>Cannot open MDF4 file</source>
        <translation>MDF4 फ़ाइल नहीं खोली जा सकती</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="441"/>
        <source>The file may be corrupted or in an unsupported format.</source>
        <translation>फ़ाइल दूषित हो सकती है या असमर्थित प्रारूप में हो सकती है।</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="448"/>
        <source>Invalid MDF4 file</source>
        <translation>अमान्य MDF4 फ़ाइल</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="449"/>
        <source>Failed to read file structure. The file may be corrupted.</source>
        <translation>फ़ाइल संरचना पढ़ने में विफल। फ़ाइल दूषित हो सकती है।</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="464"/>
        <source>No data in file</source>
        <translation>फ़ाइल में कोई डेटा नहीं</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="465"/>
        <source>The MDF4 file contains no measurement data.</source>
        <translation>MDF4 फ़ाइल में कोई माप डेटा नहीं है।</translation>
    </message>
</context>
<context>
    <name>MQTT</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="51"/>
        <source>Hostname</source>
        <translation>होस्टनेम</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="59"/>
        <source>e.g. broker.hivemq.com</source>
        <translation>उदा. broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="67"/>
        <source>Port</source>
        <translation>पोर्ट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="87"/>
        <source>Topic Filter</source>
        <translation>टॉपिक फ़िल्टर</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="95"/>
        <source>e.g. sensors/#</source>
        <translation>उदा. sensors/#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="103"/>
        <source>Client ID</source>
        <translation>क्लाइंट ID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="120"/>
        <source>Regenerate</source>
        <translation>पुनः जनरेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="130"/>
        <source>Username</source>
        <translation>उपयोगकर्ता नाम</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="145"/>
        <source>Password</source>
        <translation>पासवर्ड</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="161"/>
        <source>Version</source>
        <translation>संस्करण</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="187"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="206"/>
        <source>Clean Session</source>
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="232"/>
        <source>Use SSL/TLS</source>
        <translation>SSL/TLS उपयोग करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="258"/>
        <source>SSL Protocol</source>
        <translation>SSL प्रोटोकॉल</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="289"/>
        <source>Peer Verify</source>
        <translation>पीयर सत्यापन</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="320"/>
        <source>Verify Depth</source>
        <translation>सत्यापन गहराई</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="341"/>
        <source>CA Certificates</source>
        <translation>CA प्रमाणपत्र</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="349"/>
        <source>Load From Folder…</source>
        <translation>फ़ोल्डर से लोड करें…</translation>
    </message>
</context>
<context>
    <name>MQTT::Client</name>
    <message>
        <source>MQTT 3.1</source>
        <translation type="vanished">MQTT 3.1</translation>
    </message>
    <message>
        <source>MQTT 3.1.1</source>
        <translation type="vanished">MQTT 3.1.1</translation>
    </message>
    <message>
        <source>MQTT 5.0</source>
        <translation type="vanished">MQTT 5.0</translation>
    </message>
    <message>
        <source>TLS 1.2</source>
        <translation type="vanished">TLS 1.2</translation>
    </message>
    <message>
        <source>TLS 1.3</source>
        <translation type="vanished">TLS 1.3</translation>
    </message>
    <message>
        <source>TLS 1.3 or Later</source>
        <translation type="vanished">TLS 1.3 या बाद का</translation>
    </message>
    <message>
        <source>DTLS 1.2 or Later</source>
        <translation type="vanished">DTLS 1.2 या बाद का</translation>
    </message>
    <message>
        <source>Any Protocol</source>
        <translation type="vanished">कोई भी प्रोटोकॉल</translation>
    </message>
    <message>
        <source>Secure Protocols Only</source>
        <translation type="vanished">केवल सुरक्षित प्रोटोकॉल</translation>
    </message>
    <message>
        <source>None</source>
        <translation type="vanished">कोई नहीं</translation>
    </message>
    <message>
        <source>Query Peer</source>
        <translation type="vanished">पीयर से पूछें</translation>
    </message>
    <message>
        <source>Verify Peer</source>
        <translation type="vanished">पीयर सत्यापित करें</translation>
    </message>
    <message>
        <source>Auto Verify Peer</source>
        <translation type="vanished">स्वतः पीयर सत्यापित करें</translation>
    </message>
    <message>
        <source>Use System Database</source>
        <translation type="vanished">सिस्टम डेटाबेस उपयोग करें</translation>
    </message>
    <message>
        <source>MQTT Subscriber</source>
        <translation type="vanished">MQTT Subscriber</translation>
    </message>
    <message>
        <source>MQTT Publisher</source>
        <translation type="vanished">MQTT Publisher</translation>
    </message>
    <message>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation type="vanished">MQTT फीचर के लिए कमर्शियल लाइसेंस आवश्यक है</translation>
    </message>
    <message>
        <source>Load From Folder…</source>
        <translation type="vanished">फ़ोल्डर से लोड करें…</translation>
    </message>
    <message>
        <source>Connecting to MQTT brokers is only available with a valid Serial Studio commercial license (Hobbyist tier or above).

To unlock this feature, please activate your license or visit the store.</source>
        <translation type="vanished">MQTT ब्रोकर से कनेक्ट करना केवल वैध Serial Studio कमर्शियल लाइसेंस (Hobbyist टियर या उससे ऊपर) के साथ उपलब्ध है।

इस फीचर को अनलॉक करने के लिए, कृपया अपना लाइसेंस सक्रिय करें या स्टोर पर जाएं।</translation>
    </message>
    <message>
        <source>Missing MQTT Topic</source>
        <translation type="vanished">MQTT Topic अनुपलब्ध</translation>
    </message>
    <message>
        <source>You must specify a topic before connecting as a publisher.</source>
        <translation type="vanished">Publisher के रूप में कनेक्ट करने से पहले एक topic निर्दिष्ट करना आवश्यक है।</translation>
    </message>
    <message>
        <source>Configuration Error</source>
        <translation type="vanished">कॉन्फ़िगरेशन त्रुटि</translation>
    </message>
    <message>
        <source>MQTT Topic Not Set</source>
        <translation type="vanished">MQTT Topic सेट नहीं है</translation>
    </message>
    <message>
        <source>You won't receive any messages until a topic is configured.</source>
        <translation type="vanished">जब तक topic कॉन्फ़िगर नहीं किया जाता, तब तक कोई संदेश प्राप्त नहीं होगा।</translation>
    </message>
    <message>
        <source>Configuration Warning</source>
        <translation type="vanished">कॉन्फ़िगरेशन चेतावनी</translation>
    </message>
    <message>
        <source>Invalid MQTT Topic</source>
        <translation type="vanished">अमान्य MQTT Topic</translation>
    </message>
    <message>
        <source>The topic "%1" is not valid.</source>
        <translation type="vanished">Topic "%1" मान्य नहीं है।</translation>
    </message>
    <message>
        <source>Select PEM Certificates Directory</source>
        <translation type="vanished">PEM प्रमाणपत्र डायरेक्टरी चुनें</translation>
    </message>
    <message>
        <source>Subscription Error</source>
        <translation type="vanished">सदस्यता त्रुटि</translation>
    </message>
    <message>
        <source>Failed to subscribe to topic "%1".</source>
        <translation type="vanished">Topic "%1" की सदस्यता लेने में विफल।</translation>
    </message>
    <message>
        <source>Invalid MQTT Protocol Version</source>
        <translation type="vanished">अमान्य MQTT प्रोटोकॉल संस्करण</translation>
    </message>
    <message>
        <source>The MQTT broker rejected the connection due to an unsupported protocol version. Ensure that your client and broker support the same protocol version.</source>
        <translation type="vanished">MQTT broker ने असमर्थित प्रोटोकॉल संस्करण के कारण कनेक्शन अस्वीकार कर दिया। सुनिश्चित करें कि आपका client और broker समान प्रोटोकॉल संस्करण का समर्थन करते हैं।</translation>
    </message>
    <message>
        <source>Client ID Rejected</source>
        <translation type="vanished">Client ID अस्वीकृत</translation>
    </message>
    <message>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Try using a different client ID.</source>
        <translation type="vanished">Broker ने client ID अस्वीकार कर दिया। यह गलत स्वरूप में हो सकता है, बहुत लंबा हो सकता है, या पहले से उपयोग में हो सकता है। भिन्न client ID का उपयोग करें।</translation>
    </message>
    <message>
        <source>MQTT Server Unavailable</source>
        <translation type="vanished">MQTT सर्वर अनुपलब्ध</translation>
    </message>
    <message>
        <source>The network connection was established, but the broker is currently unavailable. Verify the broker status and try again later.</source>
        <translation type="vanished">नेटवर्क कनेक्शन स्थापित हो गया था, लेकिन ब्रोकर वर्तमान में अनुपलब्ध है। ब्रोकर की स्थिति सत्यापित करें और बाद में पुनः प्रयास करें।</translation>
    </message>
    <message>
        <source>Authentication Error</source>
        <translation type="vanished">प्रमाणीकरण त्रुटि</translation>
    </message>
    <message>
        <source>The username or password provided is incorrect or malformed. Double-check your credentials and try again.</source>
        <translation type="vanished">प्रदान किया गया उपयोगकर्ता नाम या पासवर्ड गलत या विकृत है। अपनी साख दोबारा जाँचें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <source>Authorization Error</source>
        <translation type="vanished">प्राधिकरण त्रुटि</translation>
    </message>
    <message>
        <source>The MQTT broker denied the connection due to insufficient permissions. Ensure that your account has the necessary access rights.</source>
        <translation type="vanished">MQTT ब्रोकर ने अपर्याप्त अनुमतियों के कारण कनेक्शन अस्वीकार कर दिया। सुनिश्चित करें कि आपके खाते के पास आवश्यक पहुँच अधिकार हैं।</translation>
    </message>
    <message>
        <source>Network or Transport Error</source>
        <translation type="vanished">नेटवर्क या ट्रांसपोर्ट त्रुटि</translation>
    </message>
    <message>
        <source>A network or transport layer issue occurred, causing an unexpected connection failure. Check your network connection and broker settings.</source>
        <translation type="vanished">एक नेटवर्क या ट्रांसपोर्ट लेयर समस्या उत्पन्न हुई, जिससे अप्रत्याशित कनेक्शन विफलता हुई। अपना नेटवर्क कनेक्शन और ब्रोकर सेटिंग्स जाँचें।</translation>
    </message>
    <message>
        <source>MQTT Protocol Violation</source>
        <translation type="vanished">MQTT प्रोटोकॉल उल्लंघन</translation>
    </message>
    <message>
        <source>The client detected a violation of the MQTT protocol and closed the connection. Check your MQTT implementation for compliance.</source>
        <translation type="vanished">क्लाइंट ने MQTT प्रोटोकॉल का उल्लंघन पाया और कनेक्शन बंद कर दिया। अनुपालन के लिए अपने MQTT कार्यान्वयन की जाँच करें।</translation>
    </message>
    <message>
        <source>Unknown Error</source>
        <translation type="vanished">अज्ञात त्रुटि</translation>
    </message>
    <message>
        <source>An unexpected error occurred. Check the logs for more details or restart the application.</source>
        <translation type="vanished">एक अप्रत्याशित त्रुटि हुई। अधिक विवरण के लिए लॉग देखें या एप्लिकेशन पुनः आरंभ करें।</translation>
    </message>
    <message>
        <source>MQTT 5 Error</source>
        <translation type="vanished">MQTT 5 त्रुटि</translation>
    </message>
    <message>
        <source>An MQTT protocol level 5 error occurred. Check the broker logs or reason codes for more details.</source>
        <translation type="vanished">MQTT प्रोटोकॉल स्तर 5 त्रुटि हुई। अधिक विवरण के लिए Broker लॉग या reason codes देखें।</translation>
    </message>
    <message>
        <source>MQTT Authentication Failed</source>
        <translation type="vanished">MQTT प्रमाणीकरण विफल</translation>
    </message>
    <message>
        <source>Authentication failed: %1.</source>
        <translation type="vanished">प्रमाणीकरण विफल: %1.</translation>
    </message>
    <message>
        <source>Extended authentication is required, but MQTT 5.0 is not enabled.</source>
        <translation type="vanished">विस्तारित प्रमाणीकरण आवश्यक है, लेकिन MQTT 5.0 सक्षम नहीं है।</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="vanished">अज्ञात</translation>
    </message>
    <message>
        <source>MQTT Authentication Required</source>
        <translation type="vanished">MQTT प्रमाणीकरण आवश्यक</translation>
    </message>
    <message>
        <source>The MQTT broker requires authentication using method: "%1".

Please provide the necessary credentials.</source>
        <translation type="vanished">MQTT Broker को इस विधि का उपयोग करके प्रमाणीकरण आवश्यक है: "%1".

कृपया आवश्यक क्रेडेंशियल प्रदान करें।</translation>
    </message>
    <message>
        <source>Enter MQTT Username</source>
        <translation type="vanished">MQTT उपयोगकर्ता नाम दर्ज करें</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation type="vanished">उपयोगकर्ता नाम:</translation>
    </message>
    <message>
        <source>Enter MQTT Password</source>
        <translation type="vanished">MQTT पासवर्ड दर्ज करें</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation type="vanished">पासवर्ड:</translation>
    </message>
</context>
<context>
    <name>MQTT::Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="792"/>
        <source>MQTT 3.1</source>
        <translation>MQTT 3.1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="793"/>
        <source>MQTT 3.1.1</source>
        <translation>MQTT 3.1.1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="794"/>
        <source>MQTT 5.0</source>
        <translation>MQTT 5.0</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="796"/>
        <source>TLS 1.2</source>
        <translation>TLS 1.2</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="797"/>
        <source>TLS 1.3</source>
        <translation>TLS 1.3</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="798"/>
        <source>TLS 1.3 or Later</source>
        <translation>TLS 1.3 या बाद का</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="799"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 या बाद का</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="800"/>
        <source>Any Protocol</source>
        <translation>कोई भी प्रोटोकॉल</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="801"/>
        <source>Secure Protocols Only</source>
        <translation>केवल सुरक्षित प्रोटोकॉल</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="803"/>
        <source>None</source>
        <translation>कोई नहीं</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="804"/>
        <source>Query Peer</source>
        <translation>पीयर से पूछें</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="805"/>
        <source>Verify Peer</source>
        <translation>पीयर सत्यापित करें</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="806"/>
        <source>Auto Verify Peer</source>
        <translation>स्वतः पीयर सत्यापित करें</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1123"/>
        <source>Raw RX Data</source>
        <translation>कच्चा RX डेटा</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1124"/>
        <source>Custom Script</source>
        <translation>कस्टम स्क्रिप्ट</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1125"/>
        <source>Dashboard Data (CSV)</source>
        <translation>डैशबोर्ड डेटा (CSV)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1126"/>
        <source>Dashboard Data (JSON)</source>
        <translation>डैशबोर्ड डेटा (JSON)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1281"/>
        <source>MQTT publisher unavailable</source>
        <translation>MQTT प्रकाशक अनुपलब्ध</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1282"/>
        <source>A valid commercial license is required to use MQTT publishing.</source>
        <translation>MQTT प्रकाशन उपयोग के लिए वैध व्यावसायिक लाइसेंस आवश्यक है।</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1284"/>
        <location filename="../../src/MQTT/Publisher.cpp" line="1853"/>
        <source>MQTT Test Connection</source>
        <translation>MQTT टेस्ट कनेक्शन</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1303"/>
        <source>Select PEM Certificates Directory</source>
        <translation>PEM प्रमाणपत्र डायरेक्टरी चुनें</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker reachable</source>
        <translation>MQTT ब्रोकर पहुँच योग्य है</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker unreachable</source>
        <translation>MQTT ब्रोकर पहुँच योग्य नहीं है</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT broker connection failed</source>
        <translation>MQTT ब्रोकर कनेक्शन विफल</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT Publisher</source>
        <translation>MQTT Publisher</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherScriptEditor</name>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="50"/>
        <source>MQTT Publisher Script</source>
        <translation>MQTT Publisher स्क्रिप्ट</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="87"/>
        <source>JavaScript</source>
        <translation>JavaScript</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="87"/>
        <source>Lua</source>
        <translation>Lua</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="93"/>
        <source>Sample frame bytes (text or hex)</source>
        <translation>सैंपल फ़्रेम बाइट्स (टेक्स्ट या hex)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="98"/>
        <source>Hex</source>
        <translation>Hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="99"/>
        <source>Test</source>
        <translation>परीक्षण करें</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="100"/>
        <source>Clear</source>
        <translation>साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="102"/>
        <source>Apply</source>
        <translation>लागू करें</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="103"/>
        <source>Cancel</source>
        <translation>रद्द करें</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="112"/>
        <source>Language:</source>
        <translation>भाषा:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="115"/>
        <source>Template:</source>
        <translation>टेम्पलेट:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="126"/>
        <source>Frame:</source>
        <translation>फ्रेम:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="130"/>
        <source>Output:</source>
        <translation>आउटपुट:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="270"/>
        <source>Enter a frame</source>
        <translation>फ्रेम दर्ज करें</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="277"/>
        <source>Invalid hex</source>
        <translation>अमान्य हेक्स</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="360"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>दस्तावेज़ फ़ॉर्मेट करें	Ctrl+Shift+I</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="361"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>चयन फ़ॉर्मेट करें	Ctrl+I</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="501"/>
        <source>--
-- Define a mqtt(frame) function that receives the raw bytes
-- of one parsed frame and returns the payload to publish to
-- the broker, or nil to skip this frame.
--
-- The frame argument matches what the Frame Parser script
-- sees: a Lua string holding the bytes between FrameReader
-- delimiters.
--
-- Example:
--   function mqtt(frame)
--     return frame    -- forward as-is
--   end
--
-- Use the Template dropdown for ready-made examples.
--
</source>
        <translation>--
-- एक mqtt(frame) फ़ंक्शन परिभाषित करें जो एक पार्स किए गए
-- फ़्रेम के रॉ बाइट्स प्राप्त करता है और ब्रोकर पर प्रकाशित
-- करने के लिए पेलोड लौटाता है, या इस फ़्रेम को छोड़ने के
-- लिए nil लौटाता है।
--
-- frame आर्ग्युमेंट वही है जो Frame Parser स्क्रिप्ट देखती है:
-- एक Lua स्ट्रिंग जिसमें FrameReader डिलिमिटर्स के बीच के
-- बाइट्स होते हैं।
--
-- उदाहरण:
--   function mqtt(frame)
--     return frame    -- जैसा है वैसा फ़ॉरवर्ड करें
--   end
--
-- तैयार उदाहरणों के लिए Template ड्रॉपडाउन का उपयोग करें।
--</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="518"/>
        <source>/*
 * Define a mqtt(frame) function that receives the raw bytes
 * of one parsed frame and returns the payload to publish to
 * the broker, or null to skip this frame.
 *
 * The frame argument matches what the Frame Parser script
 * sees: a string holding the bytes between FrameReader
 * delimiters.
 *
 * Example:
 *   function mqtt(frame) {
 *     return frame;   // forward as-is
 *   }
 *
 * Use the Template dropdown for ready-made examples.
 */</source>
        <translation>/*
 * एक mqtt(frame) फ़ंक्शन परिभाषित करें जो एक पार्स किए गए
 * फ़्रेम के रॉ बाइट्स प्राप्त करता है और ब्रोकर पर प्रकाशित
 * करने के लिए पेलोड लौटाता है, या इस फ़्रेम को छोड़ने के
 * लिए null लौटाता है।
 *
 * frame आर्ग्युमेंट वही है जो Frame Parser स्क्रिप्ट देखती है:
 * एक स्ट्रिंग जिसमें FrameReader डिलिमिटर्स के बीच के बाइट्स
 * होते हैं।
 *
 * उदाहरण:
 *   function mqtt(frame) {
 *     return frame;   // जैसा है वैसा फ़ॉरवर्ड करें
 *   }
 *
 * तैयार उदाहरणों के लिए Template ड्रॉपडाउन का उपयोग करें।</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="615"/>
        <source>Script is empty</source>
        <translation>स्क्रिप्ट खाली है</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="622"/>
        <source>Lua engine error</source>
        <translation>Lua इंजन त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="644"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="658"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="682"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="696"/>
        <source>Error: %1</source>
        <translation>त्रुटि: %1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="652"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="688"/>
        <source>mqtt() is not defined</source>
        <translation>mqtt() परिभाषित नहीं है</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="669"/>
        <source>(nil -- frame skipped)</source>
        <translation>(nil -- फ़्रेम छोड़ा गया)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="671"/>
        <source>(non-string return)</source>
        <translation>(non-string return)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="701"/>
        <source>(null -- frame skipped)</source>
        <translation>(null -- frame skipped)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="779"/>
        <source>Select Template…</source>
        <translation>टेम्पलेट चुनें…</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherWorker</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="674"/>
        <source>Configure broker hostname and port before testing the connection.</source>
        <translation>कनेक्शन परीक्षण से पहले ब्रोकर होस्टनेम और पोर्ट कॉन्फ़िगर करें।</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="710"/>
        <source>Successfully connected to %1:%2.</source>
        <translation>%1:%2 से सफलतापूर्वक कनेक्ट हुआ।</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="721"/>
        <source>Timed out after 5 seconds without reaching the broker.</source>
        <translation>ब्रोकर तक पहुँचे बिना 5 सेकंड के बाद टाइम आउट हुआ।</translation>
    </message>
</context>
<context>
    <name>MQTTConfiguration</name>
    <message>
        <source>MQTT Setup</source>
        <translation type="vanished">MQTT सेटअप</translation>
    </message>
    <message>
        <source>MQTT is a Pro Feature</source>
        <translation type="vanished">MQTT एक Pro फीचर है</translation>
    </message>
    <message>
        <source>Activate your license or visit the store to unlock MQTT support.</source>
        <translation type="vanished">MQTT समर्थन अनलॉक करने के लिए अपना लाइसेंस सक्रिय करें या स्टोर पर जाएं।</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="vanished">सामान्य</translation>
    </message>
    <message>
        <source>Authentication</source>
        <translation type="vanished">प्रमाणीकरण</translation>
    </message>
    <message>
        <source>MQTT Options</source>
        <translation type="vanished">MQTT विकल्प</translation>
    </message>
    <message>
        <source>SSL Properties</source>
        <translation type="vanished">SSL गुण</translation>
    </message>
    <message>
        <source>Host</source>
        <translation type="vanished">होस्ट</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="vanished">पोर्ट</translation>
    </message>
    <message>
        <source>Client ID</source>
        <translation type="vanished">क्लाइंट ID</translation>
    </message>
    <message>
        <source>Keep Alive (s)</source>
        <translation type="vanished">Keep Alive (s)</translation>
    </message>
    <message>
        <source>Clean Session</source>
        <translation type="vanished">Clean Session</translation>
    </message>
    <message>
        <source>Username</source>
        <translation type="vanished">उपयोगकर्ता नाम</translation>
    </message>
    <message>
        <source>MQTT Username</source>
        <translation type="vanished">MQTT उपयोगकर्ता नाम</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="vanished">पासवर्ड</translation>
    </message>
    <message>
        <source>MQTT Password</source>
        <translation type="vanished">MQTT पासवर्ड</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="vanished">संस्करण</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="vanished">मोड</translation>
    </message>
    <message>
        <source>Topic</source>
        <translation type="vanished">टॉपिक</translation>
    </message>
    <message>
        <source>e.g. sensors/temperature or home/+/status</source>
        <translation type="vanished">उदा. sensors/temperature या home/+/status</translation>
    </message>
    <message>
        <source>Will Retain</source>
        <translation type="vanished">Will Retain</translation>
    </message>
    <message>
        <source>Will QoS</source>
        <translation type="vanished">Will QOS</translation>
    </message>
    <message>
        <source>Will Topic</source>
        <translation type="vanished">Will टॉपिक</translation>
    </message>
    <message>
        <source>e.g. device/alerts/offline</source>
        <translation type="vanished">उदा. device/alerts/offline</translation>
    </message>
    <message>
        <source>Will Message</source>
        <translation type="vanished">Will संदेश</translation>
    </message>
    <message>
        <source>e.g. Device unexpectedly disconnected</source>
        <translation type="vanished">उदा. Device unexpectedly disconnected</translation>
    </message>
    <message>
        <source>Enable SSL</source>
        <translation type="vanished">SSL सक्षम करें</translation>
    </message>
    <message>
        <source>SSL Protocol</source>
        <translation type="vanished">SSL प्रोटोकॉल</translation>
    </message>
    <message>
        <source>Verify Depth</source>
        <translation type="vanished">सत्यापन गहराई</translation>
    </message>
    <message>
        <source>Verify Mode</source>
        <translation type="vanished">सत्यापन मोड</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">बंद करें</translation>
    </message>
    <message>
        <source>Disconnect</source>
        <translation type="vanished">डिस्कनेक्ट करें</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="vanished">कनेक्ट करें</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="205"/>
        <source>Console Only Mode</source>
        <translation>केवल कंसोल मोड</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="208"/>
        <source>Quick Plot Mode</source>
        <translation>त्वरित प्लॉट मोड</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="215"/>
        <source>Empty Project</source>
        <translation>खाली प्रोजेक्ट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="696"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="704"/>
        <source>Waiting for data…</source>
        <translation>डेटा का इंतज़ार है…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="705"/>
        <source>Connecting to device…</source>
        <translation>डिवाइस से कनेक्ट हो रहा है…</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="146"/>
        <source>Code sample</source>
        <translation>कोड सैंपल</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="148"/>
        <source>Completer</source>
        <translation>कम्प्लीटर</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="150"/>
        <source>Highlighter</source>
        <translation>हाइलाइटर</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="152"/>
        <source>Style</source>
        <translation>स्टाइल</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="214"/>
        <source> spaces</source>
        <translation>स्पेस</translation>
    </message>
</context>
<context>
    <name>MarkdownWebView</name>
    <message>
        <location filename="../../qml/Widgets/MarkdownWebView.qml" line="36"/>
        <source>Copied to Clipboard</source>
        <translation>क्लिपबोर्ड पर कॉपी किया गया</translation>
    </message>
</context>
<context>
    <name>Mdf4Player</name>
    <message>
        <location filename="../../qml/Dialogs/Mdf4Player.qml" line="24"/>
        <source>MDF4 Player</source>
        <translation>MDF4 प्लेयर</translation>
    </message>
</context>
<context>
    <name>MessageBubble</name>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="97"/>
        <source>Error</source>
        <translation>त्रुटि</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>You</source>
        <translation>आप</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>Assistant</source>
        <translation>सहायक</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="208"/>
        <source>Discovery</source>
        <translation>खोज</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="209"/>
        <source>Execution</source>
        <translation>निष्पादन</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="239"/>
        <source>Approve %1 actions in %2?</source>
        <translation>%2 में %1 क्रियाओं को स्वीकृत करें?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="249"/>
        <source>These calls will run together. Expand each card below to inspect arguments.</source>
        <translation>ये कॉल एक साथ चलेंगी। तर्कों की जांच के लिए नीचे प्रत्येक कार्ड को विस्तृत करें।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="260"/>
        <source>Approve all</source>
        <translation>सभी स्वीकृत करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="266"/>
        <source>Deny all</source>
        <translation>सभी अस्वीकृत करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="332"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="384"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="436"/>
        <source>Copy</source>
        <translation>कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="337"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="389"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="441"/>
        <source>Copy All</source>
        <translation>सभी कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="345"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="397"/>
        <source>Select All</source>
        <translation>सभी चुनें</translation>
    </message>
</context>
<context>
    <name>MessageWebView</name>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="63"/>
        <source>You</source>
        <translation>आप</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="64"/>
        <source>Assistant</source>
        <translation>असिस्टेंट</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="65"/>
        <location filename="../../qml/AI/MessageWebView.qml" line="71"/>
        <source>Error</source>
        <translation>त्रुटि</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="66"/>
        <source>Discovery</source>
        <translation>खोज</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="67"/>
        <source>Execution</source>
        <translation>निष्पादन</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="68"/>
        <source>Running</source>
        <translation>चल रहा है</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="69"/>
        <source>Awaiting approval</source>
        <translation>अनुमोदन की प्रतीक्षा में</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="70"/>
        <source>Done</source>
        <translation>पूर्ण</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="72"/>
        <source>Denied</source>
        <translation>अस्वीकृत</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="73"/>
        <source>Blocked</source>
        <translation>अवरुद्ध</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="74"/>
        <source>Approve</source>
        <translation>स्वीकृत करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="75"/>
        <source>Deny</source>
        <translation>अस्वीकार करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="76"/>
        <source>Approve all</source>
        <translation>सभी स्वीकृत करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="77"/>
        <source>Deny all</source>
        <translation>सभी अस्वीकृत करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="78"/>
        <source>Arguments</source>
        <translation>आर्ग्युमेंट्स</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="79"/>
        <source>Result</source>
        <translation>परिणाम</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="80"/>
        <source>Approve {n} actions in {family}?</source>
        <translation>{family} में {n} क्रियाएँ स्वीकृत करें?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="81"/>
        <source>These calls will run together. Expand each card to inspect arguments.</source>
        <translation>ये कॉल्स एक साथ चलेंगी। आर्ग्युमेंट्स जाँचने के लिए प्रत्येक कार्ड विस्तृत करें।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="82"/>
        <source>Copy</source>
        <translation>कॉपी करें</translation>
    </message>
</context>
<context>
    <name>Misc::Examples</name>
    <message>
        <location filename="../../src/Misc/Examples.cpp" line="282"/>
        <source>Failed to load README: %1</source>
        <translation>README लोड करने में विफल: %1</translation>
    </message>
</context>
<context>
    <name>Misc::ExtensionManager</name>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="244"/>
        <source>Theme</source>
        <translation>थीम</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="247"/>
        <source>Frame Parser</source>
        <translation>फ़्रेम पार्सर</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="250"/>
        <source>Project Template</source>
        <translation>प्रोजेक्ट टेम्पलेट</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="253"/>
        <source>Plugin</source>
        <translation>प्लगइन</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="256"/>
        <source>All Types</source>
        <translation>सभी प्रकार</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="473"/>
        <source>Reset Extensions</source>
        <translation>एक्सटेंशन रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="474"/>
        <source>This uninstalls all extensions, removes all custom repositories, and restores the default settings. Continue?</source>
        <translation>यह सभी एक्सटेंशन को अनइंस्टॉल करता है, सभी कस्टम रिपॉजिटरी को हटाता है, और डिफ़ॉल्ट सेटिंग्स को पुनर्स्थापित करता है। जारी रखें?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="513"/>
        <source>Select Extension Repository Folder</source>
        <translation>एक्सटेंशन रिपॉजिटरी फ़ोल्डर चुनें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1017"/>
        <source>Installed (repository no longer available)</source>
        <translation>इंस्टॉल किया गया (रिपॉजिटरी अब उपलब्ध नहीं)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1324"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1334"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1355"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1377"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1421"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1431"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1440"/>
        <source>Plugin Error</source>
        <translation>प्लगइन त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1324"/>
        <source>Plugin "%1" is not installed.</source>
        <translation>प्लगइन "%1" इंस्टॉल नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1335"/>
        <source>Extension "%1" is not a plugin (type: %2).</source>
        <translation>एक्सटेंशन "%1" एक प्लगइन नहीं है (प्रकार: %2)।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1356"/>
        <source>Cannot read plugin metadata file:
%1/info.json</source>
        <translation>प्लगइन मेटाडेटा फ़ाइल नहीं पढ़ी जा सकती:
%1/info.json</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1378"/>
        <source>Plugin "%1" requires gRPC but this build does not include gRPC support.</source>
        <translation>प्लगइन "%1" को GRPC की आवश्यकता है लेकिन इस बिल्ड में GRPC समर्थन शामिल नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1387"/>
        <source>This plugin uses gRPC for high-performance data streaming. The API server needs to be enabled.

Would you like to enable it now?</source>
        <translation>यह प्लगइन उच्च-प्रदर्शन डेटा स्ट्रीमिंग के लिए GRPC का उपयोग करता है। API सर्वर को सक्षम करना आवश्यक है।

क्या आप इसे अभी सक्षम करना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1393"/>
        <source>API Server Required</source>
        <translation>API सर्वर आवश्यक</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1422"/>
        <source>Plugin "%1" has no 'entry' field in info.json.</source>
        <translation>प्लगइन "%1" में info.json में 'entry' फ़ील्ड नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1432"/>
        <source>Entry point not found:
%1</source>
        <translation>एंट्री पॉइंट नहीं मिला:
%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1441"/>
        <source>Plugin "%1" has an invalid entry point path.</source>
        <translation>प्लगइन "%1" में अमान्य एंट्री पॉइंट पथ है।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1484"/>
        <source>Missing Dependency</source>
        <translation>निर्भरता अनुपलब्ध</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1485"/>
        <source>This plugin requires "%1" but it was not found on your system.

Would you like to open the download page?</source>
        <translation>इस प्लगइन को "%1" की आवश्यकता है लेकिन यह आपके सिस्टम पर नहीं मिला।

क्या आप डाउनलोड पेज खोलना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1390"/>
        <source>Plugins need the API server to communicate with Serial Studio. Would you like to enable it now?</source>
        <translation>प्लगइन को Serial Studio के साथ संचार करने के लिए API सर्वर की आवश्यकता है। क्या आप इसे अभी सक्षम करना चाहते हैं?</translation>
    </message>
</context>
<context>
    <name>Misc::GraphicsBackend</name>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="246"/>
        <source>Restart Required</source>
        <translation>रीस्टार्ट आवश्यक</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="247"/>
        <source>The new rendering backend will take effect after restarting Serial Studio. Restart now to apply the change?</source>
        <translation>नया रेंडरिंग बैकएंड Serial Studio को रीस्टार्ट करने के बाद प्रभावी होगा। बदलाव लागू करने के लिए अभी रीस्टार्ट करें?</translation>
    </message>
</context>
<context>
    <name>Misc::HelpCenter</name>
    <message>
        <location filename="../../src/Misc/HelpCenter.cpp" line="303"/>
        <source>Failed to load page: %1</source>
        <translation>पेज लोड करने में विफल: %1</translation>
    </message>
</context>
<context>
    <name>Misc::IconEngine</name>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="148"/>
        <source>Invalid icon identifier</source>
        <translation>अमान्य आइकन पहचानकर्ता</translation>
    </message>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="218"/>
        <source>Empty SVG data received</source>
        <translation>खाली SVG डेटा प्राप्त हुआ</translation>
    </message>
</context>
<context>
    <name>Misc::ShortcutGenerator</name>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="73"/>
        <source>Windows Shortcut (*.lnk)</source>
        <translation>Windows शॉर्टकट (*.lnk)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="75"/>
        <source>macOS Application (*.app)</source>
        <translation>macOS एप्लिकेशन (*.app)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="77"/>
        <source>Desktop Entry (*.desktop)</source>
        <translation>Desktop एंट्री (*.desktop)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="101"/>
        <source>Use a .icns icon for the sharpest result in Finder and the Dock.</source>
        <translation>Finder और Dock में सबसे तेज़ रिज़ल्ट के लिए .icns आइकन का उपयोग करें।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="103"/>
        <source>Leave the icon empty to inherit the Serial Studio executable icon.</source>
        <translation>Serial Studio एक्ज़ीक्यूटेबल आइकन इनहेरिट करने के लिए आइकन को खाली छोड़ें।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="105"/>
        <source>Place the file under ~/.local/share/applications/ to expose it in your application launcher.</source>
        <translation>अपने एप्लिकेशन लॉन्चर में एक्सपोज़ करने के लिए फ़ाइल को ~/.local/share/applications/ के अंदर रखें।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="116"/>
        <source>Apple Icon Image (*.icns)</source>
        <translation>Apple आइकन इमेज (*.icns)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="118"/>
        <source>Windows Icon (*.ico)</source>
        <translation>Windows आइकन (*.ico)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="120"/>
        <source>Vector or Raster Image (*.svg *.png)</source>
        <translation>वेक्टर या रास्टर छवि (*.svg *.png)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="212"/>
        <source>A Pro license is required to generate shortcuts.</source>
        <translation>शॉर्टकट जनरेट करने के लिए Pro लाइसेंस आवश्यक है।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="217"/>
        <source>No output path was provided.</source>
        <translation>कोई आउटपुट पथ प्रदान नहीं किया गया।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="258"/>
        <source>Failed to write shortcut file.</source>
        <translation>शॉर्टकट फ़ाइल लिखने में विफल।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="222"/>
        <source>Could not replace the existing shortcut at %1.</source>
        <translation>%1 पर मौजूदा शॉर्टकट को बदला नहीं जा सका।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="232"/>
        <source>Could not create the .app bundle directory layout.</source>
        <translation>.app बंडल डायरेक्टरी लेआउट नहीं बनाया जा सका।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="125"/>
        <source>Could not write the bundle launcher: %1</source>
        <translation>बंडल लॉन्चर नहीं लिखा जा सका: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="144"/>
        <source>Could not mark the bundle launcher as executable.</source>
        <translation>बंडल लॉन्चर को एक्ज़ीक्यूटेबल के रूप में चिह्नित नहीं किया जा सका।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="164"/>
        <source>Could not write Info.plist: %1</source>
        <translation>Info.plist नहीं लिखा जा सका: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="271"/>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="140"/>
        <source>Windows shortcut writer is not available on this platform.</source>
        <translation>Windows शॉर्टकट राइटर इस प्लेटफ़ॉर्म पर उपलब्ध नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="285"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="199"/>
        <source>Linux shortcut writer is not available on this platform.</source>
        <translation>Linux शॉर्टकट राइटर इस प्लेटफ़ॉर्म पर उपलब्ध नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="107"/>
        <source>Could not initialise COM (required to write .lnk shortcuts).</source>
        <translation>COM इनिशियलाइज़ नहीं हो सका (.lnk शॉर्टकट लिखने के लिए आवश्यक)।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="118"/>
        <source>CoCreateInstance(IShellLink) failed (HRESULT 0x%1).</source>
        <translation>CoCreateInstance(IShellLink) विफल (HRESULT 0x%1)।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="153"/>
        <source>QueryInterface(IPersistFile) failed (HRESULT 0x%1).</source>
        <translation>QueryInterface(IPersistFile) विफल (HRESULT 0x%1)।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="163"/>
        <source>Saving the .lnk file failed (HRESULT 0x%1).</source>
        <translation>.lnk फ़ाइल सहेजना विफल (HRESULT 0x%1)।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="185"/>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="154"/>
        <source>macOS shortcut writer is not available on this platform.</source>
        <translation>macOS शॉर्टकट राइटर इस प्लेटफ़ॉर्म पर उपलब्ध नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="86"/>
        <source>Could not open the shortcut path for writing: %1</source>
        <translation>शॉर्टकट पथ लिखने के लिए खोल नहीं सका: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="91"/>
        <source>Serial Studio shortcut</source>
        <translation>Serial Studio शॉर्टकट</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="112"/>
        <source>Could not mark the shortcut as executable.</source>
        <translation>शॉर्टकट को एक्ज़ीक्यूटेबल के रूप में मार्क नहीं कर सका।</translation>
    </message>
</context>
<context>
    <name>Misc::ThemeManager</name>
    <message>
        <location filename="../../src/Misc/ThemeManager.cpp" line="398"/>
        <source>System</source>
        <translation>सिस्टम</translation>
    </message>
</context>
<context>
    <name>Misc::Utilities</name>
    <message>
        <source>Check for updates automatically?</source>
        <translation type="vanished">अपडेट स्वचालित रूप से जाँचें?</translation>
    </message>
    <message>
        <source>Should %1 automatically check for updates? You can always check for updates manually from the "About" dialog</source>
        <translation type="vanished">क्या %1 को स्वचालित रूप से अपडेट जाँचने चाहिए? आप "About" डायलॉग से मैन्युअल रूप से अपडेट जाँच सकते हैं</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="161"/>
        <source>Ok</source>
        <translation>ठीक है</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="163"/>
        <source>Save</source>
        <translation>सहेजें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="165"/>
        <source>Save all</source>
        <translation>सभी सहेजें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="167"/>
        <source>Open</source>
        <translation>खोलें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="169"/>
        <source>Yes</source>
        <translation>हाँ</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="171"/>
        <source>Yes to all</source>
        <translation>सभी के लिए हाँ</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="173"/>
        <source>No</source>
        <translation>नहीं</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="175"/>
        <source>No to all</source>
        <translation>सभी के लिए नहीं</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="177"/>
        <source>Abort</source>
        <translation>निरस्त करें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="179"/>
        <source>Retry</source>
        <translation>पुनः प्रयास करें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="181"/>
        <source>Ignore</source>
        <translation>अनदेखा करें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="183"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="185"/>
        <source>Cancel</source>
        <translation>रद्द करें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="187"/>
        <source>Discard</source>
        <translation>त्यागें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="189"/>
        <source>Help</source>
        <translation>सहायता</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="191"/>
        <source>Apply</source>
        <translation>लागू करें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="193"/>
        <source>Reset</source>
        <translation>रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="195"/>
        <source>Restore defaults</source>
        <translation>डिफ़ॉल्ट पुनर्स्थापित करें</translation>
    </message>
</context>
<context>
    <name>Misc::WorkspaceManager</name>
    <message>
        <location filename="../../src/Misc/WorkspaceManager.cpp" line="261"/>
        <source>Select Workspace Location</source>
        <translation>वर्कस्पेस स्थान चुनें</translation>
    </message>
</context>
<context>
    <name>Modbus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="49"/>
        <source>Protocol</source>
        <translation>प्रोटोकॉल</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="70"/>
        <source>Serial Port</source>
        <translation>सीरियल पोर्ट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="93"/>
        <source>Baud Rate</source>
        <translation>बॉड रेट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="184"/>
        <source>Parity</source>
        <translation>पैरिटी</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="205"/>
        <source>Data Bits</source>
        <translation>डेटा बिट्स</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="226"/>
        <source>Stop Bits</source>
        <translation>स्टॉप बिट्स</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="247"/>
        <source>Host</source>
        <translation>होस्ट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="257"/>
        <source>IP Address</source>
        <translation>IP एड्रेस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="271"/>
        <source>Port</source>
        <translation>पोर्ट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="280"/>
        <source>TCP Port</source>
        <translation>TCP पोर्ट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="308"/>
        <source>Slave Address</source>
        <translation>स्लेव एड्रेस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="313"/>
        <source>1-247</source>
        <translation>1-247</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="328"/>
        <source>Poll Interval (ms)</source>
        <translation>पोल अंतराल (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="333"/>
        <source>Polling interval</source>
        <translation>पोलिंग अंतराल</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="361"/>
        <source>Configure Register Groups…</source>
        <translation>रजिस्टर ग्रुप कॉन्फ़िगर करें…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="371"/>
        <source>Import Register Map…</source>
        <translation>रजिस्टर मैप आयात करें…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="386"/>
        <source>%1 group(s) configured</source>
        <translation>%1 समूह कॉन्फ़िगर किए गए</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="387"/>
        <source>No groups configured</source>
        <translation>कोई समूह कॉन्फ़िगर नहीं है</translation>
    </message>
</context>
<context>
    <name>ModbusGroupsDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="45"/>
        <source>Modbus Register Groups</source>
        <translation>Modbus रजिस्टर समूह</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="166"/>
        <source>Configure multiple register groups to poll different register types in sequence.</source>
        <translation>विभिन्न रजिस्टर प्रकारों को क्रम में poll करने के लिए एकाधिक रजिस्टर समूह कॉन्फ़िगर करें।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="174"/>
        <source>Add New Group</source>
        <translation>नया समूह जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="198"/>
        <source>Register Type:</source>
        <translation>रजिस्टर प्रकार:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="210"/>
        <source>Start Address:</source>
        <translation>प्रारंभ पता:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="217"/>
        <source>0-65535</source>
        <translation>0-65535</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="223"/>
        <source>Register Count:</source>
        <translation>रजिस्टर संख्या:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="234"/>
        <source>1-125</source>
        <translation>1-125</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="239"/>
        <source>Add Group</source>
        <translation>समूह जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="262"/>
        <source>Configured Groups</source>
        <translation>कॉन्फ़िगर किए गए समूह</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="296"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="303"/>
        <source>Type</source>
        <translation>प्रकार</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="311"/>
        <source>Start</source>
        <translation>प्रारंभ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="318"/>
        <source>Count</source>
        <translation>संख्या</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="325"/>
        <source>Action</source>
        <translation>एक्शन</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="400"/>
        <source>Remove</source>
        <translation>हटाएं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="412"/>
        <source>No groups configured.
Add groups above to poll multiple register types.</source>
        <translation>कोई समूह कॉन्फ़िगर नहीं है।
एकाधिक रजिस्टर प्रकारों को पोल करने के लिए ऊपर समूह जोड़ें।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="430"/>
        <source>Total groups: %1</source>
        <translation>कुल समूह: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="434"/>
        <source>Generate Project</source>
        <translation>प्रोजेक्ट जनरेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="440"/>
        <source>Clear All</source>
        <translation>सभी साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="446"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>ModbusPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="33"/>
        <source>Modbus Register Map Preview</source>
        <translation>Modbus रजिस्टर मैप पूर्वावलोकन</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="155"/>
        <source>File: %1</source>
        <translation>फ़ाइल: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="163"/>
        <source>Review the registers to import into a new Serial Studio project.</source>
        <translation>नए Serial Studio प्रोजेक्ट में आयात करने के लिए रजिस्टरों की समीक्षा करें।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="171"/>
        <source>Registers</source>
        <translation>रजिस्टर</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="205"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="212"/>
        <source>Name</source>
        <translation>नाम</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="221"/>
        <source>Address</source>
        <translation>पता</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="227"/>
        <source>Type</source>
        <translation>प्रकार</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="235"/>
        <source>Data Type</source>
        <translation>डेटा प्रकार</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="242"/>
        <source>Units</source>
        <translation>इकाइयाँ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="343"/>
        <source>No registers found in file.</source>
        <translation>फ़ाइल में कोई रजिस्टर नहीं मिला।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="361"/>
        <source>Total: %1 registers in %2 groups</source>
        <translation>कुल: %2 समूहों में %1 रजिस्टर</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="367"/>
        <source>Cancel</source>
        <translation>रद्द करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="379"/>
        <source>Create Project</source>
        <translation>प्रोजेक्ट बनाएँ</translation>
    </message>
</context>
<context>
    <name>MqttPublisherView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="33"/>
        <source>MQTT Publisher</source>
        <translation>MQTT Publisher</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="110"/>
        <source>Connected to broker</source>
        <translation>ब्रोकर से कनेक्टेड</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="111"/>
        <source>Not connected</source>
        <translation>कनेक्ट नहीं है</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="124"/>
        <source>Test Connection</source>
        <translation>कनेक्शन परीक्षण करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="129"/>
        <source>Probe the broker with the current settings</source>
        <translation>वर्तमान सेटिंग्स के साथ broker की जांच करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="162"/>
        <source>Enable publishing first</source>
        <translation>पहले publishing सक्षम करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="140"/>
        <source>Edit Script</source>
        <translation>Script संपादित करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="146"/>
        <source>Edit the publisher script (Lua or JavaScript)</source>
        <translation>Publisher script (Lua या JavaScript) संपादित करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="158"/>
        <source>Load CA Certs</source>
        <translation>CA Certs लोड करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="164"/>
        <source>Add PEM certificates from a folder</source>
        <translation>फ़ोल्डर से PEM certificates जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="165"/>
        <source>Enable SSL/TLS first</source>
        <translation>पहले SSL/TLS सक्षम करें</translation>
    </message>
</context>
<context>
    <name>MultiPlot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">इंटरपोलेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="305"/>
        <source>Interpolation: %1</source>
        <translation>इंटरपोलेशन: %1</translation>
    </message>
    <message>
        <source>Show Legends</source>
        <translation type="vanished">लीजेंड दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="327"/>
        <source>Show X Axis Label</source>
        <translation>X अक्ष लेबल दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="338"/>
        <source>Show Y Axis Label</source>
        <translation>Y अक्ष लेबल दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="350"/>
        <source>Show Crosshair</source>
        <translation>क्रॉसहेयर दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Pause</source>
        <translation>रोकें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Resume</source>
        <translation>फिर से शुरू करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="374"/>
        <source>Sweep / Trigger Mode</source>
        <translation>स्वीप / ट्रिगर मोड</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="386"/>
        <source>Trigger Settings</source>
        <translation>ट्रिगर सेटिंग्स</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="410"/>
        <source>Reset View</source>
        <translation>व्यू रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="416"/>
        <source>Axis Range Settings</source>
        <translation>अक्ष रेंज सेटिंग्स</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">सैंपल</translation>
    </message>
</context>
<context>
    <name>NativeTemplates</name>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="292"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="430"/>
        <source>Bytes per value</source>
        <translation>प्रति मान बाइट्स</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="293"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="431"/>
        <source>Number of bytes combined into each channel value.</source>
        <translation>प्रत्येक चैनल मान में संयोजित बाइट्स की संख्या।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="301"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="439"/>
        <source>Endianness</source>
        <translation>एंडियननेस</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="302"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="440"/>
        <source>Byte order used when combining multi-byte values.</source>
        <translation>मल्टी-बाइट मानों को संयोजित करते समय उपयोग किया जाने वाला बाइट क्रम।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="310"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="448"/>
        <source>Signed values</source>
        <translation>साइन किए गए मान</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="311"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="449"/>
        <source>Interprets each value as two's-complement signed.</source>
        <translation>प्रत्येक मान को two's-complement साइन किए गए के रूप में व्याख्यायित करता है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="651"/>
        <source>Tag routing table</source>
        <translation>टैग रूटिंग तालिका</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="652"/>
        <source>Comma-separated tag:index entries, e.g. 1:0,2:1,3:2. Tags may be decimal or 0x-prefixed hex.</source>
        <translation>अल्पविराम से अलग किए गए tag:index प्रविष्टियाँ, जैसे 1:0,2:1,3:2। टैग दशमलव या 0x-उपसर्गित हेक्स हो सकते हैं।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1245"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1096"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1300"/>
        <source>Validate checksum</source>
        <translation>चेकसम सत्यापित करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1097"/>
        <source>Rejects messages with an invalid Fletcher checksum.</source>
        <translation>अमान्य Fletcher चेकसम वाले संदेशों को अस्वीकार करता है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1301"/>
        <source>Rejects messages with an invalid additive checksum.</source>
        <translation>अमान्य योगात्मक चेकसम वाले संदेशों को अस्वीकार करता है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1454"/>
        <source>Protocol version</source>
        <translation>प्रोटोकॉल संस्करण</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1455"/>
        <source>Selects the expected start marker (0xFE for v1, 0xFD for v2).</source>
        <translation>अपेक्षित स्टार्ट मार्कर का चयन करता है (v1 के लिए 0xFE, v2 के लिए 0xFD)।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1883"/>
        <source>Validate CRC</source>
        <translation>CRC सत्यापित करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1884"/>
        <source>Rejects frames with an invalid CRC-24Q checksum.</source>
        <translation>अमान्य CRC-24Q चेकसम वाले फ्रेम को अस्वीकार करता है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2059"/>
        <source>Channel count</source>
        <translation>चैनल संख्या</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2060"/>
        <source>Number of output channels (registers or coils).</source>
        <translation>आउटपुट चैनलों की संख्या (रजिस्टर या कॉइल)।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2068"/>
        <source>Register offset</source>
        <translation>रजिस्टर ऑफसेट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2069"/>
        <source>Address offset subtracted from single-write echoes (40001 for legacy Modicon maps).</source>
        <translation>सिंगल-राइट इको से घटाया गया पता ऑफसेट (लीगेसी Modicon मैप के लिए 40001)।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2079"/>
        <source>Signed registers</source>
        <translation>साइन्ड रजिस्टर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2080"/>
        <source>Interprets 16-bit registers as two's-complement signed values.</source>
        <translation>16-बिट रजिस्टर को टू'ज़-कॉम्प्लीमेंट साइन्ड मानों के रूप में व्याख्यायित करता है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2386"/>
        <source>Payload layout</source>
        <translation>पेलोड लेआउट</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2387"/>
        <source>Array emits every element in order; map routes keys through the key list.</source>
        <translation>ऐरे प्रत्येक एलिमेंट को क्रम में उत्सर्जित करता है; मैप की लिस्ट के माध्यम से की को रूट करता है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2397"/>
        <source>Keys (map mode)</source>
        <translation>कुंजियाँ (मैप मोड)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2398"/>
        <source>Comma-separated map keys in channel order. Only used for the map layout.</source>
        <translation>चैनल क्रम में अल्पविराम से अलग की गई मैप कुंजियाँ। केवल मैप लेआउट के लिए उपयोग की जाती हैं।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="184"/>
        <source>Scalar fields</source>
        <translation>स्केलर फ़ील्ड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="185"/>
        <source>Comma-separated JSON fields repeated in every generated frame.</source>
        <translation>अल्पविराम से अलग की गई JSON फ़ील्ड जो प्रत्येक जनरेट किए गए फ़्रेम में दोहराई जाती हैं।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="192"/>
        <source>Sample array field</source>
        <translation>सैंपल ऐरे फ़ील्ड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="193"/>
        <source>JSON field holding the batched sample array.</source>
        <translation>JSON फ़ील्ड जो बैच किए गए सैंपल ऐरे को रखती है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="334"/>
        <source>Records array field</source>
        <translation>रिकॉर्ड ऐरे फ़ील्ड</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="335"/>
        <source>JSON field holding the array of record objects.</source>
        <translation>JSON फ़ील्ड जो रिकॉर्ड ऑब्जेक्ट्स की ऐरे को रखती है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="341"/>
        <source>Record fields (in channel order)</source>
        <translation>रिकॉर्ड फ़ील्ड (चैनल क्रम में)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="342"/>
        <source>Comma-separated record fields. The position of each field sets its channel index.</source>
        <translation>अल्पविराम से अलग की गई रिकॉर्ड फ़ील्ड। प्रत्येक फ़ील्ड की स्थिति उसका चैनल इंडेक्स निर्धारित करती है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="605"/>
        <source>Column widths</source>
        <translation>कॉलम चौड़ाई</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="606"/>
        <source>Comma-separated character counts per field. Leave empty to split on whitespace.</source>
        <translation>प्रत्येक फ़ील्ड के लिए अल्पविराम से अलग किए गए वर्ण गणना। व्हाइटस्पेस पर विभाजित करने के लिए खाली छोड़ें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="614"/>
        <source>Trim whitespace</source>
        <translation>व्हाइटस्पेस ट्रिम करें</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="615"/>
        <source>Removes padding around every sliced field.</source>
        <translation>प्रत्येक विभाजित फ़ील्ड के चारों ओर पैडिंग हटाता है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="744"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="893"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1360"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1787"/>
        <source>Keys (in channel order)</source>
        <translation>कुंजियाँ (चैनल क्रम में)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="745"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="894"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1788"/>
        <source>Comma-separated key names. The position of each key sets its channel index.</source>
        <translation>अल्पविराम से अलग की गई कुंजी नाम। प्रत्येक कुंजी की स्थिति उसका चैनल इंडेक्स निर्धारित करती है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="753"/>
        <source>Pair separator</source>
        <translation>युग्म विभाजक</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="754"/>
        <source>Character between key=value pairs.</source>
        <translation>key=value युग्मों के बीच वर्ण।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="760"/>
        <source>Key-value separator</source>
        <translation>कुंजी-मान विभाजक</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="761"/>
        <source>Character between a key and its value.</source>
        <translation>कुंजी और उसके मान के बीच वर्ण।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="767"/>
        <source>Numeric values only</source>
        <translation>केवल संख्यात्मक मान</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="768"/>
        <source>Ignores pairs whose value is not a number.</source>
        <translation>उन जोड़ों को अनदेखा करता है जिनका मान संख्या नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1010"/>
        <source>Command routing table</source>
        <translation>कमांड रूटिंग टेबल</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1011"/>
        <source>Semicolon-separated entries of NAME:index list, e.g. CSQ:0,1;CREG:2,3;CGATT:4.</source>
        <translation>अर्धविराम से अलग की गई NAME:index सूची की प्रविष्टियाँ, जैसे CSQ:0,1;CREG:2,3;CGATT:4।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1236"/>
        <source>Talker prefix</source>
        <translation>टॉकर प्रीफ़िक्स</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1237"/>
        <source>Two-letter talker id, e.g. GP for GPS or GN for multi-constellation receivers.</source>
        <translation>दो-अक्षर टॉकर id, जैसे GPS के लिए GP या मल्टी-कॉन्स्टेलेशन रिसीवर के लिए GN।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1246"/>
        <source>Rejects sentences whose *hh checksum does not match.</source>
        <translation>उन वाक्यों को अस्वीकार करता है जिनका *hh checksum मेल नहीं खाता।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1361"/>
        <source>Comma-separated parameter names. The position of each key sets its channel index.</source>
        <translation>कॉमा-विभाजित पैरामीटर नाम। प्रत्येक की की स्थिति उसका चैनल इंडेक्स सेट करती है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1500"/>
        <source>Fields (in channel order)</source>
        <translation>फ़ील्ड (चैनल क्रम में)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1501"/>
        <source>Comma-separated field names. The position of each field sets its channel index.</source>
        <translation>कॉमा-विभाजित फ़ील्ड नाम। प्रत्येक फ़ील्ड की स्थिति उसका चैनल इंडेक्स सेट करती है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1620"/>
        <source>Tags (in channel order)</source>
        <translation>टैग (चैनल क्रम में)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1621"/>
        <source>Comma-separated tag names. The position of each tag sets its channel index.</source>
        <translation>कॉमा से अलग किए गए टैग नाम। प्रत्येक टैग की स्थिति उसका चैनल इंडेक्स निर्धारित करती है।</translation>
    </message>
    <message>
        <source>Register blocks</source>
        <translation type="vanished">रजिस्टर ब्लॉक</translation>
    </message>
    <message>
        <source>Polled register blocks with typed, scaled entries. Written by the Modbus register map importer.</source>
        <translation type="vanished">टाइप किए गए, स्केल किए गए एंट्री के साथ पोल किए गए रजिस्टर ब्लॉक। Modbus रजिस्टर मैप इम्पोर्टर द्वारा लिखा गया।</translation>
    </message>
    <message>
        <source>Signal map</source>
        <translation type="vanished">सिग्नल मैप</translation>
    </message>
    <message>
        <source>CAN messages with their signal bit layouts and scaling. Written by the DBC importer.</source>
        <translation type="vanished">CAN संदेश अपने सिग्नल बिट लेआउट और स्केलिंग के साथ। DBC इम्पोर्टर द्वारा लिखा गया।</translation>
    </message>
</context>
<context>
    <name>Network</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="78"/>
        <source>Socket Type</source>
        <translation>सॉकेट प्रकार</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="99"/>
        <source>Local Port</source>
        <translation>लोकल पोर्ट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="106"/>
        <source>Type 0 for automatic port</source>
        <translation>ऑटोमैटिक पोर्ट के लिए 0 टाइप करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="132"/>
        <source>Remote Address</source>
        <translation>रिमोट पता</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="156"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="189"/>
        <source>Remote Port</source>
        <translation>रिमोट पोर्ट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="219"/>
        <source>Multicast</source>
        <translation>मल्टीकास्ट</translation>
    </message>
</context>
<context>
    <name>NotificationLog</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="162"/>
        <source>Filter by channel…</source>
        <translation>चैनल द्वारा फ़िल्टर करें…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="187"/>
        <source>Clear all notifications</source>
        <translation>सभी नोटिफ़िकेशन साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="271"/>
        <source>(no title)</source>
        <translation>(कोई शीर्षक नहीं)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="329"/>
        <source>No notifications yet</source>
        <translation>अभी तक कोई नोटिफ़िकेशन नहीं</translation>
    </message>
    <message>
        <source>Dataset transforms and output widget scripts can post events here via notifyInfo / notifyWarning / notifyCritical.</source>
        <translation type="vanished">डेटासेट ट्रांसफ़ॉर्म और आउटपुट विजेट स्क्रिप्ट notifyInfo / notifyWarning / notifyCritical के माध्यम से यहाँ इवेंट पोस्ट कर सकते हैं।</translation>
    </message>
</context>
<context>
    <name>OnlineIconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="42"/>
        <source>Search Online Icons</source>
        <translation>ऑनलाइन आइकन खोजें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="72"/>
        <source>Download failed: %1</source>
        <translation>डाउनलोड विफल: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="97"/>
        <source>Search icons (e.g. temperature, arrow, play)…</source>
        <translation>आइकन खोजें (जैसे temperature, arrow, play)…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="109"/>
        <source>Search</source>
        <translation>खोजें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="148"/>
        <source>Search for icons above to get started</source>
        <translation>शुरू करने के लिए ऊपर आइकन खोजें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="249"/>
        <source>OK</source>
        <translation>ठीक है</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="259"/>
        <source>Cancel</source>
        <translation>रद्द करें</translation>
    </message>
</context>
<context>
    <name>OutputWidgetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="91"/>
        <source>Output widgets require a Pro license.</source>
        <translation>Output widgets के लिए Pro लाइसेंस आवश्यक है।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="125"/>
        <source>Button</source>
        <translation>बटन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="129"/>
        <source>Send a command on click</source>
        <translation>क्लिक पर कमांड भेजें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="134"/>
        <source>Slider</source>
        <translation>स्लाइडर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="138"/>
        <source>Send scaled numeric values</source>
        <translation>स्केल किए गए संख्यात्मक मान भेजें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="143"/>
        <source>Toggle</source>
        <translation>टॉगल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="147"/>
        <source>Send on/off commands</source>
        <translation>ऑन/ऑफ कमांड भेजें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="152"/>
        <source>Text Field</source>
        <translation>टेक्स्ट फ़ील्ड</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="156"/>
        <source>Type and send arbitrary commands</source>
        <translation>मनमाने कमांड टाइप करें और भेजें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="160"/>
        <source>Knob</source>
        <translation>नॉब</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="165"/>
        <source>Rotary input for setpoints</source>
        <translation>सेटपॉइंट के लिए रोटरी इनपुट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="93"/>
        <source>You can configure output widgets, but they only appear on the dashboard with a Pro license.</source>
        <translation>आप आउटपुट विजेट कॉन्फ़िगर कर सकते हैं, लेकिन वे केवल Pro लाइसेंस के साथ डैशबोर्ड पर दिखाई देते हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="182"/>
        <source>Duplicate</source>
        <translation>डुप्लिकेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="185"/>
        <source>Duplicate this output widget</source>
        <translation>इस आउटपुट विजेट को डुप्लिकेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="195"/>
        <source>Delete</source>
        <translation>डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="197"/>
        <source>Delete this output widget</source>
        <translation>इस आउटपुट विजेट को डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="274"/>
        <source>Transmit Function</source>
        <translation>ट्रांसमिट फ़ंक्शन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="284"/>
        <source>Import</source>
        <translation>इम्पोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="290"/>
        <source>Import transmit function from a .js file</source>
        <translation>.js फ़ाइल से ट्रांसमिट फ़ंक्शन इम्पोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="297"/>
        <source>Template</source>
        <translation>टेम्पलेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="301"/>
        <source>Select a pre-built transmit function template</source>
        <translation>पहले से बना ट्रांसमिट फ़ंक्शन टेम्पलेट चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="306"/>
        <source>Test</source>
        <translation>परीक्षण करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="312"/>
        <source>Test the transmit function with sample input</source>
        <translation>सैंपल इनपुट के साथ ट्रांसमिट फ़ंक्शन का परीक्षण करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="353"/>
        <source>Undo</source>
        <translation>पूर्ववत करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="359"/>
        <source>Redo</source>
        <translation>पुनः करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="367"/>
        <source>Cut</source>
        <translation>काटें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="372"/>
        <source>Copy</source>
        <translation>कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="377"/>
        <source>Paste</source>
        <translation>पेस्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="384"/>
        <source>Select All</source>
        <translation>सभी चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="391"/>
        <source>Format Document</source>
        <translation>दस्तावेज़ फ़ॉर्मेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="396"/>
        <source>Format Selection</source>
        <translation>चयन फ़ॉर्मेट करें</translation>
    </message>
</context>
<context>
    <name>Painter</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Painter.qml" line="56"/>
        <source>Painter Widget Error</source>
        <translation>पेंटर विजेट एरर</translation>
    </message>
</context>
<context>
    <name>PainterCodeDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="30"/>
        <source>Painter Widget Code Editor</source>
        <translation>पेंटर विजेट कोड एडिटर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="76"/>
        <source>paint(ctx, w, h)</source>
        <translation>paint(ctx, w, h)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="86"/>
        <source>Import</source>
        <translation>इम्पोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="92"/>
        <source>Import painter code from a .js file</source>
        <translation>.js फ़ाइल से पेंटर कोड इम्पोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="99"/>
        <source>Template</source>
        <translation>टेम्पलेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="103"/>
        <source>Select a built-in painter template</source>
        <translation>बिल्ट-इन पेंटर टेम्पलेट चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="108"/>
        <source>Format</source>
        <translation>फ़ॉर्मेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="113"/>
        <source>Reformat the painter code</source>
        <translation>पेंटर कोड को फिर से फ़ॉर्मेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="119"/>
        <source>Test</source>
        <translation>परीक्षण करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="124"/>
        <source>Open a live preview with simulated dataset values</source>
        <translation>सिम्युलेटेड डेटासेट मानों के साथ लाइव पूर्वावलोकन खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="127"/>
        <source>Preview</source>
        <translation>पूर्वावलोकन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="182"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="191"/>
        <source>Cut</source>
        <translation>कट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="192"/>
        <source>Copy</source>
        <translation>कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="193"/>
        <source>Paste</source>
        <translation>पेस्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="194"/>
        <source>Select All</source>
        <translation>सभी चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="196"/>
        <source>Undo</source>
        <translation>पूर्ववत करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="197"/>
        <source>Redo</source>
        <translation>पुनः करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="199"/>
        <source>Format Document</source>
        <translation>दस्तावेज़ फ़ॉर्मेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="200"/>
        <source>Format Selection</source>
        <translation>चयन फ़ॉर्मेट करें</translation>
    </message>
</context>
<context>
    <name>PainterTestDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="28"/>
        <source>Painter Live Preview</source>
        <translation>पेंटर लाइव पूर्वावलोकन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="32"/>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="37"/>
        <source>Preview</source>
        <translation>पूर्वावलोकन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="113"/>
        <source>Simulated datasets</source>
        <translation>सिम्युलेटेड डेटासेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="180"/>
        <source>Runtime OK</source>
        <translation>रनटाइम OK</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="181"/>
        <source>Awaiting first frame...</source>
        <translation>पहले फ़्रेम की प्रतीक्षा में...</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="194"/>
        <source>Console</source>
        <translation>कंसोल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="236"/>
        <source>Clear console</source>
        <translation>कंसोल साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="245"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>Plot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">इंटरपोलेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="294"/>
        <source>Interpolation: %1</source>
        <translation>इंटरपोलेशन: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="307"/>
        <source>Show Area Under Plot</source>
        <translation>प्लॉट के नीचे क्षेत्र दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="326"/>
        <source>Show X Axis Label</source>
        <translation>X अक्ष लेबल दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="337"/>
        <source>Show Y Axis Label</source>
        <translation>Y अक्ष लेबल दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="349"/>
        <source>Show Crosshair</source>
        <translation>क्रॉसहेयर दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Pause</source>
        <translation>रोकें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Resume</source>
        <translation>फिर से शुरू करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="373"/>
        <source>Sweep / Trigger Mode</source>
        <translation>स्वीप / ट्रिगर मोड</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="385"/>
        <source>Trigger Settings</source>
        <translation>ट्रिगर सेटिंग्स</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="409"/>
        <source>Reset View</source>
        <translation>व्यू रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="415"/>
        <source>Axis Range Settings</source>
        <translation>अक्ष रेंज सेटिंग्स</translation>
    </message>
</context>
<context>
    <name>Plot3D</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="202"/>
        <source>Interpolate</source>
        <translation>इंटरपोलेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="220"/>
        <source>Orbit Navigation</source>
        <translation>ऑर्बिट नेविगेशन</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="230"/>
        <source>Pan Navigation</source>
        <translation>पैन नेविगेशन</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="241"/>
        <source>Orthogonal View</source>
        <translation>ऑर्थोगोनल व्यू</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="247"/>
        <source>Top View</source>
        <translation>टॉप व्यू</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="253"/>
        <source>Left View</source>
        <translation>बायां दृश्य</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="259"/>
        <source>Front View</source>
        <translation>सामने का दृश्य</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="276"/>
        <source>Auto Center</source>
        <translation>स्वतः केंद्रित करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="292"/>
        <source>Anaglyph 3D</source>
        <translation>एनाग्लिफ 3D</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="306"/>
        <source>Invert Eye Positions</source>
        <translation>नेत्र स्थिति उलटें</translation>
    </message>
</context>
<context>
    <name>PlotCommon</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="71"/>
        <source>None</source>
        <translation>कोई नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="74"/>
        <source>ZOH</source>
        <translation>ZOH</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="77"/>
        <source>Stem</source>
        <translation>Stem</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="79"/>
        <source>Linear</source>
        <translation>Linear</translation>
    </message>
</context>
<context>
    <name>PlotWidget</name>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1392"/>
        <source>Time</source>
        <translation>समय</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1415"/>
        <source>ΔX: %1  ΔY: %2 — Drag to move, right-click to clear</source>
        <translation>ΔX: %1  ΔY: %2 — खींचने के लिए ड्रैग करें, हटाने के लिए राइट-क्लिक करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1418"/>
        <source>Click to place cursor</source>
        <translation>कर्सर रखने के लिए क्लिक करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1420"/>
        <source>Click to place second cursor — Drag to move</source>
        <translation>दूसरा कर्सर रखने के लिए क्लिक करें — खींचने के लिए ड्रैग करें</translation>
    </message>
</context>
<context>
    <name>ProNotice</name>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="119"/>
        <source>Visit Website</source>
        <translation>वेबसाइट पर जाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="127"/>
        <source>Buy License</source>
        <translation>लाइसेंस खरीदें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="140"/>
        <source>Activate</source>
        <translation>सक्रिय करें</translation>
    </message>
</context>
<context>
    <name>ProUpgradeNotice</name>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="26"/>
        <source>Assistant — Pro feature</source>
        <translation>असिस्टेंट — Pro फीचर</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="44"/>
        <source>The Assistant is a Serial Studio Pro feature. Activate your license to unlock it.</source>
        <translation>असिस्टेंट Serial Studio Pro का फीचर है। इसे अनलॉक करने के लिए अपना लाइसेंस एक्टिवेट करें।</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="52"/>
        <source>Activate</source>
        <translation>एक्टिवेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="66"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>Process</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="69"/>
        <source>Mode</source>
        <translation>मोड</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Launch Process</source>
        <translation>प्रोसेस लॉन्च करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Named Pipe</source>
        <translation>नेम्ड पाइप</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="101"/>
        <source>Executable</source>
        <translation>एक्ज़ीक्यूटेबल</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="116"/>
        <source>/path/to/executable</source>
        <translation>/path/to/executable</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="209"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="257"/>
        <source>Browse</source>
        <translation>ब्राउज़ करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="145"/>
        <source>Arguments</source>
        <translation>आर्ग्युमेंट्स</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="156"/>
        <source>--arg1 value1 --arg2 value2</source>
        <translation>--arg1 value1 --arg2 value2</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="177"/>
        <source>Working Dir</source>
        <translation>वर्किंग डायरेक्टरी</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="192"/>
        <source>(optional) /working/directory</source>
        <translation>(वैकल्पिक) /working/directory</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="223"/>
        <source>Pipe Path</source>
        <translation>पाइप पाथ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="273"/>
        <source>Pick Running Process…</source>
        <translation>चल रहा प्रोसेस चुनें…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="311"/>
        <source>Launch a child process and capture its stdout, or connect to a named pipe written by an existing process.</source>
        <translation>एक चाइल्ड प्रोसेस लॉन्च करें और उसका stdout कैप्चर करें, या किसी मौजूदा प्रोसेस द्वारा लिखे गए नेम्ड पाइप से कनेक्ट करें।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="319"/>
        <source>Learn about named pipes</source>
        <translation>नेम्ड पाइप के बारे में जानें</translation>
    </message>
</context>
<context>
    <name>ProcessPicker</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="60"/>
        <source>Select Running Process</source>
        <translation>चल रहा प्रोसेस चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="211"/>
        <source>Select a running process to derive a named-pipe path suggestion.</source>
        <translation>नेम्ड-पाइप पाथ सुझाव प्राप्त करने के लिए एक चल रहा प्रोसेस चुनें।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="217"/>
        <source>Filter Processes</source>
        <translation>प्रोसेस फ़िल्टर करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="231"/>
        <source>Type to filter by name…</source>
        <translation>नाम से फ़िल्टर करने के लिए टाइप करें…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="235"/>
        <source>Refresh</source>
        <translation>रिफ्रेश करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="243"/>
        <source>Running Processes</source>
        <translation>चल रही प्रोसेस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="281"/>
        <source>Process</source>
        <translation>प्रोसेस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="288"/>
        <source>PID</source>
        <translation>PID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="383"/>
        <source>No processes match the filter.</source>
        <translation>फ़िल्टर से कोई प्रोसेस मेल नहीं खाती।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="384"/>
        <source>No running processes found.
Click Refresh to update the list.</source>
        <translation>कोई चल रही प्रोसेस नहीं मिली।
सूची अपडेट करने के लिए रिफ़्रेश पर क्लिक करें।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="400"/>
        <source>%1 process(es)</source>
        <translation>%1 प्रोसेस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="404"/>
        <source>Select</source>
        <translation>चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="410"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>ProjectEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="43"/>
        <source>modified</source>
        <translation>संशोधित</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="362"/>
        <source>This project is password protected</source>
        <translation>यह प्रोजेक्ट पासवर्ड से सुरक्षित है</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="363"/>
        <source>Editing is available in Project mode</source>
        <translation>प्रोजेक्ट मोड में संपादन उपलब्ध है</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="374"/>
        <source>Enter the password to make changes, or open a different project.</source>
        <translation>परिवर्तन करने के लिए पासवर्ड दर्ज करें, या कोई अन्य प्रोजेक्ट खोलें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="375"/>
        <source>Switch to Project mode to load and edit a project.</source>
        <translation>प्रोजेक्ट लोड और संपादित करने के लिए प्रोजेक्ट मोड पर स्विच करें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="397"/>
        <source>Unlock</source>
        <translation>अनलॉक करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="398"/>
        <source>Switch to Project Mode</source>
        <translation>प्रोजेक्ट मोड पर स्विच करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="417"/>
        <source>Open Other Project</source>
        <translation>अन्य प्रोजेक्ट खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="418"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="434"/>
        <source>Create New Project</source>
        <translation>नया प्रोजेक्ट बनाएँ</translation>
    </message>
</context>
<context>
    <name>ProjectStructure</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="32"/>
        <source>Project Structure</source>
        <translation>प्रोजेक्ट संरचना</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="71"/>
        <source>Search</source>
        <translation>खोजें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="169"/>
        <source>Move Up</source>
        <translation>ऊपर ले जाएं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="174"/>
        <source>Move Down</source>
        <translation>नीचे ले जाएं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="185"/>
        <source>Duplicate Selected (%1)</source>
        <translation>चयनित को डुप्लिकेट करें (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="186"/>
        <source>Duplicate</source>
        <translation>डुप्लिकेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="199"/>
        <source>Delete Selected (%1)</source>
        <translation>चयनित को डिलीट करें (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="200"/>
        <source>Delete</source>
        <translation>डिलीट करें</translation>
    </message>
</context>
<context>
    <name>ProjectToolbar</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="142"/>
        <source>New</source>
        <translation>नया</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="145"/>
        <source>Create a new JSON project</source>
        <translation>नया JSON प्रोजेक्ट बनाएँ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="158"/>
        <source>Open</source>
        <translation>खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="162"/>
        <source>Open an existing JSON project</source>
        <translation>मौजूदा JSON प्रोजेक्ट खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="168"/>
        <source>Save</source>
        <translation>सहेजें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="175"/>
        <source>Save the current project</source>
        <translation>वर्तमान प्रोजेक्ट सहेजें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="181"/>
        <source>Save As</source>
        <translation>इस रूप में सहेजें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="188"/>
        <source>Save the current project under a new name</source>
        <translation>वर्तमान प्रोजेक्ट को नए नाम से सहेजें</translation>
    </message>
    <message>
        <source>Unlock</source>
        <translation type="vanished">अनलॉक करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="230"/>
        <source>Lock</source>
        <translation>लॉक करें</translation>
    </message>
    <message>
        <source>Unlock the Project Editor with the project password</source>
        <translation type="vanished">प्रोजेक्ट पासवर्ड से प्रोजेक्ट एडिटर अनलॉक करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="200"/>
        <source>Import</source>
        <translation>इम्पोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="204"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="208"/>
        <source>Generate a project from a Protocol Buffers (.proto) schema</source>
        <translation>Protocol Buffers (.proto) स्कीमा से प्रोजेक्ट जेनरेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="234"/>
        <source>Set a password and lock the Project Editor</source>
        <translation>पासवर्ड सेट करें और प्रोजेक्ट एडिटर लॉक करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="245"/>
        <source>Add Device</source>
        <translation>डिवाइस जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="249"/>
        <source>Add a new data source (device) to the project</source>
        <translation>प्रोजेक्ट में नया डेटा स्रोत (डिवाइस) जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="272"/>
        <source>Action</source>
        <translation>एक्शन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="275"/>
        <source>Add a new action to the project</source>
        <translation>प्रोजेक्ट में नया एक्शन जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="260"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="264"/>
        <source>Output</source>
        <translation>आउटपुट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="218"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="222"/>
        <source>Restore</source>
        <translation>पुनर्स्थापित करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="226"/>
        <source>Restore a recent automatic snapshot of the current project</source>
        <translation>वर्तमान प्रोजेक्ट के हालिया स्वचालित स्नैपशॉट को पुनर्स्थापित करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="267"/>
        <source>Add a new output control panel with a button</source>
        <translation>बटन के साथ नया आउटपुट कंट्रोल पैनल जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="288"/>
        <source>Slider</source>
        <translation>स्लाइडर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="291"/>
        <source>Add an output slider control</source>
        <translation>आउटपुट स्लाइडर कंट्रोल जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="298"/>
        <source>Toggle</source>
        <translation>टॉगल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="301"/>
        <source>Add an output toggle control</source>
        <translation>आउटपुट टॉगल कंट्रोल जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="308"/>
        <source>Knob</source>
        <translation>नॉब</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="311"/>
        <source>Add an output knob control</source>
        <translation>आउटपुट नॉब कंट्रोल जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="319"/>
        <source>Text Field</source>
        <translation>टेक्स्ट फ़ील्ड</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="321"/>
        <source>Add an output text field control</source>
        <translation>आउटपुट टेक्स्ट फ़ील्ड कंट्रोल जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="328"/>
        <source>Button</source>
        <translation>बटन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="331"/>
        <source>Add an output button control</source>
        <translation>आउटपुट बटन कंट्रोल जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="344"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="348"/>
        <source>Dataset</source>
        <translation>डेटासेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="350"/>
        <source>Add a generic dataset</source>
        <translation>सामान्य डेटासेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="364"/>
        <source>Plot</source>
        <translation>प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="367"/>
        <source>Add a 2D plot dataset</source>
        <translation>2D प्लॉट डेटासेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="374"/>
        <source>FFT Plot</source>
        <translation>FFT प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="377"/>
        <source>Add a Fast Fourier Transform plot</source>
        <translation>Fast Fourier Transform प्लॉट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="384"/>
        <source>Gauge</source>
        <translation>गेज</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="387"/>
        <source>Add a gauge widget for numeric data</source>
        <translation>संख्यात्मक डेटा के लिए गेज विजेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="395"/>
        <source>Level Indicator</source>
        <translation>स्तर संकेतक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="397"/>
        <source>Add a vertical bar level indicator</source>
        <translation>लंबवत बार स्तर संकेतक जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="404"/>
        <source>Compass</source>
        <translation>कंपास</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="407"/>
        <source>Add a compass widget for directional data</source>
        <translation>दिशात्मक डेटा के लिए कंपास विजेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="415"/>
        <source>LED Indicator</source>
        <translation>LED संकेतक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="417"/>
        <source>Add an LED-style status indicator</source>
        <translation>LED-शैली स्थिति संकेतक जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="430"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="434"/>
        <source>Group</source>
        <translation>समूह</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="436"/>
        <source>Add a dataset container group</source>
        <translation>एक डेटासेट कंटेनर ग्रुप जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="438"/>
        <source>Dataset Container</source>
        <translation>डेटासेट कंटेनर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="442"/>
        <source>Image</source>
        <translation>इमेज</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="444"/>
        <source>Add an image/video stream viewer</source>
        <translation>एक इमेज/वीडियो स्ट्रीम व्यूअर जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="446"/>
        <source>Image View</source>
        <translation>इमेज व्यू</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="450"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="454"/>
        <source>Web View</source>
        <translation>वेब व्यू</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="452"/>
        <source>Add an web viewer</source>
        <translation>वेब व्यूअर जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="462"/>
        <source>Painter</source>
        <translation>पेंटर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="466"/>
        <source>Add a custom JavaScript-rendered painter widget</source>
        <translation>कस्टम JavaScript-रेंडर किया गया पेंटर विजेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="467"/>
        <source>Painter widgets require a Pro license — adding one will fall back to a data grid</source>
        <translation>पेंटर विजेट के लिए Pro लाइसेंस आवश्यक है — एक जोड़ने पर डेटा ग्रिड में बदल जाएगा</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="468"/>
        <source>Painter Widget</source>
        <translation>पेंटर विजेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="480"/>
        <source>Table</source>
        <translation>टेबल</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="483"/>
        <source>Add a data table view</source>
        <translation>एक डेटा टेबल व्यू जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="485"/>
        <source>Data Grid</source>
        <translation>डेटा ग्रिड</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="491"/>
        <source>Multi-Plot</source>
        <translation>मल्टी-प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="493"/>
        <source>Add a 2D plot with multiple signals</source>
        <translation>एकाधिक सिग्नल के साथ 2D प्लॉट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="495"/>
        <source>Multiple Plot</source>
        <translation>मल्टीपल प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="500"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="505"/>
        <source>3D Plot</source>
        <translation>3D प्लॉट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="503"/>
        <source>Add a 3D plot visualization</source>
        <translation>3D प्लॉट विज़ुअलाइज़ेशन जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="511"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="515"/>
        <source>Accelerometer</source>
        <translation>एक्सेलेरोमीटर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="513"/>
        <source>Add a group for 3-axis accelerometer data</source>
        <translation>3-अक्ष एक्सेलेरोमीटर डेटा के लिए ग्रुप जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="521"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="524"/>
        <source>Gyroscope</source>
        <translation>जायरोस्कोप</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="525"/>
        <source>Add a group for 3-axis gyroscope data</source>
        <translation>3-अक्ष जायरोस्कोप डेटा के लिए ग्रुप जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="530"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="535"/>
        <source>GPS Map</source>
        <translation>GPS मैप</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="533"/>
        <source>Add a map widget for GPS data</source>
        <translation>GPS डेटा के लिए मैप विजेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="547"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="551"/>
        <source>Assistant</source>
        <translation>असिस्टेंट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="554"/>
        <source>Open the Assistant</source>
        <translation>असिस्टेंट खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="560"/>
        <source>Help Center</source>
        <translation>सहायता केंद्र</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="564"/>
        <source>Open the Project Editor documentation</source>
        <translation>Project Editor दस्तावेज़ खोलें</translation>
    </message>
</context>
<context>
    <name>ProjectView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="34"/>
        <source>Project Summary</source>
        <translation>प्रोजेक्ट सारांश</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="81"/>
        <source>Pro features detected in this project.</source>
        <translation>इस प्रोजेक्ट में Pro सुविधाएँ पाई गईं।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="83"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>फ़ॉलबैक विजेट उपयोग में हैं। पूर्ण कार्यक्षमता अनलॉक करने के लिए लाइसेंस खरीदें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="118"/>
        <source>Project Title:</source>
        <translation>प्रोजेक्ट शीर्षक:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="129"/>
        <source>Untitled Project</source>
        <translation>अनाम प्रोजेक्ट</translation>
    </message>
    <message>
        <source>Points:</source>
        <translation type="vanished">बिंदु:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="149"/>
        <source>Time Range:</source>
        <translation>समय रेंज:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="198"/>
        <source>Point Count:</source>
        <translation>पॉइंट काउंट:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="252"/>
        <source>Source</source>
        <translation>स्रोत</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="253"/>
        <source>Sources</source>
        <translation>स्रोत</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="258"/>
        <source>Group</source>
        <translation>समूह</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="259"/>
        <source>Groups</source>
        <translation>ग्रुप्स</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="264"/>
        <source>Dataset</source>
        <translation>डेटासेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="265"/>
        <source>Datasets</source>
        <translation>डेटासेट्स</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="270"/>
        <source>Action</source>
        <translation>एक्शन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="271"/>
        <source>Actions</source>
        <translation>एक्शन्स</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="359"/>
        <source>Double-click a block to edit it. Right-click anywhere to add a group, dataset, action, data table, or device.</source>
        <translation>संपादित करने के लिए किसी ब्लॉक पर डबल-क्लिक करें। समूह, डेटासेट, क्रिया, डेटा तालिका, या डिवाइस जोड़ने के लिए कहीं भी राइट-क्लिक करें।</translation>
    </message>
</context>
<context>
    <name>ProtoPreviewDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="41"/>
        <source>Protocol Buffers File Preview</source>
        <translation>Protocol Buffers फ़ाइल पूर्वावलोकन</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="165"/>
        <source>Proto File: %1</source>
        <translation>Proto फ़ाइल: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="173"/>
        <source>Browse the messages below, then create the project. Every message becomes a dashboard group; matching-type channel blocks get a MultiPlot and mixed-type messages get a DataGrid.</source>
        <translation>नीचे दिए गए संदेशों को ब्राउज़ करें, फिर प्रोजेक्ट बनाएं। प्रत्येक संदेश एक डैशबोर्ड समूह बन जाता है; समान-प्रकार के चैनल ब्लॉक को MultiPlot मिलता है और मिश्रित-प्रकार के संदेशों को DataGrid मिलता है।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="183"/>
        <source>Show fields for</source>
        <translation>इसके लिए फ़ील्ड दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="209"/>
        <source>Fields</source>
        <translation>फ़ील्ड</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="243"/>
        <source>Tag</source>
        <translation>टैग</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="253"/>
        <source>Field Name</source>
        <translation>फ़ील्ड नाम</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="258"/>
        <source>Type</source>
        <translation>प्रकार</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="345"/>
        <source>No fields in the selected message.</source>
        <translation>चयनित संदेश में कोई फ़ील्ड नहीं है।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="363"/>
        <source>Total: %1 messages, %2 fields</source>
        <translation>कुल: %1 संदेश, %2 फ़ील्ड</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="370"/>
        <source>Cancel</source>
        <translation>रद्द करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="381"/>
        <source>Create Project</source>
        <translation>प्रोजेक्ट बनाएँ</translation>
    </message>
</context>
<context>
    <name>Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="71"/>
        <source>No error</source>
        <translation>कोई त्रुटि नहीं</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="73"/>
        <source>The broker rejected the connection due to an unsupported protocol version. Match the broker's MQTT version and try again.</source>
        <translation>Broker ने असमर्थित प्रोटोकॉल संस्करण के कारण कनेक्शन अस्वीकार कर दिया। Broker के MQTT संस्करण से मिलान करें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="76"/>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Regenerate it and try again.</source>
        <translation>Broker ने client ID अस्वीकार कर दिया। यह गलत स्वरूप में हो सकता है, बहुत लंबा हो सकता है, या पहले से उपयोग में हो सकता है। इसे पुनः उत्पन्न करें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="79"/>
        <source>The network reached the broker, but the broker is currently unavailable. Verify its status and try again later.</source>
        <translation>नेटवर्क ब्रोकर तक पहुँच गया, लेकिन ब्रोकर वर्तमान में अनुपलब्ध है। इसकी स्थिति सत्यापित करें और बाद में पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="82"/>
        <source>The username or password is incorrect or malformed. Double-check the credentials and try again.</source>
        <translation>उपयोगकर्ता नाम या पासवर्ड गलत या विकृत है। साख दोबारा जाँचें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="85"/>
        <source>The broker denied the connection due to insufficient permissions. Verify that the account has the required ACLs.</source>
        <translation>ब्रोकर ने अपर्याप्त अनुमतियों के कारण कनेक्शन अस्वीकार कर दिया। सत्यापित करें कि खाते के पास आवश्यक ACL हैं।</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="88"/>
        <source>A network or transport-layer issue prevented the connection. Check connectivity, ports, and TLS configuration.</source>
        <translation>नेटवर्क या ट्रांसपोर्ट-लेयर समस्या ने कनेक्शन रोक दिया। कनेक्टिविटी, पोर्ट और TLS कॉन्फ़िगरेशन जाँचें।</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="91"/>
        <source>The client detected an MQTT protocol violation and closed the connection. Verify broker and client compatibility.</source>
        <translation>क्लाइंट ने MQTT प्रोटोकॉल उल्लंघन पाया और कनेक्शन बंद कर दिया। ब्रोकर और क्लाइंट संगतता सत्यापित करें।</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="94"/>
        <source>An unexpected error occurred. Check the broker logs and the application console for details.</source>
        <translation>एक अप्रत्याशित त्रुटि हुई। विवरण के लिए ब्रोकर लॉग और एप्लिकेशन कंसोल देखें।</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="97"/>
        <source>An MQTT 5 protocol-level error occurred. Inspect the broker's reason code for details.</source>
        <translation>MQTT 5 प्रोटोकॉल-स्तर त्रुटि हुई। विवरण के लिए ब्रोकर का reason code देखें।</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="101"/>
        <source>Unspecified MQTT error (code %1).</source>
        <translation>अनिर्दिष्ट MQTT त्रुटि (कोड %1)।</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../src/Misc/Translator.cpp" line="231"/>
        <source>Failed to load welcome text :(</source>
        <translation>स्वागत टेक्स्ट लोड करने में विफल :(</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="251"/>
        <source>Network error</source>
        <translation>नेटवर्क त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="254"/>
        <location filename="../../src/Licensing/Trial.cpp" line="270"/>
        <location filename="../../src/Licensing/Trial.cpp" line="302"/>
        <source>Trial Activation Error</source>
        <translation>ट्रायल एक्टिवेशन त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="267"/>
        <source>Invalid server response</source>
        <translation>अमान्य सर्वर रिस्पॉन्स</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="268"/>
        <source>The server returned malformed data: %1</source>
        <translation>सर्वर ने विकृत डेटा भेजा: %1</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="299"/>
        <source>Unexpected server response</source>
        <translation>अप्रत्याशित सर्वर रिस्पॉन्स</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="300"/>
        <source>The server response is missing required fields.</source>
        <translation>सर्वर प्रतिक्रिया में आवश्यक फ़ील्ड गायब हैं।</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="162"/>
        <source>Console Output File Error</source>
        <translation>Console आउटपुट फ़ाइल त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="163"/>
        <source>Cannot open file for writing!</source>
        <translation>लिखने के लिए फ़ाइल नहीं खोली जा सकती!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1311"/>
        <source>Invalid Bluetooth adapter!</source>
        <translation>अमान्य Bluetooth एडाप्टर!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1314"/>
        <source>Unsuported platform or operating system</source>
        <translation>असमर्थित प्लेटफ़ॉर्म या ऑपरेटिंग सिस्टम</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1317"/>
        <source>Unsupported discovery method</source>
        <translation>असमर्थित खोज विधि</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1320"/>
        <source>General I/O error</source>
        <translation>सामान्य I/O त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="252"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="273"/>
        <source>Frame Parser Disabled</source>
        <translation>Frame Parser अक्षम</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="274"/>
        <source>The Lua frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>स्रोत %1 के लिए Lua frame parser लगातार %2 फ़्रेम में टाइम आउट हो गया है और Serial Studio को उत्तरदायी रखने के लिए अक्षम कर दिया गया है।

संभावित कारण: स्क्रिप्ट बॉडी में अनंत लूप या अत्यधिक धीमा ऑपरेशन। स्क्रिप्ट को ठीक करें और पार्सिंग पुनः सक्षम करने के लिए प्रोजेक्ट को फिर से लोड करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="322"/>
        <source>Lua Syntax Error</source>
        <translation>Lua सिंटैक्स त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="323"/>
        <source>The parser code contains an error:

%1</source>
        <translation>पार्सर कोड में त्रुटि है:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="371"/>
        <source>Lua Runtime Error</source>
        <translation>Lua रनटाइम त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="372"/>
        <source>The parser code triggered an error:

%1</source>
        <translation>पार्सर कोड ने एक त्रुटि उत्पन्न की:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="478"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="393"/>
        <source>Missing Parse Function</source>
        <translation>Parse फ़ंक्शन अनुपलब्ध</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="394"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) ... end</source>
        <translation>स्क्रिप्ट में 'parse' फ़ंक्शन परिभाषित नहीं है।

कृपया सुनिश्चित करें कि आपके कोड में शामिल है:
function parse(frame) ... end</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="530"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="456"/>
        <source>Parse Function Runtime Error</source>
        <translation>Parse फ़ंक्शन रनटाइम त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="457"/>
        <source>The parse function contains an error:

%1

Please fix the error in the function body.</source>
        <translation>parse फ़ंक्शन में एक त्रुटि है:

%1

कृपया फ़ंक्शन बॉडी में त्रुटि ठीक करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="253"/>
        <source>The JavaScript frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>स्रोत %1 के लिए JavaScript frame parser लगातार %2 फ़्रेम में टाइम आउट हो गया है और Serial Studio को उत्तरदायी रखने के लिए अक्षम कर दिया गया है।

संभावित कारण: स्क्रिप्ट बॉडी में अनंत लूप या अत्यधिक धीमा ऑपरेशन। स्क्रिप्ट को ठीक करें और पार्सिंग पुनः सक्षम करने के लिए प्रोजेक्ट को फिर से लोड करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="419"/>
        <source>JavaScript Timed Out</source>
        <translation>JavaScript टाइमआउट हो गया</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="420"/>
        <source>The parser code did not finish evaluating within %1 ms and was interrupted.

Most likely cause: an infinite loop at the top level of the script.</source>
        <translation>पार्सर कोड %1 ms के भीतर मूल्यांकन पूरा नहीं कर सका और बाधित कर दिया गया।

संभावित कारण: स्क्रिप्ट के शीर्ष स्तर पर एक अनंत लूप।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="437"/>
        <source>JavaScript Syntax Error</source>
        <translation>JavaScript सिंटैक्स त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="438"/>
        <source>The parser code contains a syntax error at line %1:

%2</source>
        <translation>पार्सर कोड में लाइन %1 पर सिंटैक्स त्रुटि है:

%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="452"/>
        <source>JavaScript Exception Occurred</source>
        <translation>JavaScript अपवाद घटित हुआ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="453"/>
        <source>The parser code triggered the following exceptions:

%1</source>
        <translation>पार्सर कोड ने निम्नलिखित अपवाद उत्पन्न किए:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="479"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) { ... }</source>
        <translation>स्क्रिप्ट में 'parse' फ़ंक्शन परिभाषित नहीं है।

कृपया सुनिश्चित करें कि आपके कोड में शामिल है:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="531"/>
        <source>The parse function contains an error at line %1:

%2

Please fix the error in the function body.</source>
        <translation>parse फ़ंक्शन में लाइन %1 पर त्रुटि है:

%2

कृपया फ़ंक्शन बॉडी में त्रुटि ठीक करें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="631"/>
        <source>Invalid Function Declaration</source>
        <translation>अमान्य फ़ंक्शन घोषणा</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="632"/>
        <source>No callable 'parse' export found.

Define one of:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</source>
        <translation>कोई कॉल करने योग्य 'parse' एक्सपोर्ट नहीं मिला।

इनमें से एक डिफाइन करें:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="648"/>
        <source>The 'parse' function must accept at least one parameter (the frame payload).</source>
        <translation>'parse' फ़ंक्शन को कम से कम एक पैरामीटर (फ्रेम पेलोड) स्वीकार करना चाहिए।</translation>
    </message>
    <message>
        <source>No valid 'parse' function declaration found.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">कोई मान्य 'parse' फ़ंक्शन घोषणा नहीं मिली।

अपेक्षित प्रारूप:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="647"/>
        <source>Invalid Function Parameter</source>
        <translation>अमान्य फ़ंक्शन पैरामीटर</translation>
    </message>
    <message>
        <source>The 'parse' function must have at least one parameter.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">'parse' फ़ंक्शन में कम से कम एक पैरामीटर होना चाहिए।

अपेक्षित प्रारूप:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="613"/>
        <source>Deprecated Function Signature</source>
        <translation>पुराना फ़ंक्शन सिग्नेचर</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="614"/>
        <source>The 'parse' function uses the old two-parameter format: parse(%1, %2)

This format is no longer supported. Please update to the new single-parameter format:
function parse(%1) { ... }

The separator parameter is no longer needed.</source>
        <translation>'parse' फ़ंक्शन पुराने दो-पैरामीटर प्रारूप का उपयोग करता है: parse(%1, %2)

यह प्रारूप अब समर्थित नहीं है। कृपया नए एकल-पैरामीटर प्रारूप में अपडेट करें:
function parse(%1) { ... }

सेपरेटर पैरामीटर की अब आवश्यकता नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="191"/>
        <source>Critical</source>
        <translation>गंभीर</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="191"/>
        <source>Warning</source>
        <translation>चेतावनी</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="577"/>
        <source>Project file not found</source>
        <translation>प्रोजेक्ट फ़ाइल नहीं मिली</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="578"/>
        <source>The project file referenced by this shortcut could not be found:

%1</source>
        <translation>इस शॉर्टकट द्वारा संदर्भित प्रोजेक्ट फ़ाइल नहीं मिली:

%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="581"/>
        <source>Would you like to delete this shortcut?</source>
        <translation>क्या आप यह शॉर्टकट डिलीट करना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="585"/>
        <source>Delete Shortcut</source>
        <translation>शॉर्टकट डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="587"/>
        <source>Quit</source>
        <translation>बंद करें</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1051"/>
        <source>Time (s)</source>
        <translation>समय (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1109"/>
        <source>Freq: %1</source>
        <translation>आवृत्ति: %1</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1112"/>
        <source>Time: −%1</source>
        <translation>समय: −%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIProvider.cpp" line="362"/>
        <source>No OpenAI API key set. Open Manage Keys to add one.</source>
        <translation>कोई OpenAI API कुंजी सेट नहीं है। एक जोड़ने के लिए कुंजी प्रबंधन खोलें।</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicProvider.cpp" line="234"/>
        <source>No Anthropic API key set. Open Manage Keys to add one.</source>
        <translation>कोई Anthropic API कुंजी सेट नहीं है। एक जोड़ने के लिए कुंजी प्रबंधन खोलें।</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiProvider.cpp" line="285"/>
        <source>No Gemini API key set. Open Manage Keys to add one.</source>
        <translation>कोई Gemini API कुंजी सेट नहीं है। एक जोड़ने के लिए कुंजी प्रबंधन खोलें।</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/LocalProvider.cpp" line="351"/>
        <source>No local model server URL configured. Open Manage Keys to set one.</source>
        <translation>कोई लोकल मॉडल सर्वर URL कॉन्फ़िगर नहीं है। एक सेट करने के लिए Keys प्रबंधित करें खोलें।</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/DeepSeekProvider.cpp" line="146"/>
        <source>No DeepSeek API key set. Open Manage Keys to add one.</source>
        <translation>कोई DeepSeek API key सेट नहीं है। एक जोड़ने के लिए Keys प्रबंधित करें खोलें।</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/MistralProvider.cpp" line="168"/>
        <source>No Mistral API key set. Open Manage Keys to add one.</source>
        <translation>कोई Mistral API कुंजी सेट नहीं है। एक जोड़ने के लिए कुंजी प्रबंधन खोलें।</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenRouterProvider.cpp" line="181"/>
        <source>No OpenRouter API key set. Open Manage Keys to add one.</source>
        <translation>कोई OpenRouter API कुंजी सेट नहीं है। एक जोड़ने के लिए कुंजी प्रबंधन खोलें।</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GroqProvider.cpp" line="152"/>
        <source>No Groq API key set. Open Manage Keys to add one.</source>
        <translation>कोई Groq API कुंजी सेट नहीं है। एक जोड़ने के लिए कुंजी प्रबंधन खोलें।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1100"/>
        <source>The frame parser is using more than %1% of CPU time.</source>
        <translation>Frame parser CPU समय का %1% से अधिक उपयोग कर रहा है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1102"/>
        <source>Serial Studio is dropping frames to keep the application responsive. Please simplify or optimize the frame parser script to reduce its workload.</source>
        <translation>Serial Studio एप्लिकेशन को उत्तरदायी रखने के लिए फ़्रेम छोड़ रहा है। कृपया frame parser स्क्रिप्ट को सरल या अनुकूलित करें ताकि इसका कार्यभार कम हो।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="386"/>
        <source>Expected %1, got '%2'</source>
        <translation>अपेक्षित %1, प्राप्त '%2'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="435"/>
        <source>Expected enum name after 'enum'</source>
        <translation>'enum' के बाद enum नाम अपेक्षित</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="449"/>
        <source>Expected oneof name</source>
        <translation>oneof नाम अपेक्षित</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="476"/>
        <source>Field tag '%1' out of range (1..%2)</source>
        <translation>फ़ील्ड टैग '%1' रेंज से बाहर (1..%2)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="494"/>
        <source>Expected key type in map&lt;&gt;</source>
        <translation>map&lt;&gt; में key प्रकार अपेक्षित</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="502"/>
        <source>Expected value type in map&lt;&gt;</source>
        <translation>map&lt;&gt; में value प्रकार अपेक्षित</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="510"/>
        <source>Expected map field name</source>
        <translation>map फ़ील्ड नाम अपेक्षित</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="522"/>
        <source>Expected map field tag</source>
        <translation>map फ़ील्ड टैग अपेक्षित</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="554"/>
        <source>Expected field type, got '%1'</source>
        <translation>फ़ील्ड प्रकार अपेक्षित, '%1' मिला</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="573"/>
        <source>Expected field name after type</source>
        <translation>प्रकार के बाद फ़ील्ड नाम अपेक्षित</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="583"/>
        <source>Expected field tag number</source>
        <translation>फ़ील्ड टैग संख्या अपेक्षित</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="630"/>
        <source>Message nesting too deep (limit %1)</source>
        <translation>मैसेज नेस्टिंग बहुत गहरी (सीमा %1)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="635"/>
        <source>Expected message name</source>
        <translation>संदेश नाम अपेक्षित</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="717"/>
        <source>Unexpected token '%1' at file scope</source>
        <translation>फ़ाइल स्कोप में अप्रत्याशित टोकन '%1'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="763"/>
        <source>Unsupported top-level keyword '%1'</source>
        <translation>असमर्थित टॉप-लेवल कीवर्ड '%1'</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="294"/>
        <source>Automatic (Platform Default)</source>
        <translation>ऑटोमैटिक (प्लेटफ़ॉर्म डिफ़ॉल्ट)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="299"/>
        <source>Software (Fallback)</source>
        <translation>सॉफ़्टवेयर (फॉलबैक)</translation>
    </message>
    <message>
        <source>Row %1</source>
        <translation type="vanished">पंक्ति %1</translation>
    </message>
    <message>
        <source>[%1]</source>
        <translation type="vanished">[%1]</translation>
    </message>
    <message>
        <source>Frame %1</source>
        <translation type="vanished">फ्रेम %1</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">डिकोडर</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation type="vanished">पंक्तियाँ</translation>
    </message>
    <message>
        <source>%1 row(s)</source>
        <translation type="vanished">%1 पंक्ति(याँ)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="186"/>
        <source>The native parser configuration is not a valid JSON object.</source>
        <translation>नेटिव पार्सर कॉन्फ़िगरेशन एक मान्य JSON ऑब्जेक्ट नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="196"/>
        <source>Unknown native parser template: "%1".</source>
        <translation>अज्ञात नेटिव पार्सर टेम्पलेट: "%1"।</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="330"/>
        <source>Built-In Parser Error</source>
        <translation>Built-In पार्सर त्रुटि</translation>
    </message>
    <message>
        <source>Native Parser Error</source>
        <translation type="vanished">नेटिव पार्सर त्रुटि</translation>
    </message>
</context>
<context>
    <name>QuaGzipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="60"/>
        <source>QIODevice::Append is not supported for GZIP</source>
        <translation>QIODevice::Append GZIP के लिए समर्थित नहीं है</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="66"/>
        <source>Opening gzip for both reading and writing is not supported</source>
        <translation>gzip को पढ़ने और लिखने दोनों के लिए खोलना समर्थित नहीं है</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="75"/>
        <source>You can open a gzip either for reading or for writing. Which is it?</source>
        <translation>आप gzip को या तो पढ़ने के लिए या लिखने के लिए खोल सकते हैं। कौन सा है?</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="81"/>
        <source>Could not gzopen() file</source>
        <translation>फ़ाइल gzopen() नहीं की जा सकी</translation>
    </message>
</context>
<context>
    <name>QuaZIODevice</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="178"/>
        <source>QIODevice::Append is not supported for QuaZIODevice</source>
        <translation>QIODevice::Append QuaZIODevice के लिए समर्थित नहीं है</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="183"/>
        <source>QIODevice::ReadWrite is not supported for QuaZIODevice</source>
        <translation>QIODevice::ReadWrite QuaZIODevice के लिए समर्थित नहीं है</translation>
    </message>
</context>
<context>
    <name>QuaZipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quazipfile.cpp" line="251"/>
        <source>ZIP/UNZIP API error %1</source>
        <translation>ZIP/UNZIP API त्रुटि %1</translation>
    </message>
</context>
<context>
    <name>ReportOptionsDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate PDF Report</source>
        <translation>PDF रिपोर्ट जेनरेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate Report</source>
        <translation>रिपोर्ट जनरेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="61"/>
        <source>Solid</source>
        <translation>सॉलिड</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="62"/>
        <source>Dashed</source>
        <translation>डैश्ड</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="63"/>
        <source>Dotted</source>
        <translation>डॉटेड</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="80"/>
        <source>A4 (210 × 297 mm)</source>
        <translation>A4 (210 × 297 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="81"/>
        <source>A3 (297 × 420 mm)</source>
        <translation>A3 (297 × 420 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="82"/>
        <source>A2 (420 × 594 mm)</source>
        <translation>A2 (420 × 594 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="83"/>
        <source>A1 (594 × 841 mm)</source>
        <translation>A1 (594 × 841 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="84"/>
        <source>A0 (841 × 1189 mm)</source>
        <translation>A0 (841 × 1189 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="85"/>
        <source>A5 (148 × 210 mm)</source>
        <translation>A5 (148 × 210 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="86"/>
        <source>A6 (105 × 148 mm)</source>
        <translation>A6 (105 × 148 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="87"/>
        <source>B4 (250 × 353 mm)</source>
        <translation>B4 (250 × 353 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="88"/>
        <source>B5 (176 × 250 mm)</source>
        <translation>B5 (176 × 250 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="89"/>
        <source>Letter (8.5 × 11 in)</source>
        <translation>Letter (8.5 × 11 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="90"/>
        <source>Legal (8.5 × 14 in)</source>
        <translation>Legal (8.5 × 14 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="91"/>
        <source>Executive (7.25 × 10.5 in)</source>
        <translation>Executive (7.25 × 10.5 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="92"/>
        <source>Tabloid (11 × 17 in)</source>
        <translation>Tabloid (11 × 17 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="93"/>
        <source>Ledger (17 × 11 in)</source>
        <translation>Ledger (17 × 11 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="103"/>
        <source>%1 — Session Report</source>
        <translation>%1 — सेशन रिपोर्ट</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="105"/>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="280"/>
        <source>Session Report</source>
        <translation>सेशन रिपोर्ट</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="188"/>
        <source>Branding</source>
        <translation>ब्रांडिंग</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="194"/>
        <source>Page</source>
        <translation>पेज</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="200"/>
        <source>Sections</source>
        <translation>अनुभाग</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="248"/>
        <source>Identity</source>
        <translation>पहचान</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="262"/>
        <source>Company</source>
        <translation>कंपनी</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="269"/>
        <source>e.g. Acme Test Systems</source>
        <translation>उदा. Acme Test Systems</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="273"/>
        <source>Document title</source>
        <translation>दस्तावेज़ शीर्षक</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="284"/>
        <source>Author</source>
        <translation>लेखक</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="291"/>
        <source>Prepared by (optional)</source>
        <translation>तैयारकर्ता (वैकल्पिक)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="300"/>
        <source>Logo</source>
        <translation>लोगो</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="313"/>
        <source>File</source>
        <translation>फ़ाइल</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="324"/>
        <source>PNG, JPG or SVG (optional)</source>
        <translation>PNG, JPG या SVG (वैकल्पिक)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="326"/>
        <source>Browse…</source>
        <translation>ब्राउज़ करें…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="329"/>
        <source>Clear</source>
        <translation>साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="370"/>
        <source>Paper</source>
        <translation>पेपर</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="382"/>
        <source>Page size</source>
        <translation>पेज साइज़</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="510"/>
        <source>Annotate min, max, and mean values on plots</source>
        <translation>प्लॉट पर न्यूनतम, अधिकतम और औसत मान एनोटेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="558"/>
        <source>Export HTML</source>
        <translation>HTML एक्सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="498"/>
        <source>Cover page (logo, document title, test subtitle)</source>
        <translation>कवर पेज (लोगो, डॉक्यूमेंट शीर्षक, टेस्ट उपशीर्षक)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="501"/>
        <source>Test information (project, timestamps, classification and notes)</source>
        <translation>टेस्ट जानकारी (प्रोजेक्ट, टाइमस्टैम्प, वर्गीकरण और नोट्स)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="504"/>
        <source>Measurement summary (min, max, mean, std. deviation per parameter)</source>
        <translation>मापन सारांश (प्रति पैरामीटर न्यूनतम, अधिकतम, माध्य, मानक विचलन)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="507"/>
        <source>Parameter trends (time-series chart per numeric parameter)</source>
        <translation>पैरामीटर ट्रेंड (प्रति संख्यात्मक पैरामीटर टाइम-सीरीज़ चार्ट)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="397"/>
        <source>Plot appearance</source>
        <translation>प्लॉट अपीयरेंस</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="411"/>
        <source>Line width</source>
        <translation>लाइन चौड़ाई</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="443"/>
        <source>Line style</source>
        <translation>लाइन स्टाइल</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="483"/>
        <source>Include</source>
        <translation>शामिल करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="533"/>
        <source>Cancel</source>
        <translation>रद्द करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="558"/>
        <source>Export PDF</source>
        <translation>PDF एक्सपोर्ट करें</translation>
    </message>
</context>
<context>
    <name>ReportProgressDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="20"/>
        <source>Generating Report</source>
        <translation>रिपोर्ट जेनरेट हो रही है</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="69"/>
        <source>Working…</source>
        <translation>कार्य जारी है…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="86"/>
        <source>This can take a few seconds for sessions with many parameters. The window closes automatically when the report is ready.</source>
        <translation>अधिक पैरामीटर वाले सेशन के लिए इसमें कुछ सेकंड लग सकते हैं। रिपोर्ट तैयार होने पर विंडो स्वतः बंद हो जाएगी।</translation>
    </message>
</context>
<context>
    <name>RuntimeReconfigure</name>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="40"/>
        <source>Connection Lost</source>
        <translation>कनेक्शन खो गया</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="41"/>
        <source>Device Unavailable</source>
        <translation>डिवाइस उपलब्ध नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="95"/>
        <source>The connection to your device was lost.</source>
        <translation>आपके डिवाइस का कनेक्शन खो गया।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="96"/>
        <source>Serial Studio couldn't reach your device.</source>
        <translation>Serial Studio आपके डिवाइस तक नहीं पहुँच सका।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="105"/>
        <source>Check the cable, power, and that no other application has taken over the device. You can try reconnecting, switch to a different device, or quit.</source>
        <translation>केबल, पावर जाँचें और सुनिश्चित करें कि किसी अन्य एप्लिकेशन ने डिवाइस को नहीं लिया है। आप पुनः कनेक्ट करने का प्रयास कर सकते हैं, दूसरे डिवाइस पर स्विच कर सकते हैं, या बाहर निकल सकते हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="108"/>
        <source>Make sure it's plugged in, powered on, and not already in use by another app. You can try again, pick a different device, or quit.</source>
        <translation>सुनिश्चित करें कि यह प्लग इन है, चालू है, और किसी अन्य ऐप द्वारा उपयोग में नहीं है। आप पुनः प्रयास कर सकते हैं, कोई अन्य डिवाइस चुन सकते हैं, या बंद कर सकते हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="120"/>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="189"/>
        <source>Quit</source>
        <translation>बंद करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="130"/>
        <source>Pick Different Device</source>
        <translation>दूसरा डिवाइस चुनें</translation>
    </message>
    <message>
        <source>Reconfigure</source>
        <translation type="vanished">पुनः कॉन्फ़िगर करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Try Again</source>
        <translation>पुनः प्रयास करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Reconnect</source>
        <translation>पुनः कनेक्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="157"/>
        <source>Pick the correct device, then press Connect.</source>
        <translation>सही डिवाइस चुनें, फिर कनेक्ट करें दबाएं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="166"/>
        <source>I/O Interface: %1</source>
        <translation>I/O इंटरफ़ेस: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="199"/>
        <source>Connect</source>
        <translation>कनेक्ट करें</translation>
    </message>
</context>
<context>
    <name>SerialStudio</name>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="338"/>
        <source>Data Grids</source>
        <translation>डेटा ग्रिड</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="341"/>
        <source>Multiple Data Plots</source>
        <translation>मल्टीपल डेटा प्लॉट</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="344"/>
        <source>Accelerometers</source>
        <translation>एक्सेलेरोमीटर</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="347"/>
        <source>Gyroscopes</source>
        <translation>जायरोस्कोप</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="350"/>
        <source>GPS</source>
        <translation>GPS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="353"/>
        <source>FFT Plots</source>
        <translation>FFT प्लॉट</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="356"/>
        <source>LED Panels</source>
        <translation>LED पैनल</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="359"/>
        <source>Data Plots</source>
        <translation>डेटा प्लॉट</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="362"/>
        <source>Bars</source>
        <translation>बार</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="365"/>
        <source>Gauges</source>
        <translation>गेज</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="368"/>
        <source>Terminal</source>
        <translation>टर्मिनल</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="371"/>
        <source>Clock</source>
        <translation>घड़ी</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="374"/>
        <source>Stopwatch</source>
        <translation>स्टॉपवॉच</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="377"/>
        <source>Compasses</source>
        <translation>कंपास</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="380"/>
        <source>Meters</source>
        <translation>मीटर</translation>
    </message>
    <message>
        <source>Thermometers</source>
        <translation type="vanished">थर्मामीटर</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="383"/>
        <source>3D Plots</source>
        <translation>3D प्लॉट</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="386"/>
        <source>Web Views</source>
        <translation>वेब व्यू</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="390"/>
        <source>Image Views</source>
        <translation>इमेज व्यू</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="393"/>
        <source>Output Panels</source>
        <translation>आउटपुट पैनल</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="396"/>
        <source>Notifications</source>
        <translation>नोटिफ़िकेशन</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="399"/>
        <source>Waterfalls</source>
        <translation>वॉटरफॉल</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="402"/>
        <source>Painter Widgets</source>
        <translation>पेंटर विजेट</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="950"/>
        <source>UTF-8</source>
        <translation>UTF-8</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="951"/>
        <source>UTF-16 LE</source>
        <translation>UTF-16 LE</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="952"/>
        <source>UTF-16 BE</source>
        <translation>UTF-16 BE</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="953"/>
        <source>Latin-1</source>
        <translation>Latin-1</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="954"/>
        <source>System</source>
        <translation>सिस्टम</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="955"/>
        <source>GBK</source>
        <translation>GBK</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="956"/>
        <source>GB18030</source>
        <translation>GB18030</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="957"/>
        <source>Big5</source>
        <translation>Big5</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="958"/>
        <source>Shift-JIS</source>
        <translation>Shift-JIS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="959"/>
        <source>EUC-JP</source>
        <translation>EUC-JP</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="960"/>
        <source>EUC-KR</source>
        <translation>EUC-KR</translation>
    </message>
</context>
<context>
    <name>SessionDetail</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="19"/>
        <source>Session Details</source>
        <translation>सत्र विवरण</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="88"/>
        <source>Select a session to view details.</source>
        <translation>विवरण देखने के लिए एक सत्र चुनें।</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="130"/>
        <source>Project:</source>
        <translation>प्रोजेक्ट:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="143"/>
        <source>Started:</source>
        <translation>प्रारंभ:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="156"/>
        <source>Ended:</source>
        <translation>समाप्त:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="162"/>
        <source>(in progress)</source>
        <translation>(प्रगति में)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="169"/>
        <source>Frames:</source>
        <translation>फ़्रेम:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="185"/>
        <source>Notes</source>
        <translation>नोट्स</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="200"/>
        <source>Add session notes…</source>
        <translation>सत्र नोट्स जोड़ें…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="201"/>
        <source>Notes are read-only for completed sessions.</source>
        <translation>पूर्ण सत्रों के लिए नोट्स केवल पढ़ने योग्य हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="286"/>
        <source>New tag…</source>
        <translation>नया टैग…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="362"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>सेशन हटाने के लिए सेशन फ़ाइल अनलॉक करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="222"/>
        <source>Tags</source>
        <translation>टैग</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="293"/>
        <source>Add</source>
        <translation>जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="330"/>
        <source>Replay</source>
        <translation>रीप्ले करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="338"/>
        <source>Export CSV</source>
        <translation>CSV एक्सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="345"/>
        <source>Generate Report</source>
        <translation>रिपोर्ट जनरेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="356"/>
        <source>Delete</source>
        <translation>डिलीट करें</translation>
    </message>
</context>
<context>
    <name>SessionList</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="19"/>
        <source>Sessions</source>
        <translation>सेशन</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="71"/>
        <source>Search</source>
        <translation>खोजें</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="91"/>
        <source>Date</source>
        <translation>तारीख</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="92"/>
        <source>Frames</source>
        <translation>फ़्रेम</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="93"/>
        <source>Tags</source>
        <translation>टैग</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="193"/>
        <source>No sessions found.</source>
        <translation>कोई सेशन नहीं मिला।</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="194"/>
        <source>No session file open.</source>
        <translation>कोई सेशन फ़ाइल खुली नहीं है।</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseManager</name>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1001"/>
        <source>Select logo image</source>
        <translation>लोगो इमेज चुनें</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1003"/>
        <source>Images (*.png *.jpg *.jpeg *.svg)</source>
        <translation>इमेज (*.png *.jpg *.jpeg *.svg)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="426"/>
        <source>Open Session File</source>
        <translation>सेशन फ़ाइल खोलें</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="428"/>
        <source>Session files (*.db)</source>
        <translation>सेशन फ़ाइलें (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1200"/>
        <source>Cannot open session file</source>
        <translation>सेशन फ़ाइल नहीं खोली जा सकती</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="653"/>
        <source>Delete session from %1?</source>
        <translation>%1 से सेशन हटाएँ?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="656"/>
        <source>Delete Session</source>
        <translation>सेशन डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1061"/>
        <source>No project data</source>
        <translation>कोई प्रोजेक्ट डेटा नहीं</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="654"/>
        <source>All readings and raw data for this session are permanently removed.</source>
        <translation>इस सेशन की सभी रीडिंग और रॉ डेटा स्थायी रूप से हटा दिए जाएंगे।</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="484"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="493"/>
        <source>Lock Session File</source>
        <translation>सेशन फ़ाइल लॉक करें</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="485"/>
        <source>Choose a password to lock the session file:</source>
        <translation>सेशन फ़ाइल लॉक करने के लिए पासवर्ड चुनें:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="494"/>
        <source>Confirm the password:</source>
        <translation>पासवर्ड की पुष्टि करें:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="502"/>
        <source>Passwords do not match</source>
        <translation>पासवर्ड मेल नहीं खाते</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="503"/>
        <source>The two passwords you entered do not match. The session file was not locked.</source>
        <translation>दर्ज किए गए दोनों पासवर्ड मेल नहीं खाते। सेशन फ़ाइल लॉक नहीं की गई।</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="539"/>
        <source>Unlock Session File</source>
        <translation>सेशन फ़ाइल अनलॉक करें</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="540"/>
        <source>Enter the session file password:</source>
        <translation>सेशन फ़ाइल पासवर्ड दर्ज करें:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="550"/>
        <source>Incorrect password</source>
        <translation>गलत पासवर्ड</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="551"/>
        <source>The password you entered does not match the one stored in the session file.</source>
        <translation>दर्ज किया गया पासवर्ड सेशन फ़ाइल में संग्रहीत पासवर्ड से मेल नहीं खाता।</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="643"/>
        <source>Session file locked</source>
        <translation>सेशन फ़ाइल लॉक की गई</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="644"/>
        <source>Unlock the session file before deleting recorded sessions.</source>
        <translation>रिकॉर्ड किए गए सेशन हटाने से पहले सेशन फ़ाइल अनलॉक करें।</translation>
    </message>
    <message>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation type="vanished">इस सेशन में एम्बेडेड प्रोजेक्ट फ़ाइल नहीं है — डैशबोर्ड क्विक-प्लॉट लेआउट पर वापस आ जाता है।</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="778"/>
        <source>Export Session to CSV</source>
        <translation>सेशन को CSV में एक्सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="778"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV फ़ाइलें (*.CSV)</translation>
    </message>
    <message>
        <source>Export Complete</source>
        <translation type="vanished">एक्सपोर्ट पूर्ण</translation>
    </message>
    <message>
        <source>Session exported to:
%1</source>
        <translation type="vanished">सेशन यहाँ एक्सपोर्ट किया गया:
%1</translation>
    </message>
    <message>
        <source>Preparing export…</source>
        <translation type="vanished">एक्सपोर्ट तैयार किया जा रहा है…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="973"/>
        <source>Done</source>
        <translation>पूर्ण</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="937"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="973"/>
        <source>Failed</source>
        <translation>विफल</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="943"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="983"/>
        <source>Report Failed</source>
        <translation>रिपोर्ट विफल</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="945"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="984"/>
        <source>Could not generate the report. Check the output path and try again.</source>
        <translation>रिपोर्ट जेनरेट नहीं हो सकी। आउटपुट पथ जाँचें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="873"/>
        <source>Save PDF Report</source>
        <translation>PDF रिपोर्ट सेव करें</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="853"/>
        <source>Loading session data…</source>
        <translation>सेशन डेटा लोड हो रहा है…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="873"/>
        <source>Save HTML Report</source>
        <translation>HTML रिपोर्ट सेव करें</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="874"/>
        <source>PDF files (*.pdf)</source>
        <translation>PDF फाइलें (*.PDF)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="874"/>
        <source>HTML files (*.html)</source>
        <translation>HTML फाइलें (*.HTML)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1062"/>
        <source>This session file does not contain an embedded project.</source>
        <translation>इस सेशन फाइल में एम्बेडेड प्रोजेक्ट नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1071"/>
        <source>Invalid project data</source>
        <translation>अमान्य प्रोजेक्ट डेटा</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1072"/>
        <source>The embedded project JSON is malformed and cannot be restored.</source>
        <translation>एम्बेडेड प्रोजेक्ट JSON खराब है और रिस्टोर नहीं किया जा सकता।</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1082"/>
        <source>Restore Project</source>
        <translation>प्रोजेक्ट रिस्टोर करें</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1082"/>
        <source>Serial Studio projects (*.ssproj *.json)</source>
        <translation>Serial Studio प्रोजेक्ट (*.ssproj *.json)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1090"/>
        <source>Cannot write file</source>
        <translation>फ़ाइल नहीं लिखी जा सकती</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1090"/>
        <source>Check file permissions and try again.</source>
        <translation>फ़ाइल अनुमतियाँ जाँचें और पुनः प्रयास करें।</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseWorker</name>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="76"/>
        <source>Empty file path</source>
        <translation>फ़ाइल पथ खाली है</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="170"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="225"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="285"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="356"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="381"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="409"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="449"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="638"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="703"/>
        <source>Database not open</source>
        <translation>डेटाबेस खुला नहीं है</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="262"/>
        <source>Database not open or empty label</source>
        <translation>डेटाबेस खुला नहीं है या लेबल खाली है</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="330"/>
        <source>Invalid label</source>
        <translation>अमान्य लेबल</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="597"/>
        <source>Cancelled</source>
        <translation>रद्द किया गया</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="710"/>
        <source>Could not load session data</source>
        <translation>सेशन डेटा लोड नहीं हो सका</translation>
    </message>
</context>
<context>
    <name>Sessions::HtmlReport</name>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="209"/>
        <source>Assembling report…</source>
        <translation>रिपोर्ट असेंबल की जा रही है…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="217"/>
        <source>Writing output…</source>
        <translation>आउटपुट लिखा जा रहा है…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="282"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="342"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="700"/>
        <source>Session Report</source>
        <translation>सेशन रिपोर्ट</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="345"/>
        <source>Untitled project</source>
        <translation>अनाम प्रोजेक्ट</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="352"/>
        <source>Prepared by</source>
        <translation>तैयारकर्ता</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="355"/>
        <source>Generated on %1</source>
        <translation>%1 को जेनरेट किया गया</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="377"/>
        <source>Test ID</source>
        <translation>टेस्ट ID</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="379"/>
        <source>Duration</source>
        <translation>अवधि</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="381"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="493"/>
        <source>Samples</source>
        <translation>सैंपल</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="383"/>
        <source>Parameters</source>
        <translation>पैरामीटर</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="385"/>
        <source>Started</source>
        <translation>शुरू किया</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="387"/>
        <source>Ended</source>
        <translation>समाप्त किया</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="423"/>
        <source>Project</source>
        <translation>प्रोजेक्ट</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="425"/>
        <source>Test identifier</source>
        <translation>टेस्ट पहचानकर्ता</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="426"/>
        <source>Start time</source>
        <translation>शुरुआत का समय</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="427"/>
        <source>End time</source>
        <translation>समाप्ति का समय</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="428"/>
        <source>Total duration</source>
        <translation>कुल अवधि</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="429"/>
        <source>Samples acquired</source>
        <translation>प्राप्त सैंपल</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="430"/>
        <source>Parameters logged</source>
        <translation>लॉग किए गए पैरामीटर</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="446"/>
        <source>Classification</source>
        <translation>वर्गीकरण</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="453"/>
        <source>Notes</source>
        <translation>टिप्पणियाँ</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="461"/>
        <source>Test Information</source>
        <translation>परीक्षण जानकारी</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="482"/>
        <source>Parameter</source>
        <translation>पैरामीटर</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="485"/>
        <source>Units</source>
        <translation>इकाइयाँ</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="494"/>
        <source>Minimum</source>
        <translation>न्यूनतम</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="495"/>
        <source>Maximum</source>
        <translation>अधिकतम</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="496"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="652"/>
        <source>Mean</source>
        <translation>औसत</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="497"/>
        <source>Std. Deviation</source>
        <translation>मानक विचलन</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="542"/>
        <source>Measurement Summary</source>
        <translation>माप सारांश</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="543"/>
        <source>click a column to sort</source>
        <translation>क्रमबद्ध करने के लिए कॉलम पर क्लिक करें</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="568"/>
        <source>%1 samples over %2 seconds</source>
        <translation>%2 सेकंड में %1 सैंपल</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="586"/>
        <source>Combined Parameter View</source>
        <translation>संयुक्त पैरामीटर व्यू</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="587"/>
        <source>click legend items to toggle signals</source>
        <translation>सिग्नल टॉगल करने के लिए लीजेंड आइटम पर क्लिक करें</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="595"/>
        <source>Parameter Trends</source>
        <translation>पैरामीटर ट्रेंड</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="650"/>
        <source>Min</source>
        <translation>न्यूनतम</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="651"/>
        <source>Max</source>
        <translation>अधिकतम</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="724"/>
        <source>Page %1 of %2</source>
        <translation>पेज %1 का %2</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="794"/>
        <source>Loading rendering engine…</source>
        <translation>रेंडरिंग इंजन लोड हो रहा है…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="814"/>
        <source>Rendering charts…</source>
        <translation>चार्ट रेंडर हो रहे हैं…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="858"/>
        <source>Generating PDF…</source>
        <translation>PDF जनरेट हो रहा है…</translation>
    </message>
</context>
<context>
    <name>Sessions::Player</name>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="261"/>
        <source>Open Session File</source>
        <translation>सेशन फ़ाइल खोलें</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="263"/>
        <source>Session files (*.db)</source>
        <translation>सेशन फ़ाइलें (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="334"/>
        <source>Device Connection Active</source>
        <translation>डिवाइस कनेक्शन सक्रिय</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="335"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>इस सुविधा का उपयोग करने के लिए, डिवाइस से डिस्कनेक्ट करना आवश्यक है। क्या आप आगे बढ़ना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="383"/>
        <location filename="../../src/Sessions/Player.cpp" line="462"/>
        <source>Cannot open session file</source>
        <translation>सेशन फ़ाइल नहीं खोली जा सकती</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="384"/>
        <source>Unknown error</source>
        <translation>अज्ञात त्रुटि</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="400"/>
        <source>No project data</source>
        <translation>कोई प्रोजेक्ट डेटा नहीं</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="401"/>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation>इस सेशन में एम्बेडेड प्रोजेक्ट फ़ाइल नहीं है — डैशबोर्ड क्विक-प्लॉट लेआउट पर वापस आ जाता है।</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="463"/>
        <source>Check file permissions and try again.</source>
        <translation>फ़ाइल अनुमतियाँ जाँचें और पुनः प्रयास करें।</translation>
    </message>
    <message>
        <source>No sessions found</source>
        <translation type="vanished">कोई सेशन नहीं मिला</translation>
    </message>
    <message>
        <source>This file does not contain any recording sessions.</source>
        <translation type="vanished">इस फ़ाइल में कोई रिकॉर्डिंग सेशन नहीं है।</translation>
    </message>
    <message>
        <source>Session has no columns</source>
        <translation type="vanished">सेशन में कोई कॉलम नहीं है</translation>
    </message>
    <message>
        <source>The selected session is missing its column definitions.</source>
        <translation type="vanished">चयनित सेशन में इसकी कॉलम परिभाषाएँ गायब हैं।</translation>
    </message>
    <message>
        <source>Session has no readings</source>
        <translation type="vanished">सेशन में कोई रीडिंग नहीं है</translation>
    </message>
    <message>
        <source>The selected session does not contain any frames.</source>
        <translation type="vanished">चयनित सेशन में कोई फ्रेम नहीं है।</translation>
    </message>
</context>
<context>
    <name>Sessions::PlayerLoaderWorker</name>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="168"/>
        <source>Empty file path</source>
        <translation>खाली फ़ाइल पथ</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="69"/>
        <source>This file does not contain any recording sessions.</source>
        <translation>इस फ़ाइल में कोई रिकॉर्डिंग सेशन नहीं है।</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="144"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="205"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="224"/>
        <source>Cancelled</source>
        <translation>रद्द किया गया</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="218"/>
        <source>The selected session is missing its column definitions.</source>
        <translation>चयनित सेशन में कॉलम परिभाषाएँ गायब हैं।</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="235"/>
        <source>The selected session does not contain any frames.</source>
        <translation>चयनित सत्र में कोई फ़्रेम नहीं है।</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="34"/>
        <source>Preferences</source>
        <translation>प्राथमिकताएँ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="61"/>
        <source>General</source>
        <translation>सामान्य</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="166"/>
        <source>Language</source>
        <translation>भाषा</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="182"/>
        <source>Theme</source>
        <translation>थीम</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="289"/>
        <source>Workspace Folder</source>
        <translation>वर्कस्पेस फ़ोल्डर</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="336"/>
        <source>Enable API Server (Port 7777)</source>
        <translation>API सर्वर सक्षम करें (पोर्ट 7777)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="585"/>
        <source>Automatically Check for Updates</source>
        <translation>अपडेट स्वचालित रूप से जाँचें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="73"/>
        <source>Dashboard</source>
        <translation>डैशबोर्ड</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="402"/>
        <source>Export…</source>
        <translation>एक्सपोर्ट करें…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="649"/>
        <source>Data Plotting</source>
        <translation>डेटा प्लॉटिंग</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="716"/>
        <source>Point Count</source>
        <translation>पॉइंट काउंट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="741"/>
        <source>UI Refresh Rate (Hz)</source>
        <translation>UI रिफ्रेश रेट (Hz)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="872"/>
        <source>Show Actions Panel</source>
        <translation>एक्शन पैनल दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1051"/>
        <source>Always Show Taskbar Buttons</source>
        <translation>टास्कबार बटन हमेशा दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="85"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1163"/>
        <source>Console</source>
        <translation>कंसोल</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="151"/>
        <source>Appearance</source>
        <translation>अपीयरेंस</translation>
    </message>
    <message>
        <source>Files &amp; Updates</source>
        <translation type="vanished">फाइलें और अपडेट</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation type="vanished">उन्नत</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="354"/>
        <source>Allow External API Connections</source>
        <translation>बाहरी API कनेक्शन की अनुमति दें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="888"/>
        <source>Auto-Hide Toolbar</source>
        <translation>टूलबार स्वतः छुपाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="79"/>
        <source>Taskbar</source>
        <translation>टास्कबार</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="470"/>
        <source>Rendering Backend</source>
        <translation>रेंडरिंग बैकएंड</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="370"/>
        <source>API Access Token</source>
        <translation>API एक्सेस टोकन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="67"/>
        <source>Startup</source>
        <translation>स्टार्टअप</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="202"/>
        <source>Window</source>
        <translation>विंडो</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="221"/>
        <source>Custom Window Decorations</source>
        <translation>कस्टम विंडो सजावट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="237"/>
        <source>Window Shadow</source>
        <translation>विंडो छाया</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="264"/>
        <source>Window decoration changes apply after restarting %1.</source>
        <translation>विंडो सजावट परिवर्तन %1 को पुनः आरंभ करने के बाद लागू होते हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="273"/>
        <source>Files</source>
        <translation>फ़ाइलें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="320"/>
        <source>API &amp; Plugins</source>
        <translation>API और प्लगइन्स</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="452"/>
        <source>Graphics</source>
        <translation>ग्राफ़िक्स</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="508"/>
        <source>System</source>
        <translation>सिस्टम</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="524"/>
        <source>Apply Performance Hints</source>
        <translation>परफ़ॉर्मेंस हिंट्स लागू करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="538"/>
        <source>Keep Display Awake</source>
        <translation>डिस्प्ले को सक्रिय रखें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="558"/>
        <source>Performance hints raise process priority and opt out of OS power throttling. Changes take effect the next time Serial Studio starts.</source>
        <translation>परफ़ॉर्मेंस हिंट्स प्रोसेस प्राथमिकता बढ़ाते हैं और OS पावर थ्रॉटलिंग से बाहर निकलते हैं। बदलाव अगली बार Serial Studio शुरू होने पर लागू होंगे।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="569"/>
        <source>Updates &amp; News</source>
        <translation>अपडेट और समाचार</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="599"/>
        <source>Show What's New on Startup</source>
        <translation>स्टार्टअप पर नया क्या है दिखाएँ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="664"/>
        <source>Time Range</source>
        <translation>समय सीमा</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Small</source>
        <translation>छोटा</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Normal</source>
        <translation>सामान्य</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Large</source>
        <translation>बड़ा</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Extra Large</source>
        <translation>अतिरिक्त बड़ा</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Custom</source>
        <translation>कस्टम</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="857"/>
        <source>Layout</source>
        <translation>लेआउट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="904"/>
        <source>Auto-Layout Margin</source>
        <translation>ऑटो-लेआउट मार्जिन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="929"/>
        <source>Auto-Layout Spacing</source>
        <translation>ऑटो-लेआउट स्पेसिंग</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="961"/>
        <source>Video Export</source>
        <translation>वीडियो एक्सपोर्ट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="979"/>
        <source>Save Videos by Default</source>
        <translation>डिफ़ॉल्ट रूप से वीडियो सहेजें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1030"/>
        <source>Behavior</source>
        <translation>व्यवहार</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1066"/>
        <source>Show Search Field</source>
        <translation>खोज फ़ील्ड दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1081"/>
        <source>Auto-hide Taskbar</source>
        <translation>टास्कबार स्वतः छुपाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1099"/>
        <source>Hide Delay (ms)</source>
        <translation>छुपाने की देरी (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1123"/>
        <source>Pinned Buttons</source>
        <translation>पिन किए गए बटन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1141"/>
        <source>Drag a pinned button on the taskbar to reorder it.</source>
        <translation>क्रम बदलने के लिए टास्कबार पर पिन किए गए बटन को ड्रैग करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1162"/>
        <source>Settings</source>
        <translation>सेटिंग्स</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1165"/>
        <source>Clock</source>
        <translation>घड़ी</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1166"/>
        <source>Stopwatch</source>
        <translation>स्टॉपवॉच</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1167"/>
        <source>Pause / Resume</source>
        <translation>रोकें / फिर से शुरू करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1168"/>
        <source>File Transmission</source>
        <translation>फ़ाइल ट्रांसमिशन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1169"/>
        <source>AI Assistant</source>
        <translation>AI असिस्टेंट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1298"/>
        <source>Display</source>
        <translation>डिस्प्ले</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1313"/>
        <source>Display Mode</source>
        <translation>डिस्प्ले मोड</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="778"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1326"/>
        <source>Font Family</source>
        <translation>फ़ॉन्ट फ़ैमिली</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="92"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1164"/>
        <source>Notifications</source>
        <translation>नोटिफ़िकेशन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="400"/>
        <source>Export Protobuf File</source>
        <translation>Protobuf फ़ाइल एक्सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="763"/>
        <source>Dashboard Font</source>
        <translation>डैशबोर्ड फ़ॉन्ट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="793"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1341"/>
        <source>Font Size</source>
        <translation>फ़ॉन्ट साइज़</translation>
    </message>
    <message>
        <source>Image Export</source>
        <translation type="vanished">इमेज एक्सपोर्ट</translation>
    </message>
    <message>
        <source>Save Images by Default</source>
        <translation type="vanished">डिफ़ॉल्ट रूप से इमेज सेव करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1358"/>
        <source>Show Timestamps</source>
        <translation>टाइमस्टैम्प दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1377"/>
        <source>Data Transmission</source>
        <translation>डेटा ट्रांसमिशन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1392"/>
        <source>Line Ending</source>
        <translation>लाइन एंडिंग</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1405"/>
        <source>Input Mode</source>
        <translation>इनपुट मोड</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1418"/>
        <source>Text Encoding</source>
        <translation>टेक्स्ट एन्कोडिंग</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1431"/>
        <source>Checksum</source>
        <translation>चेकसम</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1444"/>
        <source>Echo Sent Data</source>
        <translation>भेजा गया डेटा इको करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1463"/>
        <source>Escape Codes</source>
        <translation>एस्केप कोड्स</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1478"/>
        <source>VT100 Emulation</source>
        <translation>VT100 एमुलेशन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1497"/>
        <source>ANSI Colors</source>
        <translation>ANSI रंग</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1555"/>
        <source>Delivery</source>
        <translation>डिलीवरी</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1570"/>
        <source>System Notifications</source>
        <translation>सिस्टम नोटिफिकेशन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1591"/>
        <source>Show Warning/Critical events as OS desktop notifications when Serial Studio is not the foreground window.</source>
        <translation>Serial Studio जब फोरग्राउंड विंडो नहीं है तब Warning/Critical इवेंट्स को OS डेस्कटॉप नोटिफिकेशन के रूप में दिखाएं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1601"/>
        <source>Application Logs</source>
        <translation>एप्लिकेशन लॉग्स</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1616"/>
        <source>Route Warnings to Notifications</source>
        <translation>Warnings को नोटिफिकेशन में रूट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1637"/>
        <source>Off by default — Qt and QML emit warnings frequently and enabling this can drown out real alarms. Critical messages are always routed regardless of this setting.</source>
        <translation>डिफ़ॉल्ट रूप से बंद — QT और QML अक्सर warnings उत्सर्जित करते हैं और इसे सक्षम करने से वास्तविक अलार्म दब सकते हैं। Critical संदेश हमेशा इस सेटिंग की परवाह किए बिना रूट किए जाते हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1656"/>
        <source>Reset</source>
        <translation>रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1696"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1704"/>
        <source>Apply</source>
        <translation>लागू करें</translation>
    </message>
</context>
<context>
    <name>Setup</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="35"/>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="380"/>
        <source>Device Setup</source>
        <translation>डिवाइस सेटअप</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="126"/>
        <source>API Server Active (%1)</source>
        <translation>API सर्वर सक्रिय (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="127"/>
        <source>API Server Ready</source>
        <translation>API सर्वर तैयार</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="128"/>
        <source>API Server Off</source>
        <translation>API सर्वर बंद</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="188"/>
        <source>Frame Parsing</source>
        <translation>फ़्रेम पार्सिंग</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="198"/>
        <source>Console Only (No Parsing)</source>
        <translation>केवल कंसोल (कोई पार्सिंग नहीं)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="211"/>
        <source>Quick Plot (Comma Separated Values)</source>
        <translation>त्वरित प्लॉट (कॉमा से अलग किए गए मान)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="222"/>
        <source>Parse via Project File</source>
        <translation>प्रोजेक्ट फ़ाइल के माध्यम से पार्स करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="245"/>
        <source>Change Project File (%1)</source>
        <translation>प्रोजेक्ट फ़ाइल बदलें (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="246"/>
        <source>Select Project File</source>
        <translation>प्रोजेक्ट फ़ाइल चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="261"/>
        <source>Data Export</source>
        <translation>डेटा एक्सपोर्ट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="285"/>
        <source>CSV Spreadsheet</source>
        <translation>CSV स्प्रेडशीट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="303"/>
        <source>Session Recording</source>
        <translation>सेशन रिकॉर्डिंग</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="324"/>
        <source>MDF4 Recording</source>
        <translation>MDF4 रिकॉर्डिंग</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="340"/>
        <source>Console Log</source>
        <translation>कंसोल लॉग</translation>
    </message>
    <message>
        <source>CSV File</source>
        <translation type="vanished">CSV फ़ाइल</translation>
    </message>
    <message>
        <source>Session Database</source>
        <translation type="vanished">सत्र डेटाबेस</translation>
    </message>
    <message>
        <source>MDF4 File</source>
        <translation type="vanished">MDF4 फ़ाइल</translation>
    </message>
    <message>
        <source>Console Dump</source>
        <translation type="vanished">कंसोल डंप</translation>
    </message>
    <message>
        <source>Create CSV File</source>
        <translation type="vanished">CSV फ़ाइल बनाएं</translation>
    </message>
    <message>
        <source>Create MDF4 File</source>
        <translation type="vanished">MDF4 फ़ाइल बनाएं</translation>
    </message>
    <message>
        <source>Create Session Log</source>
        <translation type="vanished">सत्र लॉग बनाएं</translation>
    </message>
    <message>
        <source>Export Console Data</source>
        <translation type="vanished">कंसोल डेटा निर्यात करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="392"/>
        <source>I/O Interface: %1</source>
        <translation>I/O इंटरफ़ेस: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="461"/>
        <source>Multi-Device Project</source>
        <translation>मल्टी-डिवाइस प्रोजेक्ट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="474"/>
        <source>This project streams data from %1 independent devices.</source>
        <translation>यह प्रोजेक्ट %1 स्वतंत्र डिवाइसों से डेटा स्ट्रीम करता है।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="487"/>
        <source>Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.</source>
        <translation>प्रत्येक डिवाइस की अपनी कनेक्शन सेटिंग्स हैं। उन्हें Project Editor में Sources टैब के अंतर्गत कॉन्फ़िगर करें।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="506"/>
        <source>Open Project Editor</source>
        <translation>Project Editor खोलें</translation>
    </message>
</context>
<context>
    <name>ShortcutGenerator</name>
    <message>
        <source>New Shortcut</source>
        <translation type="vanished">नया शॉर्टकट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="93"/>
        <source>Choose an Icon</source>
        <translation>आइकन चुनें</translation>
    </message>
    <message>
        <source>Save Shortcut</source>
        <translation type="vanished">शॉर्टकट सहेजें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="22"/>
        <source>New Deployment</source>
        <translation>नई डिप्लॉयमेंट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="109"/>
        <source>Save Deployment</source>
        <translation>डिप्लॉयमेंट सहेजें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="152"/>
        <source>General</source>
        <translation>सामान्य</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="158"/>
        <source>Taskbar</source>
        <translation>टास्कबार</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="164"/>
        <source>Logging</source>
        <translation>लॉगिंग</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="221"/>
        <source>Identity</source>
        <translation>पहचान</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="277"/>
        <source>Click to choose an icon</source>
        <translation>आइकन चुनने के लिए क्लिक करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="286"/>
        <source>Name:</source>
        <translation>नाम:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="295"/>
        <source>Deployment Name</source>
        <translation>डिप्लॉयमेंट का नाम</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="386"/>
        <source>Theme</source>
        <translation>थीम</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="396"/>
        <source>Same as Serial Studio</source>
        <translation>Serial Studio के समान</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="418"/>
        <source>Actions Panel</source>
        <translation>एक्शन पैनल</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="429"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="638"/>
        <source>File Transmission</source>
        <translation>फ़ाइल ट्रांसमिशन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="445"/>
        <source>Double-clicking this deployment takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation>इस डिप्लॉयमेंट पर डबल-क्लिक करने से कोई सीधे इस प्रोजेक्ट के लाइव डैशबोर्ड पर पहुँच जाता है। कोई टूलबार या सेटअप पेन नहीं, केवल डेटा, और डिवाइस डिस्कनेक्ट होते ही Serial Studio बंद हो जाता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="491"/>
        <source>Visibility</source>
        <translation>दृश्यता</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="506"/>
        <source>Mode</source>
        <translation>मोड</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="515"/>
        <source>Always shown</source>
        <translation>हमेशा दिखाई देता है</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="516"/>
        <source>Auto-hide</source>
        <translation>स्वतः छुपाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="517"/>
        <source>Hidden</source>
        <translation>छुपा हुआ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="532"/>
        <source>Hiding the taskbar removes window minimize/maximize/close buttons and forces auto-layout, so the dashboard always fills the available area.</source>
        <translation>टास्कबार छुपाने से विंडो के minimize/maximize/close बटन हट जाते हैं और auto-layout सक्रिय हो जाता है, जिससे डैशबोर्ड हमेशा उपलब्ध क्षेत्र को भर देता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="536"/>
        <source>The taskbar slides in when the user moves the cursor near the bottom edge.</source>
        <translation>जब उपयोगकर्ता कर्सर को निचले किनारे के पास ले जाता है तो टास्कबार स्लाइड करके आ जाता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="538"/>
        <source>The taskbar is permanently visible at the bottom of the dashboard.</source>
        <translation>टास्कबार डैशबोर्ड के निचले हिस्से में स्थायी रूप से दिखाई देता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="551"/>
        <source>Pinned Buttons</source>
        <translation>पिन किए गए बटन</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="568"/>
        <source>Console</source>
        <translation>कंसोल</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="582"/>
        <source>Notifications</source>
        <translation>सूचनाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="596"/>
        <source>Clock</source>
        <translation>घड़ी</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="610"/>
        <source>Stopwatch</source>
        <translation>स्टॉपवॉच</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="624"/>
        <source>Pause</source>
        <translation>रोकें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="749"/>
        <source>Recordings are saved in the Serial Studio workspace folder</source>
        <translation>रिकॉर्डिंग Serial Studio वर्कस्पेस फ़ोल्डर में सहेजी जाती हैं</translation>
    </message>
    <message>
        <source>Shortcut Name</source>
        <translation type="vanished">शॉर्टकट नाम</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="304"/>
        <source>Change Icon…</source>
        <translation>आइकन बदलें…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="321"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="339"/>
        <source>Project</source>
        <translation>प्रोजेक्ट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="349"/>
        <source>Choose a project file to begin</source>
        <translation>शुरू करने के लिए प्रोजेक्ट फ़ाइल चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="371"/>
        <source>Behavior</source>
        <translation>व्यवहार</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="408"/>
        <source>Fullscreen</source>
        <translation>फ़ुलस्क्रीन</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation type="vanished">इस शॉर्टकट पर डबल-क्लिक करने से कोई सीधे इस प्रोजेक्ट के लाइव डैशबोर्ड पर पहुँच जाता है। कोई टूलबार या सेटअप पेन नहीं, केवल डेटा, और डिवाइस डिस्कनेक्ट होते ही Serial Studio बंद हो जाता है।</translation>
    </message>
    <message>
        <source>Embed Project</source>
        <translation type="vanished">प्रोजेक्ट एम्बेड करें</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.

Turn on Embed Project to bake the project into the shortcut, so it keeps working even if the original file is moved or deleted.</source>
        <translation type="vanished">इस शॉर्टकट पर डबल-क्लिक करने से सीधे इस प्रोजेक्ट के लाइव डैशबोर्ड पर पहुँच जाता है। कोई टूलबार या सेटअप पैन नहीं, सिर्फ़ डेटा, और डिवाइस डिस्कनेक्ट होते ही Serial Studio बंद हो जाता है।

शॉर्टकट में प्रोजेक्ट को बेक करने के लिए प्रोजेक्ट एम्बेड करें चालू करें, ताकि मूल फ़ाइल हटाने या स्थानांतरित करने पर भी यह काम करता रहे।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="688"/>
        <source>Recorders</source>
        <translation>रिकॉर्डर</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="703"/>
        <source>CSV File</source>
        <translation>CSV फ़ाइल</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="713"/>
        <source>MDF4 File</source>
        <translation>MDF4 फ़ाइल</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="723"/>
        <source>Session Database</source>
        <translation>सत्र डेटाबेस</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="733"/>
        <source>Console Log</source>
        <translation>कंसोल लॉग</translation>
    </message>
    <message>
        <source>Recordings are saved to each module’s default location.</source>
        <translation type="vanished">रिकॉर्डिंग प्रत्येक मॉड्यूल के डिफ़ॉल्ट स्थान पर सहेजी जाती हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="778"/>
        <source>Cancel</source>
        <translation>रद्द करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="787"/>
        <source>Save</source>
        <translation>सहेजें</translation>
    </message>
</context>
<context>
    <name>SourceFrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="80"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="317"/>
        <source>Undo</source>
        <translation>पूर्ववत करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="87"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="328"/>
        <source>Redo</source>
        <translation>पुनः करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="96"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="347"/>
        <source>Cut</source>
        <translation>काटें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="101"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="357"/>
        <source>Copy</source>
        <translation>कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="106"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="367"/>
        <source>Paste</source>
        <translation>पेस्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="113"/>
        <source>Select All</source>
        <translation>सभी चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="123"/>
        <source>Format Document</source>
        <translation>दस्तावेज़ फ़ॉर्मेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="130"/>
        <source>Format Selection</source>
        <translation>चयन फ़ॉर्मेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="297"/>
        <source>Reset</source>
        <translation>रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="382"/>
        <source>Validate</source>
        <translation>सत्यापित करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="386"/>
        <source>Verify that the script compiles correctly</source>
        <translation>सत्यापित करें कि स्क्रिप्ट सही ढंग से कंपाइल होती है</translation>
    </message>
    <message>
        <source>Reset template parameters to their defaults</source>
        <translation type="vanished">टेम्पलेट पैरामीटर को उनके डिफ़ॉल्ट पर रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="302"/>
        <source>Reset to the default parsing script</source>
        <translation>डिफ़ॉल्ट पार्सिंग स्क्रिप्ट पर रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="307"/>
        <source>Open</source>
        <translation>खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="312"/>
        <source>Import a script file for data parsing</source>
        <translation>डेटा पार्सिंग के लिए स्क्रिप्ट फ़ाइल इम्पोर्ट करें</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">डेटा पार्सिंग के लिए सहायता दस्तावेज़ खोलें</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">भाषा:</translation>
    </message>
    <message>
        <source>Native</source>
        <translation type="vanished">नेटिव</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="222"/>
        <source>Select Template…</source>
        <translation>टेम्पलेट चुनें…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="322"/>
        <source>Undo the last code edit</source>
        <translation>अंतिम कोड संपादन पूर्ववत करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="334"/>
        <source>Redo the previously undone edit</source>
        <translation>पहले पूर्ववत किया गया संपादन फिर से करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="352"/>
        <source>Cut selected code to clipboard</source>
        <translation>चयनित कोड को क्लिपबोर्ड पर कट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="362"/>
        <source>Copy selected code to clipboard</source>
        <translation>चयनित कोड को क्लिपबोर्ड पर कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="371"/>
        <source>Paste code from clipboard</source>
        <translation>क्लिपबोर्ड से कोड पेस्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="247"/>
        <source>Help</source>
        <translation>सहायता</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="196"/>
        <source>Platform:</source>
        <translation>प्लेटफ़ॉर्म:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="204"/>
        <source>Built-In</source>
        <translation>Built-In</translation>
    </message>
    <message>
        <source>Template:</source>
        <translation type="vanished">टेम्पलेट:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="238"/>
        <source>Test With Sample Data</source>
        <translation>नमूना डेटा के साथ परीक्षण करें</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">मूल्यांकन करें</translation>
    </message>
</context>
<context>
    <name>SourceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="107"/>
        <source>Duplicate</source>
        <translation>डुप्लिकेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="109"/>
        <source>Create a copy of this data source</source>
        <translation>इस डेटा स्रोत की एक प्रति बनाएं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="121"/>
        <source>Delete</source>
        <translation>डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="126"/>
        <source>Remove this data source</source>
        <translation>यह डेटा स्रोत हटाएं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="127"/>
        <source>The primary data source cannot be removed</source>
        <translation>प्राथमिक डेटा स्रोत को हटाया नहीं जा सकता</translation>
    </message>
</context>
<context>
    <name>SqlitePlayer</name>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="25"/>
        <source>Session Player</source>
        <translation>सेशन प्लेयर</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="92"/>
        <source>Loading session…</source>
        <translation>सत्र लोड हो रहा है…</translation>
    </message>
</context>
<context>
    <name>StartMenu</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="278"/>
        <source>Workspaces</source>
        <translation>वर्कस्पेसेज़</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="374"/>
        <source>No Workspaces Available</source>
        <translation>कोई वर्कस्पेस उपलब्ध नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="408"/>
        <source>Actions</source>
        <translation>एक्शन्स</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="431"/>
        <source>No Actions Available</source>
        <translation>कोई एक्शन उपलब्ध नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="461"/>
        <source>Plugins</source>
        <translation>प्लगइन्स</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="359"/>
        <source>New Workspace…</source>
        <translation>नया वर्कस्पेस…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="499"/>
        <source>Manage Plugins…</source>
        <translation>प्लगइन प्रबंधित करें…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="509"/>
        <source>No Plugins Installed</source>
        <translation>कोई प्लगइन इंस्टॉल नहीं है</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="101"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="545"/>
        <source>Auto Layout</source>
        <translation>ऑटो लेआउट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="109"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="557"/>
        <source>Full Screen</source>
        <translation>पूर्ण स्क्रीन</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="115"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="569"/>
        <source>Add External Window</source>
        <translation>बाहरी विंडो जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="135"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="758"/>
        <source>Clock</source>
        <translation>घड़ी</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="143"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="765"/>
        <source>Stopwatch</source>
        <translation>स्टॉपवॉच</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="163"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="782"/>
        <source>Sessions</source>
        <translation>सेशन</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="170"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="791"/>
        <source>File Transmission</source>
        <translation>फ़ाइल ट्रांसमिशन</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="177"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="799"/>
        <source>AI Assistant</source>
        <translation>AI असिस्टेंट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="345"/>
        <source>Show "%1"</source>
        <translation>"%1" दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="350"/>
        <source>Show All Hidden Workspaces</source>
        <translation>सभी छिपे हुए वर्कस्पेसेज़ दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="589"/>
        <source>Export</source>
        <translation>एक्सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="620"/>
        <source>CSV File</source>
        <translation>CSV फ़ाइल</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="626"/>
        <source>MDF4 File</source>
        <translation>MDF4 फ़ाइल</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="632"/>
        <source>Console Transcript</source>
        <translation>कंसोल ट्रांसक्रिप्ट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="641"/>
        <source>Session Database</source>
        <translation>सेशन डेटाबेस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="655"/>
        <source>No Export Formats Available</source>
        <translation>कोई एक्सपोर्ट फॉर्मेट उपलब्ध नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="685"/>
        <source>Tools</source>
        <translation>टूल्स</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="814"/>
        <source>No Tools Available</source>
        <translation>कोई टूल उपलब्ध नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="882"/>
        <source>Reset</source>
        <translation>रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="903"/>
        <source>Quit</source>
        <translation>बंद करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="929"/>
        <source>Edit…</source>
        <translation>एडिट करें…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="121"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="741"/>
        <source>Console</source>
        <translation>कंसोल</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="127"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="750"/>
        <source>Notifications</source>
        <translation>नोटिफिकेशन</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="151"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="773"/>
        <source>Preferences</source>
        <translation>प्रेफरेंस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="157"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="854"/>
        <source>Help Center</source>
        <translation>हेल्प सेंटर</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="940"/>
        <source>Delete</source>
        <translation>डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="941"/>
        <source>Hide</source>
        <translation>छुपाएं</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="875"/>
        <source>Resume</source>
        <translation>फिर से शुरू करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="876"/>
        <source>Pause</source>
        <translation>रोकें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="903"/>
        <source>Disconnect</source>
        <translation>डिस्कनेक्ट करें</translation>
    </message>
</context>
<context>
    <name>Stopwatch</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Stop</source>
        <translation>रोकें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Start</source>
        <translation>प्रारंभ</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="210"/>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="288"/>
        <source>Lap</source>
        <translation>लैप</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="226"/>
        <source>Reset</source>
        <translation>रीसेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="279"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="297"/>
        <source>Total</source>
        <translation>कुल</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="393"/>
        <source>No laps recorded</source>
        <translation>कोई लैप रिकॉर्ड नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="401"/>
        <source>Press Lap while the stopwatch is running</source>
        <translation>स्टॉपवॉच चलते समय लैप दबाएं</translation>
    </message>
</context>
<context>
    <name>SubMenuCombo</name>
    <message>
        <location filename="../../qml/Widgets/SubMenuCombo.qml" line="86"/>
        <source>No Data Available</source>
        <translation>कोई डेटा उपलब्ध नहीं</translation>
    </message>
</context>
<context>
    <name>SystemDatasetsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="33"/>
        <source>Dataset Values</source>
        <translation>डेटासेट वैल्यूज़</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="158"/>
        <source>Search</source>
        <translation>खोजें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="179"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="180"/>
        <source>Group</source>
        <translation>समूह</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="181"/>
        <source>Dataset</source>
        <translation>डेटासेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="182"/>
        <source>Units</source>
        <translation>इकाइयाँ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="252"/>
        <source>(virtual)</source>
        <translation>(वर्चुअल)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="298"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>एक्सेस कोड %1 को क्लिपबोर्ड पर कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="374"/>
        <source>Dataset access code copied</source>
        <translation>डेटासेट एक्सेस कोड कॉपी किया गया</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="323"/>
        <source>No datasets defined in this project.</source>
        <translation>इस प्रोजेक्ट में कोई डेटासेट परिभाषित नहीं है।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="324"/>
        <source>No datasets match your search.</source>
        <translation>आपकी खोज से कोई डेटासेट मेल नहीं खाता।</translation>
    </message>
</context>
<context>
    <name>TableDelegate</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="130"/>
        <source>Parameter</source>
        <translation>पैरामीटर</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="151"/>
        <source>Value</source>
        <translation>मान</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="514"/>
        <source>(Custom Icon)</source>
        <translation>(कस्टम आइकन)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="639"/>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="645"/>
        <source>Auto</source>
        <translation>स्वतः</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="813"/>
        <source>No</source>
        <translation>नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="813"/>
        <source>Yes</source>
        <translation>हाँ</translation>
    </message>
</context>
<context>
    <name>Taskbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="66"/>
        <source>Start Menu</source>
        <translation>स्टार्ट मेनू</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="194"/>
        <source>Menu</source>
        <translation>मेनू</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="230"/>
        <source>Search…</source>
        <translation>खोजें…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="496"/>
        <source>Settings</source>
        <translation>सेटिंग्स</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="497"/>
        <source>Console</source>
        <translation>कंसोल</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="498"/>
        <source>Notifications</source>
        <translation>सूचनाएं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="499"/>
        <source>Clock</source>
        <translation>घड़ी</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="500"/>
        <source>Stopwatch</source>
        <translation>स्टॉपवॉच</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="502"/>
        <source>AI Assistant</source>
        <translation>AI असिस्टेंट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Resume</source>
        <translation>फिर से शुरू करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Pause</source>
        <translation>रोकें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="949"/>
        <source>MQTT: Connected to %1</source>
        <translation>MQTT: %1 से कनेक्ट हो गया</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="950"/>
        <source>MQTT: Not connected</source>
        <translation>MQTT: कनेक्ट नहीं है</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="974"/>
        <source>MQTT Publisher</source>
        <translation>MQTT Publisher</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="984"/>
        <source>Status:</source>
        <translation>स्थिति:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="992"/>
        <source>Connected</source>
        <translation>कनेक्टेड</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="993"/>
        <source>Disconnected</source>
        <translation>डिस्कनेक्टेड</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1000"/>
        <source>Broker:</source>
        <translation>Broker:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1013"/>
        <source>Mode:</source>
        <translation>मोड:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1026"/>
        <source>Messages sent:</source>
        <translation>भेजे गए संदेश:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1040"/>
        <source>Open MQTT Settings</source>
        <translation>MQTT सेटिंग्स खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="501"/>
        <source>File Transmission</source>
        <translation>फ़ाइल ट्रांसमिशन</translation>
    </message>
    <message>
        <source>Search widgets…</source>
        <translation type="vanished">विजेट खोजें…</translation>
    </message>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">वर्कस्पेस से हटाएं</translation>
    </message>
</context>
<context>
    <name>Terminal</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="141"/>
        <source>Copy</source>
        <translation>कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="149"/>
        <source>Select all</source>
        <translation>सभी चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="155"/>
        <source>Clear</source>
        <translation>साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="253"/>
        <source>Send Data to Device</source>
        <translation>डिवाइस को डेटा भेजें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="428"/>
        <source>Show Timestamp</source>
        <translation>टाइमस्टैम्प दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="436"/>
        <source>Echo</source>
        <translation>Echo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="453"/>
        <source>Emulate VT-100</source>
        <translation>VT-100 एमुलेट करें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="466"/>
        <source>ANSI Colors</source>
        <translation>ANSI रंग</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="486"/>
        <source>Display: %1</source>
        <translation>डिस्प्ले: %1</translation>
    </message>
</context>
<context>
    <name>Tips</name>
    <message>
        <source>Did You Know?</source>
        <translation type="vanished">क्या आप जानते हैं?</translation>
    </message>
    <message>
        <source>Keep your firmware simple by sending raw data and letting Serial Studio parse it in JavaScript, Lua, or code-free Built-In templates.</source>
        <translation type="vanished">अपने फर्मवेयर को सरल रखें, रॉ डेटा भेजें और Serial Studio को JavaScript, Lua, या कोड-फ्री बिल्ट-इन टेम्पलेट्स में पार्स करने दें।</translation>
    </message>
    <message>
        <source>Give each channel its own function to calibrate, filter, or convert units. Offload the math to Serial Studio and keep your firmware lean.</source>
        <translation type="vanished">प्रत्येक चैनल को कैलिब्रेट, फ़िल्टर या यूनिट कन्वर्ट करने के लिए अपना फ़ंक्शन दें। गणित को Serial Studio पर छोड़ें और अपने फर्मवेयर को हल्का रखें।</translation>
    </message>
    <message>
        <source>Need a value your device never sends? A virtual dataset computes its own channel, like power from voltage and current, plotted and logged as data.</source>
        <translation type="vanished">कोई वैल्यू चाहिए जो आपका डिवाइस कभी नहीं भेजता? वर्चुअल डेटासेट अपना चैनल कंप्यूट करता है, जैसे वोल्टेज और करंट से पावर, जो डेटा के रूप में प्लॉट और लॉग होता है।</translation>
    </message>
    <message>
        <source>Catch glitches like a bench scope. Time-axis plots have a sweep and trigger mode, and you can drag the trigger level right on the plot.</source>
        <translation type="vanished">बेंच स्कोप की तरह ग्लिच पकड़ें। टाइम-एक्सिस प्लॉट में स्वीप और ट्रिगर मोड है, और आप ट्रिगर लेवल को सीधे प्लॉट पर ड्रैग कर सकते हैं।</translation>
    </message>
    <message>
        <source>Stop scrolling to find the right widget. Group them into your own workspaces and jump between them from the taskbar search.</source>
        <translation type="vanished">सही विजेट खोजने के लिए स्क्रॉल करना बंद करें। उन्हें अपने वर्कस्पेस में ग्रुप करें और टास्कबार सर्च से उनके बीच जंप करें।</translation>
    </message>
    <message>
        <source>Never lose a test run again. Record sessions to a local database, then browse, tag, and replay them whenever you need them.</source>
        <translation type="vanished">कभी भी टेस्ट रन न खोएं। सेशन को लोकल डेटाबेस में रिकॉर्ड करें, फिर जब भी ज़रूरत हो उन्हें ब्राउज़, टैग और रीप्ले करें।</translation>
    </message>
    <message>
        <source>Hand a polished report to your team in seconds. Export any session to HTML or PDF, complete with charts and min/max/mean stats.</source>
        <translation type="vanished">सेकंडों में अपनी टीम को पॉलिश्ड रिपोर्ट दें। किसी भी सेशन को HTML या PDF में एक्सपोर्ट करें, चार्ट और min/max/mean स्टैट्स के साथ।</translation>
    </message>
    <message>
        <source>Close the loop without extra tooling. Output Controls let you send commands back to your device straight from the dashboard.</source>
        <translation type="vanished">अतिरिक्त टूलिंग के बिना लूप बंद करें। आउटपुट कंट्रोल आपको डैशबोर्ड से सीधे अपने डिवाइस को कमांड भेजने देते हैं।</translation>
    </message>
    <message>
        <source>Build a visualization nobody else has. The Painter widget runs your own script to draw fully custom graphics from incoming data.</source>
        <translation type="vanished">ऐसा विज़ुअलाइज़ेशन बनाएं जो किसी और के पास नहीं है। पेंटर विजेट आने वाले डेटा से पूरी तरह कस्टम ग्राफ़िक्स बनाने के लिए आपकी स्क्रिप्ट चलाता है।</translation>
    </message>
    <message>
        <source>One tool for every link. Serial Studio reads from UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT, and Process I/O.</source>
        <translation type="vanished">हर लिंक के लिए एक टूल। Serial Studio UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, ऑडियो, USB, HID, MQTT, और Process I/O से पढ़ता है।</translation>
    </message>
    <message>
        <source>Skip the terminal dance. Send and receive files over your serial link with the built-in XMODEM, YMODEM, and ZMODEM protocols.</source>
        <translation type="vanished">टर्मिनल डांस छोड़ें। बिल्ट-इन XMODEM, YMODEM, और ZMODEM प्रोटोकॉल के साथ अपने सीरियल लिंक पर फ़ाइलें भेजें और प्राप्त करें।</translation>
    </message>
    <message>
        <source>Already have a Modbus register map or a DBC file? Generate a ready-to-use project from it automatically instead of building one by hand.</source>
        <translation type="vanished">पहले से Modbus रजिस्टर मैप या DBC फ़ाइल है? हाथ से बनाने के बजाय उससे स्वचालित रूप से उपयोग के लिए तैयार प्रोजेक्ट जेनरेट करें।</translation>
    </message>
    <message>
        <source>Describe what you want and let the AI Assistant build it. It can create and edit projects for you across eight model providers.</source>
        <translation type="vanished">बताएं कि आप क्या चाहते हैं और AI असिस्टेंट को बनाने दें। यह आठ मॉडल प्रोवाइडर्स में आपके लिए प्रोजेक्ट बना और संपादित कर सकता है।</translation>
    </message>
    <message>
        <source>Tip %1 of %2</source>
        <translation type="vanished">टिप %1 में से %2</translation>
    </message>
    <message>
        <source>Learn More</source>
        <translation type="vanished">अधिक जानें</translation>
    </message>
    <message>
        <source>Show Tips on Startup</source>
        <translation type="vanished">स्टार्टअप पर टिप्स दिखाएं</translation>
    </message>
    <message>
        <source>Previous</source>
        <translation type="vanished">पिछला</translation>
    </message>
    <message>
        <source>Next</source>
        <translation type="vanished">अगला</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">बंद करें</translation>
    </message>
</context>
<context>
    <name>ToolCallCard</name>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="47"/>
        <source>Awaiting approval</source>
        <translation>अनुमोदन की प्रतीक्षा में</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="48"/>
        <source>Done</source>
        <translation>पूर्ण</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="49"/>
        <source>Error</source>
        <translation>त्रुटि</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="50"/>
        <source>Denied</source>
        <translation>अस्वीकृत</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="51"/>
        <source>Blocked</source>
        <translation>अवरुद्ध</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="52"/>
        <source>Running</source>
        <translation>चल रहा है</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="152"/>
        <source>Approve</source>
        <translation>स्वीकृत करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="158"/>
        <source>Deny</source>
        <translation>अस्वीकार करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="175"/>
        <source>Arguments</source>
        <translation>आर्ग्युमेंट्स</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="212"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="272"/>
        <source>Copy</source>
        <translation>कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="217"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="277"/>
        <source>Copy All</source>
        <translation>सभी कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="225"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="285"/>
        <source>Select All</source>
        <translation>सभी चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="233"/>
        <source>Result</source>
        <translation>परिणाम</translation>
    </message>
</context>
<context>
    <name>Toolbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="197"/>
        <source>Project Editor</source>
        <translation>प्रोजेक्ट एडिटर</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="200"/>
        <source>Open the Project Editor to create or modify your JSON layout</source>
        <translation>अपना JSON लेआउट बनाने या संशोधित करने के लिए प्रोजेक्ट एडिटर खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="214"/>
        <source>Open Project</source>
        <translation>प्रोजेक्ट खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="217"/>
        <source>Open an existing JSON project</source>
        <translation>मौजूदा JSON प्रोजेक्ट खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="226"/>
        <source>Open CSV</source>
        <translation>CSV खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="232"/>
        <source>Play a CSV file as if it were live sensor data</source>
        <translation>CSV फ़ाइल को लाइव सेंसर डेटा की तरह चलाएं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="238"/>
        <source>Open MDF4</source>
        <translation>MDF4 खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="243"/>
        <source>Play an MDF4 file as if it were live sensor data (Pro)</source>
        <translation>MDF4 फ़ाइल को लाइव सेंसर डेटा की तरह चलाएं (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="292"/>
        <source>Sessions</source>
        <translation>सेशन</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="296"/>
        <source>Browse, replay, and export recorded sessions</source>
        <translation>रिकॉर्ड किए गए सेशन ब्राउज़ करें, रीप्ले करें और एक्सपोर्ट करें</translation>
    </message>
    <message>
        <source>Shortcuts</source>
        <translation type="vanished">शॉर्टकट</translation>
    </message>
    <message>
        <source>Create an operator shortcut for the current project</source>
        <translation type="vanished">वर्तमान प्रोजेक्ट के लिए ऑपरेटर शॉर्टकट बनाएं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="302"/>
        <source>Extensions</source>
        <translation>एक्सटेंशन</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="265"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="305"/>
        <source>Browse and install extensions</source>
        <translation>एक्सटेंशन ब्राउज़ करें और इंस्टॉल करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <source>Assistant</source>
        <translation>असिस्टेंट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="264"/>
        <source>Chat with an AI to build and edit your project</source>
        <translation>अपना प्रोजेक्ट बनाने और संपादित करने के लिए AI से चैट करें</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <source>Configure MQTT connection (publish or subscribe)</source>
        <translation type="vanished">MQTT कनेक्शन कॉन्फ़िगर करें (पब्लिश या सब्सक्राइब)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="286"/>
        <source>Build an operator app for the current project</source>
        <translation>वर्तमान प्रोजेक्ट के लिए ऑपरेटर ऐप बनाएं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="319"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="323"/>
        <source>Preferences</source>
        <translation>प्राथमिकताएँ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="327"/>
        <source>Open application settings and preferences</source>
        <translation>एप्लिकेशन सेटिंग्स और प्राथमिकताएँ खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="345"/>
        <source>UART</source>
        <translation>UART</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="351"/>
        <source>Select Serial port (UART) communication</source>
        <translation>सीरियल पोर्ट (UART) संचार चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="362"/>
        <source>Audio</source>
        <translation>ऑडियो</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="366"/>
        <source>Select audio input device (Pro)</source>
        <translation>ऑडियो इनपुट डिवाइस चुनें (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="381"/>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="386"/>
        <source>Select raw USB communication (Pro)</source>
        <translation>रॉ USB संचार चुनें (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="395"/>
        <source>Network</source>
        <translation>नेटवर्क</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="400"/>
        <source>Select TCP/UDP network communication</source>
        <translation>TCP/UDP नेटवर्क संचार चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="412"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="525"/>
        <source>Browse example projects</source>
        <translation>उदाहरण प्रोजेक्ट ब्राउज़ करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="533"/>
        <source>Help Center</source>
        <translation>सहायता केंद्र</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="537"/>
        <source>Browse documentation, FAQ, and wiki</source>
        <translation>दस्तावेज़ीकरण, FAQ और wiki ब्राउज़ करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="417"/>
        <source>Select MODBUS communication (Pro)</source>
        <translation>MODBUS संचार चुनें (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="281"/>
        <source>Deploy</source>
        <translation>डिप्लॉय करें</translation>
    </message>
    <message>
        <source>Build an operator deployment for the current project</source>
        <translation type="vanished">वर्तमान प्रोजेक्ट के लिए ऑपरेटर डिप्लॉयमेंट बनाएं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="431"/>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="436"/>
        <source>Select HID device communication (Pro)</source>
        <translation>HID डिवाइस संचार चुनें (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="446"/>
        <source>Bluetooth</source>
        <translation>Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="450"/>
        <source>Select Bluetooth Low Energy communication</source>
        <translation>Bluetooth Low Energy संचार चुनें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="462"/>
        <source>CAN Bus</source>
        <translation>CAN Bus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="467"/>
        <source>Select CAN Bus communication (Pro)</source>
        <translation>CAN Bus संचार चुनें (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="481"/>
        <source>Process</source>
        <translation>प्रोसेस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="486"/>
        <source>Select process pipe communication (Pro)</source>
        <translation>प्रोसेस पाइप संचार चुनें (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="502"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="506"/>
        <source>About</source>
        <translation>परिचय</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="510"/>
        <source>Show application info and license details</source>
        <translation>एप्लिकेशन जानकारी और लाइसेंस विवरण दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="522"/>
        <source>Examples</source>
        <translation>उदाहरण</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="543"/>
        <source>AI Wiki &amp; Chat</source>
        <translation>AI विकी और चैट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="546"/>
        <source>View detailed documentation and ask questions on DeepWiki</source>
        <translation>DeepWiki पर विस्तृत दस्तावेज़ देखें और प्रश्न पूछें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="586"/>
        <source>Activate</source>
        <translation>सक्रिय करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="590"/>
        <source>Manage license and activate Serial Studio Pro</source>
        <translation>लाइसेंस प्रबंधित करें और Serial Studio Pro सक्रिय करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="627"/>
        <source>Disconnect</source>
        <translation>डिस्कनेक्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <source>Connect</source>
        <translation>कनेक्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="607"/>
        <source>Connect or disconnect from the configured device</source>
        <translation>कॉन्फ़िगर किए गए डिवाइस से कनेक्ट या डिस्कनेक्ट करें</translation>
    </message>
    <message>
        <source>Connect or disconnect from device or MQTT broker</source>
        <translation type="vanished">डिवाइस या MQTT ब्रोकर से कनेक्ट या डिस्कनेक्ट करें</translation>
    </message>
</context>
<context>
    <name>TriggerDialog</name>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="50"/>
        <source>Trigger Settings</source>
        <translation>ट्रिगर सेटिंग्स</translation>
    </message>
    <message>
        <source>Hold the waveform stationary by aligning each sweep to a trigger event.</source>
        <translation type="vanished">प्रत्येक स्वीप को ट्रिगर इवेंट से संरेखित करके तरंगरूप को स्थिर रखें।</translation>
    </message>
    <message>
        <source>Lock a repeating signal in place, like the Auto button on an oscilloscope. Each sweep starts at the same point on the waveform, so it holds still instead of scrolling past.</source>
        <translation type="vanished">एक दोहराए जाने वाले सिग्नल को स्थिर करें, जैसे ऑसिलोस्कोप पर Auto बटन। प्रत्येक स्वीप वेवफॉर्म पर समान बिंदु से शुरू होती है, इसलिए यह स्क्रॉल करने के बजाय स्थिर रहता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="173"/>
        <source>Trigger</source>
        <translation>ट्रिगर</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="vanished">मोड:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="110"/>
        <source>Mode</source>
        <translation>मोड</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Auto</source>
        <translation>स्वतः</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Normal</source>
        <translation>सामान्य</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Single</source>
        <translation>सिंगल</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="158"/>
        <source>Auto free-runs when nothing crosses the level.</source>
        <translation>Auto स्तर को पार किए बिना मुक्त रूप से चलता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="159"/>
        <source>Normal waits for a crossing.</source>
        <translation>Normal क्रॉसिंग की प्रतीक्षा करता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="160"/>
        <source>Single captures one sweep, then stops.</source>
        <translation>Single एक स्वीप कैप्चर करता है, फिर रुक जाता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="241"/>
        <source>Slope:</source>
        <translation>स्लोप:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="273"/>
        <source>Trigger on a downward crossing</source>
        <translation>नीचे की ओर क्रॉसिंग पर ट्रिगर करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="319"/>
        <source>Timebase:</source>
        <translation>टाइमबेस:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="388"/>
        <source>Leave timebase empty to use the plot's time range; lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each.</source>
        <translation>प्लॉट की समय रेंज उपयोग करने के लिए टाइमबेस खाली छोड़ें; तेज़ सिग्नल पर ज़ूम करने के लिए इसे कम करें। Holdoff प्रत्येक के बाद कुछ समय के लिए नए ट्रिगर को अनदेखा करता है।</translation>
    </message>
    <message>
        <source>Signal:</source>
        <translation type="vanished">सिग्नल:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="230"/>
        <source>Value to cross</source>
        <translation>पार करने का मान</translation>
    </message>
    <message>
        <source>Edge:</source>
        <translation type="vanished">एज:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="254"/>
        <source>Rising</source>
        <translation>राइज़िंग</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="258"/>
        <source>Trigger on an upward crossing</source>
        <translation>ऊपर की ओर क्रॉसिंग पर ट्रिगर करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="269"/>
        <source>Falling</source>
        <translation>फॉलिंग</translation>
    </message>
    <message>
        <source>A new sweep begins each time the signal crosses the level in the chosen direction. Auto also free-runs when no crossing is found.</source>
        <translation type="vanished">हर बार जब सिग्नल चुनी गई दिशा में स्तर को पार करता है तो एक नया स्वीप शुरू होता है। Auto तब भी फ्री-रन करता है जब कोई क्रॉसिंग नहीं मिलती।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="292"/>
        <source>Timing</source>
        <translation>टाइमिंग</translation>
    </message>
    <message>
        <source>Timebase (ms):</source>
        <translation type="vanished">टाइमबेस (ms):</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="332"/>
        <source>Match time range</source>
        <translation>समय रेंज से मिलाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="345"/>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="375"/>
        <source>ms</source>
        <translation>ms</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="351"/>
        <source>Holdoff:</source>
        <translation>होल्डऑफ:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="364"/>
        <source>0</source>
        <translation>0</translation>
    </message>
    <message>
        <source>Timebase sets how much time one sweep shows; leave it empty to use the plot's time range. Lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each one.</source>
        <translation type="vanished">टाइमबेस सेट करता है कि एक स्वीप कितना समय दिखाता है; प्लॉट की समय रेंज उपयोग करने के लिए इसे खाली छोड़ें। तेज़ सिग्नल पर ज़ूम करने के लिए इसे कम करें। Holdoff प्रत्येक ट्रिगर के बाद कुछ समय के लिए नए ट्रिगर को अनदेखा करता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="403"/>
        <source>Capture Next</source>
        <translation>अगला कैप्चर करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="405"/>
        <source>Arm for one more single-shot capture</source>
        <translation>एक और सिंगल-शॉट कैप्चर के लिए पुनः-सक्रिय करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="217"/>
        <source>Level:</source>
        <translation>लेवल:</translation>
    </message>
    <message>
        <source>Trigger level</source>
        <translation type="vanished">ट्रिगर लेवल</translation>
    </message>
    <message>
        <source>Holdoff (ms):</source>
        <translation type="vanished">होल्डऑफ (ms):</translation>
    </message>
    <message>
        <source>Holdoff time</source>
        <translation type="vanished">होल्डऑफ समय</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="197"/>
        <source>Source:</source>
        <translation>स्रोत:</translation>
    </message>
    <message>
        <source>Re-arm</source>
        <translation type="vanished">पुनः-सक्रिय करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="418"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>UART</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="52"/>
        <source>COM Port</source>
        <translation>COM पोर्ट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="79"/>
        <source>Baud Rate</source>
        <translation>बॉड रेट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="163"/>
        <source>Data Bits</source>
        <translation>डेटा बिट्स</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="184"/>
        <source>Parity</source>
        <translation>पैरिटी</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="205"/>
        <source>Stop Bits</source>
        <translation>स्टॉप बिट्स</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="226"/>
        <source>Flow Control</source>
        <translation>फ्लो कंट्रोल</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="258"/>
        <source>Auto Reconnect</source>
        <translation>ऑटो रीकनेक्ट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="276"/>
        <source>Send DTR Signal</source>
        <translation>DTR सिग्नल भेजें</translation>
    </message>
</context>
<context>
    <name>UI::AlarmMonitor</name>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="195"/>
        <source>Alarm</source>
        <translation>अलार्म</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>critical</source>
        <translation>गंभीर</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>warning</source>
        <translation>चेतावनी</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="200"/>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation>मान %1%2 %3 बैंड (%4–%5) में प्रवेश कर गया।</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="205"/>
        <source>Alarms</source>
        <translation>अलार्म</translation>
    </message>
</context>
<context>
    <name>UI::Dashboard</name>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1717"/>
        <source>Console</source>
        <translation>कंसोल</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1725"/>
        <source>Notifications</source>
        <translation>सूचनाएं</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1733"/>
        <source>Clock</source>
        <translation>घड़ी</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1740"/>
        <source>Stopwatch</source>
        <translation>स्टॉपवॉच</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1786"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1801"/>
        <source>%1 (Fallback)</source>
        <translation>%1 (फॉलबैक)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1823"/>
        <source>LED Panel (%1)</source>
        <translation>LED पैनल (%1)</translation>
    </message>
</context>
<context>
    <name>UI::DashboardWidget</name>
    <message>
        <location filename="../../src/UI/DashboardWidget.cpp" line="161"/>
        <source>Invalid</source>
        <translation>अमान्य</translation>
    </message>
</context>
<context>
    <name>UI::WindowManager</name>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1069"/>
        <source>Select Background Image</source>
        <translation>पृष्ठभूमि छवि चुनें</translation>
    </message>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1071"/>
        <source>Images (*.png *.jpg *.jpeg *.bmp)</source>
        <translation>छवियाँ (*.png *.jpg *.jpeg *.bmp)</translation>
    </message>
</context>
<context>
    <name>USB</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="50"/>
        <source>USB Device</source>
        <translation>USB डिवाइस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="80"/>
        <source>Transfer Mode</source>
        <translation>ट्रांसफर मोड</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="89"/>
        <source>Bulk Stream</source>
        <translation>बल्क स्ट्रीम</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="90"/>
        <source>Advanced (Bulk + Control)</source>
        <translation>उन्नत (बल्क + नियंत्रण)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="91"/>
        <source>Isochronous</source>
        <translation>आइसोक्रोनस</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="143"/>
        <source>Connect to USB devices using bulk, control, or isochronous transfers. Suitable for data loggers, custom firmware devices, and USB instruments.</source>
        <translation>बल्क, नियंत्रण, या आइसोक्रोनस ट्रांसफर का उपयोग करके USB डिवाइस से कनेक्ट करें। डेटा लॉगर, कस्टम फर्मवेयर डिवाइस, और USB उपकरणों के लिए उपयुक्त।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="152"/>
        <source>USB specifications (USB.org)</source>
        <translation>USB विनिर्देश (USB.org)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="169"/>
        <source>IN Endpoint</source>
        <translation>IN एंडपॉइंट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="205"/>
        <source>OUT Endpoint</source>
        <translation>OUT एंडपॉइंट</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="241"/>
        <source>Max Packet Size</source>
        <translation>अधिकतम पैकेट आकार</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="301"/>
        <source>Control Transfers Enabled</source>
        <translation>नियंत्रण स्थानांतरण सक्षम</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="310"/>
        <source>Sending incorrect control requests may crash or damage connected hardware. Use with caution.</source>
        <translation>गलत नियंत्रण अनुरोध भेजने से कनेक्टेड हार्डवेयर क्रैश या क्षतिग्रस्त हो सकता है। सावधानी से उपयोग करें।</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="317"/>
        <source>Learn about USB control transfers</source>
        <translation>USB नियंत्रण स्थानांतरण के बारे में जानें</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="351"/>
        <source>Packet size should match the maximum transfer size reported by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).</source>
        <translation>पैकेट आकार एंडपॉइंट द्वारा रिपोर्ट किए गए अधिकतम स्थानांतरण आकार से मेल खाना चाहिए। सामान्य मान: 192 B (FS ऑडियो), 1024 B (HS)।</translation>
    </message>
</context>
<context>
    <name>Updater</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="477"/>
        <source>Would you like to download the update now?</source>
        <translation>क्या आप अभी अपडेट डाउनलोड करना चाहते हैं?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="479"/>
        <source>Would you like to download the update now?&lt;br /&gt;This is a mandatory update, exiting now will close the application.</source>
        <translation>क्या आप अभी अपडेट डाउनलोड करना चाहते हैं?&lt;br /&gt;यह अनिवार्य अपडेट है, अभी बाहर निकलने से एप्लिकेशन बंद हो जाएगा।</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="485"/>
        <source>&lt;strong&gt;Change log:&lt;/strong&gt;&lt;br/&gt;%1</source>
        <translation>&lt;strong&gt;परिवर्तन लॉग:&lt;/strong&gt;&lt;br/&gt;%1</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="488"/>
        <source>Version %1 of %2 has been released!</source>
        <translation>%2 का संस्करण %1 जारी हो गया है!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="520"/>
        <source>No updates are available for the moment</source>
        <translation>फिलहाल कोई अपडेट उपलब्ध नहीं है</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="522"/>
        <source>Congratulations! You are running the latest version of %1</source>
        <translation>बधाई हो! आप %1 का नवीनतम संस्करण चला रहे हैं</translation>
    </message>
</context>
<context>
    <name>UserTableView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="169"/>
        <source>Add Register</source>
        <translation>रजिस्टर जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="172"/>
        <source>Add register</source>
        <translation>रजिस्टर जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="179"/>
        <source>Insert Constant</source>
        <translation>स्थिरांक डालें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="182"/>
        <source>Insert constant</source>
        <translation>स्थिरांक डालें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="189"/>
        <source>Import</source>
        <translation>इम्पोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="192"/>
        <source>Import registers from CSV</source>
        <translation>CSV से रजिस्टर इम्पोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="199"/>
        <source>Export</source>
        <translation>एक्सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="202"/>
        <source>Export registers to CSV</source>
        <translation>रजिस्टर को CSV में एक्सपोर्ट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="214"/>
        <source>Rename</source>
        <translation>नाम बदलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="217"/>
        <source>Rename table</source>
        <translation>टेबल का नाम बदलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="224"/>
        <source>Delete</source>
        <translation>डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="227"/>
        <source>Delete table</source>
        <translation>टेबल डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="241"/>
        <source>Help</source>
        <translation>सहायता</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="246"/>
        <source>Open help documentation for shared memory</source>
        <translation>शेयर्ड मेमोरी के लिए सहायता दस्तावेज़ खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="286"/>
        <source>Permissions</source>
        <translation>अनुमतियाँ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="287"/>
        <source>Register Name</source>
        <translation>रजिस्टर का नाम</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="288"/>
        <source>Default Value</source>
        <translation>डिफ़ॉल्ट वैल्यू</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="327"/>
        <source>Read-Only</source>
        <translation>केवल-पढ़ने योग्य</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="327"/>
        <source>Read/Write</source>
        <translation>पढ़ें/लिखें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="465"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>एक्सेस कोड %1 को क्लिपबोर्ड पर कॉपी करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="498"/>
        <source>Delete register</source>
        <translation>रजिस्टर डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="515"/>
        <source>No registers.</source>
        <translation>कोई रजिस्टर नहीं।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="565"/>
        <source>Register access code copied</source>
        <translation>रजिस्टर एक्सेस कोड कॉपी किया गया</translation>
    </message>
</context>
<context>
    <name>Waterfall</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="237"/>
        <source>Show Colorbar</source>
        <translation>Colorbar दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="250"/>
        <source>Show Axes &amp; Grid</source>
        <translation>अक्ष और ग्रिड दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="263"/>
        <source>Show Crosshair</source>
        <translation>क्रॉसहेयर दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Pause</source>
        <translation>रोकें</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Resume</source>
        <translation>फिर से शुरू करें</translation>
    </message>
    <message>
        <source>Clear History</source>
        <translation type="vanished">इतिहास साफ़ करें</translation>
    </message>
</context>
<context>
    <name>WebView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="70"/>
        <source>Web View Unavailable</source>
        <translation>वेब व्यू अनुपलब्ध</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="81"/>
        <source>This build of Serial Studio was compiled without Qt WebEngine, so web pages cannot be displayed.</source>
        <translation>Serial Studio का यह बिल्ड QT WebEngine के बिना कंपाइल किया गया था, इसलिए वेब पेज प्रदर्शित नहीं किए जा सकते।</translation>
    </message>
</context>
<context>
    <name>Welcome</name>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="177"/>
        <source>Welcome to %1!</source>
        <translation>%1 में आपका स्वागत है!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="188"/>
        <source>Serial Studio is a powerful real-time visualization tool, built for engineers, students, and makers.</source>
        <translation>Serial Studio एक शक्तिशाली रियल-टाइम विज़ुअलाइज़ेशन टूल है, जो इंजीनियरों, छात्रों और मेकर्स के लिए बनाया गया है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="199"/>
        <source>You can start a fully-functional 14-day trial, activate it with your license key, or download and compile the GPLv3 source code yourself.</source>
        <translation>आप पूर्ण-कार्यात्मक 14-दिन का ट्रायल शुरू कर सकते हैं, अपनी लाइसेंस की से इसे सक्रिय कर सकते हैं, या GPLv3 सोर्स कोड डाउनलोड करके स्वयं कंपाइल कर सकते हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="209"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="394"/>
        <source>Buying Pro supports the author directly and helps fund future development.</source>
        <translation>Pro खरीदने से लेखक को सीधे समर्थन मिलता है और भविष्य के विकास में मदद मिलती है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="217"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="402"/>
        <source>Building the GPLv3 version yourself helps grow the community and encourages technical contributions.</source>
        <translation>GPLv3 संस्करण स्वयं बनाने से समुदाय बढ़ता है और तकनीकी योगदान को प्रोत्साहन मिलता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="239"/>
        <source>Please wait…</source>
        <translation>कृपया प्रतीक्षा करें…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="275"/>
        <source>%1 days remaining in your trial.</source>
        <translation>आपके ट्रायल में %1 दिन शेष हैं।</translation>
    </message>
    <message>
        <source>You’re currently using the fully-featured trial of %1 Pro. It’s valid for 14 days of personal, non-commercial use.</source>
        <translation type="vanished">आप वर्तमान में %1 Pro के पूर्ण-सुविधा युक्त ट्रायल का उपयोग कर रहे हैं। यह व्यक्तिगत, गैर-व्यावसायिक उपयोग के लिए 14 दिनों तक मान्य है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="285"/>
        <source>You're currently using the fully-featured trial of %1 Pro. It's valid for 14 days of personal, non-commercial use.</source>
        <translation>आप वर्तमान में %1 Pro के पूर्ण-सुविधा युक्त ट्रायल का उपयोग कर रहे हैं। यह व्यक्तिगत, गैर-व्यावसायिक उपयोग के लिए 14 दिनों तक मान्य है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="296"/>
        <source>Upgrade to a paid plan to keep using Serial Studio Pro.</source>
        <translation>Serial Studio Pro का उपयोग जारी रखने के लिए सशुल्क योजना में अपग्रेड करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="304"/>
        <source>Or, compile the GPLv3 source code to use it for free.</source>
        <translation>या, इसे निःशुल्क उपयोग करने के लिए GPLv3 सोर्स कोड कंपाइल करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="312"/>
        <source>To see available subscription plans, click "Upgrade Now" below.</source>
        <translation>उपलब्ध सब्सक्रिप्शन योजनाएं देखने के लिए, नीचे "अभी अपग्रेड करें" पर क्लिक करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="333"/>
        <source>Don't nag me about the trial.
I understand that when it ends, I'll need to buy a license or build the GPLv3 version.</source>
        <translation>ट्रायल के बारे में मुझे याद न दिलाएं।
मैं समझता हूं कि जब यह समाप्त होगा, तो मुझे लाइसेंस खरीदना होगा या GPLv3 संस्करण बनाना होगा।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="364"/>
        <source>Your %1 trial has expired.</source>
        <translation>आपका %1 ट्रायल समाप्त हो गया है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="374"/>
        <source>Your trial period has ended. To continue using %1 with all Pro features, please upgrade to a paid plan.</source>
        <translation>आपकी ट्रायल अवधि समाप्त हो गई है। सभी Pro सुविधाओं के साथ %1 का उपयोग जारी रखने के लिए, कृपया सशुल्क योजना में अपग्रेड करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="385"/>
        <source>If you prefer, you can also compile the open-source version under the GPLv3 license.</source>
        <translation>यदि आप चाहें, तो GPLv3 लाइसेंस के तहत ओपन-सोर्स संस्करण को भी संकलित कर सकते हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="413"/>
        <source>Thank you for trying %1!</source>
        <translation>%1 को आज़माने के लिए धन्यवाद!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="455"/>
        <source>Upgrade Now</source>
        <translation>अभी अपग्रेड करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="464"/>
        <source>Activate</source>
        <translation>सक्रिय करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Open in Limited Mode</source>
        <translation>सीमित मोड में खोलें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Continue</source>
        <translation>जारी रखें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Start Trial</source>
        <translation>ट्रायल प्रारंभ करें</translation>
    </message>
</context>
<context>
    <name>WhatsNew</name>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="31"/>
        <source>What's New in %1</source>
        <translation>%1 में नया क्या है</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="58"/>
        <source>Lua &amp; Built-In Parsers</source>
        <translation>Lua और बिल्ट-इन पार्सर</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="59"/>
        <source>Parse frames in Lua 5.4 or with code-free Built-In templates, alongside JavaScript.</source>
        <translation>Lua 5.4 में फ़्रेम पार्स करें या JavaScript के साथ कोड-फ्री बिल्ट-इन टेम्पलेट्स का उपयोग करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="67"/>
        <source>AI Assistant</source>
        <translation>AI असिस्टेंट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="68"/>
        <source>An in-app assistant across eight providers that can build and edit projects for you.</source>
        <translation>आठ प्रदाताओं में एक इन-ऐप असिस्टेंट जो आपके लिए प्रोजेक्ट बना और संपादित कर सकता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="76"/>
        <source>Device Control Loops</source>
        <translation>डिवाइस नियंत्रण लूप</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="77"/>
        <source>Automate your device with an Arduino-style setup() and loop() routine that can read, write, and drive the dashboard.</source>
        <translation>Arduino-शैली setup() और loop() रूटीन के साथ अपने डिवाइस को स्वचालित करें जो डैशबोर्ड को पढ़, लिख और संचालित कर सकता है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="85"/>
        <source>Oscilloscope Sweep &amp; Trigger</source>
        <translation>ऑसिलोस्कोप स्वीप और ट्रिगर</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="86"/>
        <source>Scope-style sweep with an animated trigger cursor you can drag on the plot.</source>
        <translation>स्कोप-स्टाइल स्वीप एक एनिमेटेड ट्रिगर कर्सर के साथ जिसे आप प्लॉट पर ड्रैग कर सकते हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="94"/>
        <source>Output Controls</source>
        <translation>आउटपुट कंट्रोल्स</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="95"/>
        <source>Transmit back to your device with control widgets and a protocol-helper engine.</source>
        <translation>कंट्रोल विजेट्स और प्रोटोकॉल-हेल्पर इंजन के साथ अपने डिवाइस पर वापस ट्रांसमिट करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="103"/>
        <source>Dashboard Workspaces</source>
        <translation>डैशबोर्ड वर्कस्पेसेज़</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="104"/>
        <source>Group widgets into your own dashboards and find them from the taskbar search.</source>
        <translation>विजेट्स को अपने डैशबोर्ड में समूहित करें और उन्हें टास्कबार सर्च से खोजें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="112"/>
        <source>Session Database &amp; Reports</source>
        <translation>सेशन डेटाबेस और रिपोर्ट्स</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="113"/>
        <source>Record sessions to SQLite, replay them, and export HTML or PDF reports.</source>
        <translation>सेशन को SQLITE में रिकॉर्ड करें, उन्हें रीप्ले करें, और HTML या PDF रिपोर्ट्स एक्सपोर्ट करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="121"/>
        <source>Operator Deployments</source>
        <translation>ऑपरेटर डिप्लॉयमेंट्स</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="122"/>
        <source>Ship a locked-down, single-purpose build to operators with a runtime-only mode.</source>
        <translation>रनटाइम-ओनली मोड के साथ ऑपरेटर्स के लिए लॉक-डाउन, सिंगल-पर्पज बिल्ड शिप करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="130"/>
        <source>New Dashboard Widgets</source>
        <translation>नए डैशबोर्ड विजेट्स</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="131"/>
        <source>Gauge and Meter faces with live readouts, plus Clock, Stopwatch, and Waterfall.</source>
        <translation>लाइव रीडआउट के साथ गेज और मीटर फेसेस, साथ ही क्लॉक, स्टॉपवॉच, और वॉटरफॉल।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="139"/>
        <source>Dataset Transforms</source>
        <translation>डेटासेट ट्रांसफॉर्म्स</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="140"/>
        <source>Calibrate, filter, and convert each channel with a per-dataset function, or add virtual datasets that compute new channels.</source>
        <translation>प्रत्येक चैनल को प्रति-डेटासेट फ़ंक्शन से कैलिब्रेट, फ़िल्टर और कन्वर्ट करें, या वर्चुअल डेटासेट जोड़ें जो नए चैनल कंप्यूट करते हैं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="148"/>
        <source>Painter Widget</source>
        <translation>पेंटर विजेट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="149"/>
        <source>Draw fully custom graphics from incoming data with your own drawing script.</source>
        <translation>अपनी स्वयं की ड्रॉइंग स्क्रिप्ट से आने वाले डेटा से पूरी तरह कस्टम ग्राफ़िक्स बनाएं।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="157"/>
        <source>Data Tables</source>
        <translation>डेटा टेबल</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="158"/>
        <source>Live register-style tables with virtual datasets and editable cells.</source>
        <translation>वर्चुअल डेटासेट और संपादन योग्य सेल के साथ लाइव रजिस्टर-स्टाइल टेबल।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="166"/>
        <source>Image Widget</source>
        <translation>इमेज विजेट</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="167"/>
        <source>Decode and display image frames streamed straight from your device.</source>
        <translation>अपने डिवाइस से सीधे स्ट्रीम किए गए इमेज फ़्रेम को डिकोड और प्रदर्शित करें।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="175"/>
        <source>Notifications &amp; Alarms</source>
        <translation>नोटिफ़िकेशन और अलार्म</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="176"/>
        <source>Multi-band dataset alarms with severity tiers, routed to the Notification Center.</source>
        <translation>गंभीरता स्तरों के साथ मल्टी-बैंड डेटासेट अलार्म, नोटिफ़िकेशन सेंटर पर रूट किए गए।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="184"/>
        <source>Project Lock</source>
        <translation>प्रोजेक्ट लॉक करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="185"/>
        <source>Lock a project to separate operator use from editing, with an access code.</source>
        <translation>प्रोजेक्ट को एक्सेस कोड के साथ लॉक करें ताकि ऑपरेटर उपयोग को संपादन से अलग किया जा सके।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="193"/>
        <source>MQTT, Protobuf &amp; File Transfer</source>
        <translation>MQTT, Protobuf और फ़ाइल ट्रांसफर</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="194"/>
        <source>MQTT input and publishing, Protobuf import, and XMODEM / YMODEM / ZMODEM transfers.</source>
        <translation>MQTT इनपुट और पब्लिशिंग, Protobuf इम्पोर्ट, और XMODEM / YMODEM / ZMODEM ट्रांसफर।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="240"/>
        <source>Welcome to %1!</source>
        <translation>%1 में आपका स्वागत है!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="250"/>
        <source>Here's what's new in version %1.</source>
        <translation>संस्करण %1 में नया क्या है।</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="423"/>
        <source>Show on Startup</source>
        <translation>स्टार्टअप पर दिखाएं</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="430"/>
        <source>Close</source>
        <translation>बंद करें</translation>
    </message>
</context>
<context>
    <name>WidgetDelegate</name>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">कार्यस्थान से हटाएं</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml" line="345"/>
        <source>Device Disconnected</source>
        <translation>डिवाइस डिस्कनेक्ट हो गया</translation>
    </message>
</context>
<context>
    <name>Widgets::Bar</name>
    <message>
        <source>Alarm</source>
        <translation type="vanished">अलार्म</translation>
    </message>
    <message>
        <source>critical</source>
        <translation type="vanished">गंभीर</translation>
    </message>
    <message>
        <source>warning</source>
        <translation type="vanished">चेतावनी</translation>
    </message>
    <message>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation type="vanished">मान %1%2 %3 बैंड (%4–%5) में प्रवेश कर गया।</translation>
    </message>
    <message>
        <source>Value %1%2 reached the high alarm %3%2</source>
        <translation type="vanished">मान %1%2 उच्च अलार्म %3%2 तक पहुंच गया</translation>
    </message>
    <message>
        <source>Value %1%2 reached the low alarm %3%2</source>
        <translation type="vanished">मान %1%2 निम्न अलार्म %3%2 तक पहुंच गया</translation>
    </message>
    <message>
        <source>Alarms</source>
        <translation type="vanished">अलार्म</translation>
    </message>
</context>
<context>
    <name>Widgets::Compass</name>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="166"/>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="189"/>
        <source>N</source>
        <translation>उ</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="169"/>
        <source>NE</source>
        <translation>उ-पू</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="172"/>
        <source>E</source>
        <translation>पू</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="175"/>
        <source>SE</source>
        <translation>दक्षिण-पू</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="178"/>
        <source>S</source>
        <translation>द</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="181"/>
        <source>SW</source>
        <translation>दक्षिण-प</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="184"/>
        <source>W</source>
        <translation>प</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="187"/>
        <source>NW</source>
        <translation>उत्तर-प</translation>
    </message>
</context>
<context>
    <name>Widgets::DataGrid</name>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="132"/>
        <source>Title</source>
        <translation>शीर्षक</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="133"/>
        <source>Value</source>
        <translation>मान</translation>
    </message>
    <message>
        <source>Awaiting data…</source>
        <translation type="vanished">डेटा की प्रतीक्षा में…</translation>
    </message>
</context>
<context>
    <name>Widgets::GPS</name>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery</source>
        <translation>सैटेलाइट इमेजरी</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery with Labels</source>
        <translation>लेबल के साथ सैटेलाइट इमेजरी</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Street Map</source>
        <translation>स्ट्रीट मैप</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Topographic Map</source>
        <translation>टोपोग्राफिक मैप</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Terrain</source>
        <translation>टेरेन</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Light Gray Canvas</source>
        <translation>लाइट ग्रे कैनवास</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>Dark Gray Canvas</source>
        <translation>डार्क ग्रे कैनवास</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>National Geographic</source>
        <translation>नेशनल ज्योग्राफिक</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="373"/>
        <source>Additional map layers are available only for Pro users.</source>
        <translation>अतिरिक्त मैप लेयर केवल Pro उपयोगकर्ताओं के लिए उपलब्ध हैं।</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="374"/>
        <source>We can't offer unrestricted access because the ArcGIS API key incurs real costs.</source>
        <translation>हम अप्रतिबंधित एक्सेस प्रदान नहीं कर सकते क्योंकि ArcGIS API की की वास्तविक लागत आती है।</translation>
    </message>
</context>
<context>
    <name>Widgets::MultiPlot</name>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Time (s)</source>
        <translation>समय (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Samples</source>
        <translation>सैंपल</translation>
    </message>
</context>
<context>
    <name>Widgets::Output::Base</name>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="168"/>
        <source>Transmit script timed out after %1 ms</source>
        <translation>ट्रांसमिट स्क्रिप्ट %1 ms के बाद टाइमआउट हो गई</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="184"/>
        <source>Payload exceeds maximum size</source>
        <translation>पेलोड अधिकतम साइज़ से अधिक है</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="90"/>
        <source>Time (s)</source>
        <translation>समय (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="116"/>
        <source>Samples</source>
        <translation>सैंपल</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot3D</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot3D.cpp" line="950"/>
        <source>Grid Interval: %1 unit(s)</source>
        <translation>ग्रिड अंतराल: %1 यूनिट</translation>
    </message>
</context>
<context>
    <name>Widgets::Waterfall</name>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="395"/>
        <source>Viridis</source>
        <translation>Viridis</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="397"/>
        <source>Inferno</source>
        <translation>Inferno</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="399"/>
        <source>Magma</source>
        <translation>Magma</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="401"/>
        <source>Plasma</source>
        <translation>Plasma</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="403"/>
        <source>Turbo</source>
        <translation>Turbo</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="405"/>
        <source>Jet</source>
        <translation>Jet</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="407"/>
        <source>Hot</source>
        <translation>Hot</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="409"/>
        <source>Grayscale</source>
        <translation>ग्रेस्केल</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="411"/>
        <source>Unknown</source>
        <translation>अज्ञात</translation>
    </message>
</context>
<context>
    <name>WorkspaceDialog</name>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="51"/>
        <source>Edit Workspace</source>
        <translation>कार्यस्थान संपादित करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="52"/>
        <source>New Workspace</source>
        <translation>नया कार्यस्थान</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="157"/>
        <source>Name:</source>
        <translation>नाम:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="166"/>
        <source>My Workspace</source>
        <translation>मेरा कार्यस्थान</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="181"/>
        <source>Select widgets to include:</source>
        <translation>शामिल करने के लिए विजेट चुनें:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="187"/>
        <source>Filter widgets…</source>
        <translation>विजेट फ़िल्टर करें…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="302"/>
        <source>Cancel</source>
        <translation>रद्द करें</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="309"/>
        <source>OK</source>
        <translation>ठीक है</translation>
    </message>
</context>
<context>
    <name>WorkspaceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="41"/>
        <source>Workspace</source>
        <translation>कार्यस्थान</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="129"/>
        <source>Add Widget</source>
        <translation>विजेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="131"/>
        <source>Add widget to workspace</source>
        <translation>कार्यस्थान में विजेट जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="142"/>
        <source>Rename</source>
        <translation>नाम बदलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="144"/>
        <source>Rename workspace</source>
        <translation>कार्यस्थान का नाम बदलें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="153"/>
        <source>Delete</source>
        <translation>डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="155"/>
        <source>Delete workspace</source>
        <translation>कार्यस्थान डिलीट करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="177"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="183"/>
        <source>Group</source>
        <translation>समूह</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="178"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="184"/>
        <source>Widget</source>
        <translation>विजेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="179"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="185"/>
        <source>Type</source>
        <translation>प्रकार</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="229"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="267"/>
        <source>(unknown)</source>
        <translation>(अज्ञात)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="247"/>
        <source>(group widget)</source>
        <translation>(समूह विजेट)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="297"/>
        <source>Remove widget from workspace</source>
        <translation>वर्कस्पेस से विजेट हटाएं</translation>
    </message>
    <message>
        <source>Remove from workspace</source>
        <translation type="vanished">कार्यस्थान से हटाएं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="317"/>
        <source>No widgets in this workspace.</source>
        <translation>इस कार्यस्थान में कोई विजेट नहीं है।</translation>
    </message>
</context>
<context>
    <name>WorkspacesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="33"/>
        <source>Workspaces</source>
        <translation>वर्कस्पेसेज़</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="127"/>
        <source>Customize</source>
        <translation>अनुकूलित करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="129"/>
        <source>Edit workspaces manually</source>
        <translation>वर्कस्पेसेज़ को मैन्युअल रूप से संपादित करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="145"/>
        <source>Move Up</source>
        <translation>ऊपर ले जाएं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="147"/>
        <source>Move the selected workspace up</source>
        <translation>चयनित कार्यस्थान को ऊपर ले जाएं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="158"/>
        <source>Move Down</source>
        <translation>नीचे ले जाएं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="160"/>
        <source>Move the selected workspace down</source>
        <translation>चयनित कार्यस्थान को नीचे ले जाएं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="171"/>
        <source>Add Workspace</source>
        <translation>वर्कस्पेस जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="173"/>
        <source>Add workspace</source>
        <translation>वर्कस्पेस जोड़ें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="182"/>
        <source>Cleanup</source>
        <translation>साफ़ करें</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="185"/>
        <source>Remove %1 widget reference(s) whose target group or dataset no longer exists</source>
        <translation>%1 विजेट संदर्भ हटाएं जिनका लक्ष्य ग्रुप या डेटासेट अब मौजूद नहीं है</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="188"/>
        <source>No stale widget references in any workspace</source>
        <translation>किसी भी वर्कस्पेस में कोई पुराना विजेट संदर्भ नहीं</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="203"/>
        <source>Title</source>
        <translation>शीर्षक</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="204"/>
        <source>Widgets</source>
        <translation>विजेट</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="276"/>
        <source>No workspaces. Add one with the toolbar above, or reset to the auto layout.</source>
        <translation>कोई वर्कस्पेस नहीं। ऊपर टूलबार से एक जोड़ें, या ऑटो लेआउट पर रीसेट करें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="278"/>
        <source>Project has no eligible groups -- add a group with widgets to populate workspaces.</source>
        <translation>प्रोजेक्ट में कोई योग्य ग्रुप नहीं -- वर्कस्पेसेज़ भरने के लिए विजेट्स के साथ एक ग्रुप जोड़ें।</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="284"/>
        <source>Reset to Auto Layout</source>
        <translation>ऑटो लेआउट पर रीसेट करें</translation>
    </message>
    <message>
        <source>No workspaces.</source>
        <translation type="vanished">कोई वर्कस्पेस नहीं।</translation>
    </message>
</context>
</TS>