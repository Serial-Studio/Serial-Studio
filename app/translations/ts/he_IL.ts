<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="he_IL" sourcelanguage="en_US">
<context>
    <name>AI::AnthropicReply</name>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="164"/>
        <source>Anthropic error</source>
        <translation>שגיאת Anthropic</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="277"/>
        <source>Stream parse error: %1</source>
        <translation>שגיאת ניתוח זרם: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="324"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="327"/>
        <source>Invalid API key (%1)</source>
        <translation>מפתח API לא תקין (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="329"/>
        <source>Rate limited: %1</source>
        <translation>הגבלת קצב: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="331"/>
        <source>Anthropic %1: %2</source>
        <translation>Anthropic %1: %2</translation>
    </message>
</context>
<context>
    <name>AI::Assistant</name>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="191"/>
        <source>Allow AI Device Control?</source>
        <translation>לאפשר שליטה במכשיר דרך AI?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="192"/>
        <source>This lets the AI assistant configure devices, open and close connections, and send data to your hardware.

Every device action still requires your explicit per-call approval in the chat, even when auto-approve is enabled. Only enable this if you trust the configured AI provider with hardware access.</source>
        <translation>זה מאפשר לעוזר ה-AI להגדיר מכשירים, לפתוח ולסגור חיבורים ולשלוח נתונים לחומרה שלך.

כל פעולת מכשיר עדיין דורשת את אישורך המפורש לכל קריאה בצ'אט, גם כאשר אישור אוטומטי מופעל. הפעל זאת רק אם אתה סומך על ספק ה-AI המוגדר עם גישה לחומרה.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="390"/>
        <source>Switch AI provider?</source>
        <translation>להחליף ספק AI?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="391"/>
        <source>Switching to a different provider clears the current conversation. Do you want to continue?</source>
        <translation>מעבר לספק אחר מוחק את השיחה הנוכחית. להמשיך?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="394"/>
        <source>Assistant</source>
        <translation>עוזר</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="431"/>
        <source>AI Assistant is not available in this build</source>
        <translation>עוזר ה-AI אינו זמין בגרסה זו</translation>
    </message>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">עוזר AI דורש רישיון Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="436"/>
        <source>Set an API key first</source>
        <translation>הגדר תחילה מפתח API</translation>
    </message>
</context>
<context>
    <name>AI::Conversation</name>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">עוזר AI דורש רישיון Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="160"/>
        <source>AI Assistant is not available in this build</source>
        <translation>עוזר ה-AI אינו זמין בגרסה זו</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="166"/>
        <source>AI subsystem not initialized</source>
        <translation>תת-מערכת AI לא אותחלה</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="172"/>
        <source>Already busy with a previous request</source>
        <translation>עסוק כבר בבקשה קודמת</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="449"/>
        <source>Tool-call budget reached for this turn; no further tools will run.</source>
        <translation>תקציב קריאות הכלים הושג עבור תור זה; לא יופעלו כלים נוספים.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1691"/>
        <source>You have reached the tool-call budget for this turn. Do not request more tools. Summarize what you found so far, and if the task is incomplete, say which steps remain so the user can tell you to continue.</source>
        <translation>הגעת לתקציב קריאות הכלים עבור תור זה. אל תבקש כלים נוספים. סכם את מה שמצאת עד כה, ואם המשימה לא הושלמה, ציין אילו שלבים נותרו כדי שהמשתמש יוכל להורות לך להמשיך.</translation>
    </message>
    <message>
        <source>Tool-call budget exceeded</source>
        <translation type="vanished">תקציב קריאות כלים חרג מהמותר</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="894"/>
        <source>(The model returned an empty response. Try rephrasing, switching to a different model, or checking that the request is allowed by the provider's safety filters.)</source>
        <translation>(המודל החזיר תגובה ריקה. נסה לנסח מחדש, לעבור למודל אחר, או לבדוק שהבקשה מותרת על-ידי מסנני הבטיחות של הספק.)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="981"/>
        <source>Sending request to %1...</source>
        <translation>שולח בקשה אל %1...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="992"/>
        <source>Provider returned no reply</source>
        <translation>הספק לא החזיר תשובה</translation>
    </message>
</context>
<context>
    <name>AI::GeminiReply</name>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="145"/>
        <source>Prompt blocked by Gemini safety filter: %1</source>
        <translation>הפרומפט נחסם על-ידי מסנן הבטיחות של Gemini: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="187"/>
        <source>Gemini stopped without producing a response: %1</source>
        <translation>Gemini נעצר מבלי לייצר תגובה: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="247"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="250"/>
        <source>Invalid API key (%1)</source>
        <translation>מפתח API לא חוקי (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="252"/>
        <source>Rate limited: %1</source>
        <translation>הגבלת קצב: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="254"/>
        <source>Invalid API key</source>
        <translation>מפתח API לא חוקי</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="256"/>
        <source>Gemini %1: %2</source>
        <translation>Gemini %1: %2</translation>
    </message>
</context>
<context>
    <name>AI::OpenAIReply</name>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="326"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="329"/>
        <source>Invalid API key (%1)</source>
        <translation>מפתח API לא חוקי (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="331"/>
        <source>Rate limited: %1</source>
        <translation>הגבלת קצב: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="333"/>
        <source>%1 %2: %3</source>
        <translation>%1 %2: %3</translation>
    </message>
</context>
<context>
    <name>API::GRPC::GRPCServer</name>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="413"/>
        <source>Export Protobuf File</source>
        <translation>ייצא קובץ Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="415"/>
        <source>Protocol Buffers (*.proto)</source>
        <translation>Protocol Buffers (‎*.proto)</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="461"/>
        <source>Unable to start gRPC server</source>
        <translation>לא ניתן להפעיל שרת GRPC</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="462"/>
        <source>Failed to bind to %1</source>
        <translation>נכשל לקשור ל-%1</translation>
    </message>
</context>
<context>
    <name>API::Server</name>
    <message>
        <location filename="../../src/API/Server.cpp" line="430"/>
        <source>Unable to start API TCP server</source>
        <translation>לא ניתן להפעיל שרת TCP של API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="474"/>
        <source>Allow External API Connections?</source>
        <translation>לאפשר חיבורי API חיצוניים?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="475"/>
        <source>Exposing the API server to external hosts allows other devices on your network to connect to Serial Studio on port 7777.

Only enable this on trusted networks. Untrusted clients may read live data or send commands to your device.</source>
        <translation>חשיפת שרת ה-API למארחים חיצוניים מאפשרת למכשירים אחרים ברשת להתחבר ל-Serial Studio ביציאה 7777.

יש להפעיל אפשרות זו רק ברשתות מהימנות. לקוחות לא מהימנים עלולים לקרוא נתונים חיים או לשלוח פקודות למכשיר.</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="509"/>
        <source>Unable to restart API TCP server</source>
        <translation>לא ניתן להפעיל מחדש שרת TCP של API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="587"/>
        <source>Allow API device control?</source>
        <translation>לאפשר שליטה במכשיר דרך API?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="588"/>
        <source>A program using Serial Studio's local API is requesting to send data to the connected device. Allow API clients to write to the device?</source>
        <translation>תוכנית המשתמשת ב-API המקומי של Serial Studio מבקשת לשלוח נתונים למכשיר המחובר. לאפשר ללקוחות API לכתוב למכשיר?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="591"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1205"/>
        <source>API server</source>
        <translation>שרת API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1205"/>
        <source>Invalid pending connection</source>
        <translation>חיבור ממתין לא חוקי</translation>
    </message>
</context>
<context>
    <name>About</name>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="39"/>
        <source>About</source>
        <translation>אודות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="96"/>
        <source>Version %1</source>
        <translation>גרסה %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="106"/>
        <source>Copyright © %1 %2</source>
        <translation>זכויות יוצרים © %1 %2</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="112"/>
        <source>All Rights Reserved</source>
        <translation>כל הזכויות שמורות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="127"/>
        <source>%1 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

%1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</source>
        <translation>%1 היא תוכנה חופשית: ניתן להפיץ אותה ו/או לשנות אותה בהתאם לתנאי הרישיון הציבורי הכללי של GNU כפי שפורסם על ידי Free Software Foundation; גרסה 3 של הרישיון, או (לפי בחירתך) כל גרסה מאוחרת יותר.

%1 מופצת בתקווה שתהיה שימושית, אך ללא כל אחריות; אפילו ללא האחריות המשתמעת של סחירות או התאמה למטרה מסוימת. ראה את הרישיון הציבורי הכללי של GNU לפרטים נוספים.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="146"/>
        <source>This configuration is licensed for commercial and proprietary use. It may be used in closed-source and commercial applications, subject to the terms of the commercial license.</source>
        <translation>תצורה זו מורשית לשימוש מסחרי וקנייני. ניתן להשתמש בה ביישומים סגורים ומסחריים, בכפוף לתנאי הרישיון המסחרי.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="160"/>
        <source>This configuration is for personal and evaluation purposes only. Commercial use is prohibited unless a valid commercial license is activated.</source>
        <translation>תצורה זו מיועדת לשימוש אישי והערכה בלבד. שימוש מסחרי אסור אלא אם הופעל רישיון מסחרי תקף.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="174"/>
        <source>This software is provided 'as is' without warranty of any kind, express or implied, including but not limited to the warranties of merchantability or fitness for a particular purpose. In no event shall the author be liable for any damages arising from the use of this software.</source>
        <translation>תוכנה זו מסופקת 'כמות שהיא' ללא אחריות מכל סוג, מפורשת או משתמעת, לרבות אך לא רק אחריות לסחירות או התאמה למטרה מסוימת. בשום מקרה המחבר לא יהיה אחראי לנזקים הנובעים משימוש בתוכנה זו.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="195"/>
        <source>Manage License</source>
        <translation>ניהול רישיון</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="203"/>
        <source>Donate</source>
        <translation>תרומה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="214"/>
        <source>Check for Updates</source>
        <translation>בדיקת עדכונים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="223"/>
        <source>What's New</source>
        <translation>מה חדש</translation>
    </message>
    <message>
        <source>Tips &amp;&amp; Tricks</source>
        <translation type="vanished">טיפים וטריקים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="232"/>
        <source>License Agreement</source>
        <translation>הסכם רישיון</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="241"/>
        <source>Report Bug</source>
        <translation>דיווח על באג</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="250"/>
        <source>Acknowledgements</source>
        <translation>אישורים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="259"/>
        <source>Benchmark</source>
        <translation>מדד ביצועים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="267"/>
        <source>Website</source>
        <translation>אתר אינטרנט</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="283"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>Accelerometer</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="181"/>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="182"/>
        <source>Settings</source>
        <translation>הגדרות</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="240"/>
        <source>G-FORCE</source>
        <translation>כוח G</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="276"/>
        <source>PITCH ↕</source>
        <translation>הטיה ↕</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="311"/>
        <source>ROLL ↔</source>
        <translation>גלגול ↔</translation>
    </message>
</context>
<context>
    <name>AccelerometerConfigDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="35"/>
        <source>Accelerometer Configuration</source>
        <translation>תצורת מד תאוצה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="95"/>
        <source>Configure the accelerometer display range and input units.</source>
        <translation>הגדר את טווח התצוגה ויחידות הקלט של מד התאוצה.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="109"/>
        <source>Display Range</source>
        <translation>טווח תצוגה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="130"/>
        <source>Max G:</source>
        <translation>G מקסימלי:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="144"/>
        <source>Enter max G value</source>
        <translation>הזן ערך G מקסימלי</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="164"/>
        <source>Input Configuration</source>
        <translation>תצורת קלט</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="184"/>
        <source>Input already in G</source>
        <translation>הקלט כבר ביחידות G</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="218"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>Acknowledgements</name>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="35"/>
        <source>Acknowledgements</source>
        <translation>תודות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="76"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="87"/>
        <source>About Qt…</source>
        <translation>אודות QT…</translation>
    </message>
</context>
<context>
    <name>ActionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="136"/>
        <source>Change Icon</source>
        <translation>שנה סמל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="138"/>
        <source>Change the icon used for this action</source>
        <translation>שנה את הסמל המשמש לפעולה זו</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="156"/>
        <source>Duplicate</source>
        <translation>שכפל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="160"/>
        <source>Duplicate this action with all its settings</source>
        <translation>שכפל פעולה זו עם כל ההגדרות שלה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="169"/>
        <source>Delete</source>
        <translation>מחק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="171"/>
        <source>Delete this action from the project</source>
        <translation>מחק פעולה זו מהפרויקט</translation>
    </message>
</context>
<context>
    <name>AddWidgetDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="44"/>
        <source>Add Widget</source>
        <translation>הוסף Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="211"/>
        <source>Available Widgets</source>
        <translation>Widgets זמינים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="220"/>
        <source>Click a row to add it to the workspace.</source>
        <translation>לחץ על שורה כדי להוסיף אותה למרחב העבודה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="228"/>
        <source>Search</source>
        <translation>חיפוש</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="247"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="248"/>
        <source>Group</source>
        <translation>קבוצה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="249"/>
        <source>Name</source>
        <translation>שם</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="316"/>
        <source>(entire group)</source>
        <translation>(קבוצה שלמה)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="351"/>
        <source>Already in workspace</source>
        <translation>כבר במרחב העבודה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="352"/>
        <source>Add to workspace</source>
        <translation>הוסף למרחב העבודה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="381"/>
        <source>No widgets available.</source>
        <translation>אין ווידג'טים זמינים.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="382"/>
        <source>No widgets match.</source>
        <translation>אין ווידג'טים תואמים.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="399"/>
        <source>%1 widgets</source>
        <translation>%1 ווידג'טים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="400"/>
        <source>%1 of %2 widgets</source>
        <translation>%1 מתוך %2 ווידג'טים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="404"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>AlarmBandsEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="35"/>
        <source>Alarm Bands</source>
        <translation>תחומי אזעקה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="70"/>
        <source>Info</source>
        <translation>מידע</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="71"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="151"/>
        <source>OK</source>
        <translation>אישור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="72"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="152"/>
        <source>Warning</source>
        <translation>אזהרה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="73"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="153"/>
        <source>Critical</source>
        <translation>קריטי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="82"/>
        <source>Tachometer</source>
        <translation>מד סל"ד</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="84"/>
        <source>Idle</source>
        <translation>סרק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="85"/>
        <source>Operating</source>
        <translation>פעולה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="86"/>
        <source>Caution</source>
        <translation>זהירות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="87"/>
        <source>Redline</source>
        <translation>קו אדום</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="91"/>
        <source>Speedometer</source>
        <translation>מד מהירות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="93"/>
        <source>Cruise</source>
        <translation>שיוט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="94"/>
        <source>Fast</source>
        <translation>מהיר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="95"/>
        <source>Top Speed</source>
        <translation>מהירות מרבית</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="99"/>
        <source>Engine Temperature</source>
        <translation>טמפרטורת מנוע</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="101"/>
        <source>Cold</source>
        <translation>קר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="102"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="111"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="143"/>
        <source>Normal</source>
        <translation>רגיל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="103"/>
        <source>Warm</source>
        <translation>חמים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="104"/>
        <source>Overheat</source>
        <translation>התחממות יתר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="108"/>
        <source>Pressure</source>
        <translation>לחץ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="110"/>
        <source>Vacuum</source>
        <translation>ריק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="112"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="121"/>
        <source>High</source>
        <translation>גבוה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="113"/>
        <source>Burst</source>
        <translation>פרץ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="117"/>
        <source>Battery Voltage</source>
        <translation>מתח סוללה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="119"/>
        <source>Low</source>
        <translation>נמוך</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="120"/>
        <source>Nominal</source>
        <translation>נומינלי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="125"/>
        <source>Fuel Level</source>
        <translation>רמת דלק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="127"/>
        <source>Empty</source>
        <translation>ריק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="128"/>
        <source>Reserve</source>
        <translation>רזרבה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="133"/>
        <source>Signal Strength</source>
        <translation>עוצמת אות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="135"/>
        <source>No Signal</source>
        <translation>אין אות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="136"/>
        <source>Weak</source>
        <translation>חלש</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="137"/>
        <source>Good</source>
        <translation>טוב</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="141"/>
        <source>CPU / System Load</source>
        <translation>עומס מעבד / מערכת</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="144"/>
        <source>Busy</source>
        <translation>עסוק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="145"/>
        <source>Overload</source>
        <translation>עומס יתר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="149"/>
        <source>OK / Warning / Critical</source>
        <translation>תקין / אזהרה / קריטי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="270"/>
        <source>Choose Band Color</source>
        <translation>בחר צבע פס</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="297"/>
        <source>Presets</source>
        <translation>ערכות מוגדרות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="320"/>
        <source>Preset</source>
        <translation>ערכה מוגדרת</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="335"/>
        <source>Choose preset…</source>
        <translation>בחר הגדרה קבועה מראש…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="557"/>
        <source>Reset to severity default</source>
        <translation>אפס לברירת מחדל של חומרה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="571"/>
        <source>Click to choose a color. Right-click to reset to severity default.</source>
        <translation>לחץ כדי לבחור צבע. לחיצה ימנית כדי לאפס לברירת מחדל של חומרה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="572"/>
        <source>Click to choose a custom color.</source>
        <translation>לחץ כדי לבחור צבע מותאם אישית.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="657"/>
        <source>No bands defined. Pick a preset above or add a band to get started.</source>
        <translation>לא הוגדרו רצועות. בחר הגדרה קבועה מראש למעלה או הוסף רצועה כדי להתחיל.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="781"/>
        <source>Apply</source>
        <translation>החל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="784"/>
        <source>Apply changes to the dataset.</source>
        <translation>החל שינויים על מערך הנתונים.</translation>
    </message>
    <message>
        <source>Apply Preset</source>
        <translation type="vanished">החל ערכה מוגדרת</translation>
    </message>
    <message>
        <source>Replace the current bands with the selected preset, scaled to this dataset's range.</source>
        <translation type="vanished">החלפת התחומים הנוכחיים בערכה המוגדרת הנבחרת, מותאמת לטווח של מערך נתונים זה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="346"/>
        <source>Range</source>
        <translation>טווח</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="374"/>
        <source>Bands</source>
        <translation>תחומים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="385"/>
        <source>Add Band</source>
        <translation>הוסף תחום</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="389"/>
        <source>Add a new band continuing from the last one.</source>
        <translation>הוספת תחום חדש בהמשך לתחום האחרון.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="420"/>
        <source>Min</source>
        <translation>מינימום</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="426"/>
        <source>Max</source>
        <translation>מקסימום</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="432"/>
        <source>Severity</source>
        <translation>חומרה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="438"/>
        <source>Color</source>
        <translation>צבע</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="445"/>
        <source>Label</source>
        <translation>תווית</translation>
    </message>
    <message>
        <source>Choose a custom color.</source>
        <translation type="vanished">בחר צבע מותאם אישית.</translation>
    </message>
    <message>
        <source>auto</source>
        <translation type="vanished">אוטומטי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="594"/>
        <source>(optional)</source>
        <translation>(אופציונלי)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="611"/>
        <source>Move up.</source>
        <translation>העבר למעלה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="630"/>
        <source>Move down.</source>
        <translation>העבר למטה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="643"/>
        <source>Remove this band.</source>
        <translation>הסר רצועה זו.</translation>
    </message>
    <message>
        <source>No bands defined. Apply a preset or add a band to get started.</source>
        <translation type="vanished">לא הוגדרו רצועות. החל הגדרה קבועה מראש או הוסף רצועה כדי להתחיל.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="674"/>
        <source>Preview</source>
        <translation>תצוגה מקדימה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="770"/>
        <source>Cancel</source>
        <translation>ביטול</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="772"/>
        <source>Discard changes.</source>
        <translation>ביטול השינויים.</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="vanished">שמור</translation>
    </message>
    <message>
        <source>Save changes to the dataset.</source>
        <translation type="vanished">שמירת השינויים במערך הנתונים.</translation>
    </message>
</context>
<context>
    <name>AssistantPanel</name>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="31"/>
        <source>Assistant</source>
        <translation>עוזר</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="121"/>
        <source>Help me discover Serial Studio's features</source>
        <translation>עזור לי לגלות את התכונות של Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="122"/>
        <source>What can this app do for my telemetry?</source>
        <translation>מה האפליקציה הזו יכולה לעשות עבור הטלמטריה שלי?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="123"/>
        <source>Walk me through what this project already contains</source>
        <translation>הדרך אותי דרך מה שהפרויקט הזה כבר מכיל</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="124"/>
        <source>List the sources in this project</source>
        <translation>הצג את המקורות בפרויקט זה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="127"/>
        <source>What is a session database, and why would I use one?</source>
        <translation>מהו מסד נתוני סשן, ולמה אשתמש בו?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="128"/>
        <source>CSV vs MDF4 export - what is the difference?</source>
        <translation>ייצוא CSV לעומת MDF4 - מה ההבדל?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="129"/>
        <source>What is a frame parser, and when do I need one?</source>
        <translation>מהו מנתח מסגרות, ומתי אני צריך אחד?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="130"/>
        <source>When should I use Lua vs JavaScript for the parser?</source>
        <translation>מתי כדאי להשתמש ב-Lua לעומת JavaScript עבור המנתח?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="131"/>
        <source>Plot, Bar, and Gauge - when to use each?</source>
        <translation>גרף, עמודות ומד - מתי להשתמש בכל אחד?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="132"/>
        <source>What is the difference between a transform and a frame parser?</source>
        <translation>מה ההבדל בין טרנספורמציה למנתח מסגרות?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="135"/>
        <source>Add a UART source for an Arduino</source>
        <translation>הוסף מקור UART עבור Arduino</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="136"/>
        <source>Set up an IMU project from scratch</source>
        <translation>הגדר פרויקט IMU מאפס</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="137"/>
        <source>Configure an MQTT subscriber</source>
        <translation>הגדר מנוי MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="138"/>
        <source>Add a CAN bus source</source>
        <translation>הוסף מקור CAN Bus</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="139"/>
        <source>Set up a Modbus poller</source>
        <translation>הגדר מתשאל Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="140"/>
        <source>Add a network (TCP/UDP) source</source>
        <translation>הוסף מקור רשת (TCP/UDP)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="141"/>
        <source>Write a CSV frame parser for me</source>
        <translation>כתוב עבורי מנתח מסגרות CSV</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="142"/>
        <source>Help me parse a JSON frame</source>
        <translation>עזור לי לנתח מסגרת JSON</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="143"/>
        <source>Add an EMA smoothing transform to a dataset</source>
        <translation>הוסף טרנספורמציה של החלקת EMA למערך נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="144"/>
        <source>Decode hexadecimal frames</source>
        <translation>פענח מסגרות הקסדצימליות</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="145"/>
        <source>Calibrate a sensor with a linear transform</source>
        <translation>כיול חיישן באמצעות טרנספורמציה לינארית</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="148"/>
        <source>Suggest dashboard widgets for my data</source>
        <translation>הצעת ווידג'טים למרכז הבקרה עבור הנתונים שלי</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="149"/>
        <source>Build an executive overview workspace</source>
        <translation>בניית סביבת עבודה לסקירה ניהולית</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="150"/>
        <source>Add a painter widget for a custom visualization</source>
        <translation>הוספת ווידג'ט ציור לוויזואליזציה מותאמת אישית</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="151"/>
        <source>Show Plot, FFT, and Waterfall for one dataset</source>
        <translation>הצגת Plot, FFT ו-Waterfall עבור מערך נתונים אחד</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="152"/>
        <source>Group my datasets into useful workspaces</source>
        <translation>קיבוץ מערכי הנתונים שלי לסביבות עבודה שימושיות</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="203"/>
        <source>How can I help with your project?</source>
        <translation>כיצד אוכל לסייע בפרויקט שלך?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="204"/>
        <source>Set up your API key to get started</source>
        <translation>הגדרת מפתח API כדי להתחיל</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="216"/>
        <source>Describe what you would like to build, and I will configure the sources, groups, datasets, frame parsers, and transforms for you.</source>
        <translation>תאר מה ברצונך לבנות, ואני אגדיר עבורך את המקורות, הקבוצות, מערכי הנתונים, מנתחי ה-Frame וה-Transforms.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="219"/>
        <source>To start chatting, paste an API key for the selected provider. Keys are encrypted on this machine and never leave your computer except to talk to the provider you choose.</source>
        <translation>כדי להתחיל לשוחח, הדבק מפתח API עבור הספק שנבחר. המפתחות מוצפנים במכשיר זה ואינם עוזבים את המחשב שלך אלא כדי לתקשר עם הספק שבחרת.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="240"/>
        <source>Open API Key Setup</source>
        <translation>פתח הגדרת מפתח API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="250"/>
        <source>Get a key from %1</source>
        <translation>קבל מפתח מ-%1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="427"/>
        <source>Drop files or folders to let the assistant read them</source>
        <translation>גרור קבצים או תיקיות כדי לאפשר לעוזר לקרוא אותם</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="472"/>
        <source>Added folder "%1" - readable this session</source>
        <translation>התיקייה "%1" נוספה - ניתנת לקריאה בהפעלה זו</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="473"/>
        <source>Added "%1" - readable this session</source>
        <translation>"%1" נוסף - ניתן לקריאה בהפעלה זו</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="557"/>
        <source>Ask Serial Studio anything…</source>
        <translation>שאל את Serial Studio כל דבר…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="577"/>
        <source>Clear conversation</source>
        <translation>נקה שיחה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="621"/>
        <source>Stop generating</source>
        <translation>עצור יצירה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="622"/>
        <source>Send message (Enter)</source>
        <translation>שלח הודעה (Enter)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="664"/>
        <source>Provider</source>
        <translation>ספק</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="697"/>
        <source>Model selection</source>
        <translation>בחירת מודל</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="743"/>
        <source>Run editing actions without asking each time. Blocked actions stay blocked.</source>
        <translation>הרץ פעולות עריכה ללא שאילת אישור בכל פעם. פעולות חסומות נשארות חסומות.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="745"/>
        <source>Auto-approve edits</source>
        <translation>אשר עריכות אוטומטית</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="761"/>
        <source>Let the AI configure devices, connect/disconnect and send data. Each action still asks for your approval.</source>
        <translation>לאפשר ל-AI להגדיר מכשירים, להתחבר/להתנתק ולשלוח נתונים. כל פעולה עדיין מבקשת את אישורך.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="763"/>
        <source>Allow device control</source>
        <translation>לאפשר שליטה במכשיר</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="789"/>
        <source>Manage API keys</source>
        <translation>ניהול מפתחות API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="810"/>
        <source>Working</source>
        <translation>עובד</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="811"/>
        <source>Ready</source>
        <translation>מוכן</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="812"/>
        <source>  •  cache %1k tok</source>
        <translation>• מטמון %1k אסימונים</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="813"/>
        <source>  •  cache write %1k tok</source>
        <translation>• כתיבת מטמון %1k אסימונים</translation>
    </message>
</context>
<context>
    <name>Audio</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="89"/>
        <source>No Microphone Detected</source>
        <translation>לא זוהה מיקרופון</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="98"/>
        <source>Connect a mic or check your settings</source>
        <translation>חבר מיקרופון או בדוק את ההגדרות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="123"/>
        <source>Input Device</source>
        <translation>התקן קלט</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="142"/>
        <source>Sample Rate</source>
        <translation>קצב דגימה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="230"/>
        <source>Sample Format</source>
        <translation>פורמט דגימה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="180"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="249"/>
        <source>Channels</source>
        <translation>ערוצים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="211"/>
        <source>Output Device</source>
        <translation>התקן פלט</translation>
    </message>
</context>
<context>
    <name>AuthenticateDialog</name>
    <message>
        <source>Dialog</source>
        <translation type="vanished">דיאלוג</translation>
    </message>
    <message>
        <source>Please provide the user name and password for the download location.</source>
        <translation type="vanished">יש לספק שם משתמש וסיסמה עבור מיקום ההורדה.</translation>
    </message>
    <message>
        <source>&amp;User name:</source>
        <translation type="vanished">&amp;שם משתמש:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="vanished">&amp;סיסמה:</translation>
    </message>
</context>
<context>
    <name>AxisRangeDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="47"/>
        <source>Axis Range Configuration</source>
        <translation>הגדרת טווח צירים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="161"/>
        <source>Configure the visible range for the plot axes. Values update in real-time as you type.</source>
        <translation>הגדרת הטווח הנראה עבור צירי הגרף. הערכים מתעדכנים בזמן אמת במהלך ההקלדה.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="169"/>
        <source>X Axis</source>
        <translation>ציר X</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="194"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="265"/>
        <source>Minimum:</source>
        <translation>מינימום:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="206"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="277"/>
        <source>Enter min value</source>
        <translation>הזן ערך מינימום</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="215"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="286"/>
        <source>Maximum:</source>
        <translation>מקסימום:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="227"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="298"/>
        <source>Enter max value</source>
        <translation>הזן ערך מקסימום</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="242"/>
        <source>Y Axis</source>
        <translation>ציר Y</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="317"/>
        <source>Reset</source>
        <translation>אפס</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="327"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>BackupRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="31"/>
        <source>Recover Backup</source>
        <translation>שחזר גיבוי</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="94"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="165"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="166"/>
        <source>Untitled</source>
        <translation>ללא שם</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="97"/>
        <source>Project Loaded</source>
        <translation>פרויקט נטען</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="98"/>
        <source>Auto-save</source>
        <translation>שמירה אוטומטית</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="99"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="119"/>
        <source>Before Restore</source>
        <translation>לפני שחזור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="100"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="106"/>
        <source>Before Delete Dataset</source>
        <translation>לפני מחיקת Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="101"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="107"/>
        <source>Before Delete Group</source>
        <translation>לפני מחיקת קבוצה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="102"/>
        <source>Before New Project</source>
        <translation>לפני פרויקט חדש</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="103"/>
        <source>Before Open Project</source>
        <translation>לפני פתיחת פרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="104"/>
        <source>Before Load JSON</source>
        <translation>לפני טעינת JSON</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="105"/>
        <source>Before Apply Template</source>
        <translation>לפני החלת תבנית</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="108"/>
        <source>Before Delete Action</source>
        <translation>לפני מחיקת פעולה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="109"/>
        <source>Before Delete Output Widget</source>
        <translation>לפני מחיקת ווידג'ט פלט</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="110"/>
        <source>Before Move Dataset</source>
        <translation>לפני העברת Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="111"/>
        <source>Before Move Group</source>
        <translation>לפני העברת קבוצה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="112"/>
        <source>Before Delete Workspace</source>
        <translation>לפני מחיקת סביבת עבודה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="113"/>
        <source>Before Clear All Workspaces</source>
        <translation>לפני ניקוי כל סביבות העבודה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="114"/>
        <source>Before Remove Widget</source>
        <translation>לפני הסרת Widget</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="115"/>
        <source>Before Reorder Workspaces</source>
        <translation>לפני סידור מחדש של סביבות עבודה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="116"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="117"/>
        <source>Before Batch Operation</source>
        <translation>לפני פעולה קבוצתית</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="118"/>
        <source>Before Add Tile</source>
        <translation>לפני הוספת אריח</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="142"/>
        <source>%1 (and %2 more)</source>
        <translation>%1 (ועוד %2)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="159"/>
        <source> The frame parser code also differs and will be replaced.</source>
        <translation>קוד מנתח ה-Frame שונה גם כן ויוחלף.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="164"/>
        <source>Title changes from “%1” to “%2”. Group structure unchanged.</source>
        <translation>שינוי כותרת מ-"%1" ל-"%2". מבנה הקבוצה ללא שינוי.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="169"/>
        <source>Same groups and datasets, but the frame parser code differs — restoring will replace it.</source>
        <translation>אותן קבוצות ומערכי נתונים, אך קוד מנתח ה-Frame שונה — השחזור יחליף אותו.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="171"/>
        <source>Same groups and datasets as your current project. Restoring may still revert field-level edits.</source>
        <translation>אותן קבוצות ומערכי נתונים כמו בפרויקט הנוכחי. שחזור עדיין עשוי להחזיר עריכות ברמת השדה.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="178"/>
        <source>Restoring removes %1 and brings back %2.</source>
        <translation>שחזור מסיר %1 ומחזיר %2.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="181"/>
        <source>Restoring removes %1.</source>
        <translation>שחזור מסיר %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="183"/>
        <source>Restoring brings back %1.</source>
        <translation>שחזור מחזיר %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="209"/>
        <source>Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.</source>
        <translation>בחר גיבוי לשחזור. הפרויקט הנוכחי נשמר אוטומטית תחילה, כך שהשחזור הפיך.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="292"/>
        <source>No backups for this project yet. Edit or save the project to start the rolling backup.</source>
        <translation>אין עדיין גיבויים לפרויקט זה. ערוך או שמור את הפרויקט כדי להתחיל גיבוי מתגלגל.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="320"/>
        <source>Open Folder</source>
        <translation>פתיחת תיקייה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="328"/>
        <source>Cancel</source>
        <translation>ביטול</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="334"/>
        <source>Restore</source>
        <translation>שחזור</translation>
    </message>
</context>
<context>
    <name>Benchmark</name>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="32"/>
        <source>Benchmark</source>
        <translation>מדד ביצועים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="79"/>
        <source>%1 frames/s</source>
        <translation>%1 מסגרות/שנייה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="83"/>
        <source>%1 s</source>
        <translation>%1 שניות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="88"/>
        <source>n/a</source>
        <translation>לא זמין</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Pass</source>
        <translation>עבר</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Fail</source>
        <translation>נכשל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="111"/>
        <source>Hotpath Benchmark</source>
        <translation>מדד ביצועים נתיב קריטי</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="122"/>
        <source>Measures how fast this computer can extract, parse, and visualize frames through Serial Studio's data pipeline.</source>
        <translation>מודד כמה מהר המחשב יכול לחלץ, לנתח ולהציג מסגרות דרך צינור הנתונים של Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="168"/>
        <source>The interface will freeze while running</source>
        <translation>הממשק יקפא במהלך הריצה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="179"/>
        <source>Each phase runs flat-out on the main thread, so the window stops responding until it finishes. Your current project is reloaded automatically when the benchmark ends.</source>
        <translation>כל שלב רץ במלוא העוצמה בחוט הראשי, כך שהחלון מפסיק להגיב עד לסיומו. הפרויקט הנוכחי שלך נטען מחדש אוטומטית כאשר הבנצ'מרק מסתיים.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="236"/>
        <source>Frames per phase:</source>
        <translation>מסגרות לכל שלב:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="250"/>
        <source>Minimum duration:</source>
        <translation>משך מינימלי:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="279"/>
        <source>Stages</source>
        <translation>שלבים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="287"/>
        <source>Parsers</source>
        <translation>מנתחים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="296"/>
        <source>Data export</source>
        <translation>ייצוא נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="304"/>
        <source>Dashboard</source>
        <translation>לוח בקרה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="316"/>
        <source>Data</source>
        <translation>נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="326"/>
        <source>Numeric only</source>
        <translation>ערכים מספריים בלבד</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="335"/>
        <source>Mixed (numeric + text)</source>
        <translation>מעורב (מספרי + טקסט)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="351"/>
        <source>Select at least one stage and one data type to run a benchmark.</source>
        <translation>בחר לפחות שלב אחד וסוג נתונים אחד כדי להריץ בנצ'מרק.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="210"/>
        <source>Running %1...</source>
        <translation>מריץ %1...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="211"/>
        <source>Preparing...</source>
        <translation>מכין...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="383"/>
        <source>Pipeline</source>
        <translation>צינור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="395"/>
        <source>Throughput</source>
        <translation>תפוקה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="407"/>
        <source>Time</source>
        <translation>זמן</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="419"/>
        <source>Result</source>
        <translation>תוצאה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="520"/>
        <source>Run a test to see results</source>
        <translation>הרץ בדיקה כדי לראות תוצאות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="539"/>
        <source>Pass/Fail applies to the data-pipeline and parser stages (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard stages are informational.</source>
        <translation>עבר/נכשל חל רק על שלבי צינור הנתונים והמפענח (צינור נתונים ומנתח מקורי מספרי 1024 K מסגרות/שנייה; מנתח מקורי מעורב 512 K; Lua מספרי 256 K; JavaScript מספרי ו-Lua מעורב 128 K; JavaScript מעורב 64 K). שלבי הייצוא ולוח הבקרה הם אינפורמטיביים.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">עבר/נכשל חל רק על שלבי צינור הנתונים והמנתח (צינור נתונים ומקורי מספרי 1024 K מסגרות/שנייה; מקורי מעורב 512 K; Lua מספרי 256 K; JavaScript מספרי ו-Lua מעורב 128 K; JavaScript מעורב 64 K). שלבי הייצוא ולוח הבקרה הם אינפורמטיביים.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Native numeric 1024 K frames/s; Native mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">עבור/נכשל חל על שלבי צינור הנתונים והמפענח (צינור נתונים ומפענח מקורי מספרי 1024 K מסגרות/שנייה; מקורי מעורב 512 K; Lua מספרי 256 K; JavaScript מספרי ו-Lua מעורב 128 K; JavaScript מעורב 64 K). שלבי הייצוא ולוח הבקרה הם אינפורמטיביים.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="554"/>
        <source>Copy</source>
        <translation>העתק</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline 1024 K frames/s; numeric: Lua 256 K, JavaScript 128 K; mixed: Lua 128 K, JavaScript 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">עבר/נכשל חל רק על שלבי צינור הנתונים והמפענח (צינור נתונים 1024 K מסגרות/שנייה; מספרי: Lua 256 K, JavaScript 128 K; מעורב: Lua 128 K, JavaScript 64 K). שלבי הייצוא ולוח הבקרה הם אינפורמטיביים.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the parser phases only (Lua target 256 K frames/s, JavaScript 128 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">עבר/נכשל חל רק על שלבי המפענח (יעד Lua 256 K מסגרות/שנייה, JavaScript 128 K). שלבי הייצוא ולוח הבקרה הם אינפורמטיביים.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="561"/>
        <source>Clear</source>
        <translation>נקה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="570"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Running...</source>
        <translation>מריץ...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Run Benchmark</source>
        <translation>הרץ מדד ביצועים</translation>
    </message>
</context>
<context>
    <name>BenchmarkRunner</name>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="255"/>
        <source>Data pipeline</source>
        <translation>צינור נתונים</translation>
    </message>
    <message>
        <source>Lua parser</source>
        <translation type="vanished">מנתח Lua</translation>
    </message>
    <message>
        <source>JavaScript parser</source>
        <translation type="vanished">מנתח JavaScript</translation>
    </message>
    <message>
        <source>Lua + data export</source>
        <translation type="vanished">Lua + ייצוא נתונים</translation>
    </message>
    <message>
        <source>Lua + dashboard</source>
        <translation type="vanished">Lua + לוח בקרה</translation>
    </message>
    <message>
        <source>Native parser (numeric)</source>
        <translation type="vanished">מפענח מקורי (מספרי)</translation>
    </message>
    <message>
        <source>Native parser (mixed)</source>
        <translation type="vanished">מפענח מקורי (מעורב)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="279"/>
        <source>Built-in parser (numeric)</source>
        <translation>מנתח מקורי (מספרי)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="297"/>
        <source>Built-in parser (mixed)</source>
        <translation>מנתח מקורי (מעורב)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="281"/>
        <source>Lua parser (numeric)</source>
        <translation>מנתח Lua (מספרי)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="288"/>
        <source>JavaScript parser (numeric)</source>
        <translation>מנתח JavaScript (מספרי)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="299"/>
        <source>Lua parser (mixed)</source>
        <translation>מנתח Lua (מעורב)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="306"/>
        <source>JavaScript parser (mixed)</source>
        <translation>מנתח JavaScript (מעורב)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="322"/>
        <source>Built-in + data export (numeric)</source>
        <translation>מנתח מקורי + ייצוא נתונים (מספרי)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="329"/>
        <source>Lua + data export (numeric)</source>
        <translation>Lua + ייצוא נתונים (מספרי)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="336"/>
        <source>JavaScript + data export (numeric)</source>
        <translation>JavaScript + ייצוא נתונים (מספרי)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="345"/>
        <source>Built-in + data export (mixed)</source>
        <translation>מנתח מקורי + ייצוא נתונים (מעורב)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="347"/>
        <source>Lua + data export (mixed)</source>
        <translation>Lua + ייצוא נתונים (מעורב)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="354"/>
        <source>JavaScript + data export (mixed)</source>
        <translation>JavaScript + ייצוא נתונים (מעורב)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="370"/>
        <source>Built-in + dashboard (numeric)</source>
        <translation>מנתח מקורי + לוח בקרה (מספרי)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="372"/>
        <source>Lua + dashboard (numeric)</source>
        <translation>Lua + לוח בקרה (מספרי)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>100 K frames</source>
        <translation>100 אלף פריימים</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>250 K frames</source>
        <translation>250 אלף מסגרות</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>500 K frames</source>
        <translation>500 אלף מסגרות</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>1 M frames</source>
        <translation>מיליון מסגרות</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>1 second</source>
        <translation>שנייה אחת</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>2 seconds</source>
        <translation>שתי שניות</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>5 seconds</source>
        <translation>5 שניות</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>10 seconds</source>
        <translation>10 שניות</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="200"/>
        <source>Serial Studio %1 - Hotpath Benchmark</source>
        <translation>Serial Studio %1 - מדד ביצועים Hotpath</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="202"/>
        <source>%1 (%2), workload: %3 frames minimum, %4 s minimum</source>
        <translation>%1 (%2), עומס עבודה: %3 מסגרות מינימום, %4 שניות מינימום</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Pipeline</source>
        <translation>צינור</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Throughput</source>
        <translation>תפוקה</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Target</source>
        <translation>יעד</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Time</source>
        <translation>זמן</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Result</source>
        <translation>תוצאה</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="225"/>
        <source>%1 frames/s</source>
        <translation>%1 מסגרות/שנייה</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>n/a</source>
        <translation>לא זמין</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Pass</source>
        <translation>עבר</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Fail</source>
        <translation>נכשל</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="227"/>
        <source>%1 s</source>
        <translation>%1 שניות</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="379"/>
        <source>JavaScript + dashboard (numeric)</source>
        <translation>JavaScript + לוח בקרה (מספרי)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="388"/>
        <source>Built-in + dashboard (mixed)</source>
        <translation>מובנה + לוח בקרה (מעורב)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="390"/>
        <source>Lua + dashboard (mixed)</source>
        <translation>Lua + לוח בקרה (מעורב)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="397"/>
        <source>JavaScript + dashboard (mixed)</source>
        <translation>JavaScript + לוח בקרה (מעורב)</translation>
    </message>
    <message>
        <source>Showing dashboard...</source>
        <translation type="vanished">מציג לוח בקרה...</translation>
    </message>
</context>
<context>
    <name>BluetoothLE</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="54"/>
        <source>Device</source>
        <translation>התקן</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="106"/>
        <source>Service</source>
        <translation>שירות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="142"/>
        <source>Characteristic</source>
        <translation>מאפיין</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="200"/>
        <source>Scanning…</source>
        <translation>סורק…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="236"/>
        <source>No Bluetooth Adapter Detected</source>
        <translation>לא זוהה מתאם Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="247"/>
        <source>Connect a Bluetooth adapter or check your system settings</source>
        <translation>חבר מתאם Bluetooth או בדוק את הגדרות המערכת</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="274"/>
        <source>This OS is not Supported Yet.</source>
        <translation>מערכת הפעלה זו אינה נתמכת עדיין.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="285"/>
        <source>We'll update Serial Studio to work with this operating system as soon as Qt officially supports it</source>
        <translation>Serial Studio יעודכן לעבודה עם מערכת הפעלה זו ברגע ש-QT תתמוך בה רשמית</translation>
    </message>
</context>
<context>
    <name>CANBus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="57"/>
        <source>No CAN Drivers Found</source>
        <translation>לא נמצאו מנהלי התקן CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="70"/>
        <source>Install CAN hardware drivers for your system</source>
        <translation>התקן מנהלי התקן חומרה של CAN עבור המערכת שלך</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="97"/>
        <source>CAN Driver</source>
        <translation>מנהל התקן CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="142"/>
        <source>Interface</source>
        <translation>ממשק</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="174"/>
        <source>Bitrate</source>
        <translation>קצב סיביות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="238"/>
        <source>Flexible Data-Rate</source>
        <translation>Flexible Data-Rate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="257"/>
        <source>Loopback</source>
        <translation>Loopback</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="276"/>
        <source>Listen-Only</source>
        <translation>האזנה בלבד</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="308"/>
        <source>DBC Database</source>
        <translation>מסד נתונים DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="312"/>
        <source>Import DBC File…</source>
        <translation>ייבוא קובץ DBC…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="345"/>
        <source>No CAN Interfaces Found</source>
        <translation>לא נמצאו ממשקי CAN</translation>
    </message>
</context>
<context>
    <name>CSV::Player</name>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="205"/>
        <source>Select CSV file</source>
        <translation>בחירת קובץ CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="207"/>
        <source>CSV files (*.csv)</source>
        <translation>קבצי CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="369"/>
        <source>Device Connection Active</source>
        <translation>חיבור למכשיר פעיל</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="370"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>כדי להשתמש בתכונה זו, יש להתנתק מהמכשיר. להמשיך?</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="384"/>
        <source>Cannot read CSV file</source>
        <translation>לא ניתן לקרוא קובץ CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="384"/>
        <source>Check file permissions and location</source>
        <translation>בדוק הרשאות ומיקום קובץ</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="414"/>
        <source>Insufficient Data in CSV File</source>
        <translation>אין מספיק נתונים בקובץ CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="415"/>
        <source>The CSV file must contain at least one data row to proceed. Check the file and try again.</source>
        <translation>קובץ ה-CSV חייב להכיל לפחות שורת נתונים אחת כדי להמשיך. בדוק את הקובץ ונסה שוב.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="645"/>
        <source>Invalid CSV</source>
        <translation>CSV לא תקין</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="646"/>
        <source>The CSV file does not contain any data or headers.</source>
        <translation>קובץ ה-CSV אינו מכיל נתונים או כותרות.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="655"/>
        <source>Select a date/time column</source>
        <translation>בחר עמודת תאריך/שעה</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="655"/>
        <location filename="../../src/CSV/Player.cpp" line="667"/>
        <source>Set interval manually</source>
        <translation>הגדר מרווח באופן ידני</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="657"/>
        <source>CSV Date/Time Selection</source>
        <translation>בחירת תאריך/שעה ב-CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="658"/>
        <source>Choose how to handle the date/time data:</source>
        <translation>בחר כיצד לטפל בנתוני תאריך/שעה:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="670"/>
        <source>Set Interval</source>
        <translation>הגדרת מרווח</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="671"/>
        <source>Please enter the interval between rows in milliseconds:</source>
        <translation>הזן את המרווח בין שורות במילישניות:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="687"/>
        <source>Select Date/Time Column</source>
        <translation>בחירת עמודת תאריך/שעה</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="688"/>
        <source>Please select the column that contains the date/time data:</source>
        <translation>אנא בחר את העמודה המכילה את נתוני התאריך/שעה:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="698"/>
        <source>Invalid Selection</source>
        <translation>בחירה לא חוקית</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="698"/>
        <source>The selected column is not valid.</source>
        <translation>העמודה שנבחרה אינה חוקית.</translation>
    </message>
</context>
<context>
    <name>Console</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Console.qml" line="32"/>
        <source>Console</source>
        <translation>קונסול</translation>
    </message>
</context>
<context>
    <name>Console::Export</name>
    <message>
        <location filename="../../src/Console/Export.cpp" line="331"/>
        <source>Console Export is a Pro feature.</source>
        <translation>ייצוא קונסול הוא תכונת Pro.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="332"/>
        <source>This feature requires a license. Please purchase one to enable console export.</source>
        <translation>תכונה זו דורשת רישיון. יש לרכוש רישיון כדי לאפשר ייצוא קונסול.</translation>
    </message>
</context>
<context>
    <name>Console::Handler</name>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="224"/>
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="225"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="235"/>
        <source>No Line Ending</source>
        <translation>ללא סיום שורה</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="236"/>
        <source>New Line</source>
        <translation>שורה חדשה</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="237"/>
        <source>Carriage Return</source>
        <translation>החזרת עגלה</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="238"/>
        <source>CR + NL</source>
        <translation>CR + NL</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="248"/>
        <source>Plain Text</source>
        <translation>טקסט רגיל</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="249"/>
        <source>Hexadecimal</source>
        <translation>הקסדצימלי</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="271"/>
        <source>No Checksum</source>
        <translation>ללא סיכום ביקורת</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="908"/>
        <source>Device %1</source>
        <translation>התקן %1</translation>
    </message>
</context>
<context>
    <name>ConstantsLibraryDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="44"/>
        <source>Insert Constant</source>
        <translation>הוספת קבוע</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Fundamental</source>
        <translation>יסודי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <source>Speed of light in vacuum</source>
        <translation>מהירות האור בוואקום</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <source>Planck constant</source>
        <translation>קבוע פלאנק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <source>Elementary charge</source>
        <translation>מטען יסודי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <source>Avogadro constant</source>
        <translation>קבוע אבוגדרו</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <source>Boltzmann constant</source>
        <translation>קבוע בולצמן</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Stefan-Boltzmann constant</source>
        <translation>קבוע סטפן-בולצמן</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Mechanics</source>
        <translation>מכניקה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <source>Standard gravity</source>
        <translation>כובד תאוצה סטנדרטי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Gravitational constant</source>
        <translation>קבוע הכבידה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Pressure</source>
        <translation>לחץ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <source>Standard atmosphere</source>
        <translation>אטמוספירה סטנדרטית</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Sea-level barometric pressure</source>
        <translation>לחץ ברומטרי בגובה פני הים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Temperature</source>
        <translation>טמפרטורה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <source>Absolute zero (Celsius)</source>
        <translation>אפס מוחלט (צלזיוס)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <source>Water freezing point</source>
        <translation>נקודת קפיאת מים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Water boiling point (1 atm)</source>
        <translation>נקודת רתיחת מים (1 atm)</translation>
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
        <translation>גזים ונוזלים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <source>Universal gas constant</source>
        <translation>קבוע הגזים האוניברסלי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <source>Specific gas constant (dry air)</source>
        <translation>קבוע גז ספציפי (אוויר יבש)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <source>Specific gas constant (water vapor)</source>
        <translation>קבוע גז ספציפי (אדי מים)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <source>Air density (sea level, 15°C)</source>
        <translation>צפיפות אוויר (פני הים, 15°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <source>Water density (4°C)</source>
        <translation>צפיפות מים (4°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <source>Speed of sound in air (20°C)</source>
        <translation>מהירות הקול באוויר (20°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Heat capacity ratio (dry air)</source>
        <translation>יחס קיבולת חום (אוויר יבש)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Electromagnetism</source>
        <translation>אלקטרומגנטיות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <source>Vacuum permittivity</source>
        <translation>פרמיטיביות ריק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Vacuum permeability</source>
        <translation>פרמאביליות ריק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Math</source>
        <translation>מתמטיקה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <source>Pi</source>
        <translation>פאי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <source>Euler's number</source>
        <translation>מספר אוילר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Golden ratio</source>
        <translation>יחס הזהב</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="212"/>
        <source>Physics Constants</source>
        <translation>קבועים פיזיקליים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="221"/>
        <source>SI-unit preset values. Click a row to insert it into %1.</source>
        <translation>ערכי קבועים מוגדרים מראש ביחידות SI. לחץ על שורה כדי להוסיף אותה ל-%1.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="231"/>
        <source>Search</source>
        <translation>חיפוש</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="250"/>
        <source>Symbol</source>
        <translation>סמל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="251"/>
        <source>Name</source>
        <translation>שם</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="252"/>
        <source>Value</source>
        <translation>ערך</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="253"/>
        <source>Category</source>
        <translation>קטגוריה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="357"/>
        <source>No constants match.</source>
        <translation>אין קבועים תואמים.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="378"/>
        <source>%1 constants</source>
        <translation>%1 קבועים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="379"/>
        <source>%1 of %2 constants</source>
        <translation>%1 מתוך %2 קבועים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="383"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>CrashRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="31"/>
        <source>Recovery Options</source>
        <translation>אפשרויות שחזור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="57"/>
        <source>Serial Studio has closed unexpectedly several times in a row.</source>
        <translation>Serial Studio נסגר באופן בלתי צפוי מספר פעמים ברצף.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="82"/>
        <source>Consecutive crashes: %1</source>
        <translation>קריסות רצופות: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="90"/>
        <source>Last reported stage: %1</source>
        <translation>שלב שדווח לאחרונה: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="98"/>
        <source>Detected at: %1</source>
        <translation>זוהה ב: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="113"/>
        <source>Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.</source>
        <translation>בחר פעולה לשחזור. Serial Studio ייסגר לאחר הביצוע כדי שההרצה הבאה תתחיל נקי.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="127"/>
        <source>Reset Rendering Backend to Default</source>
        <translation>אפס את מנגנון הרינדור לברירת מחדל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="135"/>
        <source>Skip Restoring the Last Opened Project</source>
        <translation>דלג על שחזור הפרויקט האחרון שנפתח</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="142"/>
        <source>Reset all Preferences</source>
        <translation>אפס את כל ההעדפות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="160"/>
        <source>Continue Anyway</source>
        <translation>המשך בכל זאת</translation>
    </message>
</context>
<context>
    <name>CsvPlayer</name>
    <message>
        <location filename="../../qml/Dialogs/CsvPlayer.qml" line="36"/>
        <source>CSV Player</source>
        <translation>נגן CSV</translation>
    </message>
</context>
<context>
    <name>DBCPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="44"/>
        <source>DBC File Preview</source>
        <translation>תצוגה מקדימה של קובץ DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="169"/>
        <source>DBC File: %1</source>
        <translation>קובץ DBC: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="177"/>
        <source>Review the CAN messages and signals to import into a new Serial Studio project.</source>
        <translation>סקירת הודעות CAN והאותות לייבוא לפרויקט Serial Studio חדש.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="185"/>
        <source>Messages</source>
        <translation>הודעות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="219"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="229"/>
        <source>Message Name</source>
        <translation>שם הודעה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="235"/>
        <source>CAN ID</source>
        <translation>מזהה CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="242"/>
        <source>Signals</source>
        <translation>אותות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="323"/>
        <source>No messages found in DBC file.</source>
        <translation>לא נמצאו הודעות בקובץ DBC.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="341"/>
        <source>Total: %1 messages, %2 signals</source>
        <translation>סה״כ: %1 הודעות, %2 אותות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="348"/>
        <source>Cancel</source>
        <translation>ביטול</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="359"/>
        <source>Create Project</source>
        <translation>יצירת פרויקט</translation>
    </message>
</context>
<context>
    <name>Dashboard</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard.qml" line="127"/>
        <source>Dashboard %1</source>
        <translation>לוח בקרה %1</translation>
    </message>
</context>
<context>
    <name>DashboardButton</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="40"/>
        <source>Send</source>
        <translation>שליחה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="64"/>
        <source>No transmit function defined</source>
        <translation>לא הוגדרה פונקציית שידור</translation>
    </message>
</context>
<context>
    <name>DashboardCanvas</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="56"/>
        <source>Set Wallpaper…</source>
        <translation>הגדרת תמונת רקע…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="62"/>
        <source>Clear Wallpaper</source>
        <translation>ניקוי תמונת רקע</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="72"/>
        <source>Tile Windows</source>
        <translation>סידור חלונות באריחים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="91"/>
        <source>Pro features detected in this project.</source>
        <translation>זוהו תכונות Pro בפרויקט זה.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="93"/>
        <source>Fallback widgets are active. Purchase a license for full functionality.</source>
        <translation>ווידג'טים חלופיים פעילים. רכוש רישיון לפונקציונליות מלאה.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="208"/>
        <source>Empty Workspace</source>
        <translation>סביבת עבודה ריקה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="222"/>
        <source>Use the search bar to find and add widgets, or right-click a widget in another workspace to add it here.</source>
        <translation>השתמש בשורת החיפוש כדי למצוא ולהוסיף ווידג'טים, או לחץ לחיצה ימנית על ווידג'ט בסביבת עבודה אחרת כדי להוסיף אותו לכאן.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="237"/>
        <source>Search Widgets</source>
        <translation>חיפוש ווידג'טים</translation>
    </message>
</context>
<context>
    <name>DashboardLayout</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="37"/>
        <source>Dashboard</source>
        <translation>לוח בקרה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="206"/>
        <source>API Server Active (%1)</source>
        <translation>שרת API פעיל (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="207"/>
        <source>API Server Ready</source>
        <translation>שרת API מוכן</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="208"/>
        <source>API Server Off</source>
        <translation>שרת API כבוי</translation>
    </message>
</context>
<context>
    <name>DashboardOutputPanel</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="123"/>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="275"/>
        <source>Send</source>
        <translation>שלח</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="263"/>
        <source>Enter command…</source>
        <translation>הזן פקודה…</translation>
    </message>
</context>
<context>
    <name>DashboardSlider</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardSlider.qml" line="90"/>
        <source>No transmit function defined</source>
        <translation>לא הוגדרה פונקציית שידור</translation>
    </message>
</context>
<context>
    <name>DashboardTextField</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="47"/>
        <source>Enter command…</source>
        <translation>הזן פקודה…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="57"/>
        <source>Send</source>
        <translation>שלח</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="76"/>
        <source>No transmit function defined</source>
        <translation>לא הוגדרה פונקציית שידור</translation>
    </message>
</context>
<context>
    <name>DashboardToggle</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="57"/>
        <source>ON</source>
        <translation>מופעל</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="59"/>
        <source>OFF</source>
        <translation>כבוי</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="70"/>
        <source>No transmit function defined</source>
        <translation>לא הוגדרה פונקציית שידור</translation>
    </message>
</context>
<context>
    <name>DataGrid</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="86"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="87"/>
        <source>Pause</source>
        <translation>השהה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="86"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="87"/>
        <source>Resume</source>
        <translation>המשך</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="297"/>
        <source>Awaiting data…</source>
        <translation>ממתין לנתונים…</translation>
    </message>
</context>
<context>
    <name>DataModel::DBCImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="124"/>
        <source>Import DBC File</source>
        <translation>ייבא קובץ DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="124"/>
        <source>DBC Files (*.dbc);;All Files (*)</source>
        <translation>קבצי DBC (*.DBC);;כל הקבצים (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="157"/>
        <source>Failed to parse DBC file: %1</source>
        <translation>ניתוח קובץ DBC נכשל: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="158"/>
        <source>Verify the file format and try again.</source>
        <translation>יש לאמת את תבנית הקובץ ולנסות שוב.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="160"/>
        <source>DBC Import Error</source>
        <translation>שגיאת ייבוא DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="167"/>
        <source>DBC file contains no messages</source>
        <translation>קובץ DBC אינו מכיל הודעות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="168"/>
        <source>The selected file does not contain any CAN message definitions.</source>
        <translation>הקובץ שנבחר אינו מכיל הגדרות הודעות CAN.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="170"/>
        <source>DBC Import Warning</source>
        <translation>אזהרת ייבוא DBC</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">טעינת הפרויקט המיובא נכשלה</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">לא ניתן לטעון את JSON של הפרויקט שנוצר.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="214"/>
        <source>The project editor is now open for customization.</source>
        <translation>עורך הפרויקט פתוח כעת להתאמה אישית.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="216"/>
        <source> Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); only simple multiplexing is supported.</source>
        <translation>דילגו על %1 אותות המשתמשים ב-multiplexing מורחב (SG_MUL_VAL_); נתמך רק multiplexing פשוט.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="221"/>
        <source>Successfully imported DBC file with %1 messages and %2 signals.</source>
        <translation>קובץ DBC יובא בהצלחה עם %1 הודעות ו-%2 אותות.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="226"/>
        <source>DBC Import Complete</source>
        <translation>ייבוא DBC הושלם</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="248"/>
        <source>CAN Bus</source>
        <translation>אפיק CAN</translation>
    </message>
</context>
<context>
    <name>DataModel::DatasetTransformEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="58"/>
        <source>Dataset Value Transform</source>
        <translation>טרנספורמציית ערך Dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="95"/>
        <source>Lua</source>
        <translation>Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="95"/>
        <source>JavaScript</source>
        <translation>JavaScript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="101"/>
        <source>Enter raw value (e.g., 1024)</source>
        <translation>הזן ערך גולמי (לדוגמה, 1024)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="106"/>
        <source>Test</source>
        <translation>בדיקה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="107"/>
        <source>Clear</source>
        <translation>נקה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="109"/>
        <source>Apply</source>
        <translation>החל</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="110"/>
        <source>Cancel</source>
        <translation>ביטול</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="119"/>
        <source>Language:</source>
        <translation>שפה:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="122"/>
        <source>Template:</source>
        <translation>תבנית:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="133"/>
        <source>Input:</source>
        <translation>קלט:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="136"/>
        <source>Output:</source>
        <translation>פלט:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="214"/>
        <source>Transform — %1</source>
        <translation>טרנספורמציה — %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="296"/>
        <source>Enter a value</source>
        <translation>הזן ערך</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="303"/>
        <source>Invalid number</source>
        <translation>מספר לא חוקי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="372"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>עצב מסמך	Ctrl+Shift+I</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="373"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>בחירת עיצוב	Ctrl+I</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="472"/>
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
-- הגדר פונקציית transform(value) שמקבלת את קריאת
-- מערך הנתונים החיה ומחזירה מספר מעובד. אם לא מוגדרת
-- פונקציית transform(), הערך הגולמי נשמר ושום דבר
-- לא נשמר עם הפרויקט.
--
-- דוגמה:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- משתנים גלובליים שהוגדרו ברמה העליונה נשמרים בין מסגרות,
-- מה שמועיל עבור מסננים, מצברים וכל עיבוד אחר עם מצב,
-- בדיוק כמו סקריפט מנתח מסגרות:
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
-- השתמש בתפריט הנפתח תבנית עבור דוגמאות מוכנות, או
-- לחץ על בדיקה כדי לנסות את הפונקציה שלך.
--
</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="500"/>
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
 * הגדר פונקציית transform(value) שמקבלת את קריאת
 * מערך הנתונים החיה ומחזירה מספר מעובד. אם לא מוגדרת
 * פונקציית transform(), הערך הגולמי נשמר ושום דבר
 * לא נשמר עם הפרויקט.
 *
 * דוגמה:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * משתנים גלובליים שהוגדרו ברמה העליונה נשמרים בין מסגרות,
 * מה שמועיל עבור מסננים, מצברים וכל עיבוד אחר עם מצב --
 * בדיוק כמו סקריפט מנתח מסגרות:
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
 * השתמש בתפריט הנפתח תבנית עבור דוגמאות מוכנות, או
 * לחץ על בדיקה כדי לנסות את הפונקציה שלך.
 */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="635"/>
        <source>Engine error</source>
        <translation>שגיאת מנוע</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="658"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="671"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="689"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="700"/>
        <source>Error: %1</source>
        <translation>שגיאה: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="664"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="693"/>
        <source>Error: transform() not defined</source>
        <translation>שגיאה: transform() לא מוגדרת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="676"/>
        <source>Error: transform() must return a number</source>
        <translation>שגיאה: transform() חייבת להחזיר מספר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="745"/>
        <source>Select Template…</source>
        <translation>בחר תבנית…</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameBuilder</name>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1115"/>
        <source>JavaScript transform exceeded budget</source>
        <translation>תקציב ההמרה של JavaScript חרג מהמותר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1116"/>
        <source>A dataset transform took longer than %1 ms; remaining datasets in the frame fell back to raw values until the next frame. Profile or simplify the transform code.</source>
        <translation>המרת ערכת נתונים ארכה יותר מ-%1 מילישניות; ערכות הנתונים הנותרות ב-Frame חזרו לערכים גולמיים עד ל-Frame הבא. בצע פרופיל או פשט את קוד ההמרה.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="195"/>
        <source>Frame pool exhausted</source>
        <translation>מאגר ה-Frame מוצא</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="197"/>
        <source>A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API subscriber) is not draining frames fast enough. Serial Studio is falling back to per-frame allocations until the backlog clears. Disable a heavy consumer or reduce the data rate.</source>
        <translation>צרכן במורד הזרם (Dashboard, ייצוא CSV/MDF4, מסד נתוני Session, או מנוי API) לא מרוקן Frames מספיק מהר. Serial Studio עובר להקצאות לפי Frame עד שהצטברות תתפנה. השבת צרכן כבד או הפחת את קצב הנתונים.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1290"/>
        <source>Device A</source>
        <translation>התקן A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1329"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1443"/>
        <source>Channel %1</source>
        <translation>ערוץ %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1338"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1459"/>
        <source>Quick Plot</source>
        <translation>תרשים מהיר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1345"/>
        <source>Quick Plot Data</source>
        <translation>נתוני תרשים מהיר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1357"/>
        <source>Multiple Plots</source>
        <translation>תרשימים מרובים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1454"/>
        <source>Audio Input</source>
        <translation>קלט שמע</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserModel</name>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Plain text (UTF-8)</source>
        <translation>טקסט רגיל (UTF-8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Hexadecimal</source>
        <translation>הקסדצימלי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Binary (raw bytes)</source>
        <translation>בינארי (בתים גולמיים)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="265"/>
        <source>End delimiter only</source>
        <translation>תוחם סיום בלבד</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="266"/>
        <source>Start + end delimiters</source>
        <translation>תוחם התחלה + סיום</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="267"/>
        <source>Start delimiter only</source>
        <translation>תוחם התחלה בלבד</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="268"/>
        <source>No delimiters (whole chunk)</source>
        <translation>ללא תוחמים (נתח שלם)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="279"/>
        <source>No Checksum</source>
        <translation>ללא סיכום ביקורת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="310"/>
        <source>Select Frame Parser Template</source>
        <translation>בחר תבנית מפענח מסגרות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="311"/>
        <source>Choose a template to load:</source>
        <translation>בחר תבנית לטעינה:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="494"/>
        <source>Invalid hexadecimal input.</source>
        <translation>קלט הקסדצימלי לא תקין.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="521"/>
        <source>No template selected.</source>
        <translation>לא נבחרה תבנית.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="561"/>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation>%1 מסגרות חולצו | %2 בתים נצרכו | %3 בתים במאגר | %4 נדחו</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="632"/>
        <source>Invalid JSON: %1</source>
        <translation>JSON לא תקין: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="728"/>
        <source>Parameters</source>
        <translation>פרמטרים</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserTestDialog</name>
    <message>
        <source>None</source>
        <translation type="vanished">ללא</translation>
    </message>
    <message>
        <source>Invalid Hex Input</source>
        <translation type="vanished">קלט הקסדצימלי לא תקין</translation>
    </message>
    <message>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation type="vanished">יש להזין בתים הקסדצימליים תקינים.

פורמט תקין: 01 A2 FF 3C</translation>
    </message>
    <message>
        <source>(no frames)</source>
        <translation type="vanished">(אין מסגרות)</translation>
    </message>
    <message>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation type="vanished">החילוץ לא יצר מסגרת שלמה. בדוק את תוחמי ההתחלה/סיום ואת מצב הזיהוי.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 מסגרות חולצו | %2 בתים נצרכו | %3 בתים במאגר | %4 נדחו</translation>
    </message>
    <message>
        <source>Pipeline Configuration</source>
        <translation type="vanished">תצורת צינור</translation>
    </message>
    <message>
        <source>Pipeline Results</source>
        <translation type="vanished">תוצאות Pipeline</translation>
    </message>
    <message>
        <source>Detection</source>
        <translation type="vanished">זיהוי</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">מפענח</translation>
    </message>
    <message>
        <source>Checksum</source>
        <translation type="vanished">Checksum</translation>
    </message>
    <message>
        <source>Start Delimiter</source>
        <translation type="vanished">תוחם התחלה</translation>
    </message>
    <message>
        <source>End Delimiter</source>
        <translation type="vanished">תוחם סיום</translation>
    </message>
    <message>
        <source>Hex Delimiters</source>
        <translation type="vanished">תוחמים הקסדצימליים</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">תוחם סיום בלבד</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">תוחם התחלה + סיום</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">תוחם התחלה בלבד</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">ללא תוחמים (נתח שלם)</translation>
    </message>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">טקסט רגיל (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">הקסדצימלי</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">בינארי (בתים גולמיים)</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">נקה</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">הערך</translation>
    </message>
    <message>
        <source>Enter raw stream bytes here...</source>
        <translation type="vanished">הזן בתי זרם גולמיים כאן...</translation>
    </message>
    <message>
        <source>Stage</source>
        <translation type="vanished">שלב</translation>
    </message>
    <message>
        <source>Frame Data Input</source>
        <translation type="vanished">קלט נתוני מסגרת</translation>
    </message>
    <message>
        <source>Frame Parser Results</source>
        <translation type="vanished">תוצאות מנתח מסגרות</translation>
    </message>
    <message>
        <source>Enter frame data here…</source>
        <translation type="vanished">הזן נתוני מסגרת כאן…</translation>
    </message>
    <message>
        <source>Dataset Index</source>
        <translation type="vanished">אינדקס מערך נתונים</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="vanished">ערך</translation>
    </message>
    <message>
        <source>Enter frame data above, enable HEX mode if needed, then click "Evaluate" to run the frame parser.

Example (Text): a,b,c,d,e,f
Example (HEX):  48 65 6C 6C 6F</source>
        <translation type="vanished">הזן נתוני מסגרת למעלה, הפעל מצב HEX במידת הצורך, ולאחר מכן לחץ על "הערך" כדי להריץ את מנתח המסגרות.

דוגמה (טקסט): a,b,c,d,e,f
דוגמה (HEX):  48 65 6C 6C 6F</translation>
    </message>
    <message>
        <source>Test Frame Parser</source>
        <translation type="vanished">בדיקת מנתח מסגרות</translation>
    </message>
    <message>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation type="vanished">הזן בתים הקסדצימליים (לדוגמה, 01 A2 FF)</translation>
    </message>
    <message>
        <source>(empty)</source>
        <translation type="vanished">(ריק)</translation>
    </message>
    <message>
        <source>No data returned</source>
        <translation type="vanished">לא הוחזרו נתונים</translation>
    </message>
</context>
<context>
    <name>DataModel::JsCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="202"/>
        <source>Change Scripting Language?</source>
        <translation>שינוי שפת סקריפטים?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="203"/>
        <source>Switching the scripting language replaces the current parser code with the equivalent template in the new language.

Any unsaved changes are lost. Continue?</source>
        <translation>החלפת שפת הסקריפט מחליפה את קוד המפענח הנוכחי בתבנית המקבילה בשפה החדשה.

כל השינויים שלא נשמרו יאבדו. להמשיך?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="365"/>
        <source>Select Lua file to import</source>
        <translation>בחר קובץ Lua לייבוא</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="365"/>
        <source>Select Javascript file to import</source>
        <translation>בחר קובץ JavaScript לייבוא</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="397"/>
        <source>Code Validation Successful</source>
        <translation>אימות קוד הצליח</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="398"/>
        <source>No syntax errors detected in the parser code.</source>
        <translation>לא זוהו שגיאות תחביר בקוד המפענח.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="508"/>
        <source>Select Frame Parser Template</source>
        <translation>בחר תבנית מפענח מסגרות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="509"/>
        <source>Choose a template to load:</source>
        <translation>בחר תבנית לטעינה:</translation>
    </message>
</context>
<context>
    <name>DataModel::ModbusMapImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="290"/>
        <source>Import Modbus Register Map</source>
        <translation>ייבא מפת רגיסטרים של Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="294"/>
        <source>Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files (*.xml);;JSON Files (*.json);;All Files (*)</source>
        <translation>מפות רגיסטרים של Modbus (*.CSV *.XML *.JSON);;קבצי CSV (*.CSV);;קבצי XML (*.XML);;קבצי JSON (*.JSON);;כל הקבצים (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="331"/>
        <source>No registers found</source>
        <translation>לא נמצאו רגיסטרים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="332"/>
        <source>The file could not be parsed or contains no register definitions.</source>
        <translation>הקובץ לא ניתן לפענוח או שאינו מכיל הגדרות רגיסטר.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="334"/>
        <source>Modbus Import</source>
        <translation>ייבוא Modbus</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">טעינת הפרויקט המיובא נכשלה</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">לא ניתן לטעון את JSON של הפרויקט שנוצר.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="379"/>
        <source>Successfully imported %1 registers in %2 groups.</source>
        <translation>%1 רגיסטרים יובאו בהצלחה ב־%2 קבוצות.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="381"/>
        <source>The project editor is now open for customization.</source>
        <translation>עורך הפרויקט פתוח כעת להתאמה אישית.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="383"/>
        <source>Modbus Import Complete</source>
        <translation>ייבוא Modbus הושלם</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="692"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
</context>
<context>
    <name>DataModel::NativeParserEditor</name>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">טקסט רגיל (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">הקסדצימלי</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">בינארי (בתים גולמיים)</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">תוחם סיום בלבד</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">תוחם התחלה + סיום</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">תוחם התחלה בלבד</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">ללא תוחמים (נתח שלם)</translation>
    </message>
    <message>
        <source>No Checksum</source>
        <translation type="vanished">ללא סיכום ביקורת</translation>
    </message>
    <message>
        <source>Invalid hexadecimal input.</source>
        <translation type="vanished">קלט הקסדצימלי לא תקין.</translation>
    </message>
    <message>
        <source>No template selected.</source>
        <translation type="vanished">לא נבחרה תבנית.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 מסגרות חולצו | %2 בתים נצרכו | %3 בתים במאגר | %4 נדחו</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation type="vanished">פרמטרים</translation>
    </message>
</context>
<context>
    <name>DataModel::OutputCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="283"/>
        <source>Select Javascript file to import</source>
        <translation>בחר קובץ JavaScript לייבוא</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="341"/>
        <source>Select Output Widget Template</source>
        <translation>בחר תבנית Widget פלט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="342"/>
        <source>Choose a template to load:</source>
        <translation>בחר תבנית לטעינה:</translation>
    </message>
</context>
<context>
    <name>DataModel::PainterCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="295"/>
        <source>Select Javascript file to import</source>
        <translation>בחר קובץ JavaScript לייבוא</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="382"/>
        <source>Select Painter Widget Template</source>
        <translation>בחר תבנית Widget ציור</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="383"/>
        <source>Choose a template to load:</source>
        <translation>בחר תבנית לטעינה:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="427"/>
        <source>Add datasets for this template?</source>
        <translation>להוסיף מערכי נתונים עבור תבנית זו?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="428"/>
        <source>"%1" expects %2 dataset(s); the current group has %3.

Add %4 dataset(s) using the template's defaults?</source>
        <translation>"%1" מצפה ל־%2 מערכי נתונים; הקבוצה הנוכחית מכילה %3.

להוסיף %4 מערכי נתונים עם ברירות המחדל של התבנית?</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectEditor</name>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1287"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1290"/>
        <source>Frame Parser</source>
        <translation>מנתח Frame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1429"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1430"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1472"/>
        <source>Groups</source>
        <translation>קבוצות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1502"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1515"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1516"/>
        <source>Shared Memory</source>
        <translation>זיכרון משותף</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1502"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1521"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1522"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4887"/>
        <source>Dataset Values</source>
        <translation>ערכי מערך נתונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1564"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1577"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1578"/>
        <source>Workspaces</source>
        <translation>מרחבי עבודה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1616"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1619"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1620"/>
        <source>MQTT Publisher</source>
        <translation>מפרסם MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1669"/>
        <source>Publishing</source>
        <translation>פרסום</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1679"/>
        <source>Enable Publishing</source>
        <translation>הפעל פרסום</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1680"/>
        <source>Broadcast frames, raw bytes and notifications to the broker</source>
        <translation>שידור מסגרות, בתים גולמיים והתראות אל ה-Broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1691"/>
        <source>Payload</source>
        <translation>מטען</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1692"/>
        <source>Selects what gets published: parsed dashboard data or raw RX bytes</source>
        <translation>בחירת מה מפורסם: נתוני לוח מחוונים מפוענחים או בתים גולמיים מ-RX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1702"/>
        <source>Publish Rate (Hz)</source>
        <translation>קצב פרסום (Hz)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1703"/>
        <source>How many times per second to publish (1-30 Hz). Higher rates increase broker load; dashboard data is rate-limited so a slow broker never blocks frame parsing.</source>
        <translation>כמה פעמים בשנייה לפרסם (1-30 Hz). קצבים גבוהים יותר מגדילים עומס על ה-Broker; נתוני לוח הבקרה מוגבלים בקצב כך ש-Broker איטי לעולם לא חוסם ניתוח מסגרות.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1715"/>
        <source>Topic Base</source>
        <translation>בסיס נושא</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1716"/>
        <source>serial-studio/device</source>
        <translation>serial-studio/device</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1717"/>
        <source>Base topic used for frame and raw-byte publishing</source>
        <translation>נושא בסיס המשמש לפרסום מסגרות ובתים גולמיים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1727"/>
        <source>Script Topic</source>
        <translation>נושא סקריפט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1728"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1752"/>
        <source>Defaults to Topic Base when empty</source>
        <translation>ברירת מחדל לנושא בסיס כאשר ריק</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1729"/>
        <source>Topic the user script publishes to</source>
        <translation>נושא אליו מפרסם סקריפט המשתמש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1739"/>
        <source>Publish Notifications</source>
        <translation>פרסום התראות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1740"/>
        <source>Mirror dashboard notifications to a dedicated topic</source>
        <translation>שיקוף התראות לוח מחוונים לנושא ייעודי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1751"/>
        <source>Notification Topic</source>
        <translation>נושא התראות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1753"/>
        <source>Topic where dashboard notifications are mirrored</source>
        <translation>נושא שאליו משוקפות התראות לוח המחוונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1766"/>
        <source>Broker</source>
        <translation>Broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1776"/>
        <source>Hostname</source>
        <translation>שם מארח</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1777"/>
        <source>broker.hivemq.com</source>
        <translation>broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1778"/>
        <source>Hostname or IP address of the MQTT broker</source>
        <translation>שם מארח או כתובת IP של ברוקר MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1787"/>
        <source>Port</source>
        <translation>פורט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1788"/>
        <source>TCP port exposed by the broker (1883 plain, 8883 TLS)</source>
        <translation>פורט TCP שנחשף על ידי הברוקר (1883 רגיל, 8883 TLS)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1798"/>
        <source>Custom Client ID</source>
        <translation>מזהה לקוח מותאם אישית</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1800"/>
        <source>Off: a fresh random id is generated on every project load. On: use the id below.</source>
        <translation>כבוי: מזהה אקראי חדש נוצר בכל טעינת פרויקט. דולק: שימוש במזהה למטה.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1811"/>
        <source>Client ID</source>
        <translation>מזהה לקוח</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1812"/>
        <source>Identifier sent to the broker on CONNECT</source>
        <translation>מזהה הנשלח לברוקר ב-CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1825"/>
        <source>Protocol Version</source>
        <translation>גרסת פרוטוקול</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1826"/>
        <source>MQTT protocol revision used on CONNECT</source>
        <translation>גרסת פרוטוקול MQTT בשימוש ב-CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1835"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (שניות)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1836"/>
        <source>Seconds between PINGREQ packets when idle</source>
        <translation>שניות בין מנות PINGREQ במצב המתנה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1845"/>
        <source>Clean Session</source>
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1846"/>
        <source>Discard any persistent session state on CONNECT</source>
        <translation>מחיקת כל מצב סשן מתמשך ב-CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1861"/>
        <source>Username</source>
        <translation>שם משתמש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1862"/>
        <source>Username for broker authentication (leave empty for anonymous)</source>
        <translation>שם משתמש לאימות מול ה-Broker (להשאיר ריק לאנונימי)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1872"/>
        <source>Password</source>
        <translation>סיסמה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1873"/>
        <source>Password for broker authentication</source>
        <translation>סיסמה לאימות מול ה-Broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1884"/>
        <source>SSL / TLS</source>
        <translation>SSL / TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1894"/>
        <source>Use SSL/TLS</source>
        <translation>שימוש ב-SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1895"/>
        <source>Tunnel the broker connection over TLS</source>
        <translation>העברת חיבור ה-Broker דרך TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1908"/>
        <source>Protocol</source>
        <translation>פרוטוקול</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1909"/>
        <source>Negotiated TLS protocol family</source>
        <translation>משפחת פרוטוקול TLS המוסכמת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1919"/>
        <source>Peer Verify</source>
        <translation>אימות עמית</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1920"/>
        <source>How strictly the broker's certificate chain is validated</source>
        <translation>רמת הקפדה באימות שרשרת אישורי ה-Broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1930"/>
        <source>Verify Depth</source>
        <translation>עומק אימות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1931"/>
        <source>Maximum certificate chain length accepted (0 = unlimited)</source>
        <translation>אורך מרבי של שרשרת אישורים מקובל (0 = ללא הגבלה)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2142"/>
        <source>Project Information</source>
        <translation>מידע על הפרויקט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2152"/>
        <source>Project Title</source>
        <translation>כותרת הפרויקט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2153"/>
        <source>Untitled Project</source>
        <translation>פרויקט ללא שם</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2154"/>
        <source>Name or description of the project</source>
        <translation>שם או תיאור של הפרויקט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2172"/>
        <source>Group Information</source>
        <translation>מידע על קבוצה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2182"/>
        <source>Group Title</source>
        <translation>כותרת קבוצה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2183"/>
        <source>Untitled Group</source>
        <translation>קבוצה ללא שם</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2184"/>
        <source>Title or description of this dataset group</source>
        <translation>כותרת או תיאור של קבוצת מערכי נתונים זו</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2199"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2787"/>
        <source>Device %1</source>
        <translation>התקן %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2217"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2336"/>
        <source>Input Device</source>
        <translation>התקן קלט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2218"/>
        <source>Select which connected device provides data for this group</source>
        <translation>בחירת ההתקן המחובר המספק נתונים לקבוצה זו</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2234"/>
        <source>Image Configuration</source>
        <translation>תצורת תמונה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2247"/>
        <source>Detection Mode</source>
        <translation>מצב זיהוי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2249"/>
        <source>Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences</source>
        <translation>זיהוי אוטומטי קורא בתים מאגיים של JPEG/PNG; ידני משתמש ברצפי התחלה/סיום מפורשים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2259"/>
        <source>Start Sequence (Hex)</source>
        <translation>רצף התחלה (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2260"/>
        <source>e.g. FF D8 FF</source>
        <translation>לדוגמה FF D8 FF</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2261"/>
        <source>Hex bytes marking the start of an image frame</source>
        <translation>בתים הקסדצימליים המסמנים את תחילת פריים תמונה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2270"/>
        <source>End Sequence (Hex)</source>
        <translation>רצף סיום (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2271"/>
        <source>e.g. FF D9</source>
        <translation>לדוגמה FF D9</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2272"/>
        <source>Hex bytes marking the end of an image frame</source>
        <translation>בתים הקסדצימליים המסמנים את סוף פריים תמונה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2315"/>
        <source>Composite Widget</source>
        <translation>וידג'ט מורכב</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2316"/>
        <source>Select how this group of datasets should be visualized (optional)</source>
        <translation>בחר כיצד קבוצת מערכי נתונים זו תוצג (אופציונלי)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2346"/>
        <source>Device Name</source>
        <translation>שם המכשיר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2347"/>
        <source>Device 1</source>
        <translation>מכשיר 1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2348"/>
        <source>Human-readable name for this input device</source>
        <translation>שם קריא עבור מכשיר קלט זה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2357"/>
        <source>Bus Type</source>
        <translation>סוג אפיק</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2358"/>
        <source>Select the hardware interface for this input device</source>
        <translation>בחירת ממשק החומרה עבור מכשיר קלט זה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2360"/>
        <source>Serial Port</source>
        <translation>יציאה טורית</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2360"/>
        <source>Network Socket</source>
        <translation>שקע רשת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2360"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2362"/>
        <source>Audio Input</source>
        <translation>קלט שמע</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2362"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2362"/>
        <source>CAN Bus</source>
        <translation>מאגיסטרלת CAN</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2362"/>
        <source>Raw USB</source>
        <translation>USB גולמי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2363"/>
        <source>HID Device</source>
        <translation>התקן HID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2363"/>
        <source>Process</source>
        <translation>תהליך</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2363"/>
        <source>MQTT Subscriber</source>
        <translation>מנוי MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2377"/>
        <source>Frame Detection</source>
        <translation>זיהוי מסגרות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2391"/>
        <source>Frame Detection Method</source>
        <translation>שיטת זיהוי מסגרות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2392"/>
        <source>Select how incoming data frames are identified</source>
        <translation>בחר כיצד מזוהות מסגרות נתונים נכנסות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2402"/>
        <source>Hexadecimal Delimiters</source>
        <translation>תוחמים הקסדצימליים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2403"/>
        <source>Enter frame start/end sequences as hexadecimal values</source>
        <translation>הזן רצפי התחלה/סיום מסגרת כערכים הקסדצימליים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2419"/>
        <source>Frame Start Delimiter</source>
        <translation>תוחם התחלת מסגרת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2420"/>
        <source>e.g. /*</source>
        <translation>לדוגמה /*</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2421"/>
        <source>Sequence that marks the beginning of a data frame</source>
        <translation>רצף המסמן את תחילת מסגרת נתונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2433"/>
        <source>Frame End Delimiter</source>
        <translation>תוחם סיום מסגרת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2434"/>
        <source>e.g. */</source>
        <translation>לדוגמה */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2435"/>
        <source>Sequence that marks the end of a data frame</source>
        <translation>רצף המסמן את סיום מסגרת נתונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2441"/>
        <source>Payload Processing &amp; Validation</source>
        <translation>עיבוד ואימות מטען</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2452"/>
        <source>Data Conversion Method</source>
        <translation>שיטת המרת נתונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2453"/>
        <source>Select how incoming binary data is decoded before parsing</source>
        <translation>בחירת אופן פענוח נתונים בינאריים נכנסים לפני ניתוח</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2469"/>
        <source>Checksum Algorithm</source>
        <translation>אלגוריתם Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2470"/>
        <source>Select the checksum algorithm used to validate frames</source>
        <translation>בחר את אלגוריתם ה-Checksum המשמש לאימות מסגרות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2518"/>
        <source>Connection Settings</source>
        <translation>הגדרות חיבור</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2755"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3025"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4583"/>
        <source>General Information</source>
        <translation>מידע כללי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2764"/>
        <source>Action Title</source>
        <translation>כותרת פעולה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2766"/>
        <source>Untitled Action</source>
        <translation>פעולה ללא שם</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2767"/>
        <source>Name or description of this action</source>
        <translation>שם או תיאור של פעולה זו</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2776"/>
        <source>Action Icon</source>
        <translation>סמל פעולה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2777"/>
        <source>Default Icon</source>
        <translation>סמל ברירת מחדל</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2778"/>
        <source>Icon displayed for this action in the dashboard</source>
        <translation>הסמל המוצג עבור פעולה זו בלוח הבקרה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2805"/>
        <source>Target Device</source>
        <translation>התקן יעד</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2806"/>
        <source>Select which connected device this action sends data to</source>
        <translation>בחר לאיזה התקן מחובר לשלוח את הפעולה הזו</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2818"/>
        <source>Data Payload</source>
        <translation>מטען נתונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2829"/>
        <source>Send as Binary</source>
        <translation>שלח כבינארי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2830"/>
        <source>Send raw binary data when this action is triggered</source>
        <translation>שלח נתונים בינאריים גולמיים כאשר הפעולה מופעלת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2841"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2853"/>
        <source>Command</source>
        <translation>פקודה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2842"/>
        <source>Transmit Data (Hex)</source>
        <translation>שידור נתונים (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2843"/>
        <source>Hexadecimal payload to send when the action is triggered</source>
        <translation>מטען הקסדצימלי לשליחה כאשר הפעולה מופעלת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2854"/>
        <source>Transmit Data</source>
        <translation>שידור נתונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2855"/>
        <source>Text payload to send when the action is triggered</source>
        <translation>מטען טקסט לשליחה כאשר הפעולה מופעלת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2866"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4642"/>
        <source>Text Encoding</source>
        <translation>קידוד טקסט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2867"/>
        <source>Character encoding used to serialize the text payload</source>
        <translation>קידוד תווים המשמש לסידור מטען הטקסט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2891"/>
        <source>End-of-Line Sequence</source>
        <translation>רצף סוף שורה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2892"/>
        <source>EOL characters to append to the message (e.g. \n, \r\n)</source>
        <translation>תווי EOL להוספה להודעה (לדוגמה </translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2904"/>
        <source>Execution Behavior</source>
        <translation>התנהגות ביצוע</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2915"/>
        <source>Auto-Execute on Connect</source>
        <translation>ביצוע אוטומטי בהתחברות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2916"/>
        <source>Automatically trigger this action when the device connects</source>
        <translation>הפעלת פעולה זו אוטומטית כאשר ההתקן מתחבר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2922"/>
        <source>Timer Behavior</source>
        <translation>התנהגות טיימר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2931"/>
        <source>Timer Mode</source>
        <translation>מצב טיימר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2934"/>
        <source>Choose when and how this action should repeat automatically</source>
        <translation>בחירת מתי וכיצד פעולה זו תחזור אוטומטית</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2941"/>
        <source>Interval (ms)</source>
        <translation>מרווח (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2945"/>
        <source>Timer Interval (ms)</source>
        <translation>מרווח טיימר (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2946"/>
        <source>Milliseconds between each repeated trigger of this action</source>
        <translation>מילישניות בין כל הפעלה חוזרת של פעולה זו</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2953"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2957"/>
        <source>Repeat Count</source>
        <translation>מספר חזרות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2958"/>
        <source>Number of times to send the command on each trigger</source>
        <translation>מספר הפעמים לשליחת הפקודה בכל הפעלה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3035"/>
        <source>Untitled Dataset</source>
        <translation>Dataset ללא כותרת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3036"/>
        <source>Dataset Title</source>
        <translation>כותרת Dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3037"/>
        <source>Name of the dataset, used for labeling and identification</source>
        <translation>שם ה-Dataset, משמש לתיוג וזיהוי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3049"/>
        <source>Virtual Dataset</source>
        <translation>Dataset וירטואלי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3050"/>
        <source>Virtual datasets compute their value from transforms and data tables, they do not require a frame index</source>
        <translation>Datasets וירטואליים מחשבים את הערך שלהם מטרנספורמציות וטבלאות נתונים, אינם דורשים אינדקס Frame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3066"/>
        <source>Hide on Dashboard</source>
        <translation>הסתר ב-Dashboard</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3067"/>
        <source>Suppress this dataset's standalone dashboard tile; the painter widget can still read its values</source>
        <translation>דיכוי אריח לוח המחוונים העצמאי של מערך נתונים זה; ווידג'ט הציור עדיין יכול לקרוא את ערכיו</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3080"/>
        <source>Frame Index</source>
        <translation>אינדקס מסגרת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3081"/>
        <source>Frame position used for aligning datasets in time</source>
        <translation>מיקום מסגרת המשמש ליישור מערכי נתונים בזמן</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3090"/>
        <source>Measurement Unit</source>
        <translation>יחידת מדידה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3091"/>
        <source>Volts, Amps, etc.</source>
        <translation>וולט, אמפר וכו׳</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3092"/>
        <source>Unit of measurement, such as volts or amps (optional)</source>
        <translation>יחידת מדידה, כגון וולט או אמפר (אופציונלי)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3113"/>
        <source>Lower bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>גבול תחתון של טווח ערכי מערך הנתונים; ווידג'טים ו-FFT חוזרים אליו כאשר הטווח שלהם עצמם לא מוגדר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3126"/>
        <source>Upper bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>גבול עליון של טווח ערכי מערך הנתונים; ווידג'טים ו-FFT חוזרים אליו כאשר הטווח שלהם עצמם לא מוגדר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3139"/>
        <source>Plot Settings</source>
        <translation>הגדרות גרף</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3162"/>
        <source>Enable Plot Widget</source>
        <translation>הפעלת ווידג'ט גרף</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3164"/>
        <source>Plot data in real-time</source>
        <translation>הצגת נתונים בזמן אמת בגרף</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3183"/>
        <source>X-Axis Source</source>
        <translation>מקור ציר X</translation>
    </message>
    <message>
        <source>Choose which dataset to use for the X-Axis in plots</source>
        <translation type="vanished">בחר איזה מערך נתונים להשתמש עבור ציר ה-X בגרפים</translation>
    </message>
    <message>
        <source>Minimum Plot Value (optional)</source>
        <translation type="vanished">ערך מינימום לגרף (אופציונלי)</translation>
    </message>
    <message>
        <source>Lower bound for plot display range</source>
        <translation type="vanished">גבול תחתון לטווח התצוגה של הגרף</translation>
    </message>
    <message>
        <source>Maximum Plot Value (optional)</source>
        <translation type="vanished">ערך מקסימום לגרף (אופציונלי)</translation>
    </message>
    <message>
        <source>Upper bound for plot display range</source>
        <translation type="vanished">גבול עליון לטווח התצוגה של הגרף</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3197"/>
        <source>Frequency Analysis</source>
        <translation>ניתוח תדרים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3208"/>
        <source>Enable FFT Analysis</source>
        <translation>הפעל ניתוח FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3209"/>
        <source>Perform frequency-domain analysis of the dataset</source>
        <translation>בצע ניתוח בתחום התדר של מערך הנתונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3219"/>
        <source>Enable Waterfall Plot</source>
        <translation>הפעל גרף מפל</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3220"/>
        <source>Show a scrolling spectrogram of frequency content over time (Pro)</source>
        <translation>הצג ספקטרוגרמה גוללת של תוכן תדרים לאורך זמן (Pro)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3243"/>
        <source>Waterfall Y Axis</source>
        <translation>ציר Y של מפל</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3244"/>
        <source>Choose Time (default) or any dataset whose value drives the Y axis -- produces a Campbell diagram when bound to e.g. RPM</source>
        <translation>בחר זמן (ברירת מחדל) או כל מערך נתונים שערכו מניע את ציר Y -- מייצר דיאגרמת Campbell כאשר מקושר למשל ל-RPM</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3271"/>
        <source>FFT Window Size</source>
        <translation>גודל חלון FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3272"/>
        <source>Number of samples used for each FFT calculation window</source>
        <translation>מספר הדגימות המשמשות לכל חלון חישוב FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3283"/>
        <source>FFT Sampling Rate (Hz, required)</source>
        <translation>קצב דגימה FFT (Hz, נדרש)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3284"/>
        <source>Sampling frequency used for FFT (in Hz)</source>
        <translation>תדר הדגימה המשמש ל-FFT (ב-Hz)</translation>
    </message>
    <message>
        <source>Minimum Value (recommended)</source>
        <translation type="vanished">ערך מינימלי (מומלץ)</translation>
    </message>
    <message>
        <source>Lower bound for data normalization</source>
        <translation type="vanished">גבול תחתון לנורמליזציה של נתונים</translation>
    </message>
    <message>
        <source>Maximum Value (recommended)</source>
        <translation type="vanished">ערך מקסימלי (מומלץ)</translation>
    </message>
    <message>
        <source>Upper bound for data normalization</source>
        <translation type="vanished">גבול עליון לנורמליזציה של נתונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3333"/>
        <source>Widget Settings</source>
        <translation>הגדרות Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3356"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3357"/>
        <source>Select the visual widget used to display this dataset</source>
        <translation>בחירת ה-widget החזותי המשמש להצגת מערך נתונים זה</translation>
    </message>
    <message>
        <source>Minimum Display Value (required)</source>
        <translation type="vanished">ערך תצוגה מינימלי (נדרש)</translation>
    </message>
    <message>
        <source>Lower bound of the gauge or bar display range</source>
        <translation type="vanished">גבול תחתון של טווח התצוגה במד או בפס</translation>
    </message>
    <message>
        <source>Maximum Display Value (required)</source>
        <translation type="vanished">ערך תצוגה מקסימלי (נדרש)</translation>
    </message>
    <message>
        <source>Upper bound of the gauge or bar display range</source>
        <translation type="vanished">גבול עליון של טווח התצוגה במד או בפס</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3413"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3565"/>
        <source>Auto</source>
        <translation>אוטומטי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3414"/>
        <source>Tick Count</source>
        <translation>מספר סימונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3418"/>
        <source>Major-tick count on the dial scale (0 = auto-fit to widget size)</source>
        <translation>מספר הסימונים הראשיים על סולם החוגה (0 = התאמה אוטומטית לגודל הווידג'ט)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3437"/>
        <source>Label Format</source>
        <translation>עיצוב תווית</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3438"/>
        <source>Decimal places or notation used on tick labels and the value display</source>
        <translation>מספר המקומות העשרוניים או הסימון המשמש בתוויות הסימונים ובתצוגת הערך</translation>
    </message>
    <message>
        <source>Show Value Indicator</source>
        <translation type="vanished">הצג מחוון ערך</translation>
    </message>
    <message>
        <source>Toggle the boxed numeric readout that sits below or beside the widget</source>
        <translation type="vanished">הפעל או כבה את התצוגה המספרית הממוסגרת שנמצאת מתחת או לצד הווידג'ט</translation>
    </message>
    <message>
        <source>Alarm Settings</source>
        <translation type="vanished">הגדרות אזעקה</translation>
    </message>
    <message>
        <source>Enable Alarms</source>
        <translation type="vanished">הפעלת אזעקות</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds alarm thresholds</source>
        <translation type="vanished">מפעיל אזעקה חזותית כאשר הערך חורג מספי האזעקה</translation>
    </message>
    <message>
        <source>Low Threshold</source>
        <translation type="vanished">סף תחתון</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value drops below this threshold</source>
        <translation type="vanished">מפעיל התראה חזותית כאשר הערך יורד מתחת לסף זה</translation>
    </message>
    <message>
        <source>High Threshold</source>
        <translation type="vanished">סף עליון</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds this threshold</source>
        <translation type="vanished">מפעיל התראה חזותית כאשר הערך עולה על סף זה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3474"/>
        <source>LED Display Settings</source>
        <translation>הגדרות תצוגת LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3485"/>
        <source>Show in LED Panel</source>
        <translation>הצג בלוח LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3486"/>
        <source>Enable visual status monitoring using an LED display</source>
        <translation>הפעלת ניטור מצב חזותי באמצעות תצוגת LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3497"/>
        <source>LED On Threshold (required)</source>
        <translation>סף הדלקת LED (נדרש)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3498"/>
        <source>LED lights up when value meets or exceeds this threshold</source>
        <translation>נורית LED נדלקת כאשר הערך מגיע לסף זה או עולה עליו</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3517"/>
        <source>Off</source>
        <translation>כבוי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3517"/>
        <source>Auto Start</source>
        <translation>התחלה אוטומטית</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3517"/>
        <source>Start on Trigger</source>
        <translation>התחלה בהפעלה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3517"/>
        <source>Toggle on Trigger</source>
        <translation>החלפת מצב בהפעלה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3518"/>
        <source>Repeat N Times</source>
        <translation>חזרה N פעמים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3521"/>
        <source>Plain Text (UTF8)</source>
        <translation>טקסט רגיל (UTF8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3521"/>
        <source>Hexadecimal</source>
        <translation>הקסדצימלי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3521"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3522"/>
        <source>Binary (Direct)</source>
        <translation>בינארי (ישיר)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3527"/>
        <source>No Checksum</source>
        <translation>ללא Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3531"/>
        <source>End Delimiter Only</source>
        <translation>מפריד סיום בלבד</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3531"/>
        <source>Start Delimiter Only</source>
        <translation>תוחם התחלה בלבד</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3532"/>
        <source>Start + End Delimiter</source>
        <translation>תוחם התחלה + סיום</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3532"/>
        <source>No Delimiters</source>
        <translation>ללא תוחמים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3539"/>
        <source>Auto-detect</source>
        <translation>זיהוי אוטומטי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3539"/>
        <source>Manual Delimiters</source>
        <translation>תוחמים ידניים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3542"/>
        <source>Button</source>
        <translation>כפתור</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3542"/>
        <source>Slider</source>
        <translation>מחוון</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3542"/>
        <source>Toggle</source>
        <translation>מתג</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3542"/>
        <source>Text Field</source>
        <translation>שדה טקסט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3543"/>
        <source>Knob</source>
        <translation>כפתור סיבוב</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3547"/>
        <source>Data Grid</source>
        <translation>רשת נתונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3548"/>
        <source>GPS Map</source>
        <translation>מפת GPS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3549"/>
        <source>Gyroscope</source>
        <translation>ג'ירוסקופ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3550"/>
        <source>Multiple Plot</source>
        <translation>גרף מרובה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3551"/>
        <source>Accelerometer</source>
        <translation>מד תאוצה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3552"/>
        <source>3D Plot</source>
        <translation>גרף תלת־ממדי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3553"/>
        <source>Image View</source>
        <translation>תצוגת תמונה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3554"/>
        <source>Painter Widget</source>
        <translation>וידג'ט ציור</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3555"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3558"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3573"/>
        <source>None</source>
        <translation>ללא</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3559"/>
        <source>Bar</source>
        <translation>עמודות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3560"/>
        <source>Gauge</source>
        <translation>מד</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3561"/>
        <source>Compass</source>
        <translation>מצפן</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3562"/>
        <source>Meter</source>
        <translation>מד</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">מדחום</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3566"/>
        <source>Integer (0 decimals)</source>
        <translation>מספר שלם (0 מקומות עשרוניים)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3567"/>
        <source>1 decimal</source>
        <translation>ספרה עשרונית אחת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3568"/>
        <source>2 decimals</source>
        <translation>2 ספרות עשרוניות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3569"/>
        <source>3 decimals</source>
        <translation>3 ספרות עשרוניות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3570"/>
        <source>Scientific</source>
        <translation>מדעי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3574"/>
        <source>New Line (\n)</source>
        <translation>שורה חדשה (</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3575"/>
        <source>Carriage Return (\r)</source>
        <translation>החזרת עגלה (\r)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3576"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3579"/>
        <source>No</source>
        <translation>לא</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3580"/>
        <source>Yes</source>
        <translation>כן</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4593"/>
        <source>Label</source>
        <translation>תווית</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4594"/>
        <source>Display label</source>
        <translation>הצג תווית</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4604"/>
        <source>Button Icon</source>
        <translation>סמל כפתור</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4613"/>
        <source>Colorize Icon</source>
        <translation>צבע סמל</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4614"/>
        <source>Tint the icon with the button color</source>
        <translation>צביעת הסמל בצבע הכפתור</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4631"/>
        <source>Initial Value</source>
        <translation>ערך התחלתי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4643"/>
        <source>Character encoding used when transmit() returns a string value</source>
        <translation>קידוד תווים המשמש כאשר transmit() מחזירה ערך מחרוזת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4661"/>
        <source>Value Range</source>
        <translation>טווח ערכים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3112"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4671"/>
        <source>Minimum Value</source>
        <translation>ערך מינימלי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3125"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4680"/>
        <source>Maximum Value</source>
        <translation>ערך מקסימלי</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3184"/>
        <source>Choose Time or a dataset to drive the X-Axis in plots</source>
        <translation>בחר זמן או מערך נתונים להנעת ציר ה-X בגרפים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3294"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3383"/>
        <source>Minimum Value (optional)</source>
        <translation>ערך מינימלי (אופציונלי)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3295"/>
        <source>Lower bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>גבול תחתון לנורמליזציה של נתונים; ברירת מחדל לטווח הערכים של מערך הנתונים כאשר לא מוגדר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3307"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3396"/>
        <source>Maximum Value (optional)</source>
        <translation>ערך מקסימלי (אופציונלי)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3308"/>
        <source>Upper bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>גבול עליון לנורמליזציה של נתונים; ברירת מחדל לטווח הערכים של מערך הנתונים כאשר לא מוגדר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3384"/>
        <source>Lower bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>גבול תחתון של טווח המחוון או העמודה; ברירת מחדל לטווח הערכים של מערך הנתונים כאשר לא מוגדר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3397"/>
        <source>Upper bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>גבול עליון של טווח המחוון או העמודה; ברירת מחדל לטווח הערכים של מערך הנתונים כאשר לא מוגדר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4689"/>
        <source>Step Size</source>
        <translation>גודל צעד</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4888"/>
        <source>Raw and transformed values for every dataset (read-only)</source>
        <translation>ערכים גולמיים ומעובדים עבור כל Dataset (קריאה בלבד)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4897"/>
        <source>Shared table defined in this project</source>
        <translation>טבלה משותפת המוגדרת בפרויקט זה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5244"/>
        <source>Remove 1 widget reference whose target group or dataset no longer exists?</source>
        <translation>להסיר הפניה אחת לווידג'ט שקבוצת היעד או מערך הנתונים שלו אינם קיימים עוד?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5245"/>
        <source>Remove %1 widget references whose target groups or datasets no longer exist?</source>
        <translation>להסיר %1 הפניות לווידג'טים שקבוצות היעד או מערכי הנתונים שלהם אינם קיימים עוד?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5250"/>
        <source>This will only affect workspace tile placement; no groups, datasets, or data are deleted.</source>
        <translation>פעולה זו תשפיע רק על מיקום האריחים במרחב העבודה; לא נמחקות קבוצות, מערכי נתונים או נתונים.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5253"/>
        <source>Clean Up Workspaces</source>
        <translation>ניקוי מרחבי עבודה</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectModel</name>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="388"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="397"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="410"/>
        <source>Project error</source>
        <translation>שגיאת פרויקט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="388"/>
        <source>Project title cannot be empty!</source>
        <translation>כותרת הפרויקט אינה יכולה להיות ריקה!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="397"/>
        <source>You need to add at least one group!</source>
        <translation>יש להוסיף לפחות קבוצה אחת!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="410"/>
        <source>You need to add at least one dataset!</source>
        <translation>יש להוסיף לפחות מערך נתונים אחד!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="457"/>
        <source>Your project needs a title</source>
        <translation>הפרויקט שלך זקוק לכותרת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="459"/>
        <source>Add a group to get started</source>
        <translation>הוסף קבוצה כדי להתחיל</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="461"/>
        <source>Add a dataset to a group</source>
        <translation>הוסף מערך נתונים לקבוצה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="475"/>
        <source>Open the Project view at the top of the tree and enter a name. You can rename the project at any time.</source>
        <translation>פתח את תצוגת הפרויקט בראש העץ והזן שם. ניתן לשנות את שם הפרויקט בכל עת.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="478"/>
        <source>Groups organize datasets into dashboard widgets. Use the Group button in the toolbar above to create one, then add datasets to it.</source>
        <translation>קבוצות מארגנות מערכי נתונים לווידג'טים בלוח הבקרה. השתמש בלחצן קבוצה בסרגל הכלים למעלה כדי ליצור אחת, ולאחר מכן הוסף אליה מערכי נתונים.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="482"/>
        <source>Datasets are the values that appear on the dashboard. Select a group in the tree and use the Dataset button in the toolbar to add one.</source>
        <translation>מערכי נתונים הם הערכים המוצגים בלוח הבקרה. בחר קבוצה בעץ והשתמש בלחצן מערך נתונים בסרגל הכלים כדי להוסיף אחד.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="516"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="525"/>
        <source>Lock Project</source>
        <translation>נעל פרויקט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="517"/>
        <source>Choose a password to lock the project:</source>
        <translation>בחר סיסמה לנעילת הפרויקט:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="525"/>
        <source>Confirm the password:</source>
        <translation>אשר את הסיסמה:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="530"/>
        <source>Passwords do not match</source>
        <translation>הסיסמאות אינן תואמות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="531"/>
        <source>The two passwords you entered do not match. The project was not locked.</source>
        <translation>שתי הסיסמאות שהזנת אינן תואמות. הפרויקט לא ננעל.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="565"/>
        <source>Unlock Project</source>
        <translation>בטל נעילת פרויקט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="566"/>
        <source>Enter the project password:</source>
        <translation>הזן את סיסמת הפרויקט:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="576"/>
        <source>Incorrect password</source>
        <translation>סיסמה שגויה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="577"/>
        <source>The password you entered does not match the one stored in the project file.</source>
        <translation>הסיסמה שהזנת אינה תואמת לזו השמורה בקובץ הפרויקט.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="608"/>
        <source>New Project</source>
        <translation>פרויקט חדש</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">דגימות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="760"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="807"/>
        <source>Time</source>
        <translation>זמן</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1210"/>
        <source>Multiple data sources require a Pro license</source>
        <translation>מספר מקורות נתונים דורשים רישיון Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1211"/>
        <source>Serial Studio Pro allows connecting to multiple devices simultaneously. Please upgrade to unlock this feature.</source>
        <translation>Serial Studio Pro מאפשר התחברות למספר התקנים במקביל. יש לשדרג כדי לפתוח תכונה זו.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1223"/>
        <source>Device %1</source>
        <translation>התקן %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1255"/>
        <source>Do you want to delete data source "%1"?</source>
        <translation>האם למחוק את מקור הנתונים "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1256"/>
        <source>Groups using this source will move to the default source. This action cannot be undone.</source>
        <translation>קבוצות המשתמשות במקור זה יועברו למקור ברירת המחדל. לא ניתן לבטל פעולה זו.</translation>
    </message>
    <message>
        <source> (Copy)</source>
        <translation type="vanished">(עותק)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1486"/>
        <source>Do you want to save your changes?</source>
        <translation>לשמור את השינויים?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1487"/>
        <source>You have unsaved modifications in this project!</source>
        <translation>יש שינויים שלא נשמרו בפרויקט זה!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1526"/>
        <source>Save Serial Studio Project</source>
        <translation>שמירת פרויקט Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1528"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2117"/>
        <source>Serial Studio Project Files (*.ssproj)</source>
        <translation>קבצי פרויקט Serial Studio (*.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1549"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1765"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2108"/>
        <source>Untitled Project</source>
        <translation>פרויקט ללא שם</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1775"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2263"/>
        <source>Device A</source>
        <translation>התקן A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1950"/>
        <source>Select Project File</source>
        <translation>בחירת קובץ פרויקט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1952"/>
        <source>Project Files (*.json *.ssproj)</source>
        <translation>קבצי פרויקט (*.json *.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1996"/>
        <source>JSON validation error</source>
        <translation>שגיאת אימות JSON</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2082"/>
        <source>Project upgraded from an earlier file format</source>
        <translation>הפרויקט שודרג מתבנית קובץ ישנה יותר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2083"/>
        <source>This project was saved with schema version %1; the current version is %2. Defaults have been applied to any new fields. Save the project to lock in the upgrade.</source>
        <translation>פרויקט זה נשמר עם גרסת סכימה %1; הגרסה הנוכחית היא %2. ערכי ברירת מחדל הוחלו על שדות חדשים. שמור את הפרויקט כדי לנעול את השדרוג.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2115"/>
        <source>Save Imported Project</source>
        <translation>שמירת פרויקט מיובא</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2307"/>
        <source>Multi-source projects require a Pro license</source>
        <translation>פרויקטים רב-מקוריים דורשים רישיון Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2308"/>
        <source>This project contains multiple data sources. Only the first source has been loaded. A Serial Studio Pro license is required to use multi-source projects.</source>
        <translation>פרויקט זה מכיל מספר מקורות נתונים. רק המקור הראשון נטען. נדרש רישיון Serial Studio Pro לשימוש בפרויקטים רב-מקוריים.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2541"/>
        <source>Workspace IDs remapped on load</source>
        <translation>מזהי ה-Workspace הותאמו מחדש בעת הטעינה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2542"/>
        <source>%1 custom workspace ID(s) overlapped the new reserved auto range and were moved into the user range. Save the project to make the remap permanent.</source>
        <translation>‏%1 מזהי סביבות עבודה מותאמות חפפו את הטווח האוטומטי החדש והועברו לטווח המשתמש. שמור את הפרויקט כדי להפוך את המיפוי לקבוע.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2684"/>
        <source>Legacy frame parser function updated</source>
        <translation>פונקציית מנתח מסגרות מדור קודם עודכנה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2685"/>
        <source>Your project used a legacy frame parser function with a 'separator' argument. It has been automatically migrated to the new format.</source>
        <translation>הפרויקט שלך השתמש בפונקציית מנתח מסגרות מדור קודם עם ארגומנט 'separator'. היא הומרה אוטומטית לפורמט החדש.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2882"/>
        <source>Do you want to delete group "%1"?</source>
        <translation>למחוק את הקבוצה "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2883"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2928"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2960"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3698"/>
        <source>This action cannot be undone. Do you wish to proceed?</source>
        <translation>לא ניתן לבטל פעולה זו. להמשיך?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2927"/>
        <source>Do you want to delete action "%1"?</source>
        <translation>למחוק את הפעולה "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2959"/>
        <source>Do you want to delete dataset "%1"?</source>
        <translation>למחוק את מערך הנתונים "%1"?</translation>
    </message>
    <message>
        <source>%1 (Copy)</source>
        <translation type="vanished">%1 (עותק)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3610"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3646"/>
        <source>Output Controls</source>
        <translation>בקרות פלט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3658"/>
        <source>New Button</source>
        <translation>כפתור חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3661"/>
        <source>New Slider</source>
        <translation>מחוון חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3664"/>
        <source>New Toggle</source>
        <translation>מתג חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3667"/>
        <source>New Text Field</source>
        <translation>שדה טקסט חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3670"/>
        <source>New Knob</source>
        <translation>כפתור סיבוב חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3697"/>
        <source>Do you want to delete output widget "%1"?</source>
        <translation>למחוק את וידג'ט הפלט "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3862"/>
        <source>Group</source>
        <translation>קבוצה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3880"/>
        <source>New Dataset</source>
        <translation>מערך נתונים חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3883"/>
        <source>New Plot</source>
        <translation>גרף חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3887"/>
        <source>New FFT Plot</source>
        <translation>תרשים FFT חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3891"/>
        <source>New Level Indicator</source>
        <translation>מחוון רמה חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3895"/>
        <source>New Gauge</source>
        <translation>מד חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3899"/>
        <source>New Compass</source>
        <translation>מצפן חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3905"/>
        <source>New Meter</source>
        <translation>מד חדש</translation>
    </message>
    <message>
        <source>New Thermometer</source>
        <translation type="vanished">מדחום חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3909"/>
        <source>New LED Indicator</source>
        <translation>מחוון LED חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3913"/>
        <source>New Waterfall</source>
        <translation>תרשים מפל חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3981"/>
        <source>Channel %1</source>
        <translation>ערוץ %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4052"/>
        <source>New Action</source>
        <translation>פעולה חדשה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4190"/>
        <source>Are you sure you want to change the group-level widget?</source>
        <translation>האם לשנות את הווידג'ט ברמת הקבוצה?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4192"/>
        <source>Existing datasets for this group are deleted</source>
        <translation>מערכי הנתונים הקיימים של קבוצה זו יימחקו</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4255"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4256"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4257"/>
        <source>Accelerometer %1</source>
        <translation>מד תאוצה %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4272"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4272"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4272"/>
        <source>Gyro %1</source>
        <translation>ג'ירוסקופ %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4287"/>
        <source>Latitude</source>
        <translation>קו רוחב</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4287"/>
        <source>Longitude</source>
        <translation>קו אורך</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4287"/>
        <source>Altitude</source>
        <translation>גובה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4302"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4316"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4302"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4316"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4302"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4316"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4574"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5446"/>
        <source>Workspace</source>
        <translation>סביבת עבודה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4739"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4941"/>
        <source>Shared Table</source>
        <translation>טבלה משותפת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4817"/>
        <source>register</source>
        <translation>רגיסטר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4941"/>
        <source>New Shared Table</source>
        <translation>טבלה משותפת חדשה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4941"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4958"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4977"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5001"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5028"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5047"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5069"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5091"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5446"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5467"/>
        <source>Name:</source>
        <translation>שם:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4958"/>
        <source>Rename Table</source>
        <translation>שינוי שם טבלה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4977"/>
        <source>Rename Group</source>
        <translation>שינוי שם קבוצה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5001"/>
        <source>Rename Dataset</source>
        <translation>שינוי שם מערך נתונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5028"/>
        <source>Rename Data Source</source>
        <translation>שינוי שם מקור נתונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5047"/>
        <source>Rename Action</source>
        <translation>שינוי שם פעולה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5068"/>
        <source>New Register</source>
        <translation>רגיסטר חדש</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5091"/>
        <source>Rename Register</source>
        <translation>שינוי שם רגיסטר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5128"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5153"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5969"/>
        <source>This action cannot be undone.</source>
        <translation>לא ניתן לבטל פעולה זו.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5129"/>
        <source>This removes %1 register(s) along with the table. This action cannot be undone.</source>
        <translation>פעולה זו תסיר %1 רגיסטר(ים) יחד עם הטבלה. לא ניתן לבטל פעולה זו.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5132"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5152"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5968"/>
        <source>Delete "%1"?</source>
        <translation>למחוק את "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5135"/>
        <source>Delete Table</source>
        <translation>מחק טבלה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5155"/>
        <source>Delete Register</source>
        <translation>מחק רגיסטר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5177"/>
        <source>Export Table</source>
        <translation>ייצא טבלה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5179"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5219"/>
        <source>CSV files (*.csv)</source>
        <translation>קבצי CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5217"/>
        <source>Import Table</source>
        <translation>ייבא טבלה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5446"/>
        <source>New Workspace</source>
        <translation>סביבת עבודה חדשה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5467"/>
        <source>Rename Workspace</source>
        <translation>שנה שם סביבת עבודה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5550"/>
        <source>Overview</source>
        <translation>סקירה כללית</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5559"/>
        <source>All Data</source>
        <translation>כל הנתונים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5737"/>
        <source>Discard workspace customisations?</source>
        <translation>לבטל התאמות אישיות של סביבת עבודה?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5738"/>
        <source>Switching off Customize discards your edits and rebuilds the workspace list from the project's groups.</source>
        <translation>כיבוי התאמה אישית מבטל את העריכות ובונה מחדש את רשימת סביבות העבודה מקבוצות הפרויקט.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5741"/>
        <source>Customize Workspaces</source>
        <translation>התאמה אישית של סביבות עבודה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5971"/>
        <source>Delete Workspace</source>
        <translation>מחיקת סביבת עבודה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6298"/>
        <source>File save error</source>
        <translation>שגיאת שמירת קובץ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2151"/>
        <source>File open error</source>
        <translation>שגיאת פתיחת קובץ</translation>
    </message>
</context>
<context>
    <name>DataModel::ProtoImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="902"/>
        <source>Import Protocol Buffers File</source>
        <translation>ייבוא קובץ Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="904"/>
        <source>Proto Files (*.proto);;All Files (*)</source>
        <translation>קבצי Proto (*.proto);;כל הקבצים (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="938"/>
        <source>Failed to open proto file: %1</source>
        <translation>כשל בפתיחת קובץ proto: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="939"/>
        <source>Verify the file path and read permissions, then try again.</source>
        <translation>אמת את נתיב הקובץ והרשאות הקריאה, ולאחר מכן נסה שוב.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="941"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="959"/>
        <source>Protobuf Import Error</source>
        <translation>שגיאת ייבוא Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="956"/>
        <source>Failed to parse proto file at line %1: %2</source>
        <translation>ניתוח קובץ proto נכשל בשורה %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="957"/>
        <source>Only proto3 syntax is supported. Verify the file format and try again.</source>
        <translation>רק תחביר proto3 נתמך. אמת את פורמט הקובץ ונסה שוב.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="964"/>
        <source>Proto file contains no message definitions</source>
        <translation>קובץ proto אינו מכיל הגדרות message</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="965"/>
        <source>The selected file has no `message` blocks to import.</source>
        <translation>הקובץ שנבחר אינו מכיל בלוקי `message` לייבוא.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="967"/>
        <source>Protobuf Import Warning</source>
        <translation>אזהרת ייבוא Protobuf</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">טעינת הפרויקט המיובא נכשלה</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">לא ניתן לטעון את JSON של הפרויקט שנוצר.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1005"/>
        <source>Successfully imported %1 message(s) and %2 field(s) from the proto file.</source>
        <translation>%1 הודעות ו־%2 שדות יובאו בהצלחה מקובץ ה־proto.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1008"/>
        <source>The project editor is now open for customization.</source>
        <translation>עורך הפרויקט פתוח כעת להתאמה אישית.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1010"/>
        <source>Protobuf Import Complete</source>
        <translation>ייבוא Protobuf הושלם</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1044"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
</context>
<context>
    <name>DataModel::TransmitTestDialog</name>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="154"/>
        <source>Invalid Hex Input</source>
        <translation>קלט HEX לא תקין</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="155"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>יש להזין בתים הקסדצימליים תקינים.

פורמט תקין: 01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="160"/>
        <source>No transmit function code to evaluate.</source>
        <translation>אין קוד פונקציית שידור להערכה.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="177"/>
        <source>transmit function is not callable</source>
        <translation>לא ניתן לקרוא לפונקציית transmit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="238"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="239"/>
        <source>Clear</source>
        <translation>נקה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="240"/>
        <source>Evaluate</source>
        <translation>הערך</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="241"/>
        <source>Input Value</source>
        <translation>ערך קלט</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="242"/>
        <source>Transmit Function Output</source>
        <translation>פלט פונקציית שידור</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="243"/>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="267"/>
        <source>Enter value to transmit…</source>
        <translation>הזן ערך לשידור…</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="244"/>
        <source>Raw string output appears here</source>
        <translation>פלט מחרוזת גולמית מופיע כאן</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="245"/>
        <source>Hex byte output appears here</source>
        <translation>פלט בתים הקסדצימליים מופיע כאן</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="248"/>
        <source>Test Transmit Function</source>
        <translation>בדיקת פונקציית שידור</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="261"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>הזן בתים הקסדצימליים (לדוגמה, 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="367"/>
        <source>(empty) No data returned</source>
        <translation>(ריק) לא הוחזר מידע</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="369"/>
        <source>0 bytes</source>
        <translation>0 בתים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="408"/>
        <source>%1 byte(s)</source>
        <translation>%1 בתים</translation>
    </message>
</context>
<context>
    <name>DataTablesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="33"/>
        <source>Shared Memory</source>
        <translation>זיכרון משותף</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="149"/>
        <source>Add Shared Table</source>
        <translation>הוסף טבלה משותפת</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="151"/>
        <source>Add shared table</source>
        <translation>הוסף טבלה משותפת</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="160"/>
        <source>Help</source>
        <translation>עזרה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="165"/>
        <source>Open help documentation for shared memory</source>
        <translation>פתח תיעוד עזרה עבור זיכרון משותף</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="174"/>
        <source>Name</source>
        <translation>שם</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="175"/>
        <source>Description</source>
        <translation>תיאור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="176"/>
        <source>Entries</source>
        <translation>ערכים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="274"/>
        <source>No shared tables.</source>
        <translation>אין טבלאות משותפות.</translation>
    </message>
</context>
<context>
    <name>DatabaseExplorer</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="35"/>
        <source>Sessions</source>
        <translation>סשנים</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="218"/>
        <source>Open</source>
        <translation>פתח</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="220"/>
        <source>Open a session file</source>
        <translation>פתח קובץ סשן</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="226"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="229"/>
        <source>Close session file</source>
        <translation>סגור קובץ סשן</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="242"/>
        <source>Replay</source>
        <translation>הפעל מחדש</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="246"/>
        <source>Replay selected session on the dashboard</source>
        <translation>הפעל מחדש את הסשן הנבחר בלוח הבקרה</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="252"/>
        <source>Delete</source>
        <translation>מחק</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="258"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>בטל נעילת קובץ הסשן כדי למחוק סשנים</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="259"/>
        <source>Delete the selected session</source>
        <translation>מחק את הסשן הנבחר</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="276"/>
        <source>Unlock</source>
        <translation>בטל נעילה</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="277"/>
        <source>Lock</source>
        <translation>נעילה</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="282"/>
        <source>Unlock the session file to allow deletions</source>
        <translation>ביטול נעילת קובץ הסשן כדי לאפשר מחיקות</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="283"/>
        <source>Set a password to prevent session deletions</source>
        <translation>הגדרת סיסמה למניעת מחיקת סשנים</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="298"/>
        <source>Export CSV</source>
        <translation>ייצוא CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="303"/>
        <source>Export selected session to CSV</source>
        <translation>ייצוא הסשן הנבחר ל-CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="310"/>
        <source>Export PDF</source>
        <translation>ייצוא PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="315"/>
        <source>Generate a PDF report for the selected session</source>
        <translation>יצירת דוח PDF עבור הסשן הנבחר</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="329"/>
        <source>Restore Project</source>
        <translation>שחזור פרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="333"/>
        <source>Restore the project file from this session file</source>
        <translation>שחזור קובץ הפרויקט מקובץ סשן זה</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="402"/>
        <source>Loading session…</source>
        <translation>טוען סשן…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="403"/>
        <source>Working…</source>
        <translation>עובד…</translation>
    </message>
</context>
<context>
    <name>DatasetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="109"/>
        <source>Pro features detected in this project.</source>
        <translation>זוהו תכונות Pro בפרויקט זה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="111"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>משתמש בווידג'טים חלופיים. רכוש רישיון לפתיחת הפונקציונליות המלאה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="139"/>
        <source>Plots</source>
        <translation>גרפים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="144"/>
        <source>Plot</source>
        <translation>גרף</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="148"/>
        <source>Toggle 2D plot visualization for this dataset</source>
        <translation>הפעל/כבה תצוגת גרף דו-ממדי עבור מערך נתונים זה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="160"/>
        <source>FFT Plot</source>
        <translation>גרף FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="163"/>
        <source>Toggle FFT plot to visualize frequency content</source>
        <translation>הפעל/כבה גרף FFT להצגת תוכן תדרים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="175"/>
        <source>Waterfall</source>
        <translation>מפל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="179"/>
        <source>Toggle waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>הפעל/כבה גרף מפל (ספקטרוגרמה) — משתמש בהגדרות FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="196"/>
        <source>Widgets</source>
        <translation>ווידג'טים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="202"/>
        <source>Bar/Level</source>
        <translation>עמודה/רמה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="206"/>
        <source>Toggle bar/level indicator for this dataset</source>
        <translation>החלף אינדיקטור עמודה/רמה עבור מערך נתונים זה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="217"/>
        <source>Gauge</source>
        <translation>מד</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="222"/>
        <source>Toggle gauge widget for analog-style display</source>
        <translation>החלף ווידג'ט מד לתצוגה בסגנון אנלוגי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="234"/>
        <source>Compass</source>
        <translation>מצפן</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="238"/>
        <source>Toggle compass widget for directional data</source>
        <translation>החלף ווידג'ט מצפן עבור נתוני כיוון</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="249"/>
        <source>Meter</source>
        <translation>מד</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="254"/>
        <source>Toggle analog meter (half-arc) widget</source>
        <translation>החלף ווידג'ט מד אנלוגי (חצי קשת)</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">מדחום</translation>
    </message>
    <message>
        <source>Toggle thermometer widget for temperature data</source>
        <translation type="vanished">החלף ווידג'ט מדחום עבור נתוני טמפרטורה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="265"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="270"/>
        <source>Toggle LED indicator for binary or thresholded values</source>
        <translation>החלף אינדיקטור LED עבור ערכים בינאריים או סף</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="290"/>
        <source>Behavior</source>
        <translation>התנהגות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="295"/>
        <source>Alarm Bands</source>
        <translation>תחומי אזעקה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="304"/>
        <source>Define colored value ranges with severity tiers for this dataset's gauge.</source>
        <translation>הגדרת טווחי ערכים צבעוניים עם דרגות חומרה עבור מד מערך נתונים זה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="310"/>
        <source>Transform</source>
        <translation>טרנספורמציה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="314"/>
        <source>Edit a value transform expression for calibration, filtering, or unit conversion</source>
        <translation>ערוך ביטוי טרנספורמציה של ערך עבור כיול, סינון או המרת יחידות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="327"/>
        <source>Duplicate</source>
        <translation>שכפל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="332"/>
        <source>Duplicate this dataset with the same configuration</source>
        <translation>שכפל מערך נתונים זה עם אותה תצורה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="337"/>
        <source>Delete</source>
        <translation>מחק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="340"/>
        <source>Delete this dataset from the group</source>
        <translation>מחק מערך נתונים זה מהקבוצה</translation>
    </message>
</context>
<context>
    <name>Donate</name>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="37"/>
        <source>Support Serial Studio</source>
        <translation>תמוך ב־Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="86"/>
        <source>Support the development of %1!</source>
        <translation>תמוך בפיתוח של %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="97"/>
        <source>Serial Studio is free &amp; open-source software supported by volunteers. Consider donating or obtaining a Pro license to support development efforts :)</source>
        <translation>Serial Studio היא תוכנה חופשית בקוד פתוח הנתמכת על ידי מתנדבים. שקול לתרום או לרכוש רישיון Pro כדי לתמוך במאמצי הפיתוח :)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="110"/>
        <source>You can also support this project by sharing it, reporting bugs and proposing new features!</source>
        <translation>ניתן גם לתמוך בפרויקט זה על ידי שיתוף, דיווח על באגים והצעת תכונות חדשות!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="124"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="135"/>
        <source>Donate</source>
        <translation>תרום</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="150"/>
        <source>Get Serial Studio Pro</source>
        <translation>רכוש את Serial Studio Pro</translation>
    </message>
</context>
<context>
    <name>Downloader</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="127"/>
        <source>Stop</source>
        <translation>עצור</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="128"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="362"/>
        <source>Downloading updates</source>
        <translation>מוריד עדכונים</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="129"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="455"/>
        <source>Time remaining</source>
        <translation>זמן נותר</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="129"/>
        <source>unknown</source>
        <translation>לא ידוע</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="228"/>
        <source>Error</source>
        <translation>שגיאה</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="228"/>
        <source>Cannot find downloaded update!</source>
        <translation>לא ניתן למצוא את העדכון שהורד!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="244"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="245"/>
        <source>Download complete!</source>
        <translation>ההורדה הושלמה!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="246"/>
        <source>The installer opens separately</source>
        <translation>תוכנית ההתקנה נפתחת בנפרד</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="253"/>
        <source>Click "OK" to begin installing the update</source>
        <translation>לחץ "אישור" כדי להתחיל בהתקנת העדכון</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="255"/>
        <source>In order to install the update, you may need to quit the application.</source>
        <translation>כדי להתקין את העדכון, ייתכן שיהיה צורך לסגור את היישום.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="259"/>
        <source>In order to install the update, you may need to quit the application. This is a mandatory update, exiting now will close the application.</source>
        <translation>כדי להתקין את העדכון, ייתכן שיהיה צורך לסגור את היישום. זהו עדכון חובה, יציאה כעת תסגור את היישום.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="275"/>
        <source>Click the "Open" button to apply the update</source>
        <translation>לחץ על הלחצן "פתח" כדי להחיל את העדכון</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="288"/>
        <source>Updater</source>
        <translation>מעדכן</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="292"/>
        <source>Are you sure you want to cancel the download?</source>
        <translation>האם לבטל את ההורדה?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="294"/>
        <source>Are you sure you want to cancel the download? This is a mandatory update, exiting now will close the application</source>
        <translation>האם לבטל את ההורדה? זהו עדכון חובה, יציאה כעת תסגור את היישום</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="349"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="356"/>
        <source>%1 bytes</source>
        <translation>%1 בתים</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="351"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="358"/>
        <source>%1 KB</source>
        <translation>%1 KB</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="353"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="360"/>
        <source>%1 MB</source>
        <translation>%1 MB</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="362"/>
        <source>of</source>
        <translation>מתוך</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="406"/>
        <source>Downloading Updates</source>
        <translation>מוריד עדכונים</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="407"/>
        <source>Time Remaining</source>
        <translation>זמן נותר</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="407"/>
        <source>Unknown</source>
        <translation>לא ידוע</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="431"/>
        <source>about %1 hours</source>
        <translation>בערך %1 שעות</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="433"/>
        <source>about one hour</source>
        <translation>בערך שעה אחת</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="441"/>
        <source>%1 minutes</source>
        <translation>%1 דקות</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="443"/>
        <source>1 minute</source>
        <translation>דקה אחת</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="450"/>
        <source>%1 seconds</source>
        <translation>%1 שניות</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="452"/>
        <source>1 second</source>
        <translation>שנייה אחת</translation>
    </message>
    <message>
        <source>Time remaining: 0 minutes</source>
        <translation type="vanished">זמן נותר: 0 דקות</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">פתח</translation>
    </message>
</context>
<context>
    <name>ExamplesBrowser</name>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="34"/>
        <source>Examples Browser</source>
        <translation>דפדפן דוגמאות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="91"/>
        <source>Search in Examples…</source>
        <translation>חיפוש בדוגמאות…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="120"/>
        <source>Back</source>
        <translation>חזור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="152"/>
        <source>Pro</source>
        <translation>Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="173"/>
        <source>Download &amp;&amp; Open</source>
        <translation>הורד ופתח</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="188"/>
        <source>View on GitHub</source>
        <translation>הצג ב-GitHub</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="245"/>
        <source>Fetching examples…</source>
        <translation>מאחזר דוגמאות…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="568"/>
        <source>Loading...</source>
        <translation>טוען...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="569"/>
        <source>No README available.</source>
        <translation>אין README זמין.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="609"/>
        <source>Copied to Clipboard</source>
        <translation>הועתק ללוח</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="672"/>
        <source>No screenshot available</source>
        <translation>אין צילום מסך זמין</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="704"/>
        <source>Details</source>
        <translation>פרטים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="733"/>
        <source>Info</source>
        <translation>מידע</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="756"/>
        <source>Category:</source>
        <translation>קטגוריה:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="769"/>
        <source>Difficulty:</source>
        <translation>רמת קושי:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="787"/>
        <source>Project:</source>
        <translation>פרויקט:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="829"/>
        <source>No Results Found</source>
        <translation>לא נמצאו תוצאות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="840"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>בדוק את האיות או נסה מונח חיפוש אחר.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="855"/>
        <source>%1 examples</source>
        <translation>%1 דוגמאות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="864"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>ExtensionManager</name>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="32"/>
        <source>Extension Manager</source>
        <translation>מנהל הרחבות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="78"/>
        <source>Search extensions…</source>
        <translation>חיפוש הרחבות…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="119"/>
        <source>Refresh</source>
        <translation>רענן</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="133"/>
        <source>Repos</source>
        <translation>מאגרים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="163"/>
        <source>Repository Settings</source>
        <translation>הגדרות מאגר</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="175"/>
        <source>Back</source>
        <translation>חזור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="216"/>
        <source>Install</source>
        <translation>התקן</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="233"/>
        <source>Uninstall</source>
        <translation>הסר התקנה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="260"/>
        <source>Run</source>
        <translation>הפעל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="284"/>
        <source>Stop</source>
        <translation>עצור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="318"/>
        <source>Reset</source>
        <translation>אפס</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="368"/>
        <source>Fetching extensions…</source>
        <translation>מביא הרחבות…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="605"/>
        <source>Running</source>
        <translation>פועל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Update</source>
        <translation>עדכן</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Installed</source>
        <translation>מותקן</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="644"/>
        <source>Unavailable</source>
        <translation>לא זמין</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="823"/>
        <source>No description available.</source>
        <translation>אין תיאור זמין.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="864"/>
        <source>Details</source>
        <translation>פרטים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="885"/>
        <source>Type:</source>
        <translation>סוג:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="898"/>
        <source>Author:</source>
        <translation>מחבר:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="910"/>
        <source>Version:</source>
        <translation>גרסה:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="922"/>
        <source>License:</source>
        <translation>רישיון:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="983"/>
        <source>No preview</source>
        <translation>אין תצוגה מקדימה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1012"/>
        <source>  PLUGIN OUTPUT</source>
        <translation>פלט תוסף</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1042"/>
        <source>No output yet. Run the plugin to see its log here.</source>
        <translation>אין פלט עדיין. הפעל את התוסף כדי לראות את היומן שלו כאן.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1077"/>
        <source>No preview available</source>
        <translation>אין תצוגה מקדימה זמינה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1121"/>
        <source>Repositories</source>
        <translation>מאגרים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1134"/>
        <source>Add URLs to remote repositories or local folder paths.</source>
        <translation>הוסף כתובות URL למאגרים מרוחקים או נתיבי תיקיות מקומיות.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1171"/>
        <source>LOCAL</source>
        <translation>מקומי</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1228"/>
        <source>URL or local path…</source>
        <translation>כתובת URL או נתיב מקומי…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1236"/>
        <source>Add</source>
        <translation>הוסף</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1259"/>
        <source>Browse…</source>
        <translation>עיון…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1296"/>
        <source>No Results Found</source>
        <translation>לא נמצאו תוצאות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1307"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>בדוק את האיות או נסה מונח חיפוש אחר.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1331"/>
        <source>No Extensions Available</source>
        <translation>אין הרחבות זמינות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1342"/>
        <source>Add a repository URL or local path in the Repos settings, then refresh.</source>
        <translation>הוסף כתובת URL של מאגר או נתיב מקומי בהגדרות המאגרים, ולאחר מכן רענן.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1357"/>
        <source>%1 extensions</source>
        <translation>%1 הרחבות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1366"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>FFTPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="157"/>
        <source>Interpolation: %1</source>
        <translation>אינטרפולציה: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="185"/>
        <source>Show Area Under Plot</source>
        <translation>הצג שטח מתחת לגרף</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="203"/>
        <source>Show X Axis Label</source>
        <translation>הצג תווית ציר X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="215"/>
        <source>Show Y Axis Label</source>
        <translation>הצג תווית ציר Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="233"/>
        <source>Show Crosshair</source>
        <translation>הצג כוונת</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="240"/>
        <source>Pause</source>
        <translation>השהה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="240"/>
        <source>Resume</source>
        <translation>המשך</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="259"/>
        <source>Reset View</source>
        <translation>אפס תצוגה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="265"/>
        <source>Axis Range Settings</source>
        <translation>הגדרות טווח צירים</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="294"/>
        <source>Magnitude (dB)</source>
        <translation>עוצמה (dB)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="295"/>
        <source>Frequency (Hz)</source>
        <translation>תדר (Hz)</translation>
    </message>
</context>
<context>
    <name>FileDropArea</name>
    <message>
        <location filename="../../qml/Widgets/FileDropArea.qml" line="130"/>
        <source>Drop Projects and CSV files here</source>
        <translation>שחרר כאן קבצי פרויקטים וקבצי CSV</translation>
    </message>
</context>
<context>
    <name>FileTransmission</name>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="34"/>
        <source>File Transmission</source>
        <translation>שידור קובץ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="102"/>
        <source>Transfer Protocol:</source>
        <translation>פרוטוקול העברה:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="135"/>
        <source>File Selection:</source>
        <translation>בחירת קובץ:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="152"/>
        <source>Select File…</source>
        <translation>בחר קובץ…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="170"/>
        <source>Transmission Interval:</source>
        <translation>מרווח שידור:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="196"/>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="272"/>
        <source>msecs</source>
        <translation>אלפיות שנייה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="206"/>
        <source>Block Size:</source>
        <translation>גודל בלוק:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="234"/>
        <source>bytes</source>
        <translation>בתים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="244"/>
        <source>Timeout:</source>
        <translation>פסק זמן:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="282"/>
        <source>Max Retries:</source>
        <translation>ניסיונות מקסימליים:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="340"/>
        <source>Progress: %1%</source>
        <translation>התקדמות: %1%</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="373"/>
        <source>%1 / %2 bytes</source>
        <translation>%1 / %2 בתים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="381"/>
        <source>Errors: %1</source>
        <translation>שגיאות: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="419"/>
        <source>Pause Transmission</source>
        <translation>השהה שידור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="420"/>
        <source>Resume Transmission</source>
        <translation>המשך שידור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="423"/>
        <source>Stop Transmission</source>
        <translation>עצור שידור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="424"/>
        <source>Begin Transmission</source>
        <translation>התחל שידור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="461"/>
        <source>Activity Log</source>
        <translation>יומן פעילות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="465"/>
        <source>Clear</source>
        <translation>נקה</translation>
    </message>
</context>
<context>
    <name>FlowDiagram</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="159"/>
        <source>Frame Parser</source>
        <translation>מנתח מסגרות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="166"/>
        <source>Device %1</source>
        <translation>התקן %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="222"/>
        <source>Group</source>
        <translation>קבוצה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="345"/>
        <source>Action</source>
        <translation>פעולה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="395"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1402"/>
        <source>Output Panel</source>
        <translation>לוח פלט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="439"/>
        <source>Control</source>
        <translation>בקרה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="462"/>
        <source>Table</source>
        <translation>טבלה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="478"/>
        <source>%1 regs</source>
        <translation>%1 רגיסטרים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="479"/>
        <source>empty</source>
        <translation>ריק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="504"/>
        <source>MQTT Publisher</source>
        <translation>מפרסם MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="905"/>
        <source>Open the transform code editor for this dataset.</source>
        <translation>פתח את עורך קוד ההמרה עבור מערך נתונים זה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1184"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1187"/>
        <source>Dataset Container</source>
        <translation>מיכל מערך נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1196"/>
        <source>Multi-Plot</source>
        <translation>גרף מרובה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1199"/>
        <source>Multiple Plot</source>
        <translation>גרף מרובה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1208"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1211"/>
        <source>Accelerometer</source>
        <translation>מד תאוצה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1220"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1223"/>
        <source>Gyroscope</source>
        <translation>גירוסקופ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1232"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1235"/>
        <source>GPS Map</source>
        <translation>מפת GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1243"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1246"/>
        <source>3D Plot</source>
        <translation>גרף תלת־ממדי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1254"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1257"/>
        <source>Image View</source>
        <translation>תצוגת תמונה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1266"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1269"/>
        <source>Painter Widget</source>
        <translation>וידג׳ט ציור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1278"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1281"/>
        <source>Data Grid</source>
        <translation>רשת נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1294"/>
        <source>Generic</source>
        <translation>כללי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1307"/>
        <source>Plot</source>
        <translation>גרף</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1320"/>
        <source>FFT Plot</source>
        <translation>גרף FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1333"/>
        <source>Gauge</source>
        <translation>מד</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1346"/>
        <source>Level Indicator</source>
        <translation>מחוון רמה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1359"/>
        <source>Compass</source>
        <translation>מצפן</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1372"/>
        <source>Meter</source>
        <translation>מד</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">מדחום</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1385"/>
        <source>LED Indicator</source>
        <translation>מחוון LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1414"/>
        <source>Slider</source>
        <translation>סליידר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1427"/>
        <source>Toggle</source>
        <translation>מתג</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1440"/>
        <source>Knob</source>
        <translation>כפתור סיבוב</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1453"/>
        <source>Text Field</source>
        <translation>שדה טקסט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1466"/>
        <source>Button</source>
        <translation>כפתור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1490"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1565"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1652"/>
        <source>Add Group</source>
        <translation>הוסף קבוצה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1505"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1580"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1667"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1712"/>
        <source>Add Dataset</source>
        <translation>הוסף מערך נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1519"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1594"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1681"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1726"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1933"/>
        <source>Add Output</source>
        <translation>הוסף פלט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1535"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1607"/>
        <source>Add Action</source>
        <translation>הוסף פעולה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1544"/>
        <source>Add Data Source</source>
        <translation>הוסף מקור נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1551"/>
        <source>Add Data Table</source>
        <translation>הוסף טבלת נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1618"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1753"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1820"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1948"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1982"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2038"/>
        <source>Rename…</source>
        <translation>שנה שם…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1626"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1783"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1853"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1905"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1956"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2012"/>
        <source>Duplicate</source>
        <translation>שכפל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1637"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1794"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1865"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1917"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1967"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2023"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2049"/>
        <source>Delete…</source>
        <translation>מחק…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1697"/>
        <source>Edit Frame Parser…</source>
        <translation>ערוך מנתח מסגרות…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1739"/>
        <source>Edit Painter Code…</source>
        <translation>ערוך קוד ציור…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1761"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1829"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1881"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1990"/>
        <source>Move Up</source>
        <translation>העבר למעלה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1772"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1841"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1893"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2001"/>
        <source>Move Down</source>
        <translation>העבר למטה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1809"/>
        <source>Edit Transform Code…</source>
        <translation>ערוך קוד טרנספורמציה…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2064"/>
        <source>Edit Code…</source>
        <translation>ערוך קוד…</translation>
    </message>
</context>
<context>
    <name>FrameParserTest</name>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="39"/>
        <source>Test Frame Parser</source>
        <translation>בדיקת מנתח מסגרות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="96"/>
        <source>Frame %1</source>
        <translation>מסגרת %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="98"/>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="209"/>
        <source>Decoder</source>
        <translation>מפענח</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="100"/>
        <source>Rows</source>
        <translation>שורות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="101"/>
        <source>%1 row(s)</source>
        <translation>%1 שורה/שורות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="105"/>
        <source>Row %1</source>
        <translation>שורה %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="164"/>
        <source>Pipeline Configuration</source>
        <translation>תצורת צינור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="186"/>
        <source>Detection</source>
        <translation>זיהוי</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="232"/>
        <source>Start</source>
        <translation>התחלה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="253"/>
        <source>End</source>
        <translation>סוף</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="274"/>
        <source>Checksum</source>
        <translation>Checksum</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="299"/>
        <source>Hex Delimiters</source>
        <translation>תוחמים הקסדצימליים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="315"/>
        <source>Frame Data Input</source>
        <translation>קלט נתוני Frame</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="342"/>
        <source>Enter hex bytes (e.g. 01 A2 FF)</source>
        <translation>הזן בתים הקסדצימליים (לדוגמה 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="343"/>
        <source>Enter raw stream bytes here...</source>
        <translation>הזן בתי זרם גולמיים כאן...</translation>
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
        <translation>הדגימה אינה מכילה את תוחמי ה-Frame המוגדרים, ולכן לא יחולץ Frame. הקלד אותם לתוך הדגימה (לדוגמה 
 לשורה חדשה) או התאם את מצב הזיהוי.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="407"/>
        <source>Pipeline Results</source>
        <translation>תוצאות Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="480"/>
        <source>Stage</source>
        <translation>שלב</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="487"/>
        <source>Value</source>
        <translation>ערך</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="530"/>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation>החילוץ לא יצר מסגרת שלמה. בדוק את תוחמי ההתחלה / הסיום ואת מצב הזיהוי.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="532"/>
        <source>Enter sample data above and press Evaluate to preview the parsed output</source>
        <translation>הזן נתוני דוגמה למעלה ולחץ על הערך כדי לצפות בפלט המפוענח</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="614"/>
        <source>Clear</source>
        <translation>נקה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="625"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="632"/>
        <source>Evaluate</source>
        <translation>הערך</translation>
    </message>
</context>
<context>
    <name>FrameParserView</name>
    <message>
        <source>Undo</source>
        <translation type="vanished">בטל</translation>
    </message>
    <message>
        <source>Redo</source>
        <translation type="vanished">בצע שוב</translation>
    </message>
    <message>
        <source>Cut</source>
        <translation type="vanished">גזור</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="vanished">העתק</translation>
    </message>
    <message>
        <source>Paste</source>
        <translation type="vanished">הדבק</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation type="vanished">בחר הכול</translation>
    </message>
    <message>
        <source>Format Document</source>
        <translation type="vanished">עצב מסמך</translation>
    </message>
    <message>
        <source>Format Selection</source>
        <translation type="vanished">עצב בחירה</translation>
    </message>
    <message>
        <source>Reset</source>
        <translation type="vanished">אפס</translation>
    </message>
    <message>
        <source>Reset to the default parsing script</source>
        <translation type="vanished">אפס לסקריפט ניתוח ברירת המחדל</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">פתח</translation>
    </message>
    <message>
        <source>Import a script file for data parsing</source>
        <translation type="vanished">ייבא קובץ סקריפט לניתוח נתונים</translation>
    </message>
    <message>
        <source>Undo the last code edit</source>
        <translation type="vanished">בטל את עריכת הקוד האחרונה</translation>
    </message>
    <message>
        <source>Redo the previously undone edit</source>
        <translation type="vanished">בצע שוב את העריכה שבוטלה</translation>
    </message>
    <message>
        <source>Cut selected code to clipboard</source>
        <translation type="vanished">גזור קוד מסומן ללוח</translation>
    </message>
    <message>
        <source>Copy selected code to clipboard</source>
        <translation type="vanished">העתק קוד מסומן ללוח</translation>
    </message>
    <message>
        <source>Paste code from clipboard</source>
        <translation type="vanished">הדבק קוד מהלוח</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="vanished">עזרה</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">פתח תיעוד עזרה לניתוח נתונים</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">שפה:</translation>
    </message>
    <message>
        <source>Select Template…</source>
        <translation type="vanished">בחר תבנית…</translation>
    </message>
    <message>
        <source>Test With Sample Data</source>
        <translation type="vanished">בדוק עם נתוני דוגמה</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">הערך</translation>
    </message>
</context>
<context>
    <name>GPS</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="105"/>
        <source>Auto Center</source>
        <translation>מרכוז אוטומטי</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="121"/>
        <source>Plot Trajectory</source>
        <translation>הצג מסלול</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="138"/>
        <source>Zoom In</source>
        <translation>התקרב</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="149"/>
        <source>Zoom Out</source>
        <translation>התרחק</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="173"/>
        <source>Show Weather</source>
        <translation>הצג מזג אוויר</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="191"/>
        <source>NASA Weather Overlay</source>
        <translation>שכבת מזג אוויר NASA</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="223"/>
        <source>Base Map: %1</source>
        <translation>מפת בסיס: %1</translation>
    </message>
</context>
<context>
    <name>GroupView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="97"/>
        <source>Pro features detected in this project.</source>
        <translation>זוהו תכונות Pro בפרויקט זה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="99"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>נעשה שימוש בווידג׳טים חלופיים. רכוש רישיון כדי לפתוח את מלוא הפונקציונליות.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="129"/>
        <source>Add Dataset</source>
        <translation>הוסף מערך נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="136"/>
        <source>Dataset</source>
        <translation>מערך נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="139"/>
        <source>Add a generic dataset to the current group</source>
        <translation>הוסף מערך נתונים גנרי לקבוצה הנוכחית</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="146"/>
        <source>Plot</source>
        <translation>גרף</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="150"/>
        <source>Add a 2D plot to visualize numeric data</source>
        <translation>הוסף גרף דו-ממדי להצגת נתונים מספריים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="158"/>
        <source>FFT Plot</source>
        <translation>גרף FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="163"/>
        <source>Add an FFT plot for frequency domain visualization</source>
        <translation>הוסף גרף FFT להצגת תחום תדרים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="169"/>
        <source>Waterfall</source>
        <translation>מפל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="174"/>
        <source>Add a waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>הוסף גרף מפל (ספקטרוגרמה) — משתמש בהגדרות FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="180"/>
        <source>Bar/Level</source>
        <translation>פס/רמה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="184"/>
        <source>Add a bar or level indicator for scaled values</source>
        <translation>הוסף מחוון פס או רמה לערכים מדורגים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="190"/>
        <source>Gauge</source>
        <translation>מד</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="195"/>
        <source>Add a gauge widget for analog-style visualization</source>
        <translation>הוסף ווידג'ט מד להצגה בסגנון אנלוגי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="202"/>
        <source>Compass</source>
        <translation>מצפן</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="206"/>
        <source>Add a compass to display directional or angular data</source>
        <translation>הוסף מצפן להצגת נתוני כיוון או זווית</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="212"/>
        <source>Meter</source>
        <translation>מד</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="216"/>
        <source>Add an analog meter (half-arc) widget</source>
        <translation>הוספת ווידג'ט מד אנלוגי (חצי קשת)</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">מדחום</translation>
    </message>
    <message>
        <source>Add a thermometer widget for temperature data</source>
        <translation type="vanished">הוספת ווידג'ט מדחום לנתוני טמפרטורה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="223"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="228"/>
        <source>Add an LED indicator for binary status signals</source>
        <translation>הוסף מחוון LED לאותות סטטוס בינאריים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="241"/>
        <source>Add Control</source>
        <translation>הוסף בקרה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="247"/>
        <source>Button</source>
        <translation>כפתור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="251"/>
        <source>Add a button that sends a command on click</source>
        <translation>הוסף כפתור ששולח פקודה בלחיצה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="257"/>
        <source>Slider</source>
        <translation>סליידר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="261"/>
        <source>Add a slider for sending scaled numeric values</source>
        <translation>הוסף סליידר לשליחת ערכים מספריים מדורגים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="267"/>
        <source>Toggle</source>
        <translation>מתג</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="271"/>
        <source>Add a toggle switch for on/off commands</source>
        <translation>הוסף מתג להפעלה/כיבוי פקודות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="278"/>
        <source>Text Field</source>
        <translation>שדה טקסט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="281"/>
        <source>Add a text field for typing and sending commands</source>
        <translation>הוסף שדה טקסט להקלדה ושליחת פקודות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="287"/>
        <source>Knob</source>
        <translation>כפתור סיבוב</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="291"/>
        <source>Add a rotary knob for setpoint control</source>
        <translation>הוסף כפתור סיבוב לבקרת נקודת ייחוס</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="310"/>
        <source>Edit Code</source>
        <translation>ערוך קוד</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="314"/>
        <source>Edit the JavaScript that draws this painter widget</source>
        <translation>ערוך את קוד JavaScript שמצייר את ווידג'ט הציור הזה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="327"/>
        <source>Duplicate</source>
        <translation>שכפל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="331"/>
        <source>Duplicate the current group and its contents</source>
        <translation>שכפל את הקבוצה הנוכחית ואת תוכנה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="336"/>
        <source>Delete</source>
        <translation>מחק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="341"/>
        <source>Delete the current group and all contained datasets</source>
        <translation>מחק את הקבוצה הנוכחית ואת כל מערכי הנתונים שבה</translation>
    </message>
</context>
<context>
    <name>Gyroscope</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="376"/>
        <source>ROLL ↔</source>
        <translation>גלגול ↔</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="404"/>
        <source>YAW ↻</source>
        <translation>סיבוב ↻</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="432"/>
        <source>PITCH ↕</source>
        <translation>הטיה ↕</translation>
    </message>
</context>
<context>
    <name>HID</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="50"/>
        <source>HID Device</source>
        <translation>התקן HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="80"/>
        <source>Usage Page</source>
        <translation>עמוד שימוש</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="96"/>
        <source>Usage</source>
        <translation>שימוש</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="137"/>
        <source>Connect gamepads, joysticks, steering wheels, flight controllers, and other HID-class USB devices.</source>
        <translation>חיבור ג'ויסטיקים, הגאים, הגה הגה, בקרי טיסה והתקני USB ממחלקת HID אחרים.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="145"/>
        <source>HID Usage Tables (USB.org)</source>
        <translation>טבלאות שימוש HID (USB.org)</translation>
    </message>
</context>
<context>
    <name>HelpCenter</name>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="33"/>
        <source>Help Center</source>
        <translation>מרכז עזרה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="95"/>
        <source>Fetching help pages…</source>
        <translation>טוען דפי עזרה…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="129"/>
        <source>Search…</source>
        <translation>חיפוש…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="247"/>
        <source>Loading…</source>
        <translation>טוען…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="291"/>
        <source>Select a page from the sidebar</source>
        <translation>בחר דף מסרגל הצד</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="321"/>
        <source>Copied to Clipboard</source>
        <translation>הועתק ללוח</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="353"/>
        <source>View Online</source>
        <translation>הצג באינטרנט</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="372"/>
        <source>%1 pages</source>
        <translation>%1 דפים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="381"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>IO::ConnectionManager</name>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="267"/>
        <source>UART/COM</source>
        <translation>UART/COM</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="268"/>
        <source>Network Socket</source>
        <translation>Network Socket</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="269"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="271"/>
        <source>Audio</source>
        <translation>אודיו</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="272"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="273"/>
        <source>CAN Bus</source>
        <translation>אפיק CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="274"/>
        <source>USB Device</source>
        <translation>התקן USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="275"/>
        <source>HID Device</source>
        <translation>התקן HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="276"/>
        <source>Process</source>
        <translation>תהליך</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="277"/>
        <source>MQTT Subscriber</source>
        <translation>מנוי MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="620"/>
        <source>Your trial period has ended.</source>
        <translation>תקופת הניסיון הסתיימה.</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="621"/>
        <source>To continue using Serial Studio, please activate your license.</source>
        <translation>להמשך שימוש ב-Serial Studio, יש להפעיל את הרישיון.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Audio</name>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="743"/>
        <source>channels</source>
        <translation>ערוצים</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="796"/>
        <source> channels</source>
        <translation>ערוצים</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="944"/>
        <source>Unsigned 8-bit</source>
        <translation>ללא סימן 8 סיביות</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="945"/>
        <source>Signed 16-bit</source>
        <translation>עם סימן 16 סיביות</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="946"/>
        <source>Signed 24-bit</source>
        <translation>עם סימן 24 סיביות</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="947"/>
        <source>Signed 32-bit</source>
        <translation>עם סימן 32 סיביות</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="948"/>
        <source>Float 32-bit</source>
        <translation>נקודה צפה 32 סיביות</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="951"/>
        <source>Mono</source>
        <translation>מונו</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="952"/>
        <source>Stereo</source>
        <translation>סטריאו</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1332"/>
        <source>Input Device</source>
        <translation>התקן קלט</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1340"/>
        <source>Sample Rate</source>
        <translation>קצב דגימה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1348"/>
        <source>Sample Format</source>
        <translation>פורמט דגימה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1356"/>
        <source>Channels</source>
        <translation>ערוצים</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::BluetoothLE</name>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="69"/>
        <source>BLE I/O Module Error</source>
        <translation>שגיאת מודול BLE קלט/פלט</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="307"/>
        <source>Select Device</source>
        <translation>בחר התקן</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="318"/>
        <source>Select Service</source>
        <translation>בחר שירות</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="329"/>
        <source>Select Characteristic</source>
        <translation>בחר מאפיין</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="479"/>
        <source>Error while configuring BLE service</source>
        <translation>שגיאה בעת הגדרת שירות BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="629"/>
        <source>Operation error</source>
        <translation>שגיאת פעולה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="632"/>
        <source>Characteristic write error</source>
        <translation>שגיאת כתיבת מאפיין</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="635"/>
        <source>Descriptor write error</source>
        <translation>שגיאת כתיבת מתאר</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="638"/>
        <source>Unknown error</source>
        <translation>שגיאה לא ידועה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="641"/>
        <source>Characteristic read error</source>
        <translation>שגיאת קריאת מאפיין</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="644"/>
        <source>Descriptor read error</source>
        <translation>שגיאת קריאת מתאר</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="879"/>
        <source>BLE Device</source>
        <translation>התקן BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="887"/>
        <source>Service</source>
        <translation>שירות</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="905"/>
        <source>Characteristic</source>
        <translation>מאפיין</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::CANBus</name>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="259"/>
        <source>CAN Device Creation Failed</source>
        <translation>יצירת התקן CAN נכשלה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="260"/>
        <source>Unable to create CAN bus device. Check your hardware and drivers.</source>
        <translation>לא ניתן ליצור התקן CAN. בדוק את החומרה והמנהלים.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="283"/>
        <source>CAN Connection Failed</source>
        <translation>חיבור CAN נכשל</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="285"/>
        <source>Unable to connect to CAN bus device. Check your hardware connection and settings.</source>
        <translation>לא ניתן להתחבר להתקן CAN. בדוק את חיבור החומרה וההגדרות.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="302"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="308"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="314"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="319"/>
        <source>CAN Bus Not Available</source>
        <translation>CAN Bus לא זמין</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="303"/>
        <source>No CAN bus plugins found on this system.

On Linux, ensure SocketCAN kernel modules are loaded.</source>
        <translation>לא נמצאו תוספי CAN bus במערכת.

ב-Linux, יש לוודא שמודולי הליבה של SOCKETCAN טעונים.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="309"/>
        <source>No CAN bus plugins found on this system.

On Windows, install CAN hardware drivers (PEAK, Vector, etc.).</source>
        <translation>לא נמצאו תוספי CAN bus במערכת.

ב-Windows, יש להתקין מנהלי התקן לחומרת CAN (PEAK, VECTOR וכו').</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="315"/>
        <source>No CAN bus plugins found on this system.

CAN bus support on macOS is limited and may require third-party hardware drivers.</source>
        <translation>לא נמצאו תוספי CAN bus במערכת.

תמיכה ב-CAN bus ב-macOS מוגבלת ועשויה לדרוש מנהלי התקן של צד שלישי.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="320"/>
        <source>No CAN bus plugins are available on this platform.</source>
        <translation>אין תוספי CAN bus זמינים בפלטפורמה זו.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="332"/>
        <source>Invalid CAN Configuration</source>
        <translation>תצורת CAN לא תקינה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="333"/>
        <source>The CAN bus configuration is incomplete. Select a valid plugin and interface.</source>
        <translation>תצורת CAN bus אינה שלמה. יש לבחור תוסף וממשק תקינים.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="340"/>
        <source>Invalid Selection</source>
        <translation>בחירה לא תקינה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="341"/>
        <source>The selected plugin or interface is no longer available. Refresh the lists and try again.</source>
        <translation>התוסף או הממשק שנבחרו אינם זמינים עוד. יש לרענן את הרשימות ולנסות שוב.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="349"/>
        <source>No Devices Available</source>
        <translation>אין התקנים זמינים</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="350"/>
        <source>The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected.</source>
        <translation>רשימת התוספים או הממשקים ריקה. רענן את הרשימות וודא שחומרת ה-CAN שלך מחוברת.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="666"/>
        <source>CAN Bus Error</source>
        <translation>שגיאת אפיק CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="667"/>
        <source>An error occurred but the CAN device is no longer available.</source>
        <translation>אירעה שגיאה אך התקן ה-CAN אינו זמין עוד.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="674"/>
        <source>Error code: %1</source>
        <translation>קוד שגיאה: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="677"/>
        <source>CAN Bus Communication Error</source>
        <translation>שגיאת תקשורת אפיק CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="692"/>
        <source>Connect a %1 adapter, then refresh</source>
        <translation>חבר מתאם %1 ולאחר מכן רענן</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="696"/>
        <source>Load SocketCAN kernel modules first</source>
        <translation>טען תחילה מודולי ליבה של SOCKETCAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="699"/>
        <source>Set up a virtual CAN interface first</source>
        <translation>הגדר תחילה ממשק CAN וירטואלי</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="701"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="721"/>
        <source>No interfaces found for %1</source>
        <translation>לא נמצאו ממשקים עבור %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="705"/>
        <source>Install &lt;a href='https://www.peak-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN drivers&lt;/a&gt;</source>
        <translation>התקן &lt;a href='https://www.PEAK-system.com/Drivers.523.0.html?&amp;L=1'&gt;מנהלי התקן PEAK CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="709"/>
        <source>Install &lt;a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'&gt;Vector CAN drivers&lt;/a&gt;</source>
        <translation>התקן &lt;a href='https://www.VECTOR.com/us/en/products/products-a-z/libraries-drivers/'&gt;מנהלי התקן VECTOR CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="713"/>
        <source>Install &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN drivers&lt;/a&gt;</source>
        <translation>התקן &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;מנהלי התקן SysTec CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="716"/>
        <source>Install %1 drivers</source>
        <translation>התקן מנהלי התקן %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="719"/>
        <source>Install %1 drivers for macOS</source>
        <translation>התקן מנהלי התקן %1 עבור macOS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="734"/>
        <source>No CAN driver selected</source>
        <translation>לא נבחר מנהל התקן CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="815"/>
        <source>Plugin</source>
        <translation>תוסף</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="823"/>
        <source>Interface</source>
        <translation>ממשק</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="831"/>
        <source>Bitrate</source>
        <translation>קצב סיביות</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="840"/>
        <source>CAN FD</source>
        <translation>CAN FD</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="847"/>
        <source>Loopback</source>
        <translation>Loopback</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="854"/>
        <source>Listen-Only</source>
        <translation>האזנה בלבד</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::GsUsbCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="381"/>
        <source>Failed to initialize libusb for the CANable adapter.</source>
        <translation>אתחול libusb עבור מתאם CANable נכשל.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="409"/>
        <source>Unable to enumerate USB devices.</source>
        <translation>לא ניתן לספור התקני USB.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="429"/>
        <source>The selected CANable adapter is no longer connected, or another application has it open. On Windows the device must use the WinUSB driver (candleLight installs it automatically).</source>
        <translation>מתאם ה-CANable שנבחר אינו מחובר עוד, או שיישום אחר פתח אותו. ב-Windows ההתקן חייב להשתמש במנהל ההתקן WinUSB (candleLight מתקין אותו אוטומטית).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="442"/>
        <source>Could not claim the CANable USB interface.</source>
        <translation>לא ניתן לתפוס את ממשק ה-USB של CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="509"/>
        <source>CANable adapter is not open for writing.</source>
        <translation>מתאם CANable אינו פתוח לכתיבה.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="544"/>
        <source>Failed to transmit CAN frame to the adapter.</source>
        <translation>כשל בשידור מסגרת CAN למתאם.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="560"/>
        <source>CAN bus error reported by the CANable adapter.</source>
        <translation>שגיאת אפיק CAN דווחה על ידי מתאם ה-CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="615"/>
        <source>A CAN frame was not acknowledged on the bus.</source>
        <translation>מסגרת CAN לא אושרה על האפיק.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="707"/>
        <source>CANable adapter rejected the host-format handshake.</source>
        <translation>מתאם CANable דחה את לחיצת היד בפורמט המארח.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="714"/>
        <source>Could not read CANable timing constants.</source>
        <translation>לא ניתן לקרוא את קבועי התזמון של CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="720"/>
        <source>The bitrate %1 bps is not supported by this CANable adapter.</source>
        <translation>קצב הסיביות %1 bps אינו נתמך על ידי מתאם CANable זה.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="726"/>
        <source>CANable adapter rejected the requested bitrate.</source>
        <translation>מתאם CANable דחה את קצב הסיביות המבוקש.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="740"/>
        <source>Could not start the CANable channel.</source>
        <translation>לא ניתן להפעיל את ערוץ CANable.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::HID</name>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="173"/>
        <source>Unknown error</source>
        <translation>שגיאה לא ידועה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="176"/>
        <source>

Check that your user is in the 'plugdev' group or that a udev rule grants access to this device.</source>
        <translation>וודא שהמשתמש שלך נמצא בקבוצת 'plugdev' או שכלל udev מעניק גישה להתקן זה.

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="180"/>
        <source>Failed to open "%1"</source>
        <translation>נכשל לפתוח "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="285"/>
        <source>HID Device Error</source>
        <translation>שגיאת התקן HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="286"/>
        <source>The HID device was disconnected or encountered a fatal read error.</source>
        <translation>התקן ה-HID נותק או נתקל בשגיאת קריאה חמורה.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="395"/>
        <source>Select Device</source>
        <translation>בחר התקן</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="537"/>
        <source>HID Device</source>
        <translation>התקן HID</translation>
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
        <translation>TLS 1.3 או חדש יותר</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="62"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 או חדש יותר</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="63"/>
        <source>Any Protocol</source>
        <translation>כל פרוטוקול</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="64"/>
        <source>Secure Protocols Only</source>
        <translation>פרוטוקולים מאובטחים בלבד</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="66"/>
        <source>None</source>
        <translation>ללא</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="67"/>
        <source>Query Peer</source>
        <translation>שאילתת עמית</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="68"/>
        <source>Verify Peer</source>
        <translation>אימות עמית</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="69"/>
        <source>Auto Verify Peer</source>
        <translation>אימות עמית אוטומטי</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="167"/>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation>תכונת MQTT דורשת רישיון מסחרי</translation>
    </message>
    <message>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio commercial license (Hobbyist tier or above).</source>
        <translation type="vanished">הרשמה למתווך MQTT זמינה רק עם רישיון מסחרי תקף של Serial Studio (רמת Hobbyist ומעלה).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="168"/>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio license or an active trial.</source>
        <translation>הרשמה למתווך MQTT זמינה רק עם רישיון Serial Studio תקף או תקופת ניסיון פעילה.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="393"/>
        <source>Use System Database</source>
        <translation>השתמש במסד נתונים של המערכת</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="394"/>
        <source>Load From Folder…</source>
        <translation>טען מתיקייה…</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="427"/>
        <source>Select PEM Certificates Directory</source>
        <translation>בחר ספריית אישורי PEM</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="682"/>
        <source>Hostname</source>
        <translation>שם מארח</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="689"/>
        <source>Port</source>
        <translation>פורט</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="698"/>
        <source>Topic Filter</source>
        <translation>מסנן נושא</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="705"/>
        <source>Client ID</source>
        <translation>מזהה לקוח</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="712"/>
        <source>Username</source>
        <translation>שם משתמש</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="719"/>
        <source>Password</source>
        <translation>סיסמה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="726"/>
        <source>MQTT Version</source>
        <translation>גרסת MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="734"/>
        <source>Clean Session</source>
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="741"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (שניות)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="750"/>
        <source>Auto Keep Alive</source>
        <translation>Keep Alive אוטומטי</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="767"/>
        <source>SSL/TLS Enabled</source>
        <translation>SSL/TLS מופעל</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="777"/>
        <source>SSL Protocol</source>
        <translation>פרוטוקול SSL</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="785"/>
        <source>Peer Verify Mode</source>
        <translation>מצב אימות עמית</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="793"/>
        <source>Peer Verify Depth</source>
        <translation>עומק אימות עמית</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="894"/>
        <source>MQTT Subscription Error</source>
        <translation>שגיאת מנוי MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="895"/>
        <source>Failed to subscribe to topic "%1".</source>
        <translation>המנוי לנושא "%1" נכשל.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="922"/>
        <source>Invalid MQTT Protocol Version</source>
        <translation>גרסת פרוטוקול MQTT לא חוקית</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="923"/>
        <source>The broker rejected the configured MQTT protocol version.</source>
        <translation>הברוקר דחה את גרסת פרוטוקול MQTT שהוגדרה.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="926"/>
        <source>Client ID Rejected</source>
        <translation>מזהה לקוח נדחה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="927"/>
        <source>The broker rejected the client ID. Try a different identifier.</source>
        <translation>הברוקר דחה את מזהה הלקוח. נסה מזהה אחר.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="930"/>
        <source>MQTT Server Unavailable</source>
        <translation>שרת MQTT לא זמין</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="931"/>
        <source>The broker is currently unavailable. Retry later.</source>
        <translation>הברוקר אינו זמין כעת. נסה שוב מאוחר יותר.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="934"/>
        <source>Authentication Error</source>
        <translation>שגיאת אימות</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="935"/>
        <source>The credentials provided were rejected by the broker.</source>
        <translation>פרטי הגישה שסופקו נדחו על ידי הברוקר.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="938"/>
        <source>Authorization Error</source>
        <translation>שגיאת הרשאה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="939"/>
        <source>Account lacks permission for this operation.</source>
        <translation>לחשבון אין הרשאה לפעולה זו.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="942"/>
        <source>Network or Transport Error</source>
        <translation>שגיאת רשת או תעבורה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="943"/>
        <source>Network/transport layer issue while connecting to the broker.</source>
        <translation>בעיה בשכבת הרשת/תעבורה בעת התחברות ל-Broker.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="946"/>
        <source>MQTT Protocol Violation</source>
        <translation>הפרת פרוטוקול MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="947"/>
        <source>The broker reported a protocol violation and closed the connection.</source>
        <translation>ה-Broker דיווח על הפרת פרוטוקול וסגר את החיבור.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="950"/>
        <source>MQTT 5 Error</source>
        <translation>שגיאת MQTT 5</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="951"/>
        <source>An MQTT 5 protocol-level error occurred.</source>
        <translation>אירעה שגיאה ברמת פרוטוקול MQTT 5.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="954"/>
        <source>MQTT Error</source>
        <translation>שגיאת MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="955"/>
        <source>An unexpected MQTT error occurred.</source>
        <translation>אירעה שגיאת MQTT בלתי צפויה.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Modbus</name>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="364"/>
        <source>Invalid Serial Port</source>
        <translation>יציאה טורית לא חוקית</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="365"/>
        <source>The selected serial port "%1" is no longer available. Refresh the port list and try again.</source>
        <translation>היציאה הטורית שנבחרה "%1" אינה זמינה עוד. יש לרענן את רשימת הפורטים ולנסות שוב.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="409"/>
        <source>Modbus Initialization Failed</source>
        <translation>אתחול Modbus נכשל</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="410"/>
        <source>Unable to create Modbus device. Check your system configuration and try again.</source>
        <translation>לא ניתן ליצור התקן Modbus. בדוק את תצורת המערכת ונסה שוב.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="435"/>
        <source>Modbus Connection Failed</source>
        <translation>חיבור Modbus נכשל</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="437"/>
        <source>Unable to connect to "%1". Check your connection settings.</source>
        <translation>לא ניתן להתחבר אל "%1". בדוק את הגדרות החיבור.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="438"/>
        <source>"%1": %2</source>
        <translation>"%1": %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="559"/>
        <source>None</source>
        <translation>ללא</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="560"/>
        <source>Even</source>
        <translation>זוגי</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="561"/>
        <source>Odd</source>
        <translation>אי-זוגי</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="562"/>
        <source>Space</source>
        <translation>רווח</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="563"/>
        <source>Mark</source>
        <translation>סימון</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="615"/>
        <source>Holding Registers (0x03)</source>
        <translation>רגיסטרי החזקה (0x03)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="616"/>
        <source>Input Registers (0x04)</source>
        <translation>רגיסטרי קלט (0x04)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="617"/>
        <source>Coils (0x01)</source>
        <translation>סלילים (0x01)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="618"/>
        <source>Discrete Inputs (0x02)</source>
        <translation>כניסות בדידות (0x02)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="800"/>
        <source>No register groups configured</source>
        <translation>לא הוגדרו קבוצות רגיסטרים</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="801"/>
        <source>Add at least one register group before generating a project.</source>
        <translation>יש להוסיף לפחות קבוצת רגיסטרים אחת לפני יצירת פרויקט.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="803"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="815"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="840"/>
        <source>Modbus Project Generator</source>
        <translation>מחולל פרויקט Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="812"/>
        <source>Failed to load generated project</source>
        <translation>טעינת הפרויקט שנוצר נכשלה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="813"/>
        <source>The generated project JSON could not be loaded.</source>
        <translation>לא ניתן לטעון את JSON של הפרויקט שנוצר.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="835"/>
        <source>Successfully generated project with %1 groups and %2 datasets.</source>
        <translation>הפרויקט נוצר בהצלחה עם %1 קבוצות ו־%2 מערכי נתונים.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="838"/>
        <source>The project editor is now open for customization.</source>
        <translation>עורך הפרויקט פתוח כעת להתאמה אישית.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="853"/>
        <source>Modbus Project</source>
        <translation>פרויקט Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="858"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="878"/>
        <source>Holding Registers</source>
        <translation>רגיסטרי Holding</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="879"/>
        <source>Input Registers</source>
        <translation>רגיסטרי Input</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="880"/>
        <source>Coils</source>
        <translation>סלילים</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="881"/>
        <source>Discrete Inputs</source>
        <translation>כניסות בדידות</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="895"/>
        <source>Unknown</source>
        <translation>לא ידוע</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="908"/>
        <source>Register %1</source>
        <translation>רגיסטר %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="916"/>
        <source>Coil %1</source>
        <translation>סליל %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="916"/>
        <source>Discrete %1</source>
        <translation>בדיד %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1322"/>
        <source>Error code: %1</source>
        <translation>קוד שגיאה: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1325"/>
        <source>Modbus Communication Error</source>
        <translation>שגיאת תקשורת Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1337"/>
        <source>Select Port</source>
        <translation>בחר פורט</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1490"/>
        <source>Protocol</source>
        <translation>פרוטוקול</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1498"/>
        <source>Slave Address</source>
        <translation>כתובת Slave</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1507"/>
        <source>Poll Interval (ms)</source>
        <translation>מרווח סקירה (ms)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1547"/>
        <source>Host / IP</source>
        <translation>מארח / IP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1554"/>
        <source>Port</source>
        <translation>פורט</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1569"/>
        <source>Serial Port</source>
        <translation>יציאה טורית</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1577"/>
        <source>Baud Rate</source>
        <translation>קצב באוד</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1585"/>
        <source>Parity</source>
        <translation>זוגיות</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1593"/>
        <source>Data Bits</source>
        <translation>ביטי נתונים</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1601"/>
        <source>Stop Bits</source>
        <translation>ביטי עצירה</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Network</name>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="473"/>
        <source>Network socket error</source>
        <translation>שגיאת שקע רשת</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="489"/>
        <source>Socket Type</source>
        <translation>סוג שקע</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="497"/>
        <source>Remote Address</source>
        <translation>כתובת מרוחקת</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="505"/>
        <source>TCP Port</source>
        <translation>פורט TCP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="514"/>
        <source>UDP Local Port</source>
        <translation>פורט מקומי UDP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="523"/>
        <source>UDP Remote Port</source>
        <translation>פורט מרוחק UDP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="532"/>
        <source>UDP Multicast</source>
        <translation>ריבוי שידור UDP</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Process</name>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="183"/>
        <location filename="../../src/IO/Drivers/Process.cpp" line="224"/>
        <source>Failed to start process</source>
        <translation>כשל בהפעלת תהליך</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="184"/>
        <source>Executable "%1" not found in PATH.</source>
        <translation>קובץ הפעלה "%1" לא נמצא ב-PATH.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="368"/>
        <source>Select Executable</source>
        <translation>בחר קובץ הפעלה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="392"/>
        <source>Select Working Directory</source>
        <translation>בחר תיקיית עבודה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="417"/>
        <source>Select Named Pipe / FIFO</source>
        <translation>בחר Named Pipe / FIFO</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="514"/>
        <source>The process crashed.</source>
        <translation>התהליך קרס.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="515"/>
        <source>Exit code: %1</source>
        <translation>קוד יציאה: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="518"/>
        <source>Process "%1" stopped</source>
        <translation>תהליך "%1" הופסק</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="536"/>
        <source>Unknown error</source>
        <translation>שגיאה לא ידועה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="537"/>
        <source>Process Error</source>
        <translation>שגיאת תהליך</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Pipe Error</source>
        <translation>שגיאת צינור</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Could not open named pipe: %1</source>
        <translation>לא ניתן לפתוח צינור בעל שם: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="763"/>
        <source>Mode</source>
        <translation>מצב</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Launch Process</source>
        <translation>הפעלת תהליך</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Named Pipe</source>
        <translation>צינור בעל שם</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="771"/>
        <source>Executable</source>
        <translation>קובץ הפעלה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="778"/>
        <source>Arguments</source>
        <translation>ארגומנטים</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="785"/>
        <source>Working Directory</source>
        <translation>ספריית עבודה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="792"/>
        <source>Pipe Path</source>
        <translation>נתיב צינור</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SeeedCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="205"/>
        <source>The bitrate %1 bps is not supported by the USB-CAN Analyzer.</source>
        <translation>קצב הסיביות %1 bps אינו נתמך על ידי מנתח USB-CAN.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="215"/>
        <source>Could not open serial port %1: %2</source>
        <translation>לא ניתן לפתוח יציאה טורית %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="226"/>
        <source>Failed to initialize the USB-CAN Analyzer.</source>
        <translation>אתחול מנתח USB-CAN נכשל.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="263"/>
        <source>USB-CAN Analyzer is not open for writing.</source>
        <translation>מנתח USB-CAN אינו פתוח לכתיבה.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="309"/>
        <source>CAN bus error reported by the USB-CAN Analyzer.</source>
        <translation>שגיאת אפיק CAN דווחה על ידי מנתח USB-CAN.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SlcanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="166"/>
        <source>The bitrate %1 bps is not a standard slcan rate.</source>
        <translation>קצב הסיביות %1 bps אינו קצב slcan סטנדרטי.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="176"/>
        <source>Could not open serial port %1: %2</source>
        <translation>לא ניתן לפתוח יציאה טורית %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="189"/>
        <source>Failed to open the slcan channel.</source>
        <translation>פתיחת ערוץ slcan נכשלה.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="228"/>
        <source>slcan adapter is not open for writing.</source>
        <translation>מתאם slcan אינו פתוח לכתיבה.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="266"/>
        <source>CAN bus error reported by the slcan adapter.</source>
        <translation>שגיאת אפיק CAN דווחה על ידי מתאם slcan.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::UART</name>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="69"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="70"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="375"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="414"/>
        <source>None</source>
        <translation>ללא</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="245"/>
        <source>Failed to connect to serial port "%1"</source>
        <translation>כשל בהתחברות ליציאה טורית "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="333"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="717"/>
        <source>Select Port</source>
        <translation>בחר פורט</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="376"/>
        <source>Even</source>
        <translation>זוגי</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="377"/>
        <source>Odd</source>
        <translation>אי-זוגי</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="378"/>
        <source>Space</source>
        <translation>רווח</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="379"/>
        <source>Mark</source>
        <translation>סימון</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="415"/>
        <source>RTS/CTS</source>
        <translation>RTS/CTS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="416"/>
        <source>XON/XOFF</source>
        <translation>XON/XOFF</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="546"/>
        <source>"%1" is not a valid path</source>
        <translation>"%1" אינו נתיב חוקי</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="547"/>
        <source>Please type another path to register a custom serial device</source>
        <translation>נא להזין נתיב אחר לרישום התקן טורי מותאם אישית</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="782"/>
        <source>Unknown</source>
        <translation>לא ידוע</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="783"/>
        <source>Critical error on serial port "%1"</source>
        <translation>שגיאה קריטית ביציאה הטורית "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="784"/>
        <source>Unknown error</source>
        <translation>שגיאה לא ידועה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="806"/>
        <source>No error occurred.</source>
        <translation>לא אירעה שגיאה.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="807"/>
        <source>The specified device could not be found. Check the connection and try again.</source>
        <translation>ההתקן המצוין לא נמצא. יש לבדוק את החיבור ולנסות שוב.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="808"/>
        <source>Permission denied. Ensure the application has the necessary access rights to the device.</source>
        <translation>הגישה נדחתה. יש לוודא שליישום יש הרשאות גישה נדרשות להתקן.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="809"/>
        <source>Failed to open the device. It may already be in use or unavailable.</source>
        <translation>פתיחת ההתקן נכשלה. ייתכן שהוא כבר בשימוש או שאינו זמין.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="810"/>
        <source>An error occurred while writing data to the device.</source>
        <translation>אירעה שגיאה בעת כתיבת נתונים להתקן.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="811"/>
        <source>An error occurred while reading data from the device.</source>
        <translation>אירעה שגיאה בעת קריאת נתונים מההתקן.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="812"/>
        <source>A critical resource error occurred. The device may have been disconnected or is no longer accessible.</source>
        <translation>אירעה שגיאת משאב קריטית. ייתכן שההתקן נותק או שאינו זמין עוד.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="813"/>
        <source>The requested operation is not supported on this device.</source>
        <translation>הפעולה המבוקשת אינה נתמכת בהתקן זה.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="814"/>
        <source>An unknown error occurred. Check the device and try again.</source>
        <translation>אירעה שגיאה לא ידועה. בדוק את ההתקן ונסה שוב.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="815"/>
        <source>The operation timed out. The device may not be responding.</source>
        <translation>תם הזמן המוקצב לפעולה. ייתכן שההתקן אינו מגיב.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="816"/>
        <source>The device is not open. Open the device before attempting this operation.</source>
        <translation>ההתקן אינו פתוח. פתח את ההתקן לפני ביצוע פעולה זו.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="978"/>
        <source>Serial Port</source>
        <translation>יציאה טורית</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="986"/>
        <source>Baud Rate</source>
        <translation>קצב שידור</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="994"/>
        <source>Parity</source>
        <translation>זוגיות</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1002"/>
        <source>Data Bits</source>
        <translation>סיביות נתונים</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1010"/>
        <source>Stop Bits</source>
        <translation>סיביות עצירה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1018"/>
        <source>Flow Control</source>
        <translation>בקרת זרימה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1026"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1033"/>
        <source>Auto-Reconnect</source>
        <translation>התחברות מחדש אוטומטית</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::USB</name>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="169"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="177"/>
        <source>USB Error</source>
        <translation>שגיאת USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="170"/>
        <source>Failed to initialize the USB subsystem. Check that libusb is available on your system.</source>
        <translation>אתחול מערכת המשנה של USB נכשל. ודא ש-libusb זמין במערכת שלך.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="178"/>
        <source>No USB device selected. Select a device and try again.</source>
        <translation>לא נבחר התקן USB. בחר התקן ונסה שוב.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="184"/>
        <source>Unknown Device</source>
        <translation>התקן לא ידוע</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="189"/>
        <source>Failed to open "%1"</source>
        <translation>פתיחת "%1" נכשלה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="190"/>
        <source>Could not open the USB device: %1.

On Linux, ensure you have read/write permission on the device node (add a udev rule or run as root). On macOS, the kernel driver may need to be detached first.</source>
        <translation>לא ניתן לפתוח את התקן ה-USB: %1.

ב-Linux, ודא שיש לך הרשאת קריאה/כתיבה לצומת ההתקן (הוסף כלל udev או הרץ כ-root). ב-macOS, ייתכן שיהיה צורך לנתק תחילה את מנהל ההתקן של הליבה.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="212"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="227"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="612"/>
        <source>USB Device Error</source>
        <translation>שגיאת התקן USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="228"/>
        <source>Could not claim interface %1 on the USB device.

Another driver or application may already have it open. On Linux, try unloading the kernel driver (e.g. cdc_acm) or adding a udev rule.</source>
        <translation>לא ניתן לתפוס את הממשק %1 בהתקן ה-USB.

ייתכן שמנהל התקן או יישום אחר כבר פתח אותו. ב-Linux, נסה לפרוק את מנהל ההתקן של הליבה (למשל cdc_acm) או הוסף כלל udev.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="386"/>
        <source>Select Device</source>
        <translation>בחר התקן</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="405"/>
        <source>Select IN Endpoint</source>
        <translation>בחר נקודת קצה IN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="416"/>
        <source>None (Read-only)</source>
        <translation>ללא (קריאה בלבד)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="490"/>
        <source>Enable Advanced USB Control Transfers?</source>
        <translation>הפעל העברות בקרה מתקדמות של USB?</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="491"/>
        <source>This enables control transfers in addition to bulk transfers. Sending incorrect control requests can crash or damage connected hardware. Only enable this if you know what you are doing.</source>
        <translation>זה מאפשר העברות בקרה בנוסף להעברות בתפזורת. שליחת בקשות בקרה שגויות עלולה לקרוס או לפגוע בחומרה המחוברת. הפעל זאת רק אם אתה יודע מה אתה עושה.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="495"/>
        <source>Advanced USB Mode</source>
        <translation>מצב USB מתקדם</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="613"/>
        <source>The USB device was disconnected or encountered a fatal read error.</source>
        <translation>התקן ה-USB נותק או נתקל בשגיאת קריאה קריטית.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="750"/>
        <source>No isochronous IN endpoint was found on this device, but bulk endpoints are available.

Switch the Transfer Mode to "Bulk Stream" and try again.</source>
        <translation>לא נמצאה נקודת קצה IN איזוכרונית בהתקן זה, אך נקודות קצה בתפזורת זמינות.

החלף את מצב ההעברה ל-"זרם בתפזורת" ונסה שוב.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="755"/>
        <source>No bulk IN endpoint was found on this device, but isochronous endpoints are available.

Switch the Transfer Mode to "Isochronous" and try again.</source>
        <translation>לא נמצאה נקודת קצה IN בתפזורת בהתקן זה, אך נקודות קצה איזוכרוניות זמינות.

החלף את מצב ההעברה ל-"איזוכרוני" ונסה שוב.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="759"/>
        <source>No usable IN endpoint was found on this device.

The device may not expose data endpoints in its active configuration, or it may require a specific driver.</source>
        <translation>לא נמצאה נקודת קצה IN שמישה בהתקן זה.

ייתכן שההתקן אינו חושף נקודות קצה לנתונים בתצורה הפעילה שלו, או שהוא דורש מנהל התקן ספציפי.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1185"/>
        <source>USB Device</source>
        <translation>התקן USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1193"/>
        <source>Transfer Mode</source>
        <translation>מצב העברה</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1196"/>
        <source>Bulk Stream</source>
        <translation>זרם Bulk</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1196"/>
        <source>Advanced Control</source>
        <translation>בקרה מתקדמת</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1196"/>
        <source>Isochronous</source>
        <translation>Isochronous</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1201"/>
        <source>IN Endpoint</source>
        <translation>נקודת קצה IN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1209"/>
        <source>OUT Endpoint</source>
        <translation>נקודת קצה OUT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1217"/>
        <source>ISO Packet Size</source>
        <translation>גודל מנת ISO</translation>
    </message>
</context>
<context>
    <name>IO::FileTransmission</name>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="211"/>
        <source>No file selected…</source>
        <translation>לא נבחר קובץ…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="246"/>
        <source>Plain Text</source>
        <translation>טקסט רגיל</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="247"/>
        <source>Raw Binary</source>
        <translation>בינארי גולמי</translation>
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
        <translation>בחר קובץ לשידור</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="291"/>
        <source>File selected: %1 (%2 bytes)</source>
        <translation>קובץ נבחר: %1 (%2 בתים)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="294"/>
        <source>Error opening file: %1</source>
        <translation>שגיאת פתיחת קובץ: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="380"/>
        <source>Starting %1 transfer…</source>
        <translation>מתחיל העברת %1…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="608"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="628"/>
        <source>Transmission complete</source>
        <translation>השידור הושלם</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="610"/>
        <source>Plain text transmission complete</source>
        <translation>שידור טקסט רגיל הושלם</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="630"/>
        <source>Raw binary transmission complete (%1 bytes)</source>
        <translation>שידור בינארי גולמי הושלם (%1 בתים)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="654"/>
        <source>Transfer complete</source>
        <translation>ההעברה הושלמה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="655"/>
        <source>Transfer completed successfully (%1 bytes)</source>
        <translation>ההעברה הושלמה בהצלחה (%1 בתים)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="657"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="658"/>
        <source>Transfer failed: %1</source>
        <translation>ההעברה נכשלה: %1</translation>
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
        <location filename="../../src/IO/FrameReader.cpp" line="345"/>
        <source>Frames dropped</source>
        <translation>מסגרות שנשמטו</translation>
    </message>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="347"/>
        <source>Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) have been dropped. Reduce the data rate or disable a heavy consumer.</source>
        <translation>נתונים נכנסים מגיעים מהר יותר ממה ש-Serial Studio יכול לעבד; %1 מסגרות נשמטו. יש להפחית את קצב הנתונים או להשבית צרכן כבד.</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::XMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="83"/>
        <source>Cannot open file: %1</source>
        <translation>לא ניתן לפתוח קובץ: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="93"/>
        <source>Waiting for receiver…</source>
        <translation>ממתין למקלט…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="112"/>
        <source>Transfer cancelled</source>
        <translation>ההעברה בוטלה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="113"/>
        <source>Transfer cancelled by user</source>
        <translation>ההעברה בוטלה על ידי המשתמש</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="142"/>
        <source>Too many retries, transfer aborted</source>
        <translation>יותר מדי ניסיונות חוזרים, ההעברה הופסקה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="143"/>
        <source>Maximum retries exceeded</source>
        <translation>חריגה ממספר הניסיונות המרבי</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="147"/>
        <source>NAK received, retrying block %1 (%2/%3)</source>
        <translation>התקבל NAK, מנסה שוב בלוק %1 (%2/%3)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="155"/>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="384"/>
        <source>Failed to seek in file</source>
        <translation>כשל בחיפוש בקובץ</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="165"/>
        <source>Transfer cancelled by receiver</source>
        <translation>ההעברה בוטלה על ידי המקלט</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="166"/>
        <source>Receiver cancelled the transfer</source>
        <translation>המקלט ביטל את ההעברה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="180"/>
        <source>Transfer complete</source>
        <translation>ההעברה הושלמה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="207"/>
        <source>Receiver ready (CRC mode), sending data…</source>
        <translation>מקבל מוכן (מצב CRC), שולח נתונים…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="302"/>
        <source>File read returned more data than requested</source>
        <translation>קריאת הקובץ החזירה יותר נתונים מהמבוקש</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="316"/>
        <source>Sending block %1 (%2 bytes)</source>
        <translation>שולח בלוק %1 (%2 בתים)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="329"/>
        <source>Sending EOT…</source>
        <translation>שולח EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="370"/>
        <source>Transfer timed out</source>
        <translation>פסק זמן להעברה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="371"/>
        <source>Timeout: no response from receiver</source>
        <translation>פסק זמן: אין תגובה מהמקבל</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="375"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>פסק זמן, מנסה שוב (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::YMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="62"/>
        <source>Cannot open file: %1</source>
        <translation>לא ניתן לפתוח קובץ: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="74"/>
        <source>Waiting for receiver…</source>
        <translation>ממתין למקבל…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="96"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="173"/>
        <source>Transfer cancelled by receiver</source>
        <translation>ההעברה בוטלה על ידי המקבל</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="97"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="174"/>
        <source>Receiver cancelled the transfer</source>
        <translation>המקבל ביטל את ההעברה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="133"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="316"/>
        <source>Sending first EOT…</source>
        <translation>שולח EOT ראשון…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="151"/>
        <source>Too many retries, transfer aborted</source>
        <translation>יותר מדי ניסיונות חוזרים, ההעברה בוטלה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="152"/>
        <source>Maximum retries exceeded</source>
        <translation>חריגה ממספר הניסיונות המקסימלי</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="156"/>
        <source>NAK received, retrying block %1</source>
        <translation>התקבל NAK, מנסה שוב בלוק %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="161"/>
        <source>Failed to seek in file</source>
        <translation>כשל בניווט בקובץ</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="220"/>
        <source>Sending second EOT…</source>
        <translation>שולח EOT שני…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="242"/>
        <source>Transfer complete</source>
        <translation>ההעברה הושלמה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="284"/>
        <source>Sending file header: %1 (%2 bytes)</source>
        <translation>שולח כותרת קובץ: %1 (%2 בתים)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="299"/>
        <source>Sending end-of-batch marker…</source>
        <translation>שולח סמן סיום אצווה…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="330"/>
        <source>Sending block %1 (%2/%3 bytes)</source>
        <translation>שולח בלוק %1 (%2/%3 בתים)</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::ZMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="86"/>
        <source>Cannot open file: %1</source>
        <translation>לא ניתן לפתוח קובץ: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="103"/>
        <source>File is too large for ZMODEM (%1 bytes, limit 4 GiB).</source>
        <translation>הקובץ גדול מדי עבור ZMODEM (%1 בתים, מגבלה 4 GiB).</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="128"/>
        <source>Transfer cancelled</source>
        <translation>העברה בוטלה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="129"/>
        <source>Transfer cancelled by user</source>
        <translation>העברה בוטלה על ידי המשתמש</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="269"/>
        <source>Hex header CRC mismatch, dropping frame</source>
        <translation>אי-התאמה ב-CRC של כותרת הקסדצימלית, מוריד מסגרת</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="420"/>
        <source>Sending ZRQINIT, waiting for receiver…</source>
        <translation>שולח ZRQINIT, ממתין למקלט…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="444"/>
        <source>Sending file info: %1 (%2 bytes)</source>
        <translation>שולח מידע על קובץ: %1 (%2 בתים)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="459"/>
        <source>Failed to seek to offset %1</source>
        <translation>כשל במעבר לאופסט %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="488"/>
        <source>File read returned more data than requested</source>
        <translation>קריאת הקובץ החזירה יותר נתונים מהמבוקש</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="514"/>
        <source>File data sent, waiting for confirmation…</source>
        <translation>נתוני הקובץ נשלחו, ממתין לאישור…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="525"/>
        <source>Sending ZFIN…</source>
        <translation>שולח ZFIN…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="561"/>
        <source>Receiver ready, sending file info…</source>
        <translation>המקלט מוכן, שולח מידע על הקובץ…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="571"/>
        <source>Receiver requests data from offset %1</source>
        <translation>המקלט מבקש נתונים מהיסט %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="579"/>
        <source>Receiver skipped the file</source>
        <translation>המקלט דילג על הקובץ</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="591"/>
        <source>Too many errors, transfer aborted</source>
        <translation>יותר מדי שגיאות, ההעברה בוטלה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="592"/>
        <source>Maximum retries exceeded</source>
        <translation>חריגה ממספר הניסיונות המרבי</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="596"/>
        <source>NAK received, retrying (%1/%2)…</source>
        <translation>התקבל NAK, מנסה שוב (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="617"/>
        <source>Transfer complete</source>
        <translation>ההעברה הושלמה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="627"/>
        <source>Transfer cancelled by receiver</source>
        <translation>ההעברה בוטלה על ידי המקלט</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="628"/>
        <source>Receiver cancelled the transfer</source>
        <translation>המקבל ביטל את ההעברה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="636"/>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="637"/>
        <source>Receiver reported a file error</source>
        <translation>המקבל דיווח על שגיאת קובץ</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="835"/>
        <source>Transfer timed out</source>
        <translation>פסק זמן להעברה</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="836"/>
        <source>Timeout: no response from receiver</source>
        <translation>פסק זמן: אין תגובה מהמקבל</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="840"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>פסק זמן, מנסה שוב (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="42"/>
        <source>Select Icon</source>
        <translation>בחר סמל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="119"/>
        <source>Search Online…</source>
        <translation>חיפוש מקוון…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="132"/>
        <source>OK</source>
        <translation>אישור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="144"/>
        <source>Cancel</source>
        <translation>ביטול</translation>
    </message>
</context>
<context>
    <name>ImageView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="68"/>
        <source>Normal</source>
        <translation>רגיל</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="69"/>
        <source>Grayscale</source>
        <translation>גווני אפור</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="70"/>
        <source>High Contrast</source>
        <translation>ניגודיות גבוהה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="71"/>
        <source>Vivid</source>
        <translation>חי</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="72"/>
        <source>Night Vision</source>
        <translation>ראיית לילה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="73"/>
        <source>Infrared</source>
        <translation>אינפרא אדום</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="74"/>
        <source>Deep Blue</source>
        <translation>כחול עמוק</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="75"/>
        <source>Amber</source>
        <translation>ענבר</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="172"/>
        <source>Export Images</source>
        <translation>ייצוא תמונות</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="182"/>
        <source>Open Export Folder</source>
        <translation>פתח תיקיית ייצוא</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="198"/>
        <source>Zoom In</source>
        <translation>התקרב</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="211"/>
        <source>Zoom Out</source>
        <translation>התרחק</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="231"/>
        <source>Show Crosshair</source>
        <translation>הצג כוונת</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="556"/>
        <source>Waiting for Image…</source>
        <translation>ממתין לתמונה…</translation>
    </message>
</context>
<context>
    <name>KeyManagerDialog</name>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="23"/>
        <source>API Keys</source>
        <translation>מפתחות API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="47"/>
        <source>Anthropic Claude. The default is Claude Haiku 4.5 ($1 input / $5 output per million tokens). Sonnet 4.6 and Opus 4.7 are also available. Supports streaming, tool use, extended thinking, and prompt caching.</source>
        <translation>Anthropic Claude. ברירת המחדל היא Claude Haiku 4.5 ($1 קלט / $5 פלט למיליון אסימונים). Sonnet 4.6 ו-OPUS 4.7 זמינים גם כן. תומך בהזרמה, שימוש בכלים, חשיבה מורחבת ושמירת פרומפטים במטמון.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="52"/>
        <source>OpenAI Chat Completions. The default is GPT-5 mini for fast, cost-conscious agentic work. GPT-5.2 is the stronger general-purpose option, and GPT-5.2 Chat tracks the model currently used in ChatGPT.</source>
        <translation>OpenAI Chat Completions. ברירת המחדל היא GPT-5 mini לעבודה סוכנית מהירה וחסכונית. GPT-5.2 הוא האופציה החזקה יותר למטרות כלליות, ו-GPT-5.2 Chat עוקב אחר המודל הנוכחי בשימוש ב-ChatGPT.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="57"/>
        <source>Google Gemini. The default is Gemini 2.0 Flash, which has a generous free tier (subject to rate limits). Gemini 1.5 Pro and Gemini 1.5 Flash are also available.</source>
        <translation>Google Gemini. ברירת המחדל היא Gemini 2.0 Flash, שיש לה רמה חינמית נדיבה (כפופה למגבלות קצב). Gemini 1.5 Pro ו-Gemini 1.5 Flash זמינים גם כן.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="61"/>
        <source>DeepSeek. OpenAI-compatible API. The default is deepseek-chat (V3). deepseek-reasoner (R1) is also available. Often the cheapest cloud option for tool use.</source>
        <translation>DeepSeek. API תואם OpenAI. ברירת המחדל היא deepseek-chat (V3). deepseek-reasoner (R1) זמין גם כן. לעתים קרובות האופציה הזולה ביותר בענן לשימוש בכלים.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="65"/>
        <source>OpenRouter. One key, ~200 models from Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier models (suffixed :free) are rate-limited but require no additional billing. Pay-as-you-go for the rest.</source>
        <translation>OpenRouter. מפתח אחד, ~200 מודלים מ-Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen ואחרים. מודלים ברמה חינמית (עם סיומת :free) מוגבלים בקצב אך אינם דורשים חיוב נוסף. תשלום לפי שימוש עבור השאר.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="70"/>
        <source>Groq. Hardware-accelerated inference (LPUs) for very fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen models. Generous free tier with daily token limits.</source>
        <translation>Groq. היסק מואץ בחומרה (LPUs) עבור מודלים מהירים מאוד של Llama, Mixtral, Gemma, DeepSeek-R1 distill ו-Qwen. רמה חינמית נדיבה עם מגבלות אסימונים יומיות.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="74"/>
        <source>Mistral. The default is Mistral Large. Codestral targets code completion, Pixtral handles vision, and the Ministral models are tuned for edge / low-latency use.</source>
        <translation>Mistral. ברירת המחדל היא Mistral Large. Codestral מיועד להשלמת קוד, Pixtral מטפל בראייה, ומודלי Ministral מותאמים לשימוש בקצה / זמן אחזור נמוך.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="78"/>
        <source>Local model server. Works with any OpenAI-compatible endpoint -- Ollama, llama.cpp's llama-server, LM Studio, or vLLM. Nothing leaves your machine. The model list is queried live from the server.</source>
        <translation>שרת מודל מקומי. עובד עם כל נקודת קצה תואמת OpenAI -- Ollama, llama-server של llama.cpp, LM Studio או vLLM. שום דבר לא עוזב את המכשיר שלך. רשימת המודלים נשאלת בזמן אמת מהשרת.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="100"/>
        <source>Bring your own API keys. They are encrypted at rest with a per-machine key and never leave your computer except to communicate with the provider you select.</source>
        <translation>הבא מפתחות API משלך. הם מוצפנים במנוחה עם מפתח ייחודי למכשיר ואינם עוזבים את המחשב שלך אלא כדי לתקשר עם הספק שבחרת.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="166"/>
        <source>Key set</source>
        <translation>מפתח הוגדר</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="167"/>
        <source>No key</source>
        <translation>אין מפתח</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="204"/>
        <source>A key is on file -- paste a new one to replace it</source>
        <translation>מפתח קיים בקובץ -- הדבק מפתח חדש כדי להחליף אותו</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="205"/>
        <source>Paste your API key here</source>
        <translation>הדבק את מפתח ה-API שלך כאן</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="212"/>
        <source>Hide key</source>
        <translation>הסתר מפתח</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="213"/>
        <source>Show key while typing</source>
        <translation>הצג מפתח בזמן הקלדה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="224"/>
        <source>Get key</source>
        <translation>קבל מפתח</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="225"/>
        <source>Open the provider's console to create a new key</source>
        <translation>פתח את קונסולת הספק כדי ליצור מפתח חדש</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="236"/>
        <source>Save</source>
        <translation>שמור</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="254"/>
        <source>Remove the stored key for %1</source>
        <translation>הסר את המפתח השמור עבור %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="278"/>
        <source>http://localhost:11434/v1</source>
        <translation>http://localhost:11434/v1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="282"/>
        <source>Install Ollama</source>
        <translation>התקן Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="283"/>
        <source>Open the Ollama download page</source>
        <translation>פתח את דף ההורדה של Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="294"/>
        <source>Apply</source>
        <translation>החל</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="309"/>
        <source>Re-query the model list</source>
        <translation>שאל מחדש את רשימת המודלים</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="357"/>
        <source>No API keys configured yet. Add a key to get started.</source>
        <translation>לא הוגדרו מפתחות API עדיין. הוסף מפתח כדי להתחיל.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="360"/>
        <source>One provider is ready.</source>
        <translation>ספק אחד מוכן.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="362"/>
        <source>%1 providers are ready.</source>
        <translation>%1 ספקים מוכנים.</translation>
    </message>
</context>
<context>
    <name>LicenseManagement</name>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="37"/>
        <source>Licensing</source>
        <translation>רישוי</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="84"/>
        <source>Please wait…</source>
        <translation>נא להמתין…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="124"/>
        <source>Activate Serial Studio Pro</source>
        <translation>הפעל את Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="131"/>
        <source>Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.</source>
        <translation>הדבק את מפתח הרישיון שלך למטה כדי לפתוח תכונות Pro כמו MQTT, גרפים תלת־ממדיים ועוד.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="138"/>
        <source>Your license includes 5 device activations.
Plans include Monthly, Yearly, and Lifetime options.</source>
        <translation>הרישיון שלך כולל 5 הפעלות מכשיר.
התוכניות כוללות אפשרויות חודשיות, שנתיות ולכל החיים.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="151"/>
        <source>Paste your license key here…</source>
        <translation>הדבק את מפתח הרישיון שלך כאן…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="170"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="332"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="381"/>
        <source>Copy</source>
        <translation>העתק</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="176"/>
        <source>Paste</source>
        <translation>הדבק</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="182"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="338"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="387"/>
        <source>Select All</source>
        <translation>בחר הכול</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="234"/>
        <source>Product</source>
        <translation>מוצר</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="241"/>
        <source>Serial Studio %1</source>
        <translation>Serial Studio %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="252"/>
        <source>Licensee</source>
        <translation>בעל רישיון</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="271"/>
        <source>Licensee E-Mail</source>
        <translation>דוא״ל בעל הרישיון</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="288"/>
        <source>Device Usage</source>
        <translation>שימוש במכשירים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="296"/>
        <source>%1 devices in use (Unlimited plan)</source>
        <translation>%1 מכשירים בשימוש (תוכנית ללא הגבלה)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="297"/>
        <source>%1 of %2 devices used</source>
        <translation>%1 מתוך %2 מכשירים בשימוש</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="307"/>
        <source>Device ID</source>
        <translation>מזהה מכשיר</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="354"/>
        <source>License Key</source>
        <translation>מפתח רישיון</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="409"/>
        <source>Customer Portal</source>
        <translation>פורטל לקוחות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="420"/>
        <source>Buy License</source>
        <translation>רכישת רישיון</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="427"/>
        <source>Activate</source>
        <translation>הפעלה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="437"/>
        <source>Deactivate</source>
        <translation>ביטול הפעלה</translation>
    </message>
</context>
<context>
    <name>Licensing::LemonSqueezy</name>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="515"/>
        <source>There was an issue validating your license.</source>
        <translation>אירעה בעיה באימות הרישיון שלך.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="533"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="714"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="833"/>
        <source>The license key you provided does not belong to Serial Studio.</source>
        <translation>מפתח הרישיון שסיפקת אינו שייך ל-Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="534"/>
        <source>Please double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>יש לוודא שרכשת את הרישיון מהחנות הרשמית של Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="545"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="723"/>
        <source>This license key was activated on a different device.</source>
        <translation>מפתח רישיון זה הופעל במכשיר אחר.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="546"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="724"/>
        <source>Deactivate it there first or contact support for help.</source>
        <translation>יש לבטל את ההפעלה שם תחילה או לפנות לתמיכה לקבלת עזרה.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="557"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="734"/>
        <source>This license is not currently active.</source>
        <translation>רישיון זה אינו פעיל כרגע.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="558"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="735"/>
        <source>It may have expired or been deactivated (status: %1).</source>
        <translation>ייתכן שפג תוקפו או שבוטלה הפעלתו (סטטוס: %1).</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="568"/>
        <source>Something went wrong on the server.</source>
        <translation>משהו השתבש בשרת.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="569"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="745"/>
        <source>No activation ID was returned.</source>
        <translation>לא הוחזר מזהה הפעלה.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="579"/>
        <source>Could not validate your license at this time.</source>
        <translation>לא ניתן לאמת את הרישיון כעת.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="580"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="754"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="843"/>
        <source>Try again later.</source>
        <translation>יש לנסות שוב מאוחר יותר.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="598"/>
        <source>%1 %2</source>
        <translation>%1 %2</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="600"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="651"/>
        <source>Your license has been successfully activated.</source>
        <translation>הרישיון הופעל בהצלחה.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="652"/>
        <source>Thank you for supporting Serial Studio!
You now have access to all premium features.</source>
        <translation>תודה על התמיכה ב-Serial Studio!
כעת יש לך גישה לכל התכונות המתקדמות.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="706"/>
        <source>There was an issue activating your license.</source>
        <translation>הייתה בעיה בהפעלת הרישיון.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="715"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="834"/>
        <source>Double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>יש לוודא שהרישיון נרכש מהחנות הרשמית של Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="744"/>
        <source>Something went wrong on the server…</source>
        <translation>משהו השתבש בשרת…</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="753"/>
        <source>Could not activate your license at this time.</source>
        <translation>לא ניתן להפעיל את הרישיון שלך כעת.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="825"/>
        <source>There was an issue deactivating your license.</source>
        <translation>הייתה בעיה בביטול הרישיון שלך.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="842"/>
        <source>Could not deactivate your license at this time.</source>
        <translation>לא ניתן לבטל את הרישיון שלך כעת.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="852"/>
        <source>Your license has been deactivated.</source>
        <translation>הרישיון שלך בוטל.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="853"/>
        <source>Access to Pro features has been removed.
Thank you again for supporting Serial Studio!</source>
        <translation>הגישה לתכונות Pro הוסרה.
תודה שוב על תמיכתך ב־Serial Studio!</translation>
    </message>
</context>
<context>
    <name>MDF4::Export</name>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="616"/>
        <source>MDF4 Export is a Pro feature.</source>
        <translation>ייצוא MDF4 הוא תכונת Pro.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="617"/>
        <source>This feature requires a license. Please purchase one to enable MDF4 export.</source>
        <translation>תכונה זו דורשת רישיון. יש לרכוש רישיון כדי לאפשר ייצוא MDF4.</translation>
    </message>
</context>
<context>
    <name>MDF4::Player</name>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="357"/>
        <source>Select MDF4 file</source>
        <translation>בחירת קובץ MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="359"/>
        <source>MDF4 files (*.mf4 *.dat)</source>
        <translation>קבצי MDF4 (*.mf4 *.dat)</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="381"/>
        <source>Disconnect from device?</source>
        <translation>להתנתק מהמכשיר?</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="382"/>
        <source>You must disconnect from the current device before opening a MDF4 file.</source>
        <translation>יש להתנתק מההתקן הנוכחי לפני פתיחת קובץ MDF4.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="398"/>
        <source>Cannot open MDF4 file</source>
        <translation>לא ניתן לפתוח קובץ MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="399"/>
        <source>The file may be corrupted or in an unsupported format.</source>
        <translation>הקובץ עשוי להיות פגום או בפורמט לא נתמך.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="406"/>
        <source>Invalid MDF4 file</source>
        <translation>קובץ MDF4 לא תקין</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="407"/>
        <source>Failed to read file structure. The file may be corrupted.</source>
        <translation>קריאת מבנה הקובץ נכשלה. הקובץ עשוי להיות פגום.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="422"/>
        <source>No data in file</source>
        <translation>אין נתונים בקובץ</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="423"/>
        <source>The MDF4 file contains no measurement data.</source>
        <translation>קובץ MDF4 אינו מכיל נתוני מדידה.</translation>
    </message>
</context>
<context>
    <name>MQTT</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="51"/>
        <source>Hostname</source>
        <translation>שם מארח</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="59"/>
        <source>e.g. broker.hivemq.com</source>
        <translation>לדוגמה broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="67"/>
        <source>Port</source>
        <translation>פורט</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="87"/>
        <source>Topic Filter</source>
        <translation>מסנן נושא</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="95"/>
        <source>e.g. sensors/#</source>
        <translation>לדוגמה: sensors/#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="103"/>
        <source>Client ID</source>
        <translation>מזהה לקוח</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="120"/>
        <source>Regenerate</source>
        <translation>צור מחדש</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="130"/>
        <source>Username</source>
        <translation>שם משתמש</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="145"/>
        <source>Password</source>
        <translation>סיסמה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="161"/>
        <source>Version</source>
        <translation>גרסה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="187"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (שניות)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="206"/>
        <source>Clean Session</source>
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="232"/>
        <source>Use SSL/TLS</source>
        <translation>שימוש ב-SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="258"/>
        <source>SSL Protocol</source>
        <translation>פרוטוקול SSL</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="289"/>
        <source>Peer Verify</source>
        <translation>אימות עמית</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="320"/>
        <source>Verify Depth</source>
        <translation>עומק אימות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="341"/>
        <source>CA Certificates</source>
        <translation>אישורי CA</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="349"/>
        <source>Load From Folder…</source>
        <translation>טעינה מתיקייה…</translation>
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
        <translation>TLS 1.3 ומעלה</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="799"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 ומעלה</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="800"/>
        <source>Any Protocol</source>
        <translation>כל פרוטוקול</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="801"/>
        <source>Secure Protocols Only</source>
        <translation>פרוטוקולים מאובטחים בלבד</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="803"/>
        <source>None</source>
        <translation>ללא</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="804"/>
        <source>Query Peer</source>
        <translation>שאילתת עמית</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="805"/>
        <source>Verify Peer</source>
        <translation>אימות עמית</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="806"/>
        <source>Auto Verify Peer</source>
        <translation>אימות עמית אוטומטי</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1123"/>
        <source>Raw RX Data</source>
        <translation>נתוני RX גולמיים</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1124"/>
        <source>Custom Script</source>
        <translation>סקריפט מותאם אישית</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1125"/>
        <source>Dashboard Data (CSV)</source>
        <translation>נתוני לוח בקרה (CSV)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1126"/>
        <source>Dashboard Data (JSON)</source>
        <translation>נתוני לוח בקרה (JSON)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1281"/>
        <source>MQTT publisher unavailable</source>
        <translation>מפרסם MQTT אינו זמין</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1282"/>
        <source>A valid commercial license is required to use MQTT publishing.</source>
        <translation>נדרש רישיון מסחרי תקף לשימוש בפרסום MQTT.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1284"/>
        <location filename="../../src/MQTT/Publisher.cpp" line="1853"/>
        <source>MQTT Test Connection</source>
        <translation>בדיקת חיבור MQTT</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1303"/>
        <source>Select PEM Certificates Directory</source>
        <translation>בחירת ספריית אישורי PEM</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker reachable</source>
        <translation>ברוקר MQTT נגיש</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker unreachable</source>
        <translation>ברוקר MQTT לא נגיש</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT broker connection failed</source>
        <translation>חיבור לברוקר MQTT נכשל</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT Publisher</source>
        <translation>מפרסם MQTT</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherScriptEditor</name>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="49"/>
        <source>MQTT Publisher Script</source>
        <translation>סקריפט מפרסם MQTT</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="86"/>
        <source>JavaScript</source>
        <translation>JavaScript</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="86"/>
        <source>Lua</source>
        <translation>Lua</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="92"/>
        <source>Sample frame bytes (text or hex)</source>
        <translation>בתי מסגרת לדוגמה (טקסט או hex)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="97"/>
        <source>Hex</source>
        <translation>Hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="98"/>
        <source>Test</source>
        <translation>בדיקה</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="99"/>
        <source>Clear</source>
        <translation>נקה</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="101"/>
        <source>Apply</source>
        <translation>החל</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="102"/>
        <source>Cancel</source>
        <translation>ביטול</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="111"/>
        <source>Language:</source>
        <translation>שפה:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="114"/>
        <source>Template:</source>
        <translation>תבנית:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="125"/>
        <source>Frame:</source>
        <translation>מסגרת:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="129"/>
        <source>Output:</source>
        <translation>פלט:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="269"/>
        <source>Enter a frame</source>
        <translation>הזן מסגרת</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="276"/>
        <source>Invalid hex</source>
        <translation>הקסדצימלי לא תקין</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="359"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>עצב מסמך	Ctrl+Shift+I</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="360"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>בחירת עיצוב	Ctrl+I</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="487"/>
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
-- הגדר פונקציית mqtt(frame) שמקבלת את הבתים הגולמיים
-- של מסגרת מפוענחת אחת ומחזירה את המטען לפרסום
-- ל-broker, או nil כדי לדלג על מסגרת זו.
--
-- ארגומנט ה-frame תואם למה שסקריפט ה-Frame Parser
-- רואה: מחרוזת Lua המכילה את הבתים בין מפרידי
-- FrameReader.
--
-- דוגמה:
--   function mqtt(frame)
--     return frame    -- העבר כמו שהוא
--   end
--
-- השתמש בתפריט התבנית לדוגמאות מוכנות.
--
</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="504"/>
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
 * הגדר פונקציית mqtt(frame) שמקבלת את הבתים הגולמיים
 * של מסגרת מפוענחת אחת ומחזירה את המטען לפרסום
 * ל-broker, או null כדי לדלג על מסגרת זו.
 *
 * ארגומנט ה-frame תואם למה שסקריפט ה-Frame Parser
 * רואה: מחרוזת המכילה את הבתים בין מפרידי
 * FrameReader.
 *
 * דוגמה:
 *   function mqtt(frame) {
 *     return frame;   // העבר כמו שהוא
 *   }
 *
 * השתמש בתפריט התבנית לדוגמאות מוכנות.
 */</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="601"/>
        <source>Script is empty</source>
        <translation>הסקריפט ריק</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="608"/>
        <source>Lua engine error</source>
        <translation>שגיאת מנוע Lua</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="630"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="644"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="668"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="682"/>
        <source>Error: %1</source>
        <translation>שגיאה: %1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="638"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="674"/>
        <source>mqtt() is not defined</source>
        <translation>mqtt() אינו מוגדר</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="655"/>
        <source>(nil -- frame skipped)</source>
        <translation>(nil -- מסגרת דולגה)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="657"/>
        <source>(non-string return)</source>
        <translation>(החזרה שאינה מחרוזת)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="687"/>
        <source>(null -- frame skipped)</source>
        <translation>(null -- מסגרת דולגה)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="765"/>
        <source>Select Template…</source>
        <translation>בחר תבנית…</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherWorker</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="674"/>
        <source>Configure broker hostname and port before testing the connection.</source>
        <translation>הגדר שם מארח ופורט של Broker לפני בדיקת החיבור.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="710"/>
        <source>Successfully connected to %1:%2.</source>
        <translation>התחבר בהצלחה אל %1:%2.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="721"/>
        <source>Timed out after 5 seconds without reaching the broker.</source>
        <translation>תם הזמן הקצוב לאחר 5 שניות ללא הגעה ל־Broker.</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="191"/>
        <source>Console Only Mode</source>
        <translation>מצב קונסולה בלבד</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="194"/>
        <source>Quick Plot Mode</source>
        <translation>מצב תרשים מהיר</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="201"/>
        <source>Empty Project</source>
        <translation>פרויקט ריק</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="686"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="694"/>
        <source>Waiting for data…</source>
        <translation>ממתין לנתונים…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="695"/>
        <source>Connecting to device…</source>
        <translation>מתחבר להתקן…</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="146"/>
        <source>Code sample</source>
        <translation>דוגמת קוד</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="148"/>
        <source>Completer</source>
        <translation>משלים אוטומטי</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="150"/>
        <source>Highlighter</source>
        <translation>מדגיש תחביר</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="152"/>
        <source>Style</source>
        <translation>סגנון</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="214"/>
        <source> spaces</source>
        <translation>רווחים</translation>
    </message>
</context>
<context>
    <name>MarkdownWebView</name>
    <message>
        <location filename="../../qml/Widgets/MarkdownWebView.qml" line="36"/>
        <source>Copied to Clipboard</source>
        <translation>הועתק ללוח</translation>
    </message>
</context>
<context>
    <name>Mdf4Player</name>
    <message>
        <location filename="../../qml/Dialogs/Mdf4Player.qml" line="24"/>
        <source>MDF4 Player</source>
        <translation>נגן MDF4</translation>
    </message>
</context>
<context>
    <name>MessageBubble</name>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="97"/>
        <source>Error</source>
        <translation>שגיאה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>You</source>
        <translation>אתה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>Assistant</source>
        <translation>עוזר</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="208"/>
        <source>Discovery</source>
        <translation>גילוי</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="209"/>
        <source>Execution</source>
        <translation>ביצוע</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="239"/>
        <source>Approve %1 actions in %2?</source>
        <translation>לאשר %1 פעולות ב-%2?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="249"/>
        <source>These calls will run together. Expand each card below to inspect arguments.</source>
        <translation>הקריאות הללו יתבצעו ביחד. הרחב כל כרטיס למטה כדי לבדוק ארגומנטים.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="260"/>
        <source>Approve all</source>
        <translation>אשר הכול</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="266"/>
        <source>Deny all</source>
        <translation>דחה הכול</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="332"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="384"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="436"/>
        <source>Copy</source>
        <translation>העתק</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="337"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="389"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="441"/>
        <source>Copy All</source>
        <translation>העתק הכול</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="345"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="397"/>
        <source>Select All</source>
        <translation>בחר הכול</translation>
    </message>
</context>
<context>
    <name>MessageWebView</name>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="63"/>
        <source>You</source>
        <translation>אתה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="64"/>
        <source>Assistant</source>
        <translation>עוזר</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="65"/>
        <location filename="../../qml/AI/MessageWebView.qml" line="71"/>
        <source>Error</source>
        <translation>שגיאה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="66"/>
        <source>Discovery</source>
        <translation>גילוי</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="67"/>
        <source>Execution</source>
        <translation>ביצוע</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="68"/>
        <source>Running</source>
        <translation>פועל</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="69"/>
        <source>Awaiting approval</source>
        <translation>ממתין לאישור</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="70"/>
        <source>Done</source>
        <translation>הושלם</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="72"/>
        <source>Denied</source>
        <translation>נדחה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="73"/>
        <source>Blocked</source>
        <translation>חסום</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="74"/>
        <source>Approve</source>
        <translation>אשר</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="75"/>
        <source>Deny</source>
        <translation>דחה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="76"/>
        <source>Approve all</source>
        <translation>אשר הכול</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="77"/>
        <source>Deny all</source>
        <translation>דחה הכול</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="78"/>
        <source>Arguments</source>
        <translation>ארגומנטים</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="79"/>
        <source>Result</source>
        <translation>תוצאה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="80"/>
        <source>Approve {n} actions in {family}?</source>
        <translation>לאשר {n} פעולות ב-{family}?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="81"/>
        <source>These calls will run together. Expand each card to inspect arguments.</source>
        <translation>קריאות אלו יופעלו יחד. הרחב כל כרטיס כדי לבדוק ארגומנטים.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="82"/>
        <source>Copy</source>
        <translation>העתק</translation>
    </message>
</context>
<context>
    <name>Misc::Examples</name>
    <message>
        <location filename="../../src/Misc/Examples.cpp" line="282"/>
        <source>Failed to load README: %1</source>
        <translation>טעינת README נכשלה: %1</translation>
    </message>
</context>
<context>
    <name>Misc::ExtensionManager</name>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="244"/>
        <source>Theme</source>
        <translation>ערכת נושא</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="247"/>
        <source>Frame Parser</source>
        <translation>מנתח מסגרות</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="250"/>
        <source>Project Template</source>
        <translation>תבנית פרויקט</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="253"/>
        <source>Plugin</source>
        <translation>תוסף</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="256"/>
        <source>All Types</source>
        <translation>כל הסוגים</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="473"/>
        <source>Reset Extensions</source>
        <translation>אפס הרחבות</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="474"/>
        <source>This uninstalls all extensions, removes all custom repositories, and restores the default settings. Continue?</source>
        <translation>פעולה זו מסירה את כל ההרחבות, מוחקת את כל המאגרים המותאמים אישית ומשחזרת את הגדרות ברירת המחדל. להמשיך?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="513"/>
        <source>Select Extension Repository Folder</source>
        <translation>בחר תיקיית מאגר הרחבות</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1014"/>
        <source>Installed (repository no longer available)</source>
        <translation>מותקן (מאגר אינו זמין עוד)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1321"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1331"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1352"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1374"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1418"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1428"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1437"/>
        <source>Plugin Error</source>
        <translation>שגיאת Plugin</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1321"/>
        <source>Plugin "%1" is not installed.</source>
        <translation>Plugin "%1" אינו מותקן.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1332"/>
        <source>Extension "%1" is not a plugin (type: %2).</source>
        <translation>הרחבה "%1" אינה plugin (סוג: %2).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1353"/>
        <source>Cannot read plugin metadata file:
%1/info.json</source>
        <translation>לא ניתן לקרוא קובץ מטא־נתונים של plugin:
%1/info.json</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1375"/>
        <source>Plugin "%1" requires gRPC but this build does not include gRPC support.</source>
        <translation>Plugin "%1" דורש GRPC אך גרסה זו אינה כוללת תמיכה ב־GRPC.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1384"/>
        <source>This plugin uses gRPC for high-performance data streaming. The API server needs to be enabled.

Would you like to enable it now?</source>
        <translation>תוסף זה משתמש ב-GRPC להזרמת נתונים בביצועים גבוהים. יש להפעיל את שרת ה-API.

האם ברצונך להפעיל אותו כעת?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1387"/>
        <source>Plugins need the API server to communicate with Serial Studio. Would you like to enable it now?</source>
        <translation>תוספים זקוקים לשרת API כדי לתקשר עם Serial Studio. האם ברצונך להפעיל אותו כעת?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1390"/>
        <source>API Server Required</source>
        <translation>נדרש שרת API</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1419"/>
        <source>Plugin "%1" has no 'entry' field in info.json.</source>
        <translation>לתוסף "%1" אין שדה 'entry' בקובץ info.json.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1429"/>
        <source>Entry point not found:
%1</source>
        <translation>נקודת הכניסה לא נמצאה:
%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1438"/>
        <source>Plugin "%1" has an invalid entry point path.</source>
        <translation>לתוסף "%1" נתיב נקודת כניסה לא חוקי.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1481"/>
        <source>Missing Dependency</source>
        <translation>תלות חסרה</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1482"/>
        <source>This plugin requires "%1" but it was not found on your system.

Would you like to open the download page?</source>
        <translation>תוסף זה דורש את "%1" אך הוא לא נמצא במערכת שלך.

האם ברצונך לפתוח את דף ההורדה?</translation>
    </message>
</context>
<context>
    <name>Misc::GraphicsBackend</name>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="246"/>
        <source>Restart Required</source>
        <translation>נדרש אתחול מחדש</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="247"/>
        <source>The new rendering backend will take effect after restarting Serial Studio. Restart now to apply the change?</source>
        <translation>מנגנון הרינדור החדש ייכנס לתוקף לאחר אתחול מחדש של Serial Studio. לאתחל כעת כדי להחיל את השינוי?</translation>
    </message>
</context>
<context>
    <name>Misc::HelpCenter</name>
    <message>
        <location filename="../../src/Misc/HelpCenter.cpp" line="303"/>
        <source>Failed to load page: %1</source>
        <translation>טעינת הדף נכשלה: %1</translation>
    </message>
</context>
<context>
    <name>Misc::IconEngine</name>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="148"/>
        <source>Invalid icon identifier</source>
        <translation>מזהה אייקון לא חוקי</translation>
    </message>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="218"/>
        <source>Empty SVG data received</source>
        <translation>התקבל מידע SVG ריק</translation>
    </message>
</context>
<context>
    <name>Misc::ShortcutGenerator</name>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="73"/>
        <source>Windows Shortcut (*.lnk)</source>
        <translation>קיצור דרך Windows (*.lnk)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="75"/>
        <source>macOS Application (*.app)</source>
        <translation>יישום macOS (*.app)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="77"/>
        <source>Desktop Entry (*.desktop)</source>
        <translation>רשומת שולחן עבודה (*.desktop)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="101"/>
        <source>Use a .icns icon for the sharpest result in Finder and the Dock.</source>
        <translation>השתמש בסמל .icns לתוצאה החדה ביותר ב-Finder וב-Dock.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="103"/>
        <source>Leave the icon empty to inherit the Serial Studio executable icon.</source>
        <translation>השאר את הסמל ריק כדי לרשת את סמל קובץ ההפעלה של Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="105"/>
        <source>Place the file under ~/.local/share/applications/ to expose it in your application launcher.</source>
        <translation>מקם את הקובץ תחת ~/.local/share/applications/ כדי לחשוף אותו במפעיל היישומים שלך.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="116"/>
        <source>Apple Icon Image (*.icns)</source>
        <translation>תמונת סמל Apple (*.icns)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="118"/>
        <source>Windows Icon (*.ico)</source>
        <translation>סמל Windows (*.ico)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="120"/>
        <source>Vector or Raster Image (*.svg *.png)</source>
        <translation>תמונה וקטורית או רסטר (*.svg *.png)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="212"/>
        <source>A Pro license is required to generate shortcuts.</source>
        <translation>רישיון Pro נדרש ליצירת קיצורי דרך.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="217"/>
        <source>No output path was provided.</source>
        <translation>לא סופק נתיב פלט.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="258"/>
        <source>Failed to write shortcut file.</source>
        <translation>כתיבת קובץ קיצור דרך נכשלה.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="125"/>
        <source>Could not write the bundle launcher: %1</source>
        <translation>לא ניתן לכתוב את משגר ה-bundle: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="144"/>
        <source>Could not mark the bundle launcher as executable.</source>
        <translation>לא ניתן לסמן את משגר ה-bundle כקובץ הפעלה.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="164"/>
        <source>Could not write Info.plist: %1</source>
        <translation>לא ניתן לכתוב Info.plist: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="222"/>
        <source>Could not replace the existing shortcut at %1.</source>
        <translation>לא ניתן להחליף את קיצור הדרך הקיים ב-%1.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="232"/>
        <source>Could not create the .app bundle directory layout.</source>
        <translation>לא ניתן ליצור את מבנה התיקיות של bundle מסוג .app.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="140"/>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="271"/>
        <source>Windows shortcut writer is not available on this platform.</source>
        <translation>כותב קיצורי דרך של Windows אינו זמין בפלטפורמה זו.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="285"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="199"/>
        <source>Linux shortcut writer is not available on this platform.</source>
        <translation>כותב קיצורי דרך של Linux אינו זמין בפלטפורמה זו.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="107"/>
        <source>Could not initialise COM (required to write .lnk shortcuts).</source>
        <translation>לא ניתן לאתחל COM (נדרש לכתיבת קיצורי דרך .lnk).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="118"/>
        <source>CoCreateInstance(IShellLink) failed (HRESULT 0x%1).</source>
        <translation>CoCreateInstance(IShellLink) נכשל (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="153"/>
        <source>QueryInterface(IPersistFile) failed (HRESULT 0x%1).</source>
        <translation>QueryInterface(IPersistFile) נכשל (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="163"/>
        <source>Saving the .lnk file failed (HRESULT 0x%1).</source>
        <translation>שמירת קובץ .lnk נכשלה (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="154"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="185"/>
        <source>macOS shortcut writer is not available on this platform.</source>
        <translation>כותב קיצורי דרך של macOS אינו זמין בפלטפורמה זו.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="86"/>
        <source>Could not open the shortcut path for writing: %1</source>
        <translation>לא ניתן לפתוח את נתיב קיצור הדרך לכתיבה: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="91"/>
        <source>Serial Studio shortcut</source>
        <translation>קיצור דרך Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="112"/>
        <source>Could not mark the shortcut as executable.</source>
        <translation>לא ניתן לסמן את קיצור הדרך כקובץ הרצה.</translation>
    </message>
</context>
<context>
    <name>Misc::ThemeManager</name>
    <message>
        <location filename="../../src/Misc/ThemeManager.cpp" line="398"/>
        <source>System</source>
        <translation>מערכת</translation>
    </message>
</context>
<context>
    <name>Misc::Utilities</name>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="161"/>
        <source>Ok</source>
        <translation>אישור</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="163"/>
        <source>Save</source>
        <translation>שמור</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="165"/>
        <source>Save all</source>
        <translation>שמור הכול</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="167"/>
        <source>Open</source>
        <translation>פתח</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="169"/>
        <source>Yes</source>
        <translation>כן</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="171"/>
        <source>Yes to all</source>
        <translation>כן לכול</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="173"/>
        <source>No</source>
        <translation>לא</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="175"/>
        <source>No to all</source>
        <translation>לא לכול</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="177"/>
        <source>Abort</source>
        <translation>בטל</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="179"/>
        <source>Retry</source>
        <translation>נסה שוב</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="181"/>
        <source>Ignore</source>
        <translation>התעלם</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="183"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="185"/>
        <source>Cancel</source>
        <translation>ביטול</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="187"/>
        <source>Discard</source>
        <translation>התעלם</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="189"/>
        <source>Help</source>
        <translation>עזרה</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="191"/>
        <source>Apply</source>
        <translation>החל</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="193"/>
        <source>Reset</source>
        <translation>אפס</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="195"/>
        <source>Restore defaults</source>
        <translation>שחזר ברירות מחדל</translation>
    </message>
</context>
<context>
    <name>Misc::WorkspaceManager</name>
    <message>
        <location filename="../../src/Misc/WorkspaceManager.cpp" line="261"/>
        <source>Select Workspace Location</source>
        <translation>בחר מיקום סביבת עבודה</translation>
    </message>
</context>
<context>
    <name>Modbus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="49"/>
        <source>Protocol</source>
        <translation>פרוטוקול</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="72"/>
        <source>Serial Port</source>
        <translation>יציאה טורית</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="97"/>
        <source>Baud Rate</source>
        <translation>קצב שידור</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="201"/>
        <source>Parity</source>
        <translation>זוגיות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="222"/>
        <source>Data Bits</source>
        <translation>סיביות נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="245"/>
        <source>Stop Bits</source>
        <translation>סיביות עצירה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="268"/>
        <source>Host</source>
        <translation>מארח</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="278"/>
        <source>IP Address</source>
        <translation>כתובת IP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="292"/>
        <source>Port</source>
        <translation>פורט</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="301"/>
        <source>TCP Port</source>
        <translation>פורט TCP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="329"/>
        <source>Slave Address</source>
        <translation>כתובת Slave</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="334"/>
        <source>1-247</source>
        <translation>1-247</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="349"/>
        <source>Poll Interval (ms)</source>
        <translation>מרווח סקירה (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="354"/>
        <source>Polling interval</source>
        <translation>מרווח סקירה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="382"/>
        <source>Configure Register Groups…</source>
        <translation>הגדר קבוצות רגיסטרים…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="392"/>
        <source>Import Register Map…</source>
        <translation>ייבא מפת רגיסטרים…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="407"/>
        <source>%1 group(s) configured</source>
        <translation>%1 קבוצות מוגדרות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="408"/>
        <source>No groups configured</source>
        <translation>אין קבוצות מוגדרות</translation>
    </message>
</context>
<context>
    <name>ModbusGroupsDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="45"/>
        <source>Modbus Register Groups</source>
        <translation>קבוצות רגיסטרים של Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="166"/>
        <source>Configure multiple register groups to poll different register types in sequence.</source>
        <translation>הגדר מספר קבוצות רגיסטרים לסקירה של סוגי רגיסטרים שונים ברצף.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="174"/>
        <source>Add New Group</source>
        <translation>הוסף קבוצה חדשה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="198"/>
        <source>Register Type:</source>
        <translation>סוג רגיסטר:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="210"/>
        <source>Start Address:</source>
        <translation>כתובת התחלה:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="217"/>
        <source>0-65535</source>
        <translation>0-65535</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="223"/>
        <source>Register Count:</source>
        <translation>מספר רגיסטרים:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="234"/>
        <source>1-125</source>
        <translation>1-125</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="239"/>
        <source>Add Group</source>
        <translation>הוסף קבוצה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="262"/>
        <source>Configured Groups</source>
        <translation>קבוצות מוגדרות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="296"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="303"/>
        <source>Type</source>
        <translation>סוג</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="311"/>
        <source>Start</source>
        <translation>התחלה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="318"/>
        <source>Count</source>
        <translation>מספר</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="325"/>
        <source>Action</source>
        <translation>פעולה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="400"/>
        <source>Remove</source>
        <translation>הסר</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="412"/>
        <source>No groups configured.
Add groups above to poll multiple register types.</source>
        <translation>לא הוגדרו קבוצות.
הוסף קבוצות למעלה כדי לבצע polling למספר סוגי רגיסטרים.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="430"/>
        <source>Total groups: %1</source>
        <translation>סך הכל קבוצות: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="434"/>
        <source>Generate Project</source>
        <translation>צור פרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="440"/>
        <source>Clear All</source>
        <translation>נקה הכל</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="446"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>ModbusPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="33"/>
        <source>Modbus Register Map Preview</source>
        <translation>תצוגה מקדימה של מפת רגיסטרים Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="155"/>
        <source>File: %1</source>
        <translation>קובץ: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="163"/>
        <source>Review the registers to import into a new Serial Studio project.</source>
        <translation>סקור את הרגיסטרים לייבוא לפרויקט Serial Studio חדש.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="171"/>
        <source>Registers</source>
        <translation>רגיסטרים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="205"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="212"/>
        <source>Name</source>
        <translation>שם</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="221"/>
        <source>Address</source>
        <translation>כתובת</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="227"/>
        <source>Type</source>
        <translation>סוג</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="235"/>
        <source>Data Type</source>
        <translation>סוג נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="242"/>
        <source>Units</source>
        <translation>יחידות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="343"/>
        <source>No registers found in file.</source>
        <translation>לא נמצאו רגיסטרים בקובץ.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="361"/>
        <source>Total: %1 registers in %2 groups</source>
        <translation>סה"כ: %1 רגיסטרים ב-%2 קבוצות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="367"/>
        <source>Cancel</source>
        <translation>ביטול</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="379"/>
        <source>Create Project</source>
        <translation>יצירת פרויקט</translation>
    </message>
</context>
<context>
    <name>MqttPublisherView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="33"/>
        <source>MQTT Publisher</source>
        <translation>מפרסם MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="110"/>
        <source>Connected to broker</source>
        <translation>מחובר ל-Broker</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="111"/>
        <source>Not connected</source>
        <translation>לא מחובר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="124"/>
        <source>Test Connection</source>
        <translation>בדיקת חיבור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="129"/>
        <source>Probe the broker with the current settings</source>
        <translation>בדיקת ה-Broker עם ההגדרות הנוכחיות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="162"/>
        <source>Enable publishing first</source>
        <translation>יש להפעיל פרסום תחילה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="140"/>
        <source>Edit Script</source>
        <translation>עריכת סקריפט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="146"/>
        <source>Edit the publisher script (Lua or JavaScript)</source>
        <translation>עריכת סקריפט המפרסם (Lua או JavaScript)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="158"/>
        <source>Load CA Certs</source>
        <translation>טעינת אישורי CA</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="164"/>
        <source>Add PEM certificates from a folder</source>
        <translation>הוסף אישורי PEM מתיקייה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="165"/>
        <source>Enable SSL/TLS first</source>
        <translation>הפעל SSL/TLS תחילה</translation>
    </message>
</context>
<context>
    <name>MultiPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="261"/>
        <source>Interpolation: %1</source>
        <translation>אינטרפולציה: %1</translation>
    </message>
    <message>
        <source>Show Legends</source>
        <translation type="vanished">הצג מקרא</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="283"/>
        <source>Show X Axis Label</source>
        <translation>הצג תווית ציר X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="294"/>
        <source>Show Y Axis Label</source>
        <translation>הצג תווית ציר Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="306"/>
        <source>Show Crosshair</source>
        <translation>הצג כוונת</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="313"/>
        <source>Pause</source>
        <translation>השהה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="313"/>
        <source>Resume</source>
        <translation>המשך</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="330"/>
        <source>Sweep / Trigger Mode</source>
        <translation>מצב סריקה / טריגר</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="342"/>
        <source>Trigger Settings</source>
        <translation>הגדרות טריגר</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="366"/>
        <source>Reset View</source>
        <translation>אפס תצוגה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="372"/>
        <source>Axis Range Settings</source>
        <translation>הגדרות טווח צירים</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">דגימות</translation>
    </message>
</context>
<context>
    <name>NativeTemplates</name>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="292"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="430"/>
        <source>Bytes per value</source>
        <translation>בתים לערך</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="293"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="431"/>
        <source>Number of bytes combined into each channel value.</source>
        <translation>מספר הבתים המשולבים לכל ערך ערוץ.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="301"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="439"/>
        <source>Endianness</source>
        <translation>סדר בתים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="302"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="440"/>
        <source>Byte order used when combining multi-byte values.</source>
        <translation>סדר בתים המשמש בעת שילוב ערכים רב-בתיים.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="310"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="448"/>
        <source>Signed values</source>
        <translation>ערכים מסומנים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="311"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="449"/>
        <source>Interprets each value as two's-complement signed.</source>
        <translation>מפרש כל ערך כמספר מסומן בשיטת משלים-לשניים.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="651"/>
        <source>Tag routing table</source>
        <translation>טבלת ניתוב תגיות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="652"/>
        <source>Comma-separated tag:index entries, e.g. 1:0,2:1,3:2. Tags may be decimal or 0x-prefixed hex.</source>
        <translation>ערכי תגית:אינדקס מופרדים בפסיקים, למשל 1:0,2:1,3:2. תגיות יכולות להיות עשרוניות או הקסדצימליות עם קידומת 0x.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1096"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1300"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1245"/>
        <source>Validate checksum</source>
        <translation>אימות Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1097"/>
        <source>Rejects messages with an invalid Fletcher checksum.</source>
        <translation>דוחה הודעות עם Checksum של Fletcher לא חוקי.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1301"/>
        <source>Rejects messages with an invalid additive checksum.</source>
        <translation>דוחה הודעות עם Checksum חיבורי לא חוקי.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1454"/>
        <source>Protocol version</source>
        <translation>גרסת פרוטוקול</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1455"/>
        <source>Selects the expected start marker (0xFE for v1, 0xFD for v2).</source>
        <translation>בוחר את סמן ההתחלה הצפוי (0xFE עבור v1, 0xFD עבור v2).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1883"/>
        <source>Validate CRC</source>
        <translation>אמת CRC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1884"/>
        <source>Rejects frames with an invalid CRC-24Q checksum.</source>
        <translation>דוחה מסגרות עם סכום בקרה CRC-24Q לא תקין.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2059"/>
        <source>Channel count</source>
        <translation>מספר ערוצים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2060"/>
        <source>Number of output channels (registers or coils).</source>
        <translation>מספר ערוצי פלט (רגיסטרים או סלילים).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2068"/>
        <source>Register offset</source>
        <translation>היסט רגיסטר</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2069"/>
        <source>Address offset subtracted from single-write echoes (40001 for legacy Modicon maps).</source>
        <translation>היסט כתובת המופחת מהדים של כתיבה בודדת (40001 עבור מפות Modicon מדור קודם).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2079"/>
        <source>Signed registers</source>
        <translation>רגיסטרים מסומנים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2080"/>
        <source>Interprets 16-bit registers as two's-complement signed values.</source>
        <translation>מפרש רגיסטרים של 16 סיביות כערכים מסומנים בשיטת משלים לשניים.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2386"/>
        <source>Payload layout</source>
        <translation>פריסת מטען</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2387"/>
        <source>Array emits every element in order; map routes keys through the key list.</source>
        <translation>מערך פולט כל אלמנט לפי הסדר; מפה מנתבת מפתחות דרך רשימת המפתחות.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2397"/>
        <source>Keys (map mode)</source>
        <translation>מפתחות (מצב מיפוי)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2398"/>
        <source>Comma-separated map keys in channel order. Only used for the map layout.</source>
        <translation>מפתחות מיפוי מופרדים בפסיקים בסדר הערוצים. משמש רק עבור פריסת מיפוי.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="184"/>
        <source>Scalar fields</source>
        <translation>שדות סקלריים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="185"/>
        <source>Comma-separated JSON fields repeated in every generated frame.</source>
        <translation>שדות JSON מופרדים בפסיקים החוזרים בכל פריים שנוצר.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="192"/>
        <source>Sample array field</source>
        <translation>שדה מערך דגימות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="193"/>
        <source>JSON field holding the batched sample array.</source>
        <translation>שדה JSON המכיל את מערך הדגימות המאוגד.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="334"/>
        <source>Records array field</source>
        <translation>שדה מערך רשומות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="335"/>
        <source>JSON field holding the array of record objects.</source>
        <translation>שדה JSON המכיל את מערך אובייקטי הרשומות.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="341"/>
        <source>Record fields (in channel order)</source>
        <translation>שדות רשומה (בסדר הערוצים)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="342"/>
        <source>Comma-separated record fields. The position of each field sets its channel index.</source>
        <translation>שדות רשומה מופרדים בפסיקים. המיקום של כל שדה קובע את אינדקס הערוץ שלו.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="605"/>
        <source>Column widths</source>
        <translation>רוחבי עמודות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="606"/>
        <source>Comma-separated character counts per field. Leave empty to split on whitespace.</source>
        <translation>מספרי תווים מופרדים בפסיקים לכל שדה. השאר ריק כדי לפצל לפי רווחים.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="614"/>
        <source>Trim whitespace</source>
        <translation>חיתוך רווחים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="615"/>
        <source>Removes padding around every sliced field.</source>
        <translation>מסיר ריפוד מסביב לכל שדה שנחתך.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="744"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="893"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1360"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1787"/>
        <source>Keys (in channel order)</source>
        <translation>מפתחות (בסדר הערוצים)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="745"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="894"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1788"/>
        <source>Comma-separated key names. The position of each key sets its channel index.</source>
        <translation>שמות מפתחות מופרדים בפסיקים. מיקום כל מפתח קובע את אינדקס הערוץ שלו.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="753"/>
        <source>Pair separator</source>
        <translation>מפריד זוגות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="754"/>
        <source>Character between key=value pairs.</source>
        <translation>תו בין זוגות key=value.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="760"/>
        <source>Key-value separator</source>
        <translation>מפריד מפתח-ערך</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="761"/>
        <source>Character between a key and its value.</source>
        <translation>תו בין מפתח לערך שלו.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="767"/>
        <source>Numeric values only</source>
        <translation>ערכים מספריים בלבד</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="768"/>
        <source>Ignores pairs whose value is not a number.</source>
        <translation>מתעלם מזוגות שערכם אינו מספר.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1010"/>
        <source>Command routing table</source>
        <translation>טבלת ניתוב פקודות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1011"/>
        <source>Semicolon-separated entries of NAME:index list, e.g. CSQ:0,1;CREG:2,3;CGATT:4.</source>
        <translation>רשומות מופרדות בנקודה-פסיק של רשימת NAME:index, לדוגמה CSQ:0,1;CREG:2,3;CGATT:4.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1236"/>
        <source>Talker prefix</source>
        <translation>קידומת Talker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1237"/>
        <source>Two-letter talker id, e.g. GP for GPS or GN for multi-constellation receivers.</source>
        <translation>מזהה talker בן שתי אותיות, לדוגמה GP עבור GPS או GN עבור מקלטים רב-קבוצתיים.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1246"/>
        <source>Rejects sentences whose *hh checksum does not match.</source>
        <translation>דוחה משפטים שבהם ה-checksum *hh אינו תואם.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1361"/>
        <source>Comma-separated parameter names. The position of each key sets its channel index.</source>
        <translation>שמות פרמטרים מופרדים בפסיקים. מיקום כל מפתח קובע את אינדקס הערוץ שלו.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1500"/>
        <source>Fields (in channel order)</source>
        <translation>שדות (בסדר ערוצים)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1501"/>
        <source>Comma-separated field names. The position of each field sets its channel index.</source>
        <translation>שמות שדות מופרדים בפסיקים. מיקום כל שדה קובע את אינדקס הערוץ שלו.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1620"/>
        <source>Tags (in channel order)</source>
        <translation>תגיות (בסדר הערוצים)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1621"/>
        <source>Comma-separated tag names. The position of each tag sets its channel index.</source>
        <translation>שמות תגיות מופרדים בפסיקים. המיקום של כל תגית קובע את אינדקס הערוץ שלה.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MapTemplates.cpp" line="483"/>
        <source>Register blocks</source>
        <translation>בלוקי רגיסטרים</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MapTemplates.cpp" line="484"/>
        <source>Polled register blocks with typed, scaled entries. Written by the Modbus register map importer.</source>
        <translation>בלוקי רגיסטרים נשאלים עם ערכים מוקלדים ומוקנים. נכתב על ידי מייבא מפת רגיסטרי Modbus.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MapTemplates.cpp" line="791"/>
        <source>Signal map</source>
        <translation>מפת אותות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MapTemplates.cpp" line="792"/>
        <source>CAN messages with their signal bit layouts and scaling. Written by the DBC importer.</source>
        <translation>הודעות CAN עם פריסות הביטים של האותות והקנייה שלהן. נכתב על ידי מייבא DBC.</translation>
    </message>
</context>
<context>
    <name>Network</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="78"/>
        <source>Socket Type</source>
        <translation>סוג Socket</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="99"/>
        <source>Local Port</source>
        <translation>פורט מקומי</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="106"/>
        <source>Type 0 for automatic port</source>
        <translation>הקלד 0 לפורט אוטומטי</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="132"/>
        <source>Remote Address</source>
        <translation>כתובת מרוחקת</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="156"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="189"/>
        <source>Remote Port</source>
        <translation>פורט מרוחק</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="219"/>
        <source>Multicast</source>
        <translation>Multicast</translation>
    </message>
</context>
<context>
    <name>NotificationLog</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="162"/>
        <source>Filter by channel…</source>
        <translation>סינון לפי ערוץ…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="187"/>
        <source>Clear all notifications</source>
        <translation>נקה את כל ההתראות</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="271"/>
        <source>(no title)</source>
        <translation>התראות</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="329"/>
        <source>No notifications yet</source>
        <translation>אין התראות עדיין</translation>
    </message>
</context>
<context>
    <name>OnlineIconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="42"/>
        <source>Search Online Icons</source>
        <translation>חיפוש סמלים מקוונים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="72"/>
        <source>Download failed: %1</source>
        <translation>ההורדה נכשלה: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="97"/>
        <source>Search icons (e.g. temperature, arrow, play)…</source>
        <translation>חיפוש סמלים (לדוגמה: טמפרטורה, חץ, הפעלה)…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="109"/>
        <source>Search</source>
        <translation>חיפוש</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="148"/>
        <source>Search for icons above to get started</source>
        <translation>חפש סמלים למעלה כדי להתחיל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="249"/>
        <source>OK</source>
        <translation>אישור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="259"/>
        <source>Cancel</source>
        <translation>ביטול</translation>
    </message>
</context>
<context>
    <name>OutputWidgetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="91"/>
        <source>Output widgets require a Pro license.</source>
        <translation>ווידג'טי פלט דורשים רישיון Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="93"/>
        <source>You can configure output widgets, but they only appear on the dashboard with a Pro license.</source>
        <translation>ניתן להגדיר ווידג'טים לפלט, אך הם מופיעים בלוח הבקרה רק עם רישיון Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="125"/>
        <source>Button</source>
        <translation>כפתור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="129"/>
        <source>Send a command on click</source>
        <translation>שליחת פקודה בלחיצה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="134"/>
        <source>Slider</source>
        <translation>סליידר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="138"/>
        <source>Send scaled numeric values</source>
        <translation>שליחת ערכים מספריים מדורגים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="143"/>
        <source>Toggle</source>
        <translation>מתג</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="147"/>
        <source>Send on/off commands</source>
        <translation>שליחת פקודות הפעלה/כיבוי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="152"/>
        <source>Text Field</source>
        <translation>שדה טקסט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="156"/>
        <source>Type and send arbitrary commands</source>
        <translation>הקלדה ושליחת פקודות שרירותיות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="160"/>
        <source>Knob</source>
        <translation>כפתור סיבוב</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="165"/>
        <source>Rotary input for setpoints</source>
        <translation>קלט סיבובי לנקודות ייחוס</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="182"/>
        <source>Duplicate</source>
        <translation>שכפל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="185"/>
        <source>Duplicate this output widget</source>
        <translation>שכפל ווידג'ט פלט זה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="195"/>
        <source>Delete</source>
        <translation>מחק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="197"/>
        <source>Delete this output widget</source>
        <translation>מחק ווידג'ט פלט זה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="274"/>
        <source>Transmit Function</source>
        <translation>פונקציית שידור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="284"/>
        <source>Import</source>
        <translation>ייבא</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="290"/>
        <source>Import transmit function from a .js file</source>
        <translation>ייבא פונקציית שידור מקובץ .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="297"/>
        <source>Template</source>
        <translation>תבנית</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="301"/>
        <source>Select a pre-built transmit function template</source>
        <translation>בחר תבנית פונקציית שידור מוכנה מראש</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="306"/>
        <source>Test</source>
        <translation>בדיקה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="312"/>
        <source>Test the transmit function with sample input</source>
        <translation>בדוק את פונקציית השידור עם קלט לדוגמה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="353"/>
        <source>Undo</source>
        <translation>בטל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="359"/>
        <source>Redo</source>
        <translation>בצע שוב</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="367"/>
        <source>Cut</source>
        <translation>גזור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="372"/>
        <source>Copy</source>
        <translation>העתק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="377"/>
        <source>Paste</source>
        <translation>הדבק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="384"/>
        <source>Select All</source>
        <translation>בחר הכל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="391"/>
        <source>Format Document</source>
        <translation>עצב מסמך</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="396"/>
        <source>Format Selection</source>
        <translation>עצב בחירה</translation>
    </message>
</context>
<context>
    <name>Painter</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Painter.qml" line="56"/>
        <source>Painter Widget Error</source>
        <translation>שגיאת וידג׳ט ציור</translation>
    </message>
</context>
<context>
    <name>PainterCodeDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="30"/>
        <source>Painter Widget Code Editor</source>
        <translation>עורך קוד וידג׳ט ציור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="76"/>
        <source>paint(ctx, w, h)</source>
        <translation>paint(ctx, w, h)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="86"/>
        <source>Import</source>
        <translation>ייבוא</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="92"/>
        <source>Import painter code from a .js file</source>
        <translation>ייבוא קוד ציור מקובץ .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="99"/>
        <source>Template</source>
        <translation>תבנית</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="103"/>
        <source>Select a built-in painter template</source>
        <translation>בחר תבנית ציור מובנית</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="108"/>
        <source>Format</source>
        <translation>עיצוב</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="113"/>
        <source>Reformat the painter code</source>
        <translation>עצב מחדש את קוד הציור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="119"/>
        <source>Test</source>
        <translation>בדיקה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="124"/>
        <source>Open a live preview with simulated dataset values</source>
        <translation>פתח תצוגה מקדימה חיה עם ערכי מערך נתונים מדומים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="127"/>
        <source>Preview</source>
        <translation>תצוגה מקדימה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="182"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="191"/>
        <source>Cut</source>
        <translation>גזור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="192"/>
        <source>Copy</source>
        <translation>העתק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="193"/>
        <source>Paste</source>
        <translation>הדבק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="194"/>
        <source>Select All</source>
        <translation>בחר הכול</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="196"/>
        <source>Undo</source>
        <translation>בטל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="197"/>
        <source>Redo</source>
        <translation>בצע שוב</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="199"/>
        <source>Format Document</source>
        <translation>עצב מסמך</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="200"/>
        <source>Format Selection</source>
        <translation>עיצוב הבחירה</translation>
    </message>
</context>
<context>
    <name>PainterTestDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="28"/>
        <source>Painter Live Preview</source>
        <translation>תצוגה מקדימה חיה של Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="32"/>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="37"/>
        <source>Preview</source>
        <translation>תצוגה מקדימה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="113"/>
        <source>Simulated datasets</source>
        <translation>מערכי נתונים מדומים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="180"/>
        <source>Runtime OK</source>
        <translation>Runtime תקין</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="181"/>
        <source>Awaiting first frame...</source>
        <translation>ממתין לפריים ראשון...</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="194"/>
        <source>Console</source>
        <translation>קונסול</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="236"/>
        <source>Clear console</source>
        <translation>נקה קונסול</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="245"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>Plot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="257"/>
        <source>Interpolation: %1</source>
        <translation>אינטרפולציה: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="270"/>
        <source>Show Area Under Plot</source>
        <translation>הצג שטח מתחת לגרף</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="288"/>
        <source>Show X Axis Label</source>
        <translation>הצג תווית ציר X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="299"/>
        <source>Show Y Axis Label</source>
        <translation>הצג תווית ציר Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="311"/>
        <source>Show Crosshair</source>
        <translation>הצג כוונת</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="318"/>
        <source>Pause</source>
        <translation>השהה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="318"/>
        <source>Resume</source>
        <translation>המשך</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="335"/>
        <source>Sweep / Trigger Mode</source>
        <translation>מצב סריקה / טריגר</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="347"/>
        <source>Trigger Settings</source>
        <translation>הגדרות טריגר</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="371"/>
        <source>Reset View</source>
        <translation>אפס תצוגה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="377"/>
        <source>Axis Range Settings</source>
        <translation>הגדרות טווח צירים</translation>
    </message>
</context>
<context>
    <name>Plot3D</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="203"/>
        <source>Interpolate</source>
        <translation>אינטרפולציה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="221"/>
        <source>Orbit Navigation</source>
        <translation>ניווט מסלולי</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="231"/>
        <source>Pan Navigation</source>
        <translation>ניווט הזזה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="242"/>
        <source>Orthogonal View</source>
        <translation>תצוגה אורתוגונלית</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="248"/>
        <source>Top View</source>
        <translation>תצוגה עליונה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="254"/>
        <source>Left View</source>
        <translation>תצוגה שמאלית</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="260"/>
        <source>Front View</source>
        <translation>תצוגה קדמית</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="277"/>
        <source>Auto Center</source>
        <translation>מרכוז אוטומטי</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="293"/>
        <source>Anaglyph 3D</source>
        <translation>תלת-ממד אנגליפי</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="307"/>
        <source>Invert Eye Positions</source>
        <translation>היפוך מיקומי עיניים</translation>
    </message>
</context>
<context>
    <name>PlotCommon</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="59"/>
        <source>None</source>
        <translation>ללא</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="62"/>
        <source>ZOH</source>
        <translation>ZOH</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="65"/>
        <source>Stem</source>
        <translation>גזע</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="67"/>
        <source>Linear</source>
        <translation>לינארי</translation>
    </message>
</context>
<context>
    <name>PlotWidget</name>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1326"/>
        <source>Time</source>
        <translation>זמן</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1349"/>
        <source>ΔX: %1  ΔY: %2 — Drag to move, right-click to clear</source>
        <translation>ΔX: %1  ΔY: %2 — גרור להזזה, לחץ ימני לניקוי</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1352"/>
        <source>Click to place cursor</source>
        <translation>לחץ למיקום סמן</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1354"/>
        <source>Click to place second cursor — Drag to move</source>
        <translation>לחץ למיקום סמן שני — גרור להזזה</translation>
    </message>
</context>
<context>
    <name>ProNotice</name>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="119"/>
        <source>Visit Website</source>
        <translation>בקר באתר אינטרנט</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="127"/>
        <source>Buy License</source>
        <translation>רכוש רישיון</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="140"/>
        <source>Activate</source>
        <translation>הפעל</translation>
    </message>
</context>
<context>
    <name>ProUpgradeNotice</name>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="26"/>
        <source>Assistant — Pro feature</source>
        <translation>עוזר — תכונת Pro</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="44"/>
        <source>The Assistant is a Serial Studio Pro feature. Activate your license to unlock it.</source>
        <translation>העוזר הוא תכונת Serial Studio Pro. הפעל את הרישיון לפתיחתה.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="52"/>
        <source>Activate</source>
        <translation>הפעל</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="66"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>Process</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="69"/>
        <source>Mode</source>
        <translation>מצב</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Launch Process</source>
        <translation>הפעל תהליך</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Named Pipe</source>
        <translation>צינור בעל שם</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="101"/>
        <source>Executable</source>
        <translation>קובץ הפעלה</translation>
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
        <translation>עיון</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="145"/>
        <source>Arguments</source>
        <translation>ארגומנטים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="156"/>
        <source>--arg1 value1 --arg2 value2</source>
        <translation>--arg1 value1 --arg2 value2</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="177"/>
        <source>Working Dir</source>
        <translation>תיקיית עבודה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="192"/>
        <source>(optional) /working/directory</source>
        <translation>(אופציונלי) /working/directory</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="223"/>
        <source>Pipe Path</source>
        <translation>נתיב צינור</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="273"/>
        <source>Pick Running Process…</source>
        <translation>בחר תהליך פועל…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="311"/>
        <source>Launch a child process and capture its stdout, or connect to a named pipe written by an existing process.</source>
        <translation>הפעל תהליך צאצא וקלוט את stdout שלו, או התחבר לצינור בעל שם שנכתב על ידי תהליך קיים.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="319"/>
        <source>Learn about named pipes</source>
        <translation>למד על צינורות בעלי שם</translation>
    </message>
</context>
<context>
    <name>ProcessPicker</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="60"/>
        <source>Select Running Process</source>
        <translation>בחר תהליך פועל</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="211"/>
        <source>Select a running process to derive a named-pipe path suggestion.</source>
        <translation>בחר תהליך פועל כדי לגזור הצעת נתיב צינור בעל שם.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="217"/>
        <source>Filter Processes</source>
        <translation>סנן תהליכים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="231"/>
        <source>Type to filter by name…</source>
        <translation>הקלד כדי לסנן לפי שם…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="235"/>
        <source>Refresh</source>
        <translation>רענן</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="243"/>
        <source>Running Processes</source>
        <translation>תהליכים פעילים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="281"/>
        <source>Process</source>
        <translation>תהליך</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="288"/>
        <source>PID</source>
        <translation>PID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="383"/>
        <source>No processes match the filter.</source>
        <translation>אין תהליכים התואמים את הסינון.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="384"/>
        <source>No running processes found.
Click Refresh to update the list.</source>
        <translation>לא נמצאו תהליכים פעילים.
לחץ על רענן כדי לעדכן את הרשימה.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="400"/>
        <source>%1 process(es)</source>
        <translation>%1 תהליך/ים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="404"/>
        <source>Select</source>
        <translation>בחר</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="410"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>ProjectEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="43"/>
        <source>modified</source>
        <translation>שונה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="361"/>
        <source>This project is password protected</source>
        <translation>פרויקט זה מוגן בסיסמה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="362"/>
        <source>Editing is available in Project mode</source>
        <translation>עריכה זמינה במצב פרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="373"/>
        <source>Enter the password to make changes, or open a different project.</source>
        <translation>הזן את הסיסמה כדי לבצע שינויים, או פתח פרויקט אחר.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="374"/>
        <source>Switch to Project mode to load and edit a project.</source>
        <translation>עבור למצב פרויקט כדי לטעון ולערוך פרויקט.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="396"/>
        <source>Unlock</source>
        <translation>בטל נעילה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="397"/>
        <source>Switch to Project Mode</source>
        <translation>עבור למצב פרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="416"/>
        <source>Open Other Project</source>
        <translation>פתח פרויקט אחר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="417"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="433"/>
        <source>Create New Project</source>
        <translation>צור פרויקט חדש</translation>
    </message>
</context>
<context>
    <name>ProjectStructure</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="32"/>
        <source>Project Structure</source>
        <translation>מבנה הפרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="71"/>
        <source>Search</source>
        <translation>חיפוש</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="169"/>
        <source>Move Up</source>
        <translation>העבר למעלה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="174"/>
        <source>Move Down</source>
        <translation>העבר למטה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="185"/>
        <source>Duplicate Selected (%1)</source>
        <translation>שכפל את הנבחר (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="186"/>
        <source>Duplicate</source>
        <translation>שכפל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="199"/>
        <source>Delete Selected (%1)</source>
        <translation>מחק את הנבחר (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="200"/>
        <source>Delete</source>
        <translation>מחק</translation>
    </message>
</context>
<context>
    <name>ProjectToolbar</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="142"/>
        <source>New</source>
        <translation>חדש</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="145"/>
        <source>Create a new JSON project</source>
        <translation>צור פרויקט JSON חדש</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="158"/>
        <source>Open</source>
        <translation>פתח</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="162"/>
        <source>Open an existing JSON project</source>
        <translation>פתח פרויקט JSON קיים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="168"/>
        <source>Save</source>
        <translation>שמור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="175"/>
        <source>Save the current project</source>
        <translation>שמירת הפרויקט הנוכחי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="181"/>
        <source>Save As</source>
        <translation>שמירה בשם</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="188"/>
        <source>Save the current project under a new name</source>
        <translation>שמירת הפרויקט הנוכחי תחת שם חדש</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="200"/>
        <source>Import</source>
        <translation>ייבוא</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="204"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="208"/>
        <source>Generate a project from a Protocol Buffers (.proto) schema</source>
        <translation>יצירת פרויקט מסכמת Protocol Buffers (‎.proto)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="230"/>
        <source>Lock</source>
        <translation>נעילה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="234"/>
        <source>Set a password and lock the Project Editor</source>
        <translation>הגדרת סיסמה ונעילת עורך הפרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="245"/>
        <source>Add Device</source>
        <translation>הוספת התקן</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="249"/>
        <source>Add a new data source (device) to the project</source>
        <translation>הוספת מקור נתונים (התקן) חדש לפרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="272"/>
        <source>Action</source>
        <translation>פעולה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="275"/>
        <source>Add a new action to the project</source>
        <translation>הוסף פעולה חדשה לפרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="260"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="264"/>
        <source>Output</source>
        <translation>פלט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="218"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="222"/>
        <source>Restore</source>
        <translation>שחזר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="226"/>
        <source>Restore a recent automatic snapshot of the current project</source>
        <translation>שחזר תמונת מצב אוטומטית אחרונה של הפרויקט הנוכחי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="267"/>
        <source>Add a new output control panel with a button</source>
        <translation>הוסף לוח בקרת פלט חדש עם כפתור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="288"/>
        <source>Slider</source>
        <translation>סליידר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="291"/>
        <source>Add an output slider control</source>
        <translation>הוסף בקרת סליידר לפלט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="298"/>
        <source>Toggle</source>
        <translation>מתג</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="301"/>
        <source>Add an output toggle control</source>
        <translation>הוסף בקרת מתג לפלט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="308"/>
        <source>Knob</source>
        <translation>כפתור סיבוב</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="311"/>
        <source>Add an output knob control</source>
        <translation>הוסף בקרת כפתור סיבוב לפלט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="319"/>
        <source>Text Field</source>
        <translation>שדה טקסט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="321"/>
        <source>Add an output text field control</source>
        <translation>הוסף פקד שדה טקסט פלט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="328"/>
        <source>Button</source>
        <translation>כפתור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="331"/>
        <source>Add an output button control</source>
        <translation>הוסף פקד כפתור פלט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="344"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="348"/>
        <source>Dataset</source>
        <translation>מערך נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="350"/>
        <source>Add a generic dataset</source>
        <translation>הוסף מערך נתונים גנרי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="364"/>
        <source>Plot</source>
        <translation>גרף</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="367"/>
        <source>Add a 2D plot dataset</source>
        <translation>הוסף מערך נתונים גרף דו־ממדי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="374"/>
        <source>FFT Plot</source>
        <translation>גרף FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="377"/>
        <source>Add a Fast Fourier Transform plot</source>
        <translation>הוסף גרף Fast Fourier Transform</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="384"/>
        <source>Gauge</source>
        <translation>מד</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="387"/>
        <source>Add a gauge widget for numeric data</source>
        <translation>הוספת ווידג'ט מד לנתונים מספריים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="395"/>
        <source>Level Indicator</source>
        <translation>מחוון רמה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="397"/>
        <source>Add a vertical bar level indicator</source>
        <translation>הוספת מחוון רמה אנכי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="404"/>
        <source>Compass</source>
        <translation>מצפן</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="407"/>
        <source>Add a compass widget for directional data</source>
        <translation>הוספת ווידג'ט מצפן לנתוני כיוון</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="415"/>
        <source>LED Indicator</source>
        <translation>מחוון LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="417"/>
        <source>Add an LED-style status indicator</source>
        <translation>הוספת מחוון סטטוס בסגנון LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="430"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="434"/>
        <source>Group</source>
        <translation>קבוצה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="436"/>
        <source>Add a dataset container group</source>
        <translation>הוספת קבוצת מכל לנתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="438"/>
        <source>Dataset Container</source>
        <translation>מיכל מערך נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="442"/>
        <source>Image</source>
        <translation>תמונה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="444"/>
        <source>Add an image/video stream viewer</source>
        <translation>הוסף מציג זרם תמונה/וידאו</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="446"/>
        <source>Image View</source>
        <translation>תצוגת תמונה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="454"/>
        <source>Painter</source>
        <translation>ציור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="458"/>
        <source>Add a custom JavaScript-rendered painter widget</source>
        <translation>הוסף וידג'ט ציור מותאם אישית מעובד ב-JavaScript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="459"/>
        <source>Painter widgets require a Pro license — adding one will fall back to a data grid</source>
        <translation>וידג'טי ציור דורשים רישיון Pro — הוספת אחד יחזור לטבלת נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="460"/>
        <source>Painter Widget</source>
        <translation>וידג'ט ציור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="472"/>
        <source>Table</source>
        <translation>טבלה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="475"/>
        <source>Add a data table view</source>
        <translation>הוסף תצוגת טבלת נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="477"/>
        <source>Data Grid</source>
        <translation>רשת נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="483"/>
        <source>Multi-Plot</source>
        <translation>גרף מרובה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="485"/>
        <source>Add a 2D plot with multiple signals</source>
        <translation>הוספת גרף דו־ממדי עם אותות מרובים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="487"/>
        <source>Multiple Plot</source>
        <translation>גרף מרובה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="492"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="497"/>
        <source>3D Plot</source>
        <translation>גרף תלת־ממדי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="495"/>
        <source>Add a 3D plot visualization</source>
        <translation>הוספת ויזואליזציה של גרף תלת־ממדי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="503"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="507"/>
        <source>Accelerometer</source>
        <translation>מד תאוצה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="505"/>
        <source>Add a group for 3-axis accelerometer data</source>
        <translation>הוספת קבוצה לנתוני מד תאוצה תלת־צירי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="513"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="516"/>
        <source>Gyroscope</source>
        <translation>ג'ירוסקופ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="517"/>
        <source>Add a group for 3-axis gyroscope data</source>
        <translation>הוספת קבוצה לנתוני ג'ירוסקופ תלת־צירי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="522"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="527"/>
        <source>GPS Map</source>
        <translation>מפת GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="525"/>
        <source>Add a map widget for GPS data</source>
        <translation>הוסף ווידג'ט מפה לנתוני GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="539"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="543"/>
        <source>Assistant</source>
        <translation>עוזר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="546"/>
        <source>Open the Assistant</source>
        <translation>פתח את העוזר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="552"/>
        <source>Help Center</source>
        <translation>מרכז עזרה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="556"/>
        <source>Open the Project Editor documentation</source>
        <translation>פתח את התיעוד של עורך הפרויקט</translation>
    </message>
</context>
<context>
    <name>ProjectView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="34"/>
        <source>Project Summary</source>
        <translation>סיכום הפרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="81"/>
        <source>Pro features detected in this project.</source>
        <translation>זוהו תכונות Pro בפרויקט זה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="83"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>משתמש בווידג'טים חלופיים. רכוש רישיון לפתיחת הפונקציונליות המלאה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="118"/>
        <source>Project Title:</source>
        <translation>כותרת הפרויקט:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="129"/>
        <source>Untitled Project</source>
        <translation>פרויקט ללא שם</translation>
    </message>
    <message>
        <source>Points:</source>
        <translation type="vanished">נקודות:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="149"/>
        <source>Time Range:</source>
        <translation>טווח זמן:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="235"/>
        <source>Source</source>
        <translation>מקור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="236"/>
        <source>Sources</source>
        <translation>מקורות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="241"/>
        <source>Group</source>
        <translation>קבוצה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="242"/>
        <source>Groups</source>
        <translation>קבוצות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="247"/>
        <source>Dataset</source>
        <translation>מערך נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="248"/>
        <source>Datasets</source>
        <translation>מערכי נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="253"/>
        <source>Action</source>
        <translation>פעולה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="254"/>
        <source>Actions</source>
        <translation>פעולות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="342"/>
        <source>Double-click a block to edit it. Right-click anywhere to add a group, dataset, action, data table, or device.</source>
        <translation>לחץ לחיצה כפולה על בלוק כדי לערוך אותו. לחץ לחיצה ימנית בכל מקום כדי להוסיף קבוצה, מערך נתונים, פעולה, טבלת נתונים או התקן.</translation>
    </message>
</context>
<context>
    <name>ProtoPreviewDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="41"/>
        <source>Protocol Buffers File Preview</source>
        <translation>תצוגה מקדימה של קובץ Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="165"/>
        <source>Proto File: %1</source>
        <translation>קובץ Proto: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="173"/>
        <source>Browse the messages below, then create the project. Every message becomes a dashboard group; matching-type channel blocks get a MultiPlot and mixed-type messages get a DataGrid.</source>
        <translation>עיין בהודעות למטה, ולאחר מכן צור את הפרויקט. כל הודעה הופכת לקבוצת לוח בקרה; בלוקי ערוצים מאותו סוג מקבלים MultiPlot והודעות מסוג מעורב מקבלות DataGrid.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="183"/>
        <source>Show fields for</source>
        <translation>הצג שדות עבור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="209"/>
        <source>Fields</source>
        <translation>שדות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="243"/>
        <source>Tag</source>
        <translation>תג</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="253"/>
        <source>Field Name</source>
        <translation>שם שדה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="258"/>
        <source>Type</source>
        <translation>סוג</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="345"/>
        <source>No fields in the selected message.</source>
        <translation>אין שדות בהודעה שנבחרה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="363"/>
        <source>Total: %1 messages, %2 fields</source>
        <translation>סה״כ: %1 הודעות, %2 שדות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="370"/>
        <source>Cancel</source>
        <translation>ביטול</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="381"/>
        <source>Create Project</source>
        <translation>יצירת פרויקט</translation>
    </message>
</context>
<context>
    <name>Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="71"/>
        <source>No error</source>
        <translation>אין שגיאה</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="73"/>
        <source>The broker rejected the connection due to an unsupported protocol version. Match the broker's MQTT version and try again.</source>
        <translation>ה-Broker דחה את החיבור עקב גרסת פרוטוקול לא נתמכת. יש להתאים את גרסת MQTT של ה-Broker ולנסות שוב.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="76"/>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Regenerate it and try again.</source>
        <translation>ה-Broker דחה את מזהה הלקוח. ייתכן שהוא פגום, ארוך מדי או כבר בשימוש. יש ליצור אותו מחדש ולנסות שוב.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="79"/>
        <source>The network reached the broker, but the broker is currently unavailable. Verify its status and try again later.</source>
        <translation>הרשת הגיעה ל-Broker, אך ה-Broker אינו זמין כעת. יש לאמת את מצבו ולנסות שוב מאוחר יותר.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="82"/>
        <source>The username or password is incorrect or malformed. Double-check the credentials and try again.</source>
        <translation>שם המשתמש או הסיסמה שגויים או פגומים. יש לבדוק שוב את פרטי ההזדהות ולנסות שוב.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="85"/>
        <source>The broker denied the connection due to insufficient permissions. Verify that the account has the required ACLs.</source>
        <translation>ה-Broker דחה את החיבור עקב הרשאות לא מספיקות. יש לאמת שלחשבון יש את הרשאות ה-ACL הנדרשות.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="88"/>
        <source>A network or transport-layer issue prevented the connection. Check connectivity, ports, and TLS configuration.</source>
        <translation>בעיית רשת או שכבת תעבורה מנעה את החיבור. יש לבדוק קישוריות, פורטים והגדרות TLS.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="91"/>
        <source>The client detected an MQTT protocol violation and closed the connection. Verify broker and client compatibility.</source>
        <translation>הלקוח זיהה הפרה של פרוטוקול MQTT וסגר את החיבור. יש לוודא תאימות בין Broker ללקוח.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="94"/>
        <source>An unexpected error occurred. Check the broker logs and the application console for details.</source>
        <translation>אירעה שגיאה בלתי צפויה. יש לבדוק את יומני ה-Broker ואת קונסולת היישום לפרטים.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="97"/>
        <source>An MQTT 5 protocol-level error occurred. Inspect the broker's reason code for details.</source>
        <translation>אירעה שגיאה ברמת פרוטוקול MQTT 5. יש לבדוק את קוד הסיבה של ה-Broker לפרטים.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="101"/>
        <source>Unspecified MQTT error (code %1).</source>
        <translation>שגיאת MQTT לא מוגדרת (קוד %1).</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../src/Misc/Translator.cpp" line="231"/>
        <source>Failed to load welcome text :(</source>
        <translation>טעינת טקסט הפתיחה נכשלה :(</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="180"/>
        <source>Critical</source>
        <translation>קריטי</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="180"/>
        <source>Warning</source>
        <translation>אזהרה</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="522"/>
        <source>Project file not found</source>
        <translation>קובץ הפרויקט לא נמצא</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="523"/>
        <source>The project file referenced by this shortcut could not be found:

%1</source>
        <translation>קובץ הפרויקט שאליו מפנה קיצור דרך זה לא נמצא:

%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="526"/>
        <source>Would you like to delete this shortcut?</source>
        <translation>האם למחוק את קיצור הדרך?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="530"/>
        <source>Delete Shortcut</source>
        <translation>מחק קיצור דרך</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="532"/>
        <source>Quit</source>
        <translation>יציאה</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1013"/>
        <source>Time (s)</source>
        <translation>זמן (שניות)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1071"/>
        <source>Freq: %1</source>
        <translation>תדר: %1</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1074"/>
        <source>Time: −%1</source>
        <translation>זמן: −%1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="750"/>
        <source>Invalid Bluetooth adapter!</source>
        <translation>מתאם Bluetooth לא תקין!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="753"/>
        <source>Unsuported platform or operating system</source>
        <translation>פלטפורמה או מערכת הפעלה לא נתמכת</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="756"/>
        <source>Unsupported discovery method</source>
        <translation>שיטת גילוי לא נתמכת</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="759"/>
        <source>General I/O error</source>
        <translation>שגיאת קלט/פלט כללית</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/LocalProvider.cpp" line="319"/>
        <source>No local model server URL configured. Open Manage Keys to set one.</source>
        <translation>לא הוגדר כתובת שרת מודל מקומי. פתח את ניהול מפתחות כדי להגדיר אחת.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/DeepSeekProvider.cpp" line="145"/>
        <source>No DeepSeek API key set. Open Manage Keys to add one.</source>
        <translation>לא הוגדר מפתח API של DeepSeek. פתח ניהול מפתחות כדי להוסיף אחד.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/MistralProvider.cpp" line="168"/>
        <source>No Mistral API key set. Open Manage Keys to add one.</source>
        <translation>לא הוגדר מפתח API של Mistral. פתח ניהול מפתחות כדי להוסיף אחד.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIProvider.cpp" line="360"/>
        <source>No OpenAI API key set. Open Manage Keys to add one.</source>
        <translation>לא הוגדר מפתח API של OpenAI. פתח ניהול מפתחות כדי להוסיף אחד.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenRouterProvider.cpp" line="181"/>
        <source>No OpenRouter API key set. Open Manage Keys to add one.</source>
        <translation>לא הוגדר מפתח API של OpenRouter. פתח ניהול מפתחות כדי להוסיף אחד.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GroqProvider.cpp" line="152"/>
        <source>No Groq API key set. Open Manage Keys to add one.</source>
        <translation>לא הוגדר מפתח API של Groq. פתח ניהול מפתחות כדי להוסיף אחד.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicProvider.cpp" line="198"/>
        <source>No Anthropic API key set. Open Manage Keys to add one.</source>
        <translation>לא הוגדר מפתח API של Anthropic. פתח ניהול מפתחות כדי להוסיף אחד.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiProvider.cpp" line="279"/>
        <source>No Gemini API key set. Open Manage Keys to add one.</source>
        <translation>לא הוגדר מפתח API של Gemini. פתח ניהול מפתחות כדי להוסיף אחד.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="251"/>
        <source>Network error</source>
        <translation>שגיאת רשת</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="254"/>
        <location filename="../../src/Licensing/Trial.cpp" line="270"/>
        <location filename="../../src/Licensing/Trial.cpp" line="302"/>
        <source>Trial Activation Error</source>
        <translation>שגיאת הפעלת תקופת ניסיון</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="267"/>
        <source>Invalid server response</source>
        <translation>תגובת שרת לא חוקית</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="268"/>
        <source>The server returned malformed data: %1</source>
        <translation>השרת החזיר נתונים פגומים: %1</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="299"/>
        <source>Unexpected server response</source>
        <translation>תגובת שרת לא צפויה</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="300"/>
        <source>The server response is missing required fields.</source>
        <translation>תגובת השרת חסרה שדות נדרשים.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="873"/>
        <source>The frame parser is using more than %1% of CPU time.</source>
        <translation>מנתח המסגרות משתמש ביותר מ-%1% מזמן המעבד.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="875"/>
        <source>Serial Studio is dropping frames to keep the application responsive. Please simplify or optimize the frame parser script to reduce its workload.</source>
        <translation>Serial Studio משמיט מסגרות כדי לשמור על היענות האפליקציה. יש לפשט או לייעל את סקריפט מנתח המסגרות כדי להפחית את עומס העבודה שלו.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="262"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="271"/>
        <source>Frame Parser Disabled</source>
        <translation>מנתח מסגרות מושבת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="272"/>
        <source>The Lua frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>מנתח המסגרות Lua עבור מקור %1 חרג מזמן קצוב ב-%2 מסגרות ברצף והושבת כדי לשמור על היענות Serial Studio.

הסיבה הסבירה ביותר: לולאה אינסופית או פעולה איטית במיוחד בגוף הסקריפט. יש לתקן את הסקריפט ולטעון מחדש את הפרויקט כדי להפעיל מחדש את הניתוח.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="320"/>
        <source>Lua Syntax Error</source>
        <translation>שגיאת תחביר Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="321"/>
        <source>The parser code contains an error:

%1</source>
        <translation>קוד המפענח מכיל שגיאה:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="369"/>
        <source>Lua Runtime Error</source>
        <translation>שגיאת זמן ריצה Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="370"/>
        <source>The parser code triggered an error:

%1</source>
        <translation>קוד המפענח גרם לשגיאה:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="488"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="391"/>
        <source>Missing Parse Function</source>
        <translation>פונקציית Parse חסרה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="392"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) ... end</source>
        <translation>הפונקציה 'parse' אינה מוגדרת בסקריפט.

וודא שהקוד שלך כולל:
function parse(frame) ... end</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="540"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="454"/>
        <source>Parse Function Runtime Error</source>
        <translation>שגיאת זמן ריצה בפונקציית Parse</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="455"/>
        <source>The parse function contains an error:

%1

Please fix the error in the function body.</source>
        <translation>פונקציית parse מכילה שגיאה:

%1

תקן את השגיאה בגוף הפונקציה.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="263"/>
        <source>The JavaScript frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>מפענח ה-JavaScript עבור מקור %1 חרג מזמן התגובה ב-%2 פריימים ברצף והושבת כדי לשמור על תגובתיות Serial Studio.

הסיבה הסבירה ביותר: לולאה אינסופית או פעולה איטית במיוחד בגוף הסקריפט. תקן את הסקריפט וטען מחדש את הפרויקט כדי להפעיל מחדש את הניתוח.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="429"/>
        <source>JavaScript Timed Out</source>
        <translation>פסק זמן ל-JavaScript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="430"/>
        <source>The parser code did not finish evaluating within %1 ms and was interrupted.

Most likely cause: an infinite loop at the top level of the script.</source>
        <translation>קוד המנתח לא סיים להתבצע תוך %1 ms והופסק.

הסיבה הסבירה ביותר: לולאה אינסופית ברמה העליונה של התסריט.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="447"/>
        <source>JavaScript Syntax Error</source>
        <translation>שגיאת תחביר JavaScript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="448"/>
        <source>The parser code contains a syntax error at line %1:

%2</source>
        <translation>קוד המפענח מכיל שגיאת תחביר בשורה %1:

%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="462"/>
        <source>JavaScript Exception Occurred</source>
        <translation>אירעה חריגה ב-JavaScript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="463"/>
        <source>The parser code triggered the following exceptions:

%1</source>
        <translation>קוד המפענח גרם לחריגות הבאות:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="489"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) { ... }</source>
        <translation>הפונקציה 'parse' אינה מוגדרת בסקריפט.

יש לוודא שהקוד שלך כולל:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="541"/>
        <source>The parse function contains an error at line %1:

%2

Please fix the error in the function body.</source>
        <translation>הפונקציה parse מכילה שגיאה בשורה %1:

%2

יש לתקן את השגיאה בגוף הפונקציה.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="641"/>
        <source>Invalid Function Declaration</source>
        <translation>הצהרת פונקציה לא חוקית</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="642"/>
        <source>No callable 'parse' export found.

Define one of:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</source>
        <translation>לא נמצא ייצוא 'parse' שניתן לקריאה.

הגדר אחד מהבאים:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="658"/>
        <source>The 'parse' function must accept at least one parameter (the frame payload).</source>
        <translation>הפונקציה 'parse' חייבת לקבל לפחות פרמטר אחד (המטען של ה-Frame).</translation>
    </message>
    <message>
        <source>No valid 'parse' function declaration found.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">לא נמצאה הצהרת פונקציה 'parse' תקינה.

פורמט צפוי:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="657"/>
        <source>Invalid Function Parameter</source>
        <translation>פרמטר פונקציה לא חוקי</translation>
    </message>
    <message>
        <source>The 'parse' function must have at least one parameter.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">הפונקציה 'parse' חייבת לכלול לפחות פרמטר אחד.

פורמט צפוי:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="623"/>
        <source>Deprecated Function Signature</source>
        <translation>חתימת פונקציה מיושנת</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="624"/>
        <source>The 'parse' function uses the old two-parameter format: parse(%1, %2)

This format is no longer supported. Please update to the new single-parameter format:
function parse(%1) { ... }

The separator parameter is no longer needed.</source>
        <translation>הפונקציה 'parse' משתמשת בפורמט הישן של שני פרמטרים: parse(%1, %2)

פורמט זה אינו נתמך עוד. יש לעדכן לפורמט החדש של פרמטר יחיד:
function parse(%1) { ... }

פרמטר המפריד אינו נחוץ עוד.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="386"/>
        <source>Expected %1, got '%2'</source>
        <translation>צפוי %1, התקבל '%2'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="435"/>
        <source>Expected enum name after 'enum'</source>
        <translation>צפוי שם enum אחרי 'enum'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="449"/>
        <source>Expected oneof name</source>
        <translation>צפוי שם oneof</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="476"/>
        <source>Field tag '%1' out of range (1..%2)</source>
        <translation>תג שדה '%1' מחוץ לטווח (1..%2)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="494"/>
        <source>Expected key type in map&lt;&gt;</source>
        <translation>צפוי סוג מפתח ב-map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="502"/>
        <source>Expected value type in map&lt;&gt;</source>
        <translation>צפוי סוג ערך ב-map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="510"/>
        <source>Expected map field name</source>
        <translation>צפוי שם שדה map</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="522"/>
        <source>Expected map field tag</source>
        <translation>צפוי תג שדה map</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="554"/>
        <source>Expected field type, got '%1'</source>
        <translation>צפוי סוג שדה, התקבל '%1'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="573"/>
        <source>Expected field name after type</source>
        <translation>צפוי שם שדה אחרי הסוג</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="583"/>
        <source>Expected field tag number</source>
        <translation>צפוי מספר תג שדה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="630"/>
        <source>Message nesting too deep (limit %1)</source>
        <translation>קינון הודעות עמוק מדי (מגבלה %1)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="635"/>
        <source>Expected message name</source>
        <translation>צפוי שם הודעה</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="717"/>
        <source>Unexpected token '%1' at file scope</source>
        <translation>אסימון לא צפוי '%1' בהיקף הקובץ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="763"/>
        <source>Unsupported top-level keyword '%1'</source>
        <translation>מילת מפתח ברמה עליונה לא נתמכת '%1'</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="162"/>
        <source>Console Output File Error</source>
        <translation>שגיאת קובץ פלט קונסול</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="163"/>
        <source>Cannot open file for writing!</source>
        <translation>לא ניתן לפתוח קובץ לכתיבה!</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="294"/>
        <source>Automatic (Platform Default)</source>
        <translation>אוטומטי (ברירת מחדל של הפלטפורמה)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="299"/>
        <source>Software (Fallback)</source>
        <translation>תוכנה (חלופי)</translation>
    </message>
    <message>
        <source>Row %1</source>
        <translation type="vanished">שורה %1</translation>
    </message>
    <message>
        <source>[%1]</source>
        <translation type="vanished">[%1]</translation>
    </message>
    <message>
        <source>Frame %1</source>
        <translation type="vanished">מסגרת %1</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">מפענח</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation type="vanished">שורות</translation>
    </message>
    <message>
        <source>%1 row(s)</source>
        <translation type="vanished">%1 שורה/שורות</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="186"/>
        <source>The native parser configuration is not a valid JSON object.</source>
        <translation>תצורת המנתח המקורי אינה אובייקט JSON תקין.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="196"/>
        <source>Unknown native parser template: "%1".</source>
        <translation>תבנית מנתח מקורי לא ידועה: "%1".</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="330"/>
        <source>Built-In Parser Error</source>
        <translation>שגיאת מנתח מקורי</translation>
    </message>
    <message>
        <source>Native Parser Error</source>
        <translation type="vanished">שגיאת מנתח מקורי</translation>
    </message>
</context>
<context>
    <name>QuaGzipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="60"/>
        <source>QIODevice::Append is not supported for GZIP</source>
        <translation>QIODevice::Append אינו נתמך עבור GZIP</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="66"/>
        <source>Opening gzip for both reading and writing is not supported</source>
        <translation>פתיחת gzip גם לקריאה וגם לכתיבה אינה נתמכת</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="75"/>
        <source>You can open a gzip either for reading or for writing. Which is it?</source>
        <translation>ניתן לפתוח gzip לקריאה או לכתיבה בלבד. מה מהם?</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="81"/>
        <source>Could not gzopen() file</source>
        <translation>כשל ב-gzopen() של הקובץ</translation>
    </message>
</context>
<context>
    <name>QuaZIODevice</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="178"/>
        <source>QIODevice::Append is not supported for QuaZIODevice</source>
        <translation>QIODevice::Append אינו נתמך עבור QuaZIODevice</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="183"/>
        <source>QIODevice::ReadWrite is not supported for QuaZIODevice</source>
        <translation>QIODevice::ReadWrite אינו נתמך עבור QuaZIODevice</translation>
    </message>
</context>
<context>
    <name>QuaZipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quazipfile.cpp" line="251"/>
        <source>ZIP/UNZIP API error %1</source>
        <translation>שגיאת ZIP/UNZIP API %1</translation>
    </message>
</context>
<context>
    <name>ReportOptionsDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate PDF Report</source>
        <translation>יצירת דוח PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate Report</source>
        <translation>יצירת דוח</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="61"/>
        <source>Solid</source>
        <translation>אחיד</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="62"/>
        <source>Dashed</source>
        <translation>מקווקו</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="63"/>
        <source>Dotted</source>
        <translation>מנוקד</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="80"/>
        <source>A4 (210 × 297 mm)</source>
        <translation>A4 (210 × 297 מ"מ)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="81"/>
        <source>A3 (297 × 420 mm)</source>
        <translation>A3 (297 × 420 מ"מ)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="82"/>
        <source>A2 (420 × 594 mm)</source>
        <translation>A2 (420 × 594 מ"מ)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="83"/>
        <source>A1 (594 × 841 mm)</source>
        <translation>A1 (594 × 841 מ"מ)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="84"/>
        <source>A0 (841 × 1189 mm)</source>
        <translation>A0 (841 × 1189 מ"מ)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="85"/>
        <source>A5 (148 × 210 mm)</source>
        <translation>A5 (148 × 210 מ"מ)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="86"/>
        <source>A6 (105 × 148 mm)</source>
        <translation>A6 (105 × 148 מ"מ)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="87"/>
        <source>B4 (250 × 353 mm)</source>
        <translation>B4 (250 × 353 מ"מ)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="88"/>
        <source>B5 (176 × 250 mm)</source>
        <translation>B5 (176 × 250 מ"מ)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="89"/>
        <source>Letter (8.5 × 11 in)</source>
        <translation>Letter (8.5 × 11 אינץ')</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="90"/>
        <source>Legal (8.5 × 14 in)</source>
        <translation>Legal (8.5 × 14 אינץ')</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="91"/>
        <source>Executive (7.25 × 10.5 in)</source>
        <translation>Executive (7.25 × 10.5 אינץ')</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="92"/>
        <source>Tabloid (11 × 17 in)</source>
        <translation>Tabloid (11 × 17 אינץ')</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="93"/>
        <source>Ledger (17 × 11 in)</source>
        <translation>Ledger (17 × 11 אינץ')</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="103"/>
        <source>%1 — Session Report</source>
        <translation>%1 — דוח סשן</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="105"/>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="279"/>
        <source>Session Report</source>
        <translation>דוח סשן</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="187"/>
        <source>Branding</source>
        <translation>מיתוג</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="193"/>
        <source>Page</source>
        <translation>עמוד</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="199"/>
        <source>Sections</source>
        <translation>קטעים</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="247"/>
        <source>Identity</source>
        <translation>זהות</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="261"/>
        <source>Company</source>
        <translation>חברה</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="268"/>
        <source>e.g. Acme Test Systems</source>
        <translation>לדוגמה: Acme Test Systems</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="272"/>
        <source>Document title</source>
        <translation>כותרת מסמך</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="283"/>
        <source>Author</source>
        <translation>מחבר</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="290"/>
        <source>Prepared by (optional)</source>
        <translation>הוכן על ידי (אופציונלי)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="299"/>
        <source>Logo</source>
        <translation>לוגו</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="312"/>
        <source>File</source>
        <translation>קובץ</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="323"/>
        <source>PNG, JPG or SVG (optional)</source>
        <translation>PNG, JPG או SVG (אופציונלי)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="325"/>
        <source>Browse…</source>
        <translation>עיון…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="328"/>
        <source>Clear</source>
        <translation>נקה</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="369"/>
        <source>Paper</source>
        <translation>נייר</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="381"/>
        <source>Page size</source>
        <translation>גודל עמוד</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="396"/>
        <source>Plot appearance</source>
        <translation>מראה גרף</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="410"/>
        <source>Line width</source>
        <translation>רוחב קו</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="442"/>
        <source>Line style</source>
        <translation>סגנון קו</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="482"/>
        <source>Include</source>
        <translation>כלול</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="497"/>
        <source>Cover page (logo, document title, test subtitle)</source>
        <translation>עמוד שער (לוגו, כותרת מסמך, כותרת משנה של בדיקה)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="500"/>
        <source>Test information (project, timestamps, classification and notes)</source>
        <translation>מידע על הבדיקה (פרויקט, חותמות זמן, סיווג והערות)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="503"/>
        <source>Measurement summary (min, max, mean, std. deviation per parameter)</source>
        <translation>סיכום מדידות (מינימום, מקסימום, ממוצע, סטיית תקן לכל פרמטר)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="506"/>
        <source>Parameter trends (time-series chart per numeric parameter)</source>
        <translation>מגמות פרמטרים (גרף סדרות זמן לכל פרמטר מספרי)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="509"/>
        <source>Annotate min, max, and mean values on plots</source>
        <translation>הוסף הערות לערכי מינימום, מקסימום וממוצע בגרפים</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="532"/>
        <source>Cancel</source>
        <translation>ביטול</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="557"/>
        <source>Export PDF</source>
        <translation>ייצוא PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="557"/>
        <source>Export HTML</source>
        <translation>ייצוא HTML</translation>
    </message>
</context>
<context>
    <name>ReportProgressDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="20"/>
        <source>Generating Report</source>
        <translation>יוצר דוח</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="69"/>
        <source>Working…</source>
        <translation>עובד…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="86"/>
        <source>This can take a few seconds for sessions with many parameters. The window closes automatically when the report is ready.</source>
        <translation>פעולה זו עשויה לקחת מספר שניות עבור הפעלות עם פרמטרים רבים. החלון ייסגר אוטומטית כאשר הדוח מוכן.</translation>
    </message>
</context>
<context>
    <name>RuntimeReconfigure</name>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="40"/>
        <source>Connection Lost</source>
        <translation>החיבור אבד</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="41"/>
        <source>Device Unavailable</source>
        <translation>התקן לא זמין</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="95"/>
        <source>The connection to your device was lost.</source>
        <translation>החיבור להתקן אבד.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="96"/>
        <source>Serial Studio couldn't reach your device.</source>
        <translation>Serial Studio לא הצליח להגיע להתקן.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="104"/>
        <source>Check the cable, power, and that no other application has taken over the device. You can try reconnecting, switch to a different device, or quit.</source>
        <translation>בדוק את הכבל, החשמל, ושאף יישום אחר לא השתלט על ההתקן. ניתן לנסות להתחבר מחדש, לעבור להתקן אחר, או לצאת.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="107"/>
        <source>Make sure it's plugged in, powered on, and not already in use by another app. You can try again, pick a different device, or quit.</source>
        <translation>ודא שההתקן מחובר, מופעל, ולא נמצא כבר בשימוש על ידי יישום אחר. ניתן לנסות שוב, לבחור התקן אחר, או לצאת.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="119"/>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="188"/>
        <source>Quit</source>
        <translation>יציאה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="129"/>
        <source>Pick Different Device</source>
        <translation>בחר התקן אחר</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="140"/>
        <source>Reconnect</source>
        <translation>התחבר מחדש</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="140"/>
        <source>Try Again</source>
        <translation>נסה שוב</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="156"/>
        <source>Pick the correct device, then press Connect.</source>
        <translation>בחר את ההתקן הנכון ולחץ על התחבר.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="165"/>
        <source>I/O Interface: %1</source>
        <translation>ממשק קלט/פלט: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="198"/>
        <source>Connect</source>
        <translation>התחבר</translation>
    </message>
</context>
<context>
    <name>SerialStudio</name>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="334"/>
        <source>Data Grids</source>
        <translation>רשתות נתונים</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="337"/>
        <source>Multiple Data Plots</source>
        <translation>תרשימי נתונים מרובים</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="340"/>
        <source>Accelerometers</source>
        <translation>מדי תאוצה</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="343"/>
        <source>Gyroscopes</source>
        <translation>גירוסקופים</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="346"/>
        <source>GPS</source>
        <translation>GPS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="349"/>
        <source>FFT Plots</source>
        <translation>תרשימי FFT</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="352"/>
        <source>LED Panels</source>
        <translation>לוחות LED</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="355"/>
        <source>Data Plots</source>
        <translation>תרשימי נתונים</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="358"/>
        <source>Bars</source>
        <translation>עמודות</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="361"/>
        <source>Gauges</source>
        <translation>מדי מחוונים</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="364"/>
        <source>Terminal</source>
        <translation>מסוף</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="367"/>
        <source>Clock</source>
        <translation>שעון</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="370"/>
        <source>Stopwatch</source>
        <translation>שעון עצר</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="373"/>
        <source>Compasses</source>
        <translation>מצפנים</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="376"/>
        <source>Meters</source>
        <translation>מדים</translation>
    </message>
    <message>
        <source>Thermometers</source>
        <translation type="vanished">מדחומים</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="379"/>
        <source>3D Plots</source>
        <translation>תרשימי תלת-ממד</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="383"/>
        <source>Image Views</source>
        <translation>תצוגות תמונה</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="386"/>
        <source>Output Panels</source>
        <translation>לוחות פלט</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="389"/>
        <source>Notifications</source>
        <translation>התראות</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="392"/>
        <source>Waterfalls</source>
        <translation>מפלי מים</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="395"/>
        <source>Painter Widgets</source>
        <translation>ווידג'טי ציור</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="912"/>
        <source>UTF-8</source>
        <translation>UTF-8</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="913"/>
        <source>UTF-16 LE</source>
        <translation>UTF-16 LE</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="914"/>
        <source>UTF-16 BE</source>
        <translation>UTF-16 BE</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="915"/>
        <source>Latin-1</source>
        <translation>Latin-1</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="916"/>
        <source>System</source>
        <translation>מערכת</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="917"/>
        <source>GBK</source>
        <translation>GBK</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="918"/>
        <source>GB18030</source>
        <translation>GB18030</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="919"/>
        <source>Big5</source>
        <translation>Big5</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="920"/>
        <source>Shift-JIS</source>
        <translation>Shift-JIS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="921"/>
        <source>EUC-JP</source>
        <translation>EUC-JP</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="922"/>
        <source>EUC-KR</source>
        <translation>EUC-KR</translation>
    </message>
</context>
<context>
    <name>SessionDetail</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="19"/>
        <source>Session Details</source>
        <translation>פרטי סשן</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="88"/>
        <source>Select a session to view details.</source>
        <translation>בחר סשן כדי להציג פרטים.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="130"/>
        <source>Project:</source>
        <translation>פרויקט:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="143"/>
        <source>Started:</source>
        <translation>התחיל:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="156"/>
        <source>Ended:</source>
        <translation>הסתיים:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="162"/>
        <source>(in progress)</source>
        <translation>(בתהליך)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="169"/>
        <source>Frames:</source>
        <translation>מסגרות:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="185"/>
        <source>Notes</source>
        <translation>הערות</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="200"/>
        <source>Add session notes…</source>
        <translation>הוסף הערות סשן…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="201"/>
        <source>Notes are read-only for completed sessions.</source>
        <translation>הערות הן לקריאה בלבד עבור סשנים שהושלמו.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="222"/>
        <source>Tags</source>
        <translation>תגיות</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="286"/>
        <source>New tag…</source>
        <translation>תגית חדשה…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="293"/>
        <source>Add</source>
        <translation>הוסף</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="330"/>
        <source>Replay</source>
        <translation>הפעל מחדש</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="338"/>
        <source>Export CSV</source>
        <translation>ייצוא CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="345"/>
        <source>Generate Report</source>
        <translation>צור דוח</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="356"/>
        <source>Delete</source>
        <translation>מחק</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="362"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>בטל נעילת קובץ הסשן כדי למחוק סשנים</translation>
    </message>
</context>
<context>
    <name>SessionList</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="19"/>
        <source>Sessions</source>
        <translation>סשנים</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="71"/>
        <source>Search</source>
        <translation>חיפוש</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="91"/>
        <source>Date</source>
        <translation>תאריך</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="92"/>
        <source>Frames</source>
        <translation>מסגרות</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="93"/>
        <source>Tags</source>
        <translation>תגיות</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="193"/>
        <source>No sessions found.</source>
        <translation>לא נמצאו סשנים.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="194"/>
        <source>No session file open.</source>
        <translation>אין קובץ סשן פתוח.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseManager</name>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="415"/>
        <source>Open Session File</source>
        <translation>פתח קובץ סשן</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="417"/>
        <source>Session files (*.db)</source>
        <translation>קבצי סשן (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="473"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="482"/>
        <source>Lock Session File</source>
        <translation>נעל קובץ סשן</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="474"/>
        <source>Choose a password to lock the session file:</source>
        <translation>בחר סיסמה לנעילת קובץ הסשן:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="483"/>
        <source>Confirm the password:</source>
        <translation>אשר את הסיסמה:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="491"/>
        <source>Passwords do not match</source>
        <translation>הסיסמאות אינן תואמות</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="492"/>
        <source>The two passwords you entered do not match. The session file was not locked.</source>
        <translation>שתי הסיסמאות שהזנת אינן תואמות. קובץ הסשן לא ננעל.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="528"/>
        <source>Unlock Session File</source>
        <translation>בטל נעילת קובץ סשן</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="529"/>
        <source>Enter the session file password:</source>
        <translation>הזן את סיסמת קובץ הסשן:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="539"/>
        <source>Incorrect password</source>
        <translation>סיסמה שגויה</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="540"/>
        <source>The password you entered does not match the one stored in the session file.</source>
        <translation>הסיסמה שהזנת אינה תואמת לזו השמורה בקובץ הסשן.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="632"/>
        <source>Session file locked</source>
        <translation>קובץ הסשן נעול</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="633"/>
        <source>Unlock the session file before deleting recorded sessions.</source>
        <translation>בטל נעילת קובץ הסשן לפני מחיקת סשנים מוקלטים.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="642"/>
        <source>Delete session from %1?</source>
        <translation>למחוק סשן מ-%1?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="643"/>
        <source>All readings and raw data for this session are permanently removed.</source>
        <translation>כל הקריאות והנתונים הגולמיים של סשן זה יוסרו לצמיתות.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="645"/>
        <source>Delete Session</source>
        <translation>מחק סשן</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="767"/>
        <source>Export Session to CSV</source>
        <translation>ייצוא סשן ל-CSV</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="767"/>
        <source>CSV files (*.csv)</source>
        <translation>קבצי CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="842"/>
        <source>Loading session data…</source>
        <translation>טוען נתוני סשן…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="862"/>
        <source>Save PDF Report</source>
        <translation>שמור דוח PDF</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="862"/>
        <source>Save HTML Report</source>
        <translation>שמור דוח HTML</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="863"/>
        <source>PDF files (*.pdf)</source>
        <translation>קבצי PDF (*.PDF)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="863"/>
        <source>HTML files (*.html)</source>
        <translation>קבצי HTML (*.HTML)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="926"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="962"/>
        <source>Failed</source>
        <translation>נכשל</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="932"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="972"/>
        <source>Report Failed</source>
        <translation>הדוח נכשל</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="934"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="973"/>
        <source>Could not generate the report. Check the output path and try again.</source>
        <translation>לא ניתן ליצור את הדוח. בדוק את נתיב הפלט ונסה שוב.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="962"/>
        <source>Done</source>
        <translation>הושלם</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="990"/>
        <source>Select logo image</source>
        <translation>בחירת תמונת לוגו</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="992"/>
        <source>Images (*.png *.jpg *.jpeg *.svg)</source>
        <translation>תמונות (*.png *.jpg *.jpeg *.svg)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1050"/>
        <source>No project data</source>
        <translation>אין נתוני פרויקט</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1051"/>
        <source>This session file does not contain an embedded project.</source>
        <translation>קובץ סשן זה אינו מכיל פרויקט משובץ.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1060"/>
        <source>Invalid project data</source>
        <translation>נתוני פרויקט לא תקינים</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1061"/>
        <source>The embedded project JSON is malformed and cannot be restored.</source>
        <translation>ה-JSON המוטמע של הפרויקט פגום ולא ניתן לשחזרו.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1071"/>
        <source>Restore Project</source>
        <translation>שחזור פרויקט</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1071"/>
        <source>Serial Studio projects (*.ssproj *.json)</source>
        <translation>פרויקטי Serial Studio (*.ssproj *.json)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1079"/>
        <source>Cannot write file</source>
        <translation>לא ניתן לכתוב קובץ</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1079"/>
        <source>Check file permissions and try again.</source>
        <translation>בדוק הרשאות קובץ ונסה שוב.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1189"/>
        <source>Cannot open session file</source>
        <translation>לא ניתן לפתוח קובץ סשן</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseWorker</name>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="76"/>
        <source>Empty file path</source>
        <translation>נתיב קובץ ריק</translation>
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
        <translation>מסד הנתונים אינו פתוח</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="262"/>
        <source>Database not open or empty label</source>
        <translation>מסד הנתונים אינו פתוח או תווית ריקה</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="330"/>
        <source>Invalid label</source>
        <translation>תווית לא חוקית</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="597"/>
        <source>Cancelled</source>
        <translation>בוטל</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="710"/>
        <source>Could not load session data</source>
        <translation>לא ניתן לטעון נתוני סשן</translation>
    </message>
</context>
<context>
    <name>Sessions::HtmlReport</name>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="209"/>
        <source>Assembling report…</source>
        <translation>הרכבת דוח…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="217"/>
        <source>Writing output…</source>
        <translation>כתיבת פלט…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="282"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="342"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="700"/>
        <source>Session Report</source>
        <translation>דוח סשן</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="345"/>
        <source>Untitled project</source>
        <translation>פרויקט ללא שם</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="352"/>
        <source>Prepared by</source>
        <translation>הוכן על ידי</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="355"/>
        <source>Generated on %1</source>
        <translation>נוצר ב-%1</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="377"/>
        <source>Test ID</source>
        <translation>מזהה בדיקה</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="379"/>
        <source>Duration</source>
        <translation>משך זמן</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="381"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="493"/>
        <source>Samples</source>
        <translation>דגימות</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="383"/>
        <source>Parameters</source>
        <translation>פרמטרים</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="385"/>
        <source>Started</source>
        <translation>התחיל</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="387"/>
        <source>Ended</source>
        <translation>הסתיים</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="423"/>
        <source>Project</source>
        <translation>פרויקט</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="425"/>
        <source>Test identifier</source>
        <translation>מזהה בדיקה</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="426"/>
        <source>Start time</source>
        <translation>זמן התחלה</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="427"/>
        <source>End time</source>
        <translation>זמן סיום</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="428"/>
        <source>Total duration</source>
        <translation>משך זמן כולל</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="429"/>
        <source>Samples acquired</source>
        <translation>דגימות שנרכשו</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="430"/>
        <source>Parameters logged</source>
        <translation>פרמטרים שנרשמו</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="446"/>
        <source>Classification</source>
        <translation>סיווג</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="453"/>
        <source>Notes</source>
        <translation>הערות</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="461"/>
        <source>Test Information</source>
        <translation>מידע על בדיקה</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="482"/>
        <source>Parameter</source>
        <translation>פרמטר</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="485"/>
        <source>Units</source>
        <translation>יחידות</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="494"/>
        <source>Minimum</source>
        <translation>מינימום</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="495"/>
        <source>Maximum</source>
        <translation>מקסימום</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="496"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="652"/>
        <source>Mean</source>
        <translation>ממוצע</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="497"/>
        <source>Std. Deviation</source>
        <translation>סטיית תקן</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="542"/>
        <source>Measurement Summary</source>
        <translation>סיכום מדידות</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="543"/>
        <source>click a column to sort</source>
        <translation>לחץ על עמודה למיון</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="568"/>
        <source>%1 samples over %2 seconds</source>
        <translation>%1 דגימות על פני %2 שניות</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="586"/>
        <source>Combined Parameter View</source>
        <translation>תצוגת פרמטרים משולבת</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="587"/>
        <source>click legend items to toggle signals</source>
        <translation>לחץ על פריטי מקרא להחלפת אותות</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="595"/>
        <source>Parameter Trends</source>
        <translation>מגמות פרמטרים</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="650"/>
        <source>Min</source>
        <translation>מינימום</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="651"/>
        <source>Max</source>
        <translation>מקסימום</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="724"/>
        <source>Page %1 of %2</source>
        <translation>עמוד %1 מתוך %2</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="794"/>
        <source>Loading rendering engine…</source>
        <translation>טוען מנוע רינדור…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="814"/>
        <source>Rendering charts…</source>
        <translation>מרנדר גרפים…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="858"/>
        <source>Generating PDF…</source>
        <translation>מייצר PDF…</translation>
    </message>
</context>
<context>
    <name>Sessions::Player</name>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="258"/>
        <source>Open Session File</source>
        <translation>פתח קובץ סשן</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="260"/>
        <source>Session files (*.db)</source>
        <translation>קבצי סשן (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="330"/>
        <source>Device Connection Active</source>
        <translation>חיבור למכשיר פעיל</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="331"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>כדי להשתמש בתכונה זו, יש להתנתק מהמכשיר. להמשיך?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="379"/>
        <location filename="../../src/Sessions/Player.cpp" line="458"/>
        <source>Cannot open session file</source>
        <translation>לא ניתן לפתוח קובץ סשן</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="380"/>
        <source>Unknown error</source>
        <translation>שגיאה לא ידועה</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="396"/>
        <source>No project data</source>
        <translation>אין נתוני פרויקט</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="397"/>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation>הסשן אינו מכיל קובץ פרויקט משובץ — לוח הבקרה עובר לפריסת quick-plot.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="459"/>
        <source>Check file permissions and try again.</source>
        <translation>בדוק הרשאות קובץ ונסה שוב.</translation>
    </message>
</context>
<context>
    <name>Sessions::PlayerLoaderWorker</name>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="69"/>
        <source>This file does not contain any recording sessions.</source>
        <translation>קובץ זה אינו מכיל סשני הקלטה.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="144"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="205"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="224"/>
        <source>Cancelled</source>
        <translation>בוטל</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="168"/>
        <source>Empty file path</source>
        <translation>נתיב קובץ ריק</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="218"/>
        <source>The selected session is missing its column definitions.</source>
        <translation>הסשן שנבחר חסר את הגדרות העמודות שלו.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="235"/>
        <source>The selected session does not contain any frames.</source>
        <translation>הסשן שנבחר אינו מכיל מסגרות.</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="34"/>
        <source>Preferences</source>
        <translation>העדפות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="61"/>
        <source>General</source>
        <translation>כללי</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="67"/>
        <source>Dashboard</source>
        <translation>לוח בקרה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="73"/>
        <source>Taskbar</source>
        <translation>שורת משימות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="79"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="849"/>
        <source>Console</source>
        <translation>קונסול</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="86"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="850"/>
        <source>Notifications</source>
        <translation>התראות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="144"/>
        <source>Appearance</source>
        <translation>מראה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="159"/>
        <source>Language</source>
        <translation>שפה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="175"/>
        <source>Theme</source>
        <translation>ערכת נושא</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="194"/>
        <source>Files &amp; Updates</source>
        <translation>קבצים ועדכונים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="210"/>
        <source>Workspace Folder</source>
        <translation>תיקיית סביבת עבודה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="237"/>
        <source>Automatically Check for Updates</source>
        <translation>בדוק עדכונים באופן אוטומטי</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="255"/>
        <source>Advanced</source>
        <translation>מתקדם</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="270"/>
        <source>Rendering Backend</source>
        <translation>מנגנון רינדור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="303"/>
        <source>Auto-Hide Toolbar</source>
        <translation>הסתר סרגל כלים אוטומטית</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="320"/>
        <source>Enable API Server (Port 7777)</source>
        <translation>הפעל שרת API (פורט 7777)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="338"/>
        <source>Allow External API Connections</source>
        <translation>אפשר חיבורי API חיצוניים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="354"/>
        <source>API Access Token</source>
        <translation>אסימון גישה ל-API</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="384"/>
        <source>Export Protobuf File</source>
        <translation>ייצא קובץ Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="386"/>
        <source>Export…</source>
        <translation>ייצא…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="435"/>
        <source>Data Plotting</source>
        <translation>שרטוט נתונים</translation>
    </message>
    <message>
        <source>Point Count</source>
        <translation type="vanished">מספר נקודות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="450"/>
        <source>Time Range</source>
        <translation>טווח זמן</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="500"/>
        <source>UI Refresh Rate (Hz)</source>
        <translation>קצב רענון ממשק (Hz)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="522"/>
        <source>Dashboard Font</source>
        <translation>גופן לוח בקרה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="537"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1012"/>
        <source>Font Family</source>
        <translation>משפחת גופן</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="552"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1027"/>
        <source>Font Size</source>
        <translation>גודל גופן</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Small</source>
        <translation>קטן</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Normal</source>
        <translation>רגיל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Large</source>
        <translation>גדול</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Extra Large</source>
        <translation>גדול במיוחד</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Custom</source>
        <translation>מותאם אישית</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="609"/>
        <source>Layout</source>
        <translation>פריסה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="624"/>
        <source>Show Actions Panel</source>
        <translation>הצג לוח פעולות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="647"/>
        <source>Video Export</source>
        <translation>ייצוא וידאו</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="665"/>
        <source>Save Videos by Default</source>
        <translation>שמור סרטונים כברירת מחדל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="716"/>
        <source>Behavior</source>
        <translation>התנהגות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="737"/>
        <source>Always Show Taskbar Buttons</source>
        <translation>הצג תמיד כפתורי שורת משימות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="752"/>
        <source>Show Search Field</source>
        <translation>הצג שדה חיפוש</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="767"/>
        <source>Auto-hide Taskbar</source>
        <translation>הסתר אוטומטית שורת משימות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="785"/>
        <source>Hide Delay (ms)</source>
        <translation>עיכוב הסתרה (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="809"/>
        <source>Pinned Buttons</source>
        <translation>כפתורים מוצמדים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="827"/>
        <source>Drag a pinned button on the taskbar to reorder it.</source>
        <translation>גרור כפתור מוצמד בשורת המשימות כדי לשנות את סדרו.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="848"/>
        <source>Settings</source>
        <translation>הגדרות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="851"/>
        <source>Clock</source>
        <translation>שעון</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="852"/>
        <source>Stopwatch</source>
        <translation>שעון עצר</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="853"/>
        <source>Pause / Resume</source>
        <translation>השהה / המשך</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="854"/>
        <source>File Transmission</source>
        <translation>שידור קובץ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="855"/>
        <source>AI Assistant</source>
        <translation>עוזר AI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="984"/>
        <source>Display</source>
        <translation>תצוגה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="999"/>
        <source>Display Mode</source>
        <translation>מצב תצוגה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1044"/>
        <source>Show Timestamps</source>
        <translation>הצג חותמות זמן</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1063"/>
        <source>Data Transmission</source>
        <translation>שידור נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1078"/>
        <source>Line Ending</source>
        <translation>סיום שורה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1091"/>
        <source>Input Mode</source>
        <translation>מצב קלט</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1104"/>
        <source>Text Encoding</source>
        <translation>קידוד טקסט</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1117"/>
        <source>Checksum</source>
        <translation>Checksum</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1130"/>
        <source>Echo Sent Data</source>
        <translation>הדהד נתונים שנשלחו</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1149"/>
        <source>Escape Codes</source>
        <translation>קודי Escape</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1164"/>
        <source>VT100 Emulation</source>
        <translation>אמולציית VT100</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1183"/>
        <source>ANSI Colors</source>
        <translation>צבעי ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1241"/>
        <source>Delivery</source>
        <translation>מסירה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1256"/>
        <source>System Notifications</source>
        <translation>התראות מערכת</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1277"/>
        <source>Show Warning/Critical events as OS desktop notifications when Serial Studio is not the foreground window.</source>
        <translation>הצג אירועי אזהרה/קריטיים כהתראות שולחן עבודה של מערכת ההפעלה כאשר Serial Studio אינו בחלון הקדמי.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1287"/>
        <source>Application Logs</source>
        <translation>יומני יישום</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1302"/>
        <source>Route Warnings to Notifications</source>
        <translation>נתב אזהרות להתראות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1323"/>
        <source>Off by default — Qt and QML emit warnings frequently and enabling this can drown out real alarms. Critical messages are always routed regardless of this setting.</source>
        <translation>כבוי כברירת מחדל — QT ו-QML פולטים אזהרות לעיתים קרובות והפעלת אפשרות זו עלולה להציף התראות אמיתיות. הודעות קריטיות תמיד מנותבות ללא קשר להגדרה זו.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1342"/>
        <source>Reset</source>
        <translation>אפס</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1377"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1385"/>
        <source>Apply</source>
        <translation>החל</translation>
    </message>
</context>
<context>
    <name>Setup</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="35"/>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="380"/>
        <source>Device Setup</source>
        <translation>הגדרת התקן</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="126"/>
        <source>API Server Active (%1)</source>
        <translation>שרת API פעיל (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="127"/>
        <source>API Server Ready</source>
        <translation>שרת API מוכן</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="128"/>
        <source>API Server Off</source>
        <translation>שרת API כבוי</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="188"/>
        <source>Frame Parsing</source>
        <translation>ניתוח מסגרות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="198"/>
        <source>Console Only (No Parsing)</source>
        <translation>קונסול בלבד (ללא ניתוח)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="211"/>
        <source>Quick Plot (Comma Separated Values)</source>
        <translation>תרשים מהיר (ערכים מופרדים בפסיקים)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="222"/>
        <source>Parse via Project File</source>
        <translation>ניתוח באמצעות קובץ פרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="245"/>
        <source>Change Project File (%1)</source>
        <translation>שינוי קובץ פרויקט (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="246"/>
        <source>Select Project File</source>
        <translation>בחירת קובץ פרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="261"/>
        <source>Data Export</source>
        <translation>ייצוא נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="285"/>
        <source>CSV Spreadsheet</source>
        <translation>גיליון אלקטרוני CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="303"/>
        <source>Session Recording</source>
        <translation>הקלטת סשן</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="324"/>
        <source>MDF4 Recording</source>
        <translation>הקלטת MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="340"/>
        <source>Console Log</source>
        <translation>יומן קונסול</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="392"/>
        <source>I/O Interface: %1</source>
        <translation>ממשק קלט/פלט: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="461"/>
        <source>Multi-Device Project</source>
        <translation>פרויקט רב-מכשירי</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="474"/>
        <source>This project streams data from %1 independent devices.</source>
        <translation>פרויקט זה מזרים נתונים מ־%1 מכשירים עצמאיים.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="487"/>
        <source>Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.</source>
        <translation>לכל מכשיר הגדרות חיבור משלו. ניתן להגדיר אותן בעורך הפרויקט תחת הלשונית מקורות.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="506"/>
        <source>Open Project Editor</source>
        <translation>פתח עורך פרויקט</translation>
    </message>
</context>
<context>
    <name>ShortcutGenerator</name>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="22"/>
        <source>New Deployment</source>
        <translation>פריסה חדשה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="93"/>
        <source>Choose an Icon</source>
        <translation>בחר סמל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="106"/>
        <source>Save Deployment</source>
        <translation>שמור פריסה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="146"/>
        <source>General</source>
        <translation>כללי</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="152"/>
        <source>Taskbar</source>
        <translation>שורת משימות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="158"/>
        <source>Logging</source>
        <translation>רישום</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="215"/>
        <source>Identity</source>
        <translation>זהות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="271"/>
        <source>Click to choose an icon</source>
        <translation>לחץ לבחירת סמל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="280"/>
        <source>Name:</source>
        <translation>שם:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="289"/>
        <source>Deployment Name</source>
        <translation>שם פריסה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="298"/>
        <source>Change Icon…</source>
        <translation>שנה סמל…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="315"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="333"/>
        <source>Project</source>
        <translation>פרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="343"/>
        <source>Choose a project file to begin</source>
        <translation>בחר קובץ פרויקט כדי להתחיל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="365"/>
        <source>Behavior</source>
        <translation>התנהגות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="380"/>
        <source>Theme</source>
        <translation>ערכת נושא</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="390"/>
        <source>Same as Serial Studio</source>
        <translation>זהה ל-Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="402"/>
        <source>Fullscreen</source>
        <translation>מסך מלא</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="412"/>
        <source>Actions Panel</source>
        <translation>לוח פעולות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="423"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="632"/>
        <source>File Transmission</source>
        <translation>שידור קובץ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="439"/>
        <source>Double-clicking this deployment takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation>לחיצה כפולה על פריסה זו מעבירה ישירות ל-Dashboard החי של הפרויקט. אין סרגל כלים או חלונית הגדרות, רק הנתונים, ו-Serial Studio נסגר ברגע שההתקן מתנתק.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="485"/>
        <source>Visibility</source>
        <translation>נראות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="500"/>
        <source>Mode</source>
        <translation>מצב</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="509"/>
        <source>Always shown</source>
        <translation>מוצג תמיד</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="510"/>
        <source>Auto-hide</source>
        <translation>הסתרה אוטומטית</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="511"/>
        <source>Hidden</source>
        <translation>מוסתר</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="526"/>
        <source>Hiding the taskbar removes window minimize/maximize/close buttons and forces auto-layout, so the dashboard always fills the available area.</source>
        <translation>הסתרת סרגל המשימות מסירה את כפתורי מזעור/הגדלה/סגירה של החלון ומאלצת פריסה אוטומטית, כך ש-Dashboard תמיד ממלא את השטח הזמין.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="530"/>
        <source>The taskbar slides in when the user moves the cursor near the bottom edge.</source>
        <translation>סרגל המשימות מופיע כאשר המשתמש מזיז את הסמן לקצה התחתון.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="532"/>
        <source>The taskbar is permanently visible at the bottom of the dashboard.</source>
        <translation>סרגל המשימות נראה באופן קבוע בתחתית ה-Dashboard.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="545"/>
        <source>Pinned Buttons</source>
        <translation>כפתורים מוצמדים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="562"/>
        <source>Console</source>
        <translation>קונסול</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="576"/>
        <source>Notifications</source>
        <translation>התראות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="590"/>
        <source>Clock</source>
        <translation>שעון</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="604"/>
        <source>Stopwatch</source>
        <translation>שעון עצר</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="618"/>
        <source>Pause</source>
        <translation>השהה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="682"/>
        <source>Recorders</source>
        <translation>מקליטים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="697"/>
        <source>CSV File</source>
        <translation>קובץ CSV</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="707"/>
        <source>MDF4 File</source>
        <translation>קובץ MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="717"/>
        <source>Session Database</source>
        <translation>מסד נתוני סשן</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="727"/>
        <source>Console Log</source>
        <translation>יומן קונסול</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="743"/>
        <source>Recordings are saved in the Serial Studio workspace folder</source>
        <translation>הקלטות נשמרות בתיקיית עבודה של Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="772"/>
        <source>Cancel</source>
        <translation>ביטול</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="781"/>
        <source>Save</source>
        <translation>שמור</translation>
    </message>
</context>
<context>
    <name>SourceFrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="116"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="353"/>
        <source>Undo</source>
        <translation>בטל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="123"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="364"/>
        <source>Redo</source>
        <translation>בצע שוב</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="132"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="383"/>
        <source>Cut</source>
        <translation>גזור</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="137"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="393"/>
        <source>Copy</source>
        <translation>העתק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="142"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="403"/>
        <source>Paste</source>
        <translation>הדבק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="149"/>
        <source>Select All</source>
        <translation>בחר הכל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="159"/>
        <source>Format Document</source>
        <translation>עצב מסמך</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="166"/>
        <source>Format Selection</source>
        <translation>עצב בחירה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="333"/>
        <source>Reset</source>
        <translation>אפס</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="418"/>
        <source>Validate</source>
        <translation>אמת</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="422"/>
        <source>Verify that the script compiles correctly</source>
        <translation>אמת שהסקריפט מתקמפל כראוי</translation>
    </message>
    <message>
        <source>Reset template parameters to their defaults</source>
        <translation type="vanished">אפס פרמטרי תבנית לברירות המחדל שלהם</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="338"/>
        <source>Reset to the default parsing script</source>
        <translation>אפס לסקריפט ניתוח ברירת המחדל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="343"/>
        <source>Open</source>
        <translation>פתח</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="348"/>
        <source>Import a script file for data parsing</source>
        <translation>ייבא קובץ סקריפט לניתוח נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="358"/>
        <source>Undo the last code edit</source>
        <translation>בטל את עריכת הקוד האחרונה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="370"/>
        <source>Redo the previously undone edit</source>
        <translation>בצע שוב את העריכה שבוטלה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="388"/>
        <source>Cut selected code to clipboard</source>
        <translation>גזור קוד מסומן ללוח</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="398"/>
        <source>Copy selected code to clipboard</source>
        <translation>העתק קוד מסומן ללוח</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="407"/>
        <source>Paste code from clipboard</source>
        <translation>הדבק קוד מהלוח</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="283"/>
        <source>Help</source>
        <translation>עזרה</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">פתח תיעוד עזרה לניתוח נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="232"/>
        <source>Platform:</source>
        <translation>פלטפורמה:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="240"/>
        <source>Built-In</source>
        <translation>מקורי</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">שפה:</translation>
    </message>
    <message>
        <source>Native</source>
        <translation type="vanished">מקורי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="258"/>
        <source>Select Template…</source>
        <translation>בחר תבנית…</translation>
    </message>
    <message>
        <source>Template:</source>
        <translation type="vanished">תבנית:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="274"/>
        <source>Test With Sample Data</source>
        <translation>בדוק עם נתוני דוגמה</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">הערך</translation>
    </message>
</context>
<context>
    <name>SourceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="107"/>
        <source>Duplicate</source>
        <translation>שכפל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="109"/>
        <source>Create a copy of this data source</source>
        <translation>צור עותק של מקור נתונים זה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="121"/>
        <source>Delete</source>
        <translation>מחק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="126"/>
        <source>Remove this data source</source>
        <translation>הסר מקור נתונים זה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="127"/>
        <source>The primary data source cannot be removed</source>
        <translation>לא ניתן להסיר את מקור הנתונים הראשי</translation>
    </message>
</context>
<context>
    <name>SqlitePlayer</name>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="25"/>
        <source>Session Player</source>
        <translation>נגן סשן</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="92"/>
        <source>Loading session…</source>
        <translation>טוען סשן…</translation>
    </message>
</context>
<context>
    <name>StartMenu</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="99"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="543"/>
        <source>Auto Layout</source>
        <translation>פריסה אוטומטית</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="107"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="555"/>
        <source>Full Screen</source>
        <translation>מסך מלא</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="113"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="568"/>
        <source>Add External Window</source>
        <translation>הוסף חלון חיצוני</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="119"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="740"/>
        <source>Console</source>
        <translation>קונסול</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="125"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="749"/>
        <source>Notifications</source>
        <translation>התראות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="757"/>
        <source>Clock</source>
        <translation>שעון</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="141"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="764"/>
        <source>Stopwatch</source>
        <translation>שעון עצר</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="149"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="772"/>
        <source>Preferences</source>
        <translation>העדפות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="155"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="853"/>
        <source>Help Center</source>
        <translation>מרכז עזרה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="781"/>
        <source>Sessions</source>
        <translation>סשנים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="168"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="790"/>
        <source>File Transmission</source>
        <translation>שידור קובץ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="175"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="798"/>
        <source>AI Assistant</source>
        <translation>עוזר AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="276"/>
        <source>Workspaces</source>
        <translation>מרחבי עבודה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="343"/>
        <source>Show "%1"</source>
        <translation>הצג "%1"</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="348"/>
        <source>Show All Hidden Workspaces</source>
        <translation>הצג את כל מרחבי העבודה המוסתרים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="357"/>
        <source>New Workspace…</source>
        <translation>סביבת עבודה חדשה…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="372"/>
        <source>No Workspaces Available</source>
        <translation>אין מרחבי עבודה זמינים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="406"/>
        <source>Actions</source>
        <translation>פעולות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="429"/>
        <source>No Actions Available</source>
        <translation>אין פעולות זמינות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="459"/>
        <source>Plugins</source>
        <translation>תוספים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="497"/>
        <source>Manage Plugins…</source>
        <translation>ניהול תוספים…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="507"/>
        <source>No Plugins Installed</source>
        <translation>אין תוספים מותקנים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="588"/>
        <source>Export</source>
        <translation>ייצוא</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="619"/>
        <source>CSV File</source>
        <translation>קובץ CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="625"/>
        <source>MDF4 File</source>
        <translation>קובץ MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="631"/>
        <source>Console Transcript</source>
        <translation>תמליל קונסול</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="640"/>
        <source>Session Database</source>
        <translation>מסד נתוני סשן</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="654"/>
        <source>No Export Formats Available</source>
        <translation>אין פורמטי ייצוא זמינים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="684"/>
        <source>Tools</source>
        <translation>כלים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="813"/>
        <source>No Tools Available</source>
        <translation>אין כלים זמינים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="874"/>
        <source>Resume</source>
        <translation>המשך</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="875"/>
        <source>Pause</source>
        <translation>השהה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="881"/>
        <source>Reset</source>
        <translation>אפס</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="902"/>
        <source>Quit</source>
        <translation>צא</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="902"/>
        <source>Disconnect</source>
        <translation>התנתק</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="928"/>
        <source>Edit…</source>
        <translation>ערוך…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="939"/>
        <source>Delete</source>
        <translation>מחק</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="940"/>
        <source>Hide</source>
        <translation>הסתר</translation>
    </message>
</context>
<context>
    <name>Stopwatch</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Stop</source>
        <translation>עצור</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Start</source>
        <translation>התחלה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="210"/>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="288"/>
        <source>Lap</source>
        <translation>הקפה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="226"/>
        <source>Reset</source>
        <translation>אפס</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="279"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="297"/>
        <source>Total</source>
        <translation>סה"כ</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="393"/>
        <source>No laps recorded</source>
        <translation>לא נרשמו הקפות</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="401"/>
        <source>Press Lap while the stopwatch is running</source>
        <translation>לחץ על הקפה בזמן שהשעון פועל</translation>
    </message>
</context>
<context>
    <name>SubMenuCombo</name>
    <message>
        <location filename="../../qml/Widgets/SubMenuCombo.qml" line="86"/>
        <source>No Data Available</source>
        <translation>אין נתונים זמינים</translation>
    </message>
</context>
<context>
    <name>SystemDatasetsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="33"/>
        <source>Dataset Values</source>
        <translation>ערכי מערך נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="158"/>
        <source>Search</source>
        <translation>חיפוש</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="179"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="180"/>
        <source>Group</source>
        <translation>קבוצה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="181"/>
        <source>Dataset</source>
        <translation>מערך נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="182"/>
        <source>Units</source>
        <translation>יחידות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="252"/>
        <source>(virtual)</source>
        <translation>(וירטואלי)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="298"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>העתק קוד גישה %1 ללוח</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="323"/>
        <source>No datasets defined in this project.</source>
        <translation>לא הוגדרו מערכי נתונים בפרויקט זה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="324"/>
        <source>No datasets match your search.</source>
        <translation>אין מערכי נתונים התואמים לחיפוש.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="374"/>
        <source>Dataset access code copied</source>
        <translation>קוד גישה למערך נתונים הועתק</translation>
    </message>
</context>
<context>
    <name>TableDelegate</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="130"/>
        <source>Parameter</source>
        <translation>פרמטר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="151"/>
        <source>Value</source>
        <translation>ערך</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="514"/>
        <source>(Custom Icon)</source>
        <translation>(סמל מותאם אישית)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="639"/>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="645"/>
        <source>Auto</source>
        <translation>אוטומטי</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="811"/>
        <source>No</source>
        <translation>לא</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="811"/>
        <source>Yes</source>
        <translation>כן</translation>
    </message>
</context>
<context>
    <name>Taskbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="66"/>
        <source>Start Menu</source>
        <translation>תפריט התחלה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="194"/>
        <source>Menu</source>
        <translation>תפריט</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="230"/>
        <source>Search…</source>
        <translation>חיפוש…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="496"/>
        <source>Settings</source>
        <translation>הגדרות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="497"/>
        <source>Console</source>
        <translation>קונסול</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="498"/>
        <source>Notifications</source>
        <translation>התראות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="499"/>
        <source>Clock</source>
        <translation>שעון</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="500"/>
        <source>Stopwatch</source>
        <translation>שעון עצר</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="501"/>
        <source>File Transmission</source>
        <translation>שידור קובץ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="502"/>
        <source>AI Assistant</source>
        <translation>עוזר AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Resume</source>
        <translation>המשך</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Pause</source>
        <translation>השהה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="949"/>
        <source>MQTT: Connected to %1</source>
        <translation>MQTT: מחובר אל %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="950"/>
        <source>MQTT: Not connected</source>
        <translation>MQTT: לא מחובר</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="974"/>
        <source>MQTT Publisher</source>
        <translation>מפרסם MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="984"/>
        <source>Status:</source>
        <translation>מצב:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="992"/>
        <source>Connected</source>
        <translation>מחובר</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="993"/>
        <source>Disconnected</source>
        <translation>מנותק</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1000"/>
        <source>Broker:</source>
        <translation>Broker:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1013"/>
        <source>Mode:</source>
        <translation>מצב:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1026"/>
        <source>Messages sent:</source>
        <translation>הודעות שנשלחו:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1040"/>
        <source>Open MQTT Settings</source>
        <translation>פתח הגדרות MQTT</translation>
    </message>
</context>
<context>
    <name>Terminal</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="141"/>
        <source>Copy</source>
        <translation>העתק</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="149"/>
        <source>Select all</source>
        <translation>בחר הכל</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="155"/>
        <source>Clear</source>
        <translation>נקה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="253"/>
        <source>Send Data to Device</source>
        <translation>שלח נתונים למכשיר</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="428"/>
        <source>Show Timestamp</source>
        <translation>הצג חותמת זמן</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="436"/>
        <source>Echo</source>
        <translation>הד</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="453"/>
        <source>Emulate VT-100</source>
        <translation>אמולציה של VT-100</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="466"/>
        <source>ANSI Colors</source>
        <translation>צבעי ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="486"/>
        <source>Display: %1</source>
        <translation>תצוגה: %1</translation>
    </message>
</context>
<context>
    <name>Tips</name>
    <message>
        <source>Did You Know?</source>
        <translation type="vanished">הידעת?</translation>
    </message>
    <message>
        <source>Keep your firmware simple by sending raw data and letting Serial Studio parse it in JavaScript, Lua, or code-free Built-In templates.</source>
        <translation type="vanished">שמור על הקושחה שלך פשוטה על ידי שליחת נתונים גולמיים ומתן אפשרות ל-Serial Studio לנתח אותם ב-JavaScript, Lua או בתבניות מובנות ללא קוד.</translation>
    </message>
    <message>
        <source>Give each channel its own function to calibrate, filter, or convert units. Offload the math to Serial Studio and keep your firmware lean.</source>
        <translation type="vanished">תן לכל ערוץ פונקציה משלו לכיול, סינון או המרת יחידות. העבר את החישובים ל-Serial Studio ושמור על הקושחה רזה.</translation>
    </message>
    <message>
        <source>Need a value your device never sends? A virtual dataset computes its own channel, like power from voltage and current, plotted and logged as data.</source>
        <translation type="vanished">צריך ערך שהמכשיר שלך לעולם לא שולח? מערך נתונים וירטואלי מחשב ערוץ משלו, כמו הספק ממתח וזרם, המוצג ונרשם כנתונים.</translation>
    </message>
    <message>
        <source>Catch glitches like a bench scope. Time-axis plots have a sweep and trigger mode, and you can drag the trigger level right on the plot.</source>
        <translation type="vanished">תפוס תקלות כמו אוסצילוסקופ שולחני. לגרפי ציר זמן יש מצב סריקה וטריגר, וניתן לגרור את רמת הטריגר ישירות על הגרף.</translation>
    </message>
    <message>
        <source>Stop scrolling to find the right widget. Group them into your own workspaces and jump between them from the taskbar search.</source>
        <translation type="vanished">הפסק לגלול כדי למצוא את הווידג'ט הנכון. קבץ אותם למרחבי עבודה משלך וקפוץ ביניהם מחיפוש שורת המשימות.</translation>
    </message>
    <message>
        <source>Never lose a test run again. Record sessions to a local database, then browse, tag, and replay them whenever you need them.</source>
        <translation type="vanished">לעולם אל תאבד ריצת בדיקה שוב. הקלט סשנים למסד נתונים מקומי, ולאחר מכן עיין, תייג והפעל אותם מחדש מתי שתצטרך.</translation>
    </message>
    <message>
        <source>Hand a polished report to your team in seconds. Export any session to HTML or PDF, complete with charts and min/max/mean stats.</source>
        <translation type="vanished">מסור דוח מלוטש לצוות שלך תוך שניות. ייצא כל סשן ל-HTML או PDF, עם תרשימים וסטטיסטיקות מינימום/מקסימום/ממוצע.</translation>
    </message>
    <message>
        <source>Close the loop without extra tooling. Output Controls let you send commands back to your device straight from the dashboard.</source>
        <translation type="vanished">סגור את הלולאה ללא כלים נוספים. בקרי פלט מאפשרים לשלוח פקודות חזרה למכשיר שלך ישירות מהלוח.</translation>
    </message>
    <message>
        <source>Build a visualization nobody else has. The Painter widget runs your own script to draw fully custom graphics from incoming data.</source>
        <translation type="vanished">בנה ויזואליזציה שאף אחד אחר אין לו. ה-Widget Painter מריץ סקריפט משלך לציור גרפיקה מותאמת אישית לחלוטין מנתונים נכנסים.</translation>
    </message>
    <message>
        <source>One tool for every link. Serial Studio reads from UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT, and Process I/O.</source>
        <translation type="vanished">כלי אחד לכל קישור. Serial Studio קורא מ-UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, אודיו, USB, HID, MQTT ו-Process I/O.</translation>
    </message>
    <message>
        <source>Skip the terminal dance. Send and receive files over your serial link with the built-in XMODEM, YMODEM, and ZMODEM protocols.</source>
        <translation type="vanished">דלג על ריקוד הטרמינל. שלח וקבל קבצים דרך קישור סריאלי עם פרוטוקולי XMODEM, YMODEM ו-ZMODEM המובנים.</translation>
    </message>
    <message>
        <source>Already have a Modbus register map or a DBC file? Generate a ready-to-use project from it automatically instead of building one by hand.</source>
        <translation type="vanished">כבר יש לך מפת רגיסטרים של Modbus או קובץ DBC? צור פרויקט מוכן לשימוש ממנו באופן אוטומטי במקום לבנות אחד ידנית.</translation>
    </message>
    <message>
        <source>Describe what you want and let the AI Assistant build it. It can create and edit projects for you across eight model providers.</source>
        <translation type="vanished">תאר מה אתה רוצה ותן לעוזר ה-AI לבנות את זה. הוא יכול ליצור ולערוך פרויקטים עבורך על פני שמונה ספקי מודלים.</translation>
    </message>
    <message>
        <source>Tip %1 of %2</source>
        <translation type="vanished">טיפ %1 מתוך %2</translation>
    </message>
    <message>
        <source>Learn More</source>
        <translation type="vanished">למד עוד</translation>
    </message>
    <message>
        <source>Show Tips on Startup</source>
        <translation type="vanished">הצג טיפים בהפעלה</translation>
    </message>
    <message>
        <source>Previous</source>
        <translation type="vanished">הקודם</translation>
    </message>
    <message>
        <source>Next</source>
        <translation type="vanished">הבא</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">סגור</translation>
    </message>
</context>
<context>
    <name>ToolCallCard</name>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="47"/>
        <source>Awaiting approval</source>
        <translation>ממתין לאישור</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="48"/>
        <source>Done</source>
        <translation>בוצע</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="49"/>
        <source>Error</source>
        <translation>שגיאה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="50"/>
        <source>Denied</source>
        <translation>נדחה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="51"/>
        <source>Blocked</source>
        <translation>חסום</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="52"/>
        <source>Running</source>
        <translation>פועל</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="152"/>
        <source>Approve</source>
        <translation>אשר</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="158"/>
        <source>Deny</source>
        <translation>דחה</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="175"/>
        <source>Arguments</source>
        <translation>ארגומנטים</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="212"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="272"/>
        <source>Copy</source>
        <translation>העתק</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="217"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="277"/>
        <source>Copy All</source>
        <translation>העתק הכול</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="225"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="285"/>
        <source>Select All</source>
        <translation>בחר הכול</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="233"/>
        <source>Result</source>
        <translation>תוצאה</translation>
    </message>
</context>
<context>
    <name>Toolbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="197"/>
        <source>Project Editor</source>
        <translation>עורך פרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="200"/>
        <source>Open the Project Editor to create or modify your JSON layout</source>
        <translation>פתח את עורך הפרויקט כדי ליצור או לשנות את פריסת ה-JSON שלך</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="214"/>
        <source>Open Project</source>
        <translation>פתח פרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="217"/>
        <source>Open an existing JSON project</source>
        <translation>פתח פרויקט JSON קיים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="226"/>
        <source>Open CSV</source>
        <translation>פתח CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="232"/>
        <source>Play a CSV file as if it were live sensor data</source>
        <translation>הפעל קובץ CSV כאילו היה נתוני חיישן חיים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="238"/>
        <source>Open MDF4</source>
        <translation>פתח MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="243"/>
        <source>Play an MDF4 file as if it were live sensor data (Pro)</source>
        <translation>הפעל קובץ MDF4 כאילו היה נתוני חיישן חיים (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <source>Assistant</source>
        <translation>עוזר</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="302"/>
        <source>Extensions</source>
        <translation>הרחבות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="264"/>
        <source>Chat with an AI to build and edit your project</source>
        <translation>שוחח עם AI כדי לבנות ולערוך את הפרויקט שלך</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="265"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="305"/>
        <source>Browse and install extensions</source>
        <translation>עיין והתקן הרחבות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="281"/>
        <source>Deploy</source>
        <translation>פרוס</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="286"/>
        <source>Build an operator app for the current project</source>
        <translation>בנה אפליקציית מפעיל עבור הפרויקט הנוכחי</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="292"/>
        <source>Sessions</source>
        <translation>סשנים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="296"/>
        <source>Browse, replay, and export recorded sessions</source>
        <translation>עיין, הפעל מחדש וייצא סשנים מוקלטים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="319"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="323"/>
        <source>Preferences</source>
        <translation>העדפות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="327"/>
        <source>Open application settings and preferences</source>
        <translation>פתח הגדרות והעדפות אפליקציה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="345"/>
        <source>UART</source>
        <translation>UART</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="351"/>
        <source>Select Serial port (UART) communication</source>
        <translation>בחר תקשורת יציאה טורית (UART)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="362"/>
        <source>Audio</source>
        <translation>אודיו</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="366"/>
        <source>Select audio input device (Pro)</source>
        <translation>בחר התקן קלט שמע (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="381"/>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="386"/>
        <source>Select raw USB communication (Pro)</source>
        <translation>בחר תקשורת USB גולמית (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="395"/>
        <source>Network</source>
        <translation>רשת</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="400"/>
        <source>Select TCP/UDP network communication</source>
        <translation>בחר תקשורת רשת TCP/UDP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="412"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="417"/>
        <source>Select MODBUS communication (Pro)</source>
        <translation>בחר תקשורת MODBUS (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="431"/>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="436"/>
        <source>Select HID device communication (Pro)</source>
        <translation>בחר תקשורת התקן HID (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="446"/>
        <source>Bluetooth</source>
        <translation>Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="450"/>
        <source>Select Bluetooth Low Energy communication</source>
        <translation>בחר תקשורת Bluetooth Low Energy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="462"/>
        <source>CAN Bus</source>
        <translation>אפיק CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="467"/>
        <source>Select CAN Bus communication (Pro)</source>
        <translation>בחר תקשורת CAN Bus (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="481"/>
        <source>Process</source>
        <translation>תהליך</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="486"/>
        <source>Select process pipe communication (Pro)</source>
        <translation>בחר תקשורת צינור תהליך (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="502"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="506"/>
        <source>About</source>
        <translation>אודות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="510"/>
        <source>Show application info and license details</source>
        <translation>הצג מידע על האפליקציה ופרטי רישיון</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="522"/>
        <source>Examples</source>
        <translation>דוגמאות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="525"/>
        <source>Browse example projects</source>
        <translation>עיין בפרויקטים לדוגמה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="533"/>
        <source>Help Center</source>
        <translation>מרכז עזרה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="537"/>
        <source>Browse documentation, FAQ, and wiki</source>
        <translation>עיין בתיעוד, שאלות נפוצות ו-Wiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="543"/>
        <source>AI Wiki &amp; Chat</source>
        <translation>Wiki וצ'אט AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="546"/>
        <source>View detailed documentation and ask questions on DeepWiki</source>
        <translation>הצג תיעוד מפורט ושאל שאלות ב-DeepWiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="586"/>
        <source>Activate</source>
        <translation>הפעל</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="590"/>
        <source>Manage license and activate Serial Studio Pro</source>
        <translation>נהל רישיון והפעל את Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="627"/>
        <source>Disconnect</source>
        <translation>התנתק</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <source>Connect</source>
        <translation>התחבר</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="607"/>
        <source>Connect or disconnect from the configured device</source>
        <translation>התחבר או התנתק מההתקן המוגדר</translation>
    </message>
</context>
<context>
    <name>TriggerDialog</name>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="50"/>
        <source>Trigger Settings</source>
        <translation>הגדרות טריגר</translation>
    </message>
    <message>
        <source>Hold the waveform stationary by aligning each sweep to a trigger event.</source>
        <translation type="vanished">החזקת צורת הגל במצב נייח על ידי יישור כל סריקה לאירוע טריגר.</translation>
    </message>
    <message>
        <source>Lock a repeating signal in place, like the Auto button on an oscilloscope. Each sweep starts at the same point on the waveform, so it holds still instead of scrolling past.</source>
        <translation type="vanished">נעילת אות חוזר במקום, כמו כפתור Auto באוסילוסקופ. כל סריקה מתחילה באותה נקודה בצורת הגל, כך שהיא נשארת במקום במקום לגלול הלאה.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="173"/>
        <source>Trigger</source>
        <translation>טריגר</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="vanished">מצב:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="110"/>
        <source>Mode</source>
        <translation>מצב</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Auto</source>
        <translation>אוטומטי</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Normal</source>
        <translation>רגיל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Single</source>
        <translation>בודד</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="158"/>
        <source>Auto free-runs when nothing crosses the level.</source>
        <translation>Auto רץ חופשי כאשר שום דבר לא חוצה את הרמה.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="159"/>
        <source>Normal waits for a crossing.</source>
        <translation>Normal ממתין לחציה.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="160"/>
        <source>Single captures one sweep, then stops.</source>
        <translation>Single לוכד סריקה אחת ואז עוצר.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="238"/>
        <source>Slope:</source>
        <translation>שיפוע:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="270"/>
        <source>Trigger on a downward crossing</source>
        <translation>טריגר על חציה יורדת</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="316"/>
        <source>Timebase:</source>
        <translation>בסיס זמן:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="381"/>
        <source>Leave timebase empty to use the plot's time range; lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each.</source>
        <translation>השאר בסיס זמן ריק כדי להשתמש בטווח הזמן של הגרף; הקטן אותו כדי להתקרב לאות מהיר. Holdoff מתעלם מטריגרים חדשים לרגע אחרי כל אחד.</translation>
    </message>
    <message>
        <source>Signal:</source>
        <translation type="vanished">אות:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="230"/>
        <source>Value to cross</source>
        <translation>ערך לחציה</translation>
    </message>
    <message>
        <source>Edge:</source>
        <translation type="vanished">קצה:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="251"/>
        <source>Rising</source>
        <translation>עולה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="255"/>
        <source>Trigger on an upward crossing</source>
        <translation>טריגר על חציה עולה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="266"/>
        <source>Falling</source>
        <translation>יורד</translation>
    </message>
    <message>
        <source>A new sweep begins each time the signal crosses the level in the chosen direction. Auto also free-runs when no crossing is found.</source>
        <translation type="vanished">סריקה חדשה מתחילה בכל פעם שהאות חוצה את הרמה בכיוון הנבחר. Auto גם רץ חופשי כאשר לא נמצאת חציה.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="289"/>
        <source>Timing</source>
        <translation>תזמון</translation>
    </message>
    <message>
        <source>Timebase (ms):</source>
        <translation type="vanished">בסיס זמן (ms):</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="329"/>
        <source>Match time range</source>
        <translation>התאמה לטווח זמן</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="341"/>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="368"/>
        <source>ms</source>
        <translation>ms</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="347"/>
        <source>Holdoff:</source>
        <translation>Holdoff:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="360"/>
        <source>0</source>
        <translation>0</translation>
    </message>
    <message>
        <source>Timebase sets how much time one sweep shows; leave it empty to use the plot's time range. Lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each one.</source>
        <translation type="vanished">בסיס זמן קובע כמה זמן מציגה סריקה אחת; השאר ריק כדי להשתמש בטווח הזמן של הגרף. הקטן אותו כדי להתקרב לאות מהיר. Holdoff מתעלם מטריגרים חדשים לרגע אחרי כל אחד.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="396"/>
        <source>Capture Next</source>
        <translation>לכידת הבא</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="398"/>
        <source>Arm for one more single-shot capture</source>
        <translation>חימוש ללכידה חד-פעמית נוספת</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="217"/>
        <source>Level:</source>
        <translation>רמה:</translation>
    </message>
    <message>
        <source>Trigger level</source>
        <translation type="vanished">רמת טריגר</translation>
    </message>
    <message>
        <source>Holdoff (ms):</source>
        <translation type="vanished">זמן החזקה (ms):</translation>
    </message>
    <message>
        <source>Holdoff time</source>
        <translation type="vanished">זמן החזקה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="197"/>
        <source>Source:</source>
        <translation>מקור:</translation>
    </message>
    <message>
        <source>Re-arm</source>
        <translation type="vanished">חימוש מחדש</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="411"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>UART</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="52"/>
        <source>COM Port</source>
        <translation>פורט COM</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="81"/>
        <source>Baud Rate</source>
        <translation>קצב באוד</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="174"/>
        <source>Data Bits</source>
        <translation>סיביות נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="195"/>
        <source>Parity</source>
        <translation>זוגיות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="216"/>
        <source>Stop Bits</source>
        <translation>סיביות עצירה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="237"/>
        <source>Flow Control</source>
        <translation>בקרת זרימה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="269"/>
        <source>Auto Reconnect</source>
        <translation>התחברות אוטומטית מחדש</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="287"/>
        <source>Send DTR Signal</source>
        <translation>שלח אות DTR</translation>
    </message>
</context>
<context>
    <name>UI::Dashboard</name>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1192"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1898"/>
        <source>Console</source>
        <translation>קונסול</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1274"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1909"/>
        <source>Notifications</source>
        <translation>התראות</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1355"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1920"/>
        <source>Clock</source>
        <translation>שעון</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1435"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1930"/>
        <source>Stopwatch</source>
        <translation>שעון עצר</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1978"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1993"/>
        <source>%1 (Fallback)</source>
        <translation>%1 (גיבוי)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="2015"/>
        <source>LED Panel (%1)</source>
        <translation>לוח LED (%1)</translation>
    </message>
</context>
<context>
    <name>UI::DashboardWidget</name>
    <message>
        <location filename="../../src/UI/DashboardWidget.cpp" line="148"/>
        <source>Invalid</source>
        <translation>לא תקין</translation>
    </message>
</context>
<context>
    <name>UI::WindowManager</name>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1057"/>
        <source>Select Background Image</source>
        <translation>בחר תמונת רקע</translation>
    </message>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1059"/>
        <source>Images (*.png *.jpg *.jpeg *.bmp)</source>
        <translation>תמונות (*.png *.jpg *.jpeg *.bmp)</translation>
    </message>
</context>
<context>
    <name>USB</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="50"/>
        <source>USB Device</source>
        <translation>התקן USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="80"/>
        <source>Transfer Mode</source>
        <translation>מצב העברה</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="89"/>
        <source>Bulk Stream</source>
        <translation>זרם Bulk</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="90"/>
        <source>Advanced (Bulk + Control)</source>
        <translation>מתקדם (Bulk + Control)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="91"/>
        <source>Isochronous</source>
        <translation>Isochronous</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="143"/>
        <source>Connect to USB devices using bulk, control, or isochronous transfers. Suitable for data loggers, custom firmware devices, and USB instruments.</source>
        <translation>התחברות להתקני USB באמצעות העברות bulk, control או isochronous. מתאים לרושמי נתונים, התקני קושחה מותאמים אישית ומכשירי USB.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="152"/>
        <source>USB specifications (USB.org)</source>
        <translation>מפרטי USB (USB.org)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="169"/>
        <source>IN Endpoint</source>
        <translation>נקודת קצה IN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="205"/>
        <source>OUT Endpoint</source>
        <translation>נקודת קצה OUT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="241"/>
        <source>Max Packet Size</source>
        <translation>גודל מנה מקסימלי</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="301"/>
        <source>Control Transfers Enabled</source>
        <translation>העברות בקרה מופעלות</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="310"/>
        <source>Sending incorrect control requests may crash or damage connected hardware. Use with caution.</source>
        <translation>שליחת בקשות בקרה שגויות עלולה לגרום לקריסה או נזק לחומרה המחוברת. יש להשתמש בזהירות.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="317"/>
        <source>Learn about USB control transfers</source>
        <translation>מידע על העברות בקרה של USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="351"/>
        <source>Packet size should match the maximum transfer size reported by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).</source>
        <translation>גודל המנה צריך להתאים לגודל ההעברה המקסימלי המדווח על ידי נקודת הקצה. ערכים טיפוסיים: 192 B (אודיו FS), 1024 B (HS).</translation>
    </message>
</context>
<context>
    <name>Updater</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="477"/>
        <source>Would you like to download the update now?</source>
        <translation>להוריד את העדכון כעת?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="479"/>
        <source>Would you like to download the update now?&lt;br /&gt;This is a mandatory update, exiting now will close the application.</source>
        <translation>להוריד את העדכון כעת?&lt;br /&gt;זהו עדכון חובה, יציאה כעת תסגור את היישום.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="485"/>
        <source>&lt;strong&gt;Change log:&lt;/strong&gt;&lt;br/&gt;%1</source>
        <translation>&lt;strong&gt;יומן שינויים:&lt;/strong&gt;&lt;br/&gt;%1</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="488"/>
        <source>Version %1 of %2 has been released!</source>
        <translation>גרסה %1 של %2 שוחררה!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="520"/>
        <source>No updates are available for the moment</source>
        <translation>אין עדכונים זמינים כרגע</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="522"/>
        <source>Congratulations! You are running the latest version of %1</source>
        <translation>ברכות! אתה מריץ את הגרסה העדכנית ביותר של %1</translation>
    </message>
</context>
<context>
    <name>UserTableView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="166"/>
        <source>Add Register</source>
        <translation>הוספת רגיסטר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="169"/>
        <source>Add register</source>
        <translation>הוספת רגיסטר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="176"/>
        <source>Insert Constant</source>
        <translation>הוספת קבוע</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="179"/>
        <source>Insert constant</source>
        <translation>הוספת קבוע</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="186"/>
        <source>Import</source>
        <translation>ייבוא</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="189"/>
        <source>Import registers from CSV</source>
        <translation>ייבוא רגיסטרים מ-CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="196"/>
        <source>Export</source>
        <translation>ייצוא</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="199"/>
        <source>Export registers to CSV</source>
        <translation>ייצוא רגיסטרים ל-CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="211"/>
        <source>Rename</source>
        <translation>שנה שם</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="214"/>
        <source>Rename table</source>
        <translation>שינוי שם טבלה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="221"/>
        <source>Delete</source>
        <translation>מחק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="224"/>
        <source>Delete table</source>
        <translation>מחק טבלה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="238"/>
        <source>Help</source>
        <translation>עזרה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="243"/>
        <source>Open help documentation for shared memory</source>
        <translation>פתח תיעוד עזרה עבור זיכרון משותף</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="283"/>
        <source>Permissions</source>
        <translation>הרשאות</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="284"/>
        <source>Register Name</source>
        <translation>שם רגיסטר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="285"/>
        <source>Default Value</source>
        <translation>ערך ברירת מחדל</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="324"/>
        <source>Read-Only</source>
        <translation>קריאה בלבד</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="324"/>
        <source>Read/Write</source>
        <translation>קריאה/כתיבה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="462"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>העתק קוד גישה %1 ללוח</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="495"/>
        <source>Delete register</source>
        <translation>מחק רגיסטר</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="512"/>
        <source>No registers.</source>
        <translation>אין רגיסטרים.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="562"/>
        <source>Register access code copied</source>
        <translation>קוד גישה לרגיסטר הועתק</translation>
    </message>
</context>
<context>
    <name>Waterfall</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="232"/>
        <source>Show Colorbar</source>
        <translation>הצג סרגל צבעים</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="245"/>
        <source>Show Axes &amp; Grid</source>
        <translation>הצג צירים ורשת</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="258"/>
        <source>Show Crosshair</source>
        <translation>הצג כוונת</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="272"/>
        <source>Pause</source>
        <translation>השהה</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="272"/>
        <source>Resume</source>
        <translation>המשך</translation>
    </message>
</context>
<context>
    <name>Welcome</name>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="177"/>
        <source>Welcome to %1!</source>
        <translation>ברוכים הבאים ל-%1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="188"/>
        <source>Serial Studio is a powerful real-time visualization tool, built for engineers, students, and makers.</source>
        <translation>Serial Studio הוא כלי הדמיה רב עוצמה בזמן אמת, שנבנה עבור מהנדסים, סטודנטים ויוצרים.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="199"/>
        <source>You can start a fully-functional 14-day trial, activate it with your license key, or download and compile the GPLv3 source code yourself.</source>
        <translation>ניתן להתחיל תקופת ניסיון מלאה בת 14 ימים, להפעיל עם מפתח רישיון, או להוריד ולקמפל את קוד המקור GPLv3 בעצמך.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="209"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="394"/>
        <source>Buying Pro supports the author directly and helps fund future development.</source>
        <translation>רכישת Pro תומכת ישירות במפתח ומסייעת במימון פיתוח עתידי.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="217"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="402"/>
        <source>Building the GPLv3 version yourself helps grow the community and encourages technical contributions.</source>
        <translation>קימפול גרסת GPLv3 בעצמך מסייע בצמיחת הקהילה ומעודד תרומות טכניות.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="239"/>
        <source>Please wait…</source>
        <translation>נא להמתין…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="275"/>
        <source>%1 days remaining in your trial.</source>
        <translation>%1 ימים נותרו בתקופת הניסיון שלך.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="285"/>
        <source>You're currently using the fully-featured trial of %1 Pro. It's valid for 14 days of personal, non-commercial use.</source>
        <translation>אתה משתמש כעת בגרסת הניסיון המלאה של %1 Pro. היא תקפה ל-14 ימים לשימוש אישי ולא מסחרי.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="296"/>
        <source>Upgrade to a paid plan to keep using Serial Studio Pro.</source>
        <translation>שדרג לתוכנית בתשלום כדי להמשיך להשתמש ב-Serial Studio Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="304"/>
        <source>Or, compile the GPLv3 source code to use it for free.</source>
        <translation>לחלופין, קמפל את קוד המקור GPLv3 כדי להשתמש בו בחינם.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="312"/>
        <source>To see available subscription plans, click "Upgrade Now" below.</source>
        <translation>כדי לראות את תוכניות המנוי הזמינות, לחץ על "שדרג עכשיו" למטה.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="333"/>
        <source>Don't nag me about the trial.
I understand that when it ends, I'll need to buy a license or build the GPLv3 version.</source>
        <translation>אל תטרידו אותי לגבי תקופת הניסיון.
אני מבין שכאשר היא תסתיים, אצטרך לרכוש רישיון או לבנות את גרסת GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="364"/>
        <source>Your %1 trial has expired.</source>
        <translation>תקופת הניסיון של %1 הסתיימה.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="374"/>
        <source>Your trial period has ended. To continue using %1 with all Pro features, please upgrade to a paid plan.</source>
        <translation>תקופת הניסיון הסתיימה. כדי להמשיך להשתמש ב-%1 עם כל תכונות Pro, יש לשדרג לתוכנית בתשלום.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="385"/>
        <source>If you prefer, you can also compile the open-source version under the GPLv3 license.</source>
        <translation>לחלופין, ניתן גם לקמפל את הגרסה בקוד פתוח תחת רישיון GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="413"/>
        <source>Thank you for trying %1!</source>
        <translation>תודה שניסית את %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="455"/>
        <source>Upgrade Now</source>
        <translation>שדרג עכשיו</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="464"/>
        <source>Activate</source>
        <translation>הפעל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Open in Limited Mode</source>
        <translation>פתח במצב מוגבל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Continue</source>
        <translation>המשך</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Start Trial</source>
        <translation>התחל ניסיון</translation>
    </message>
</context>
<context>
    <name>WhatsNew</name>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="31"/>
        <source>What's New in %1</source>
        <translation>מה חדש ב-%1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="58"/>
        <source>Lua &amp; Built-In Parsers</source>
        <translation>Lua ומנתחים מובנים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="59"/>
        <source>Parse frames in Lua 5.4 or with code-free Built-In templates, alongside JavaScript.</source>
        <translation>ניתוח מסגרות ב-Lua 5.4 או באמצעות תבניות מובנות ללא קוד, לצד JavaScript.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="67"/>
        <source>AI Assistant</source>
        <translation>עוזר AI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="68"/>
        <source>An in-app assistant across eight providers that can build and edit projects for you.</source>
        <translation>עוזר מובנה באפליקציה על פני שמונה ספקים שיכול לבנות ולערוך פרויקטים עבורך.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="76"/>
        <source>Oscilloscope Sweep &amp; Trigger</source>
        <translation>סריקה וטריגר אוסילוסקופ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="77"/>
        <source>Scope-style sweep with an animated trigger cursor you can drag on the plot.</source>
        <translation>סריקה בסגנון אוסילוסקופ עם סמן טריגר מונפש שניתן לגרור על הגרף.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="85"/>
        <source>Output Controls</source>
        <translation>בקרות פלט</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="86"/>
        <source>Transmit back to your device with control widgets and a protocol-helper engine.</source>
        <translation>שדר חזרה למכשיר שלך עם ווידג'טים לבקרה ומנוע עזר פרוטוקול.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="94"/>
        <source>Dashboard Workspaces</source>
        <translation>מרחבי עבודה ללוח בקרה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="95"/>
        <source>Group widgets into your own dashboards and find them from the taskbar search.</source>
        <translation>קבץ ווידג'טים ללוחות בקרה משלך ומצא אותם מחיפוש שורת המשימות.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="103"/>
        <source>Session Database &amp; Reports</source>
        <translation>מסד נתוני סשן ודוחות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="104"/>
        <source>Record sessions to SQLite, replay them, and export HTML or PDF reports.</source>
        <translation>הקלט סשנים ל־SQLITE, הפעל אותם מחדש, וייצא דוחות HTML או PDF.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="112"/>
        <source>Operator Deployments</source>
        <translation>פריסות למפעיל</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="113"/>
        <source>Ship a locked-down, single-purpose build to operators with a runtime-only mode.</source>
        <translation>שלח גרסה נעולה ייעודית למפעילים עם מצב זמן ריצה בלבד.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="121"/>
        <source>New Dashboard Widgets</source>
        <translation>ווידג'טים חדשים ללוח בקרה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="122"/>
        <source>Gauge and Meter faces with live readouts, plus Clock, Stopwatch, and Waterfall.</source>
        <translation>פני מד ומחוון עם קריאות חיות, בתוספת שעון, שעון עצר ומפל.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="130"/>
        <source>Dataset Transforms</source>
        <translation>טרנספורמציות מערך נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="131"/>
        <source>Calibrate, filter, and convert each channel with a per-dataset function, or add virtual datasets that compute new channels.</source>
        <translation>כייל, סנן והמר כל ערוץ עם פונקציה לכל Dataset, או הוסף Datasets וירטואליים שמחשבים ערוצים חדשים.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="139"/>
        <source>Painter Widget</source>
        <translation>וידג'ט ציור</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="140"/>
        <source>Draw fully custom graphics from incoming data with your own drawing script.</source>
        <translation>צייר גרפיקה מותאמת אישית מנתונים נכנסים עם סקריפט ציור משלך.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="148"/>
        <source>Data Tables</source>
        <translation>טבלאות נתונים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="149"/>
        <source>Live register-style tables with virtual datasets and editable cells.</source>
        <translation>טבלאות חיות בסגנון רגיסטרים עם Datasets וירטואליים ותאים הניתנים לעריכה.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="157"/>
        <source>Image Widget</source>
        <translation>וידג'ט תמונה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="158"/>
        <source>Decode and display image frames streamed straight from your device.</source>
        <translation>פענח והצג מסגרות תמונה המוזרמות ישירות מהמכשיר שלך.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="166"/>
        <source>Notifications &amp; Alarms</source>
        <translation>התראות ואזעקות</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="167"/>
        <source>Multi-band dataset alarms with severity tiers, routed to the Notification Center.</source>
        <translation>אזעקות Dataset רב-רצועתיות עם דרגות חומרה, המנותבות למרכז ההתראות.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="175"/>
        <source>Project Lock</source>
        <translation>נעל פרויקט</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="176"/>
        <source>Lock a project to separate operator use from editing, with an access code.</source>
        <translation>נעילת פרויקט להפרדת שימוש מפעיל מעריכה, עם קוד גישה.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="184"/>
        <source>MQTT, Protobuf &amp; File Transfer</source>
        <translation>MQTT, Protobuf והעברת קבצים</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="185"/>
        <source>MQTT input and publishing, Protobuf import, and XMODEM / YMODEM / ZMODEM transfers.</source>
        <translation>קלט ופרסום MQTT, ייבוא Protobuf והעברות XMODEM / YMODEM / ZMODEM.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="231"/>
        <source>Welcome to %1!</source>
        <translation>ברוכים הבאים ל-%1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="241"/>
        <source>Here's what's new in version %1.</source>
        <translation>הנה מה חדש בגרסה %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="414"/>
        <source>Show on Startup</source>
        <translation>הצג בהפעלה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="421"/>
        <source>Close</source>
        <translation>סגור</translation>
    </message>
</context>
<context>
    <name>WidgetDelegate</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml" line="335"/>
        <source>Device Disconnected</source>
        <translation>התקן מנותק</translation>
    </message>
</context>
<context>
    <name>Widgets::Bar</name>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="331"/>
        <source>Alarm</source>
        <translation>אזעקה</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="332"/>
        <source>critical</source>
        <translation>קריטי</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="332"/>
        <source>warning</source>
        <translation>אזהרה</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="336"/>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation>הערך %1%2 נכנס לתחום %3 (%4–%5).</translation>
    </message>
    <message>
        <source>Value %1%2 reached the high alarm %3%2</source>
        <translation type="vanished">הערך %1%2 הגיע לאזעקת הסף העליון %3%2</translation>
    </message>
    <message>
        <source>Value %1%2 reached the low alarm %3%2</source>
        <translation type="vanished">הערך %1%2 הגיע לאזעקת הסף התחתון %3%2</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="342"/>
        <source>Alarms</source>
        <translation>אזעקות</translation>
    </message>
</context>
<context>
    <name>Widgets::Compass</name>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="157"/>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="180"/>
        <source>N</source>
        <translation>צ</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="160"/>
        <source>NE</source>
        <translation>צ-מ</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="163"/>
        <source>E</source>
        <translation>מ</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="166"/>
        <source>SE</source>
        <translation>ד-מ</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="169"/>
        <source>S</source>
        <translation>ד</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="172"/>
        <source>SW</source>
        <translation>ד-מע</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="175"/>
        <source>W</source>
        <translation>מע</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="178"/>
        <source>NW</source>
        <translation>צ-מע</translation>
    </message>
</context>
<context>
    <name>Widgets::DataGrid</name>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="128"/>
        <source>Title</source>
        <translation>כותרת</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="129"/>
        <source>Value</source>
        <translation>ערך</translation>
    </message>
    <message>
        <source>Awaiting data…</source>
        <translation type="vanished">ממתין לנתונים…</translation>
    </message>
</context>
<context>
    <name>Widgets::GPS</name>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery</source>
        <translation>תמונות לוויין</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery with Labels</source>
        <translation>תמונות לוויין עם תוויות</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Street Map</source>
        <translation>מפת רחובות</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Topographic Map</source>
        <translation>מפה טופוגרפית</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Terrain</source>
        <translation>פני שטח</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Light Gray Canvas</source>
        <translation>קנבס אפור בהיר</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>Dark Gray Canvas</source>
        <translation>קנבס אפור כהה</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>National Geographic</source>
        <translation>National Geographic</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="373"/>
        <source>Additional map layers are available only for Pro users.</source>
        <translation>שכבות מפה נוספות זמינות למשתמשי Pro בלבד.</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="374"/>
        <source>We can't offer unrestricted access because the ArcGIS API key incurs real costs.</source>
        <translation>לא ניתן להציע גישה בלתי מוגבלת מכיוון שמפתח API של ArcGIS כרוך בעלויות ממשיות.</translation>
    </message>
</context>
<context>
    <name>Widgets::MultiPlot</name>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="97"/>
        <source>Time (s)</source>
        <translation>זמן (שניות)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="97"/>
        <source>Samples</source>
        <translation>דגימות</translation>
    </message>
</context>
<context>
    <name>Widgets::Output::Base</name>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="167"/>
        <source>Transmit script timed out after %1 ms</source>
        <translation>תסריט שידור פג זמן לאחר %1 ms</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="183"/>
        <source>Payload exceeds maximum size</source>
        <translation>מטען חורג מהגודל המרבי</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="86"/>
        <source>Time (s)</source>
        <translation>זמן (שניות)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="112"/>
        <source>Samples</source>
        <translation>דגימות</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot3D</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot3D.cpp" line="959"/>
        <source>Grid Interval: %1 unit(s)</source>
        <translation>מרווח רשת: %1 יחידות</translation>
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
        <translation>טורבו</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="405"/>
        <source>Jet</source>
        <translation>ג'ט</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="407"/>
        <source>Hot</source>
        <translation>חם</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="409"/>
        <source>Grayscale</source>
        <translation>גווני אפור</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="411"/>
        <source>Unknown</source>
        <translation>לא ידוע</translation>
    </message>
</context>
<context>
    <name>WorkspaceDialog</name>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="51"/>
        <source>Edit Workspace</source>
        <translation>עריכת סביבת עבודה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="52"/>
        <source>New Workspace</source>
        <translation>סביבת עבודה חדשה</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="157"/>
        <source>Name:</source>
        <translation>שם:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="166"/>
        <source>My Workspace</source>
        <translation>סביבת העבודה שלי</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="181"/>
        <source>Select widgets to include:</source>
        <translation>בחירת ווידג'טים לכלול:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="187"/>
        <source>Filter widgets…</source>
        <translation>סנן Widgets…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="302"/>
        <source>Cancel</source>
        <translation>ביטול</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="309"/>
        <source>OK</source>
        <translation>אישור</translation>
    </message>
</context>
<context>
    <name>WorkspaceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="41"/>
        <source>Workspace</source>
        <translation>סביבת עבודה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="129"/>
        <source>Add Widget</source>
        <translation>הוסף Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="131"/>
        <source>Add widget to workspace</source>
        <translation>הוסף widget לסביבת העבודה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="142"/>
        <source>Rename</source>
        <translation>שנה שם</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="144"/>
        <source>Rename workspace</source>
        <translation>שנה שם סביבת עבודה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="153"/>
        <source>Delete</source>
        <translation>מחק</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="155"/>
        <source>Delete workspace</source>
        <translation>מחק סביבת עבודה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="177"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="183"/>
        <source>Group</source>
        <translation>קבוצה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="178"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="184"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="179"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="185"/>
        <source>Type</source>
        <translation>סוג</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="229"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="267"/>
        <source>(unknown)</source>
        <translation>(לא ידוע)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="247"/>
        <source>(group widget)</source>
        <translation>(widget קבוצה)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="297"/>
        <source>Remove widget from workspace</source>
        <translation>הסר widget ממרחב העבודה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="317"/>
        <source>No widgets in this workspace.</source>
        <translation>אין widgets במרחב עבודה זה.</translation>
    </message>
</context>
<context>
    <name>WorkspacesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="33"/>
        <source>Workspaces</source>
        <translation>מרחבי עבודה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="127"/>
        <source>Customize</source>
        <translation>התאמה אישית</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="129"/>
        <source>Edit workspaces manually</source>
        <translation>ערוך מרחבי עבודה באופן ידני</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="145"/>
        <source>Move Up</source>
        <translation>העבר למעלה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="147"/>
        <source>Move the selected workspace up</source>
        <translation>העבר את מרחב העבודה הנבחר למעלה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="158"/>
        <source>Move Down</source>
        <translation>העבר למטה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="160"/>
        <source>Move the selected workspace down</source>
        <translation>העבר את מרחב העבודה הנבחר למטה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="171"/>
        <source>Add Workspace</source>
        <translation>הוסף מרחב עבודה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="173"/>
        <source>Add workspace</source>
        <translation>הוסף מרחב עבודה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="182"/>
        <source>Cleanup</source>
        <translation>נקה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="185"/>
        <source>Remove %1 widget reference(s) whose target group or dataset no longer exists</source>
        <translation>הסר %1 הפניות לווידג'טים שקבוצת היעד או מערך הנתונים שלהם אינם קיימים עוד</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="188"/>
        <source>No stale widget references in any workspace</source>
        <translation>אין הפניות מיושנות לווידג'טים באף מרחב עבודה</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="203"/>
        <source>Title</source>
        <translation>כותרת</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="204"/>
        <source>Widgets</source>
        <translation>ווידג'טים</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="276"/>
        <source>No workspaces. Add one with the toolbar above, or reset to the auto layout.</source>
        <translation>אין סביבות עבודה. הוסף אחת באמצעות סרגל הכלים למעלה, או אפס לפריסה אוטומטית.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="278"/>
        <source>Project has no eligible groups -- add a group with widgets to populate workspaces.</source>
        <translation>הפרויקט אינו מכיל קבוצות מתאימות -- הוסף קבוצה עם ווידג'טים כדי למלא סביבות עבודה.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="284"/>
        <source>Reset to Auto Layout</source>
        <translation>אפס לפריסה אוטומטית</translation>
    </message>
</context>
</TS>