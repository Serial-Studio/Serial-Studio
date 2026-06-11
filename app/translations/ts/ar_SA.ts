<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="ar_SA" sourcelanguage="en_US">
<context>
    <name>AI::AnthropicReply</name>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="164"/>
        <source>Anthropic error</source>
        <translation>خطأ Anthropic</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="310"/>
        <source>Stream parse error: %1</source>
        <translation>خطأ في تحليل التدفق: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="359"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="362"/>
        <source>Invalid API key (%1)</source>
        <translation>مفتاح API غير صالح (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="364"/>
        <source>Rate limited: %1</source>
        <translation>تم تجاوز الحد المسموح: %1</translation>
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
        <translation>السماح بالتحكم بالجهاز عبر AI؟</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="192"/>
        <source>This lets the AI assistant configure devices, open and close connections, and send data to your hardware.

Every device action still requires your explicit per-call approval in the chat, even when auto-approve is enabled. Only enable this if you trust the configured AI provider with hardware access.</source>
        <translation>يتيح هذا لمساعد AI تكوين الأجهزة وفتح وإغلاق الاتصالات وإرسال البيانات إلى الأجهزة.

كل إجراء للجهاز لا يزال يتطلب موافقتك الصريحة لكل استدعاء في المحادثة، حتى عند تفعيل الموافقة التلقائية. فعّل هذا فقط إذا كنت تثق بمزود AI المكوّن للوصول إلى الأجهزة.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="390"/>
        <source>Switch AI provider?</source>
        <translation>تبديل موفر الذكاء الاصطناعي؟</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="391"/>
        <source>Switching to a different provider clears the current conversation. Do you want to continue?</source>
        <translation>سيؤدي التبديل إلى موفر مختلف إلى مسح المحادثة الحالية. هل تريد المتابعة؟</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="394"/>
        <source>Assistant</source>
        <translation>المساعد</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="431"/>
        <source>AI Assistant is not available in this build</source>
        <translation>مساعد الذكاء الاصطناعي غير متوفر في هذا الإصدار</translation>
    </message>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">يتطلب مساعد الذكاء الاصطناعي ترخيص Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="436"/>
        <source>Set an API key first</source>
        <translation>قم بتعيين مفتاح API أولاً</translation>
    </message>
</context>
<context>
    <name>AI::Conversation</name>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">يتطلب مساعد AI ترخيص Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="161"/>
        <source>AI Assistant is not available in this build</source>
        <translation>مساعد الذكاء الاصطناعي غير متوفر في هذا الإصدار</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="167"/>
        <source>AI subsystem not initialized</source>
        <translation>نظام AI الفرعي غير مهيأ</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="173"/>
        <source>Already busy with a previous request</source>
        <translation>مشغول بالفعل بطلب سابق</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="489"/>
        <source>Tool-call budget reached for this turn; no further tools will run.</source>
        <translation>تم الوصول إلى حد استدعاءات الأدوات لهذه الدورة؛ لن يتم تشغيل المزيد من الأدوات.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1937"/>
        <source>You have reached the tool-call budget for this turn. Do not request more tools. Summarize what you found so far, and if the task is incomplete, say which steps remain so the user can tell you to continue.</source>
        <translation>لقد وصلت إلى حد استدعاءات الأدوات لهذه الدورة. لا تطلب المزيد من الأدوات. لخص ما وجدته حتى الآن، وإذا كانت المهمة غير مكتملة، اذكر الخطوات المتبقية حتى يتمكن المستخدم من إخبارك بالمتابعة.</translation>
    </message>
    <message>
        <source>Tool-call budget exceeded</source>
        <translation type="vanished">تم تجاوز حد استدعاءات الأدوات</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="932"/>
        <source>(The model returned an empty response. Try rephrasing, switching to a different model, or checking that the request is allowed by the provider's safety filters.)</source>
        <translation>(أعاد النموذج استجابة فارغة. حاول إعادة الصياغة، أو التبديل إلى نموذج مختلف، أو التحقق من أن الطلب مسموح به بواسطة مرشحات الأمان الخاصة بالمزود.)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1084"/>
        <source>Sending request to %1...</source>
        <translation>إرسال الطلب إلى %1...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1095"/>
        <source>Provider returned no reply</source>
        <translation>لم يُرجع المزود أي رد</translation>
    </message>
</context>
<context>
    <name>AI::GeminiReply</name>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="145"/>
        <source>Prompt blocked by Gemini safety filter: %1</source>
        <translation>تم حظر الطلب بواسطة مرشح أمان Gemini: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="199"/>
        <source>Gemini stopped without producing a response: %1</source>
        <translation>توقف Gemini دون إنتاج استجابة: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="261"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="264"/>
        <source>Invalid API key (%1)</source>
        <translation>مفتاح API غير صالح (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="266"/>
        <source>Rate limited: %1</source>
        <translation>تم تجاوز الحد المسموح: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="268"/>
        <source>Invalid API key</source>
        <translation>مفتاح API غير صالح</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="270"/>
        <source>Gemini %1: %2</source>
        <translation>Gemini %1: %2</translation>
    </message>
</context>
<context>
    <name>AI::OpenAIReply</name>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="340"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="343"/>
        <source>Invalid API key (%1)</source>
        <translation>مفتاح API غير صالح (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="345"/>
        <source>Rate limited: %1</source>
        <translation>تم تجاوز الحد المسموح: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="347"/>
        <source>%1 %2: %3</source>
        <translation>%1 %2: %3</translation>
    </message>
</context>
<context>
    <name>API::GRPC::GRPCServer</name>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="421"/>
        <source>Export Protobuf File</source>
        <translation>تصدير ملف Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="423"/>
        <source>Protocol Buffers (*.proto)</source>
        <translation>Protocol Buffers ‏(*.proto)</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="469"/>
        <source>Unable to start gRPC server</source>
        <translation>تعذر بدء خادم GRPC</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="470"/>
        <source>Failed to bind to %1</source>
        <translation>فشل الربط بـ %1</translation>
    </message>
</context>
<context>
    <name>API::Server</name>
    <message>
        <location filename="../../src/API/Server.cpp" line="434"/>
        <source>Unable to start API TCP server</source>
        <translation>تعذر بدء خادم API عبر TCP</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="478"/>
        <source>Allow External API Connections?</source>
        <translation>السماح بالاتصالات الخارجية لـ API؟</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="479"/>
        <source>Exposing the API server to external hosts allows other devices on your network to connect to Serial Studio on port 7777.

Only enable this on trusted networks. Untrusted clients may read live data or send commands to your device.</source>
        <translation>يتيح عرض خادم API للمضيفين الخارجيين للأجهزة الأخرى على شبكتك الاتصال بـ Serial Studio على المنفذ 7777.

فعّل هذا فقط على الشبكات الموثوقة. قد يتمكن العملاء غير الموثوقين من قراءة البيانات المباشرة أو إرسال أوامر إلى جهازك.</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="513"/>
        <source>Unable to restart API TCP server</source>
        <translation>تعذرت إعادة تشغيل خادم API عبر TCP</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="600"/>
        <source>Allow API device control?</source>
        <translation>السماح بالتحكم بالجهاز عبر API؟</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="601"/>
        <source>A program using Serial Studio's local API is requesting to send data to the connected device. Allow API clients to write to the device?</source>
        <translation>يطلب برنامج يستخدم API المحلي لـ Serial Studio إرسال بيانات إلى الجهاز المتصل. السماح لعملاء API بالكتابة إلى الجهاز؟</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="604"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1242"/>
        <source>API server</source>
        <translation>خادم API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1242"/>
        <source>Invalid pending connection</source>
        <translation>اتصال معلق غير صالح</translation>
    </message>
</context>
<context>
    <name>About</name>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="39"/>
        <source>About</source>
        <translation>حول</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="96"/>
        <source>Version %1</source>
        <translation>الإصدار %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="106"/>
        <source>Copyright © %1 %2</source>
        <translation>حقوق النشر © %1 %2</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="112"/>
        <source>All Rights Reserved</source>
        <translation>جميع الحقوق محفوظة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="127"/>
        <source>%1 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

%1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</source>
        <translation>%1 برنامج حر: يمكن إعادة توزيعه و/أو تعديله بموجب شروط رخصة GNU العمومية العامة كما نشرتها مؤسسة البرمجيات الحرة؛ سواء الإصدار 3 من الرخصة، أو (حسب اختيارك) أي إصدار لاحق.

يتم توزيع %1 على أمل أن يكون مفيداً، ولكن دون أي ضمان؛ حتى دون الضمان الضمني لقابلية التسويق أو الملاءمة لغرض معين. راجع رخصة GNU العمومية العامة لمزيد من التفاصيل.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="146"/>
        <source>This configuration is licensed for commercial and proprietary use. It may be used in closed-source and commercial applications, subject to the terms of the commercial license.</source>
        <translation>هذا التكوين مرخص للاستخدام التجاري والاحتكاري. يمكن استخدامه في التطبيقات المغلقة المصدر والتجارية، وفقاً لشروط الرخصة التجارية.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="160"/>
        <source>This configuration is for personal and evaluation purposes only. Commercial use is prohibited unless a valid commercial license is activated.</source>
        <translation>هذا التكوين للاستخدام الشخصي والتقييمي فقط. الاستخدام التجاري محظور ما لم يتم تفعيل رخصة تجارية صالحة.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="174"/>
        <source>This software is provided 'as is' without warranty of any kind, express or implied, including but not limited to the warranties of merchantability or fitness for a particular purpose. In no event shall the author be liable for any damages arising from the use of this software.</source>
        <translation>يتم توفير هذا البرنامج 'كما هو' دون ضمان من أي نوع، صريح أو ضمني، بما في ذلك على سبيل المثال لا الحصر ضمانات قابلية التسويق أو الملاءمة لغرض معين. لن يكون المؤلف مسؤولاً بأي حال عن أي أضرار ناشئة عن استخدام هذا البرنامج.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="195"/>
        <source>Manage License</source>
        <translation>إدارة الرخصة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="203"/>
        <source>Donate</source>
        <translation>تبرع</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="214"/>
        <source>Check for Updates</source>
        <translation>التحقق من التحديثات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="223"/>
        <source>What's New</source>
        <translation>ما الجديد</translation>
    </message>
    <message>
        <source>Tips &amp;&amp; Tricks</source>
        <translation type="vanished">نصائح وحيل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="232"/>
        <source>License Agreement</source>
        <translation>اتفاقية الترخيص</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="241"/>
        <source>Report Bug</source>
        <translation>الإبلاغ عن خطأ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="250"/>
        <source>Acknowledgements</source>
        <translation>الشكر والتقدير</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="259"/>
        <source>Benchmark</source>
        <translation>قياس الأداء</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="267"/>
        <source>Website</source>
        <translation>الموقع الإلكتروني</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="283"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
</context>
<context>
    <name>Accelerometer</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="181"/>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="182"/>
        <source>Settings</source>
        <translation>الإعدادات</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="240"/>
        <source>G-FORCE</source>
        <translation>قوة الجاذبية</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="276"/>
        <source>PITCH ↕</source>
        <translation>الميل ↕</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="311"/>
        <source>ROLL ↔</source>
        <translation>الدوران ↔</translation>
    </message>
</context>
<context>
    <name>AccelerometerConfigDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="35"/>
        <source>Accelerometer Configuration</source>
        <translation>إعدادات مقياس التسارع</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="95"/>
        <source>Configure the accelerometer display range and input units.</source>
        <translation>تكوين نطاق العرض ووحدات الإدخال لمقياس التسارع.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="109"/>
        <source>Display Range</source>
        <translation>نطاق العرض</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="130"/>
        <source>Max G:</source>
        <translation>الحد الأقصى G:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="144"/>
        <source>Enter max G value</source>
        <translation>أدخل قيمة G القصوى</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="164"/>
        <source>Input Configuration</source>
        <translation>إعدادات الإدخال</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="184"/>
        <source>Input already in G</source>
        <translation>الإدخال بوحدة G بالفعل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="218"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
</context>
<context>
    <name>Acknowledgements</name>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="35"/>
        <source>Acknowledgements</source>
        <translation>شكر وتقدير</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="76"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="87"/>
        <source>About Qt…</source>
        <translation>حول QT…</translation>
    </message>
</context>
<context>
    <name>ActionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="136"/>
        <source>Change Icon</source>
        <translation>تغيير الأيقونة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="138"/>
        <source>Change the icon used for this action</source>
        <translation>تغيير الأيقونة المستخدمة لهذا الإجراء</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="156"/>
        <source>Duplicate</source>
        <translation>تكرار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="160"/>
        <source>Duplicate this action with all its settings</source>
        <translation>تكرار هذا الإجراء مع جميع إعداداته</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="169"/>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="171"/>
        <source>Delete this action from the project</source>
        <translation>حذف هذا الإجراء من المشروع</translation>
    </message>
</context>
<context>
    <name>AddWidgetDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="44"/>
        <source>Add Widget</source>
        <translation>إضافة عنصر واجهة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="211"/>
        <source>Available Widgets</source>
        <translation>عناصر الواجهة المتاحة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="220"/>
        <source>Click a row to add it to the workspace.</source>
        <translation>انقر على صف لإضافته إلى مساحة العمل.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="228"/>
        <source>Search</source>
        <translation>بحث</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="247"/>
        <source>Widget</source>
        <translation>عنصر واجهة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="248"/>
        <source>Group</source>
        <translation>مجموعة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="249"/>
        <source>Name</source>
        <translation>الاسم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="316"/>
        <source>(entire group)</source>
        <translation>(المجموعة بأكملها)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="351"/>
        <source>Already in workspace</source>
        <translation>موجود في مساحة العمل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="352"/>
        <source>Add to workspace</source>
        <translation>إضافة إلى مساحة العمل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="381"/>
        <source>No widgets available.</source>
        <translation>لا توجد عناصر واجهة متاحة.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="382"/>
        <source>No widgets match.</source>
        <translation>لا توجد عناصر واجهة مطابقة.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="399"/>
        <source>%1 widgets</source>
        <translation>%1 عنصر واجهة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="400"/>
        <source>%1 of %2 widgets</source>
        <translation>%1 من %2 عنصر واجهة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="404"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
</context>
<context>
    <name>AlarmBandsEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="35"/>
        <source>Alarm Bands</source>
        <translation>نطاقات الإنذار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="71"/>
        <source>Info</source>
        <translation>معلومات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="72"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="152"/>
        <source>OK</source>
        <translation>موافق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="73"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="153"/>
        <source>Warning</source>
        <translation>تحذير</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="74"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="154"/>
        <source>Critical</source>
        <translation>حرج</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="83"/>
        <source>Tachometer</source>
        <translation>مقياس سرعة الدوران</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="85"/>
        <source>Idle</source>
        <translation>وضع الخمول</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="86"/>
        <source>Operating</source>
        <translation>وضع التشغيل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="87"/>
        <source>Caution</source>
        <translation>تنبيه</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="88"/>
        <source>Redline</source>
        <translation>الخط الأحمر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="92"/>
        <source>Speedometer</source>
        <translation>عداد السرعة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="94"/>
        <source>Cruise</source>
        <translation>سرعة ثابتة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="95"/>
        <source>Fast</source>
        <translation>سريع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="96"/>
        <source>Top Speed</source>
        <translation>السرعة القصوى</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="100"/>
        <source>Engine Temperature</source>
        <translation>درجة حرارة المحرك</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="102"/>
        <source>Cold</source>
        <translation>بارد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="103"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="112"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="144"/>
        <source>Normal</source>
        <translation>عادي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="104"/>
        <source>Warm</source>
        <translation>دافئ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="105"/>
        <source>Overheat</source>
        <translation>ارتفاع درجة الحرارة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="109"/>
        <source>Pressure</source>
        <translation>الضغط</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="111"/>
        <source>Vacuum</source>
        <translation>الفراغ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="113"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="122"/>
        <source>High</source>
        <translation>عالٍ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="114"/>
        <source>Burst</source>
        <translation>انفجار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="118"/>
        <source>Battery Voltage</source>
        <translation>جهد البطارية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="120"/>
        <source>Low</source>
        <translation>منخفض</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="121"/>
        <source>Nominal</source>
        <translation>اسمي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="126"/>
        <source>Fuel Level</source>
        <translation>مستوى الوقود</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="128"/>
        <source>Empty</source>
        <translation>فارغ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="129"/>
        <source>Reserve</source>
        <translation>احتياطي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="134"/>
        <source>Signal Strength</source>
        <translation>قوة الإشارة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="136"/>
        <source>No Signal</source>
        <translation>لا توجد إشارة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="137"/>
        <source>Weak</source>
        <translation>ضعيفة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="138"/>
        <source>Good</source>
        <translation>جيدة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="142"/>
        <source>CPU / System Load</source>
        <translation>حمل المعالج / النظام</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="145"/>
        <source>Busy</source>
        <translation>مشغول</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="146"/>
        <source>Overload</source>
        <translation>حمل زائد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="150"/>
        <source>OK / Warning / Critical</source>
        <translation>طبيعي / تحذير / حرج</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="158"/>
        <source>Indicator (On / Off)</source>
        <translation>المؤشر (تشغيل / إيقاف)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="160"/>
        <source>On</source>
        <translation>تشغيل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="164"/>
        <source>Fault Indicator</source>
        <translation>مؤشر العطل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="166"/>
        <source>Fault</source>
        <translation>عطل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="287"/>
        <source>Choose Band Color</source>
        <translation>اختيار لون النطاق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="314"/>
        <source>Presets</source>
        <translation>الإعدادات المسبقة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="337"/>
        <source>Preset</source>
        <translation>إعداد مسبق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="352"/>
        <source>Choose preset…</source>
        <translation>اختر إعداداً مسبقاً…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="462"/>
        <source>Blink</source>
        <translation>وميض</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="582"/>
        <source>Reset to severity default</source>
        <translation>إعادة تعيين إلى افتراضي الخطورة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="596"/>
        <source>Click to choose a color. Right-click to reset to severity default.</source>
        <translation>انقر لاختيار لون. انقر بزر الماوس الأيمن لإعادة التعيين إلى افتراضي الخطورة.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="597"/>
        <source>Click to choose a custom color.</source>
        <translation>انقر لاختيار لون مخصص.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="628"/>
        <source>Flash the LED while the value sits in this band.</source>
        <translation>وميض LED أثناء وجود القيمة في هذا النطاق.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="702"/>
        <source>No bands defined. Pick a preset above or add a band to get started.</source>
        <translation>لا توجد نطاقات محددة. اختر إعداداً مسبقاً أعلاه أو أضف نطاقاً للبدء.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="826"/>
        <source>Apply</source>
        <translation>تطبيق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="829"/>
        <source>Apply changes to the dataset.</source>
        <translation>تطبيق التغييرات على مجموعة البيانات.</translation>
    </message>
    <message>
        <source>Apply Preset</source>
        <translation type="vanished">تطبيق الإعداد المسبق</translation>
    </message>
    <message>
        <source>Replace the current bands with the selected preset, scaled to this dataset's range.</source>
        <translation type="vanished">استبدال النطاقات الحالية بالإعداد المسبق المحدد، مع تحجيمها لنطاق مجموعة البيانات هذه.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="363"/>
        <source>Range</source>
        <translation>النطاق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="391"/>
        <source>Bands</source>
        <translation>النطاقات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="402"/>
        <source>Add Band</source>
        <translation>إضافة نطاق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="406"/>
        <source>Add a new band continuing from the last one.</source>
        <translation>إضافة نطاق جديد يستمر من النطاق الأخير.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="437"/>
        <source>Min</source>
        <translation>الأدنى</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="443"/>
        <source>Max</source>
        <translation>الأقصى</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="449"/>
        <source>Severity</source>
        <translation>الخطورة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="455"/>
        <source>Color</source>
        <translation>لون</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="469"/>
        <source>Label</source>
        <translation>تسمية</translation>
    </message>
    <message>
        <source>Choose a custom color.</source>
        <translation type="vanished">اختر لوناً مخصصاً.</translation>
    </message>
    <message>
        <source>auto</source>
        <translation type="vanished">تلقائي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="639"/>
        <source>(optional)</source>
        <translation>(اختياري)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="656"/>
        <source>Move up.</source>
        <translation>نقل لأعلى.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="675"/>
        <source>Move down.</source>
        <translation>نقل لأسفل.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="688"/>
        <source>Remove this band.</source>
        <translation>إزالة هذا النطاق.</translation>
    </message>
    <message>
        <source>No bands defined. Apply a preset or add a band to get started.</source>
        <translation type="vanished">لا توجد نطاقات محددة. طبّق إعداداً مسبقاً أو أضف نطاقاً للبدء.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="719"/>
        <source>Preview</source>
        <translation>معاينة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="815"/>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="817"/>
        <source>Discard changes.</source>
        <translation>تجاهل التغييرات.</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="vanished">حفظ</translation>
    </message>
    <message>
        <source>Save changes to the dataset.</source>
        <translation type="vanished">حفظ التغييرات على مجموعة البيانات.</translation>
    </message>
</context>
<context>
    <name>AssistantPanel</name>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="31"/>
        <source>Assistant</source>
        <translation>المساعد</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="122"/>
        <source>Help me discover Serial Studio's features</source>
        <translation>ساعدني في اكتشاف ميزات Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="123"/>
        <source>What can this app do for my telemetry?</source>
        <translation>ما الذي يمكن لهذا التطبيق فعله لبيانات القياس عن بُعد الخاصة بي؟</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="124"/>
        <source>Walk me through what this project already contains</source>
        <translation>اشرح لي ما يحتويه هذا المشروع بالفعل</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="125"/>
        <source>List the sources in this project</source>
        <translation>اعرض المصادر في هذا المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="128"/>
        <source>What is a session database, and why would I use one?</source>
        <translation>ما هي قاعدة بيانات الجلسة، ولماذا قد أستخدمها؟</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="129"/>
        <source>CSV vs MDF4 export - what is the difference?</source>
        <translation>تصدير CSV مقابل MDF4 - ما الفرق؟</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="130"/>
        <source>What is a frame parser, and when do I need one?</source>
        <translation>ما هو محلل الإطارات، ومتى أحتاج إليه؟</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="131"/>
        <source>When should I use Lua vs JavaScript for the parser?</source>
        <translation>متى يجب استخدام Lua مقابل JavaScript للمحلل؟</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="132"/>
        <source>Plot, Bar, and Gauge - when to use each?</source>
        <translation>الرسم البياني والشريط والمقياس - متى يُستخدم كل منها؟</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="133"/>
        <source>What is the difference between a transform and a frame parser?</source>
        <translation>ما الفرق بين التحويل ومحلل الإطارات؟</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="136"/>
        <source>Add a UART source for an Arduino</source>
        <translation>إضافة مصدر UART لجهاز Arduino</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="137"/>
        <source>Set up an IMU project from scratch</source>
        <translation>إعداد مشروع IMU من الصفر</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="138"/>
        <source>Configure an MQTT subscriber</source>
        <translation>تكوين مشترك MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="139"/>
        <source>Add a CAN bus source</source>
        <translation>إضافة مصدر ناقل CAN</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="140"/>
        <source>Set up a Modbus poller</source>
        <translation>إعداد مستطلع Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="141"/>
        <source>Add a network (TCP/UDP) source</source>
        <translation>إضافة مصدر شبكة (TCP/UDP)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="142"/>
        <source>Write a CSV frame parser for me</source>
        <translation>كتابة محلل إطارات CSV</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="143"/>
        <source>Help me parse a JSON frame</source>
        <translation>مساعدتي في تحليل إطار JSON</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="144"/>
        <source>Add an EMA smoothing transform to a dataset</source>
        <translation>إضافة تحويل تمهيد EMA إلى مجموعة بيانات</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="145"/>
        <source>Decode hexadecimal frames</source>
        <translation>فك تشفير الإطارات السداسية عشرية</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="146"/>
        <source>Calibrate a sensor with a linear transform</source>
        <translation>معايرة مستشعر بتحويل خطي</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="149"/>
        <source>Suggest dashboard widgets for my data</source>
        <translation>اقتراح عناصر لوحة القيادة لبياناتي</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="150"/>
        <source>Build an executive overview workspace</source>
        <translation>بناء مساحة عمل نظرة عامة تنفيذية</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="151"/>
        <source>Add a painter widget for a custom visualization</source>
        <translation>إضافة عنصر رسام لتصور مخصص</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="152"/>
        <source>Show Plot, FFT, and Waterfall for one dataset</source>
        <translation>عرض الرسم البياني وFFT والشلال لمجموعة بيانات واحدة</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="153"/>
        <source>Group my datasets into useful workspaces</source>
        <translation>تجميع مجموعات البيانات في مساحات عمل مفيدة</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="204"/>
        <source>How can I help with your project?</source>
        <translation>كيف يمكنني المساعدة في مشروعك؟</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="205"/>
        <source>Set up your API key to get started</source>
        <translation>إعداد مفتاح API للبدء</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="217"/>
        <source>Describe what you would like to build, and I will configure the sources, groups, datasets, frame parsers, and transforms for you.</source>
        <translation>وصف ما تريد بناءه، وسأقوم بتكوين المصادر والمجموعات ومجموعات البيانات ومحللات الإطارات والتحويلات نيابة عنك.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="220"/>
        <source>To start chatting, paste an API key for the selected provider. Keys are encrypted on this machine and never leave your computer except to talk to the provider you choose.</source>
        <translation>لبدء المحادثة، الصق مفتاح API للموفر المحدد. يتم تشفير المفاتيح على هذا الجهاز ولا تغادر حاسوبك أبدًا إلا للاتصال بالموفر الذي تختاره.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="241"/>
        <source>Open API Key Setup</source>
        <translation>فتح إعداد مفتاح API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="251"/>
        <source>Get a key from %1</source>
        <translation>احصل على مفتاح من %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="432"/>
        <source>Drop files or folders to let the assistant read them</source>
        <translation>أسقط الملفات أو المجلدات للسماح للمساعد بقراءتها</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="477"/>
        <source>Added folder "%1" - readable this session</source>
        <translation>تمت إضافة المجلد "%1" - قابل للقراءة في هذه الجلسة</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="478"/>
        <source>Added "%1" - readable this session</source>
        <translation>تمت إضافة "%1" - قابل للقراءة في هذه الجلسة</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="562"/>
        <source>Ask Serial Studio anything…</source>
        <translation>اسأل Serial Studio عن أي شيء…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="582"/>
        <source>Clear conversation</source>
        <translation>مسح المحادثة</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="626"/>
        <source>Stop generating</source>
        <translation>إيقاف التوليد</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="627"/>
        <source>Send message (Enter)</source>
        <translation>إرسال رسالة (Enter)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="669"/>
        <source>Provider</source>
        <translation>المزود</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="702"/>
        <source>Model selection</source>
        <translation>اختيار النموذج</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="748"/>
        <source>Run editing actions without asking each time. Blocked actions stay blocked.</source>
        <translation>تنفيذ إجراءات التحرير دون السؤال في كل مرة. الإجراءات المحظورة تبقى محظورة.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="750"/>
        <source>Auto-approve edits</source>
        <translation>الموافقة التلقائية على التعديلات</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="766"/>
        <source>Let the AI configure devices, connect/disconnect and send data. Each action still asks for your approval.</source>
        <translation>السماح لـ AI بتكوين الأجهزة والاتصال/قطع الاتصال وإرسال البيانات. كل إجراء لا يزال يطلب موافقتك.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="768"/>
        <source>Allow device control</source>
        <translation>السماح بالتحكم بالجهاز</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="794"/>
        <source>Manage API keys</source>
        <translation>إدارة مفاتيح API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="815"/>
        <source>Working</source>
        <translation>جارٍ العمل</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="816"/>
        <source>Ready</source>
        <translation>جاهز</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="817"/>
        <source>  •  cache %1k tok</source>
        <translation>•  ذاكرة مؤقتة %1k رمز</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="818"/>
        <source>  •  cache write %1k tok</source>
        <translation>•  كتابة ذاكرة مؤقتة %1k رمز</translation>
    </message>
</context>
<context>
    <name>Audio</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="89"/>
        <source>No Microphone Detected</source>
        <translation>لم يتم اكتشاف ميكروفون</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="98"/>
        <source>Connect a mic or check your settings</source>
        <translation>قم بتوصيل ميكروفون أو تحقق من الإعدادات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="123"/>
        <source>Input Device</source>
        <translation>جهاز الإدخال</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="142"/>
        <source>Sample Rate</source>
        <translation>معدل العينة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="230"/>
        <source>Sample Format</source>
        <translation>تنسيق العينة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="180"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="249"/>
        <source>Channels</source>
        <translation>القنوات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="211"/>
        <source>Output Device</source>
        <translation>جهاز الإخراج</translation>
    </message>
</context>
<context>
    <name>AuthenticateDialog</name>
    <message>
        <source>Dialog</source>
        <translation type="vanished">مربع حوار</translation>
    </message>
    <message>
        <source>Please provide the user name and password for the download location.</source>
        <translation type="vanished">يُرجى تقديم اسم المستخدم وكلمة المرور لموقع التنزيل.</translation>
    </message>
    <message>
        <source>&amp;User name:</source>
        <translation type="vanished">اسم المست&amp;خدم:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="vanished">كلمة الم&amp;رور:</translation>
    </message>
</context>
<context>
    <name>AxisRangeDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="47"/>
        <source>Axis Range Configuration</source>
        <translation>إعدادات نطاق المحور</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="161"/>
        <source>Configure the visible range for the plot axes. Values update in real-time as you type.</source>
        <translation>تكوين النطاق المرئي لمحاور الرسم البياني. تُحدَّث القيم فوريًا أثناء الكتابة.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="169"/>
        <source>X Axis</source>
        <translation>المحور X</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="194"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="265"/>
        <source>Minimum:</source>
        <translation>الحد الأدنى:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="206"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="277"/>
        <source>Enter min value</source>
        <translation>أدخل القيمة الدنيا</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="215"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="286"/>
        <source>Maximum:</source>
        <translation>الحد الأقصى:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="227"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="298"/>
        <source>Enter max value</source>
        <translation>أدخل القيمة القصوى</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="242"/>
        <source>Y Axis</source>
        <translation>المحور Y</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="317"/>
        <source>Reset</source>
        <translation>إعادة تعيين</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="327"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
</context>
<context>
    <name>BackupRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="31"/>
        <source>Recover Backup</source>
        <translation>استرجاع النسخة الاحتياطية</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="94"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="165"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="166"/>
        <source>Untitled</source>
        <translation>بدون عنوان</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="97"/>
        <source>Project Loaded</source>
        <translation>تم تحميل المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="98"/>
        <source>Auto-save</source>
        <translation>حفظ تلقائي</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="99"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="119"/>
        <source>Before Restore</source>
        <translation>قبل الاستعادة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="100"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="106"/>
        <source>Before Delete Dataset</source>
        <translation>قبل حذف مجموعة البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="101"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="107"/>
        <source>Before Delete Group</source>
        <translation>قبل حذف المجموعة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="102"/>
        <source>Before New Project</source>
        <translation>قبل مشروع جديد</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="103"/>
        <source>Before Open Project</source>
        <translation>قبل فتح مشروع</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="104"/>
        <source>Before Load JSON</source>
        <translation>قبل تحميل JSON</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="105"/>
        <source>Before Apply Template</source>
        <translation>قبل تطبيق القالب</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="108"/>
        <source>Before Delete Action</source>
        <translation>قبل حذف الإجراء</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="109"/>
        <source>Before Delete Output Widget</source>
        <translation>قبل حذف عنصر الإخراج</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="110"/>
        <source>Before Move Dataset</source>
        <translation>قبل نقل مجموعة البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="111"/>
        <source>Before Move Group</source>
        <translation>قبل نقل المجموعة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="112"/>
        <source>Before Delete Workspace</source>
        <translation>قبل حذف مساحة العمل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="113"/>
        <source>Before Clear All Workspaces</source>
        <translation>قبل مسح جميع مساحات العمل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="114"/>
        <source>Before Remove Widget</source>
        <translation>قبل إزالة عنصر الواجهة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="115"/>
        <source>Before Reorder Workspaces</source>
        <translation>قبل إعادة ترتيب مساحات العمل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="116"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="117"/>
        <source>Before Batch Operation</source>
        <translation>قبل العملية الدفعية</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="118"/>
        <source>Before Add Tile</source>
        <translation>قبل إضافة لوحة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="142"/>
        <source>%1 (and %2 more)</source>
        <translation>%1 (و %2 أخرى)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="159"/>
        <source> The frame parser code also differs and will be replaced.</source>
        <translation>كود محلل الإطارات يختلف أيضًا وسيتم استبداله.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="164"/>
        <source>Title changes from “%1” to “%2”. Group structure unchanged.</source>
        <translation>تغيير العنوان من "%1" إلى "%2". بنية المجموعة دون تغيير.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="169"/>
        <source>Same groups and datasets, but the frame parser code differs — restoring will replace it.</source>
        <translation>نفس المجموعات ومجموعات البيانات، لكن كود محلل الإطارات يختلف — الاستعادة ستستبدله.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="171"/>
        <source>Same groups and datasets as your current project. Restoring may still revert field-level edits.</source>
        <translation>نفس المجموعات ومجموعات البيانات الموجودة في مشروعك الحالي. قد تؤدي الاستعادة إلى التراجع عن التعديلات على مستوى الحقول.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="178"/>
        <source>Restoring removes %1 and brings back %2.</source>
        <translation>تزيل الاستعادة %1 وتسترجع %2.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="181"/>
        <source>Restoring removes %1.</source>
        <translation>تزيل الاستعادة %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="183"/>
        <source>Restoring brings back %1.</source>
        <translation>تسترجع الاستعادة %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="209"/>
        <source>Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.</source>
        <translation>اختر نسخة احتياطية للاستعادة. يتم حفظ المشروع الحالي تلقائياً أولاً، لذا يمكن التراجع عن الاستعادة.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="292"/>
        <source>No backups for this project yet. Edit or save the project to start the rolling backup.</source>
        <translation>لا توجد نسخ احتياطية لهذا المشروع بعد. قم بتحرير أو حفظ المشروع لبدء النسخ الاحتياطي المتجدد.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="320"/>
        <source>Open Folder</source>
        <translation>فتح المجلد</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="328"/>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="334"/>
        <source>Restore</source>
        <translation>استعادة</translation>
    </message>
</context>
<context>
    <name>Benchmark</name>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="32"/>
        <source>Benchmark</source>
        <translation>قياس الأداء</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="79"/>
        <source>%1 frames/s</source>
        <translation>%1 إطار/ث</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="83"/>
        <source>%1 s</source>
        <translation>%1 ث</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="88"/>
        <source>n/a</source>
        <translation>غير متاح</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Pass</source>
        <translation>نجح</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Fail</source>
        <translation>فشل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="111"/>
        <source>Hotpath Benchmark</source>
        <translation>قياس أداء المسار الساخن</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="122"/>
        <source>Measures how fast this computer can extract, parse, and visualize frames through Serial Studio's data pipeline.</source>
        <translation>يقيس سرعة هذا الحاسوب في استخراج الإطارات وتحليلها وتصويرها عبر خط بيانات Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="168"/>
        <source>The interface will freeze while running</source>
        <translation>ستتجمد الواجهة أثناء التشغيل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="179"/>
        <source>Each phase runs flat-out on the main thread, so the window stops responding until it finishes. Your current project is reloaded automatically when the benchmark ends.</source>
        <translation>تعمل كل مرحلة بكامل طاقتها على الخيط الرئيسي، لذا تتوقف النافذة عن الاستجابة حتى تنتهي. يُعاد تحميل مشروعك الحالي تلقائيًا عند انتهاء المعيار.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="236"/>
        <source>Frames per phase:</source>
        <translation>الإطارات لكل مرحلة:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="250"/>
        <source>Minimum duration:</source>
        <translation>الحد الأدنى للمدة:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="279"/>
        <source>Stages</source>
        <translation>المراحل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="287"/>
        <source>Parsers</source>
        <translation>المحللات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="296"/>
        <source>Data export</source>
        <translation>تصدير البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="304"/>
        <source>Dashboard</source>
        <translation>لوحة المعلومات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="316"/>
        <source>Data</source>
        <translation>البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="326"/>
        <source>Numeric only</source>
        <translation>القيم الرقمية فقط</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="335"/>
        <source>Mixed (numeric + text)</source>
        <translation>مختلط (رقمي + نص)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="351"/>
        <source>Select at least one stage and one data type to run a benchmark.</source>
        <translation>حدد مرحلة واحدة على الأقل ونوع بيانات واحد لتشغيل معيار الأداء.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="210"/>
        <source>Running %1...</source>
        <translation>قيد التشغيل %1...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="211"/>
        <source>Preparing...</source>
        <translation>جارٍ التحضير...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="383"/>
        <source>Pipeline</source>
        <translation>خط الأنابيب</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="395"/>
        <source>Throughput</source>
        <translation>الإنتاجية</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="407"/>
        <source>Time</source>
        <translation>الوقت</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="419"/>
        <source>Result</source>
        <translation>النتيجة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="520"/>
        <source>Run a test to see results</source>
        <translation>شغّل اختباراً لعرض النتائج</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="539"/>
        <source>Pass/Fail applies to the data-pipeline and parser stages (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard stages are informational.</source>
        <translation>النجاح/الفشل ينطبق على مراحل خط البيانات والمحلل (خط البيانات ومحلل مدمج رقمي 1024 ألف إطار/ث؛ مدمج مختلط 512 ألف؛ Lua رقمي 256 ألف؛ JavaScript رقمي وLua مختلط 128 ألف؛ JavaScript مختلط 64 ألف). مراحل التصدير ولوحة المعلومات إعلامية فقط.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">النجاح/الفشل ينطبق على مراحل خط البيانات والمحلل (خط البيانات والمحلل المدمج الرقمي 1024 ألف إطار/ث؛ المدمج المختلط 512 ألف؛ Lua الرقمي 256 ألف؛ JavaScript الرقمي و Lua المختلط 128 ألف؛ JavaScript المختلط 64 ألف). مراحل التصدير ولوحة المعلومات إعلامية فقط.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Native numeric 1024 K frames/s; Native mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">ينطبق النجاح/الفشل على مراحل خط الأنابيب والمحلل (خط الأنابيب: Native رقمي 1024 ألف إطار/ث؛ Native مختلط 512 ألف؛ Lua رقمي 256 ألف؛ JavaScript رقمي و Lua مختلط 128 ألف؛ JavaScript مختلط 64 ألف). مراحل التصدير ولوحة المعلومات للإعلام فقط.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="554"/>
        <source>Copy</source>
        <translation>نسخ</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline 1024 K frames/s; numeric: Lua 256 K, JavaScript 128 K; mixed: Lua 128 K, JavaScript 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">النجاح/الفشل ينطبق على مراحل خط البيانات والمحلل (خط البيانات 1024 ألف إطار/ث؛ رقمي: Lua 256 ألف، JavaScript 128 ألف؛ مختلط: Lua 128 ألف، JavaScript 64 ألف). مراحل التصدير ولوحة المعلومات إعلامية فقط.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the parser phases only (Lua target 256 K frames/s, JavaScript 128 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">النجاح/الفشل ينطبق على مراحل المحلل فقط (هدف Lua 256 ألف إطار/ث، JavaScript 128 ألف). مراحل التصدير ولوحة المعلومات إعلامية فقط.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="561"/>
        <source>Clear</source>
        <translation>مسح</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="570"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Running...</source>
        <translation>جارٍ التشغيل...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Run Benchmark</source>
        <translation>تشغيل المعيار</translation>
    </message>
</context>
<context>
    <name>BenchmarkRunner</name>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="255"/>
        <source>Data pipeline</source>
        <translation>خط معالجة البيانات</translation>
    </message>
    <message>
        <source>Lua parser</source>
        <translation type="vanished">محلل Lua</translation>
    </message>
    <message>
        <source>JavaScript parser</source>
        <translation type="vanished">محلل JavaScript</translation>
    </message>
    <message>
        <source>Lua + data export</source>
        <translation type="vanished">Lua + تصدير البيانات</translation>
    </message>
    <message>
        <source>Lua + dashboard</source>
        <translation type="vanished">Lua + لوحة المعلومات</translation>
    </message>
    <message>
        <source>Native parser (numeric)</source>
        <translation type="vanished">محلل Native (رقمي)</translation>
    </message>
    <message>
        <source>Native parser (mixed)</source>
        <translation type="vanished">محلل Native (مختلط)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="279"/>
        <source>Built-in parser (numeric)</source>
        <translation>محلل مدمج (رقمي)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="297"/>
        <source>Built-in parser (mixed)</source>
        <translation>محلل مدمج (مختلط)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="281"/>
        <source>Lua parser (numeric)</source>
        <translation>محلل Lua (رقمي)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="288"/>
        <source>JavaScript parser (numeric)</source>
        <translation>محلل JavaScript (رقمي)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="299"/>
        <source>Lua parser (mixed)</source>
        <translation>محلل Lua (مختلط)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="306"/>
        <source>JavaScript parser (mixed)</source>
        <translation>محلل JavaScript (مختلط)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="322"/>
        <source>Built-in + data export (numeric)</source>
        <translation>محلل مدمج + تصدير البيانات (رقمي)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="329"/>
        <source>Lua + data export (numeric)</source>
        <translation>Lua + تصدير البيانات (رقمي)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="336"/>
        <source>JavaScript + data export (numeric)</source>
        <translation>JavaScript + تصدير البيانات (رقمي)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="345"/>
        <source>Built-in + data export (mixed)</source>
        <translation>محلل مدمج + تصدير البيانات (مختلط)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="347"/>
        <source>Lua + data export (mixed)</source>
        <translation>Lua + تصدير البيانات (مختلط)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="354"/>
        <source>JavaScript + data export (mixed)</source>
        <translation>JavaScript + تصدير البيانات (مختلط)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="370"/>
        <source>Built-in + dashboard (numeric)</source>
        <translation>محلل مدمج + لوحة المعلومات (رقمي)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="372"/>
        <source>Lua + dashboard (numeric)</source>
        <translation>Lua + لوحة المعلومات (رقمي)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>100 K frames</source>
        <translation>100 ألف إطار</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>250 K frames</source>
        <translation>250 ألف إطار</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>500 K frames</source>
        <translation>500 ألف إطار</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>1 M frames</source>
        <translation>1 مليون إطار</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>1 second</source>
        <translation>ثانية واحدة</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>2 seconds</source>
        <translation>ثانيتان</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>5 seconds</source>
        <translation>5 ثوانٍ</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>10 seconds</source>
        <translation>10 ثوانٍ</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="200"/>
        <source>Serial Studio %1 - Hotpath Benchmark</source>
        <translation>Serial Studio %1 - قياس أداء المسار الحرج</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="202"/>
        <source>%1 (%2), workload: %3 frames minimum, %4 s minimum</source>
        <translation>%1 (%2)، حمل العمل: %3 إطار كحد أدنى، %4 ث كحد أدنى</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Pipeline</source>
        <translation>خط الأنابيب</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Throughput</source>
        <translation>الإنتاجية</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Target</source>
        <translation>الهدف</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Time</source>
        <translation>الوقت</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Result</source>
        <translation>النتيجة</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="225"/>
        <source>%1 frames/s</source>
        <translation>%1 إطار/ث</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>n/a</source>
        <translation>غير متاح</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Pass</source>
        <translation>نجح</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Fail</source>
        <translation>فشل</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="227"/>
        <source>%1 s</source>
        <translation>%1 ث</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="379"/>
        <source>JavaScript + dashboard (numeric)</source>
        <translation>JavaScript + لوحة القيادة (رقمي)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="388"/>
        <source>Built-in + dashboard (mixed)</source>
        <translation>مدمج + لوحة القيادة (مختلط)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="390"/>
        <source>Lua + dashboard (mixed)</source>
        <translation>Lua + لوحة القيادة (مختلط)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="397"/>
        <source>JavaScript + dashboard (mixed)</source>
        <translation>JavaScript + لوحة القيادة (مختلط)</translation>
    </message>
    <message>
        <source>Showing dashboard...</source>
        <translation type="vanished">عرض لوحة المعلومات...</translation>
    </message>
</context>
<context>
    <name>BluetoothLE</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="54"/>
        <source>Device</source>
        <translation>الجهاز</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="106"/>
        <source>Service</source>
        <translation>الخدمة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="142"/>
        <source>Characteristic</source>
        <translation>الخاصية</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="200"/>
        <source>Scanning…</source>
        <translation>جارٍ المسح…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="236"/>
        <source>No Bluetooth Adapter Detected</source>
        <translation>لم يتم اكتشاف محول Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="247"/>
        <source>Connect a Bluetooth adapter or check your system settings</source>
        <translation>قم بتوصيل محول Bluetooth أو تحقق من إعدادات النظام</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="274"/>
        <source>This OS is not Supported Yet.</source>
        <translation>نظام التشغيل هذا غير مدعوم بعد.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="285"/>
        <source>We'll update Serial Studio to work with this operating system as soon as Qt officially supports it</source>
        <translation>سيتم تحديث Serial Studio للعمل مع نظام التشغيل هذا فور دعم QT الرسمي له</translation>
    </message>
</context>
<context>
    <name>CANBus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="57"/>
        <source>No CAN Drivers Found</source>
        <translation>لم يتم العثور على برامج تشغيل CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="70"/>
        <source>Install CAN hardware drivers for your system</source>
        <translation>قم بتثبيت برامج تشغيل أجهزة CAN لنظامك</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="97"/>
        <source>CAN Driver</source>
        <translation>برنامج تشغيل CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="142"/>
        <source>Interface</source>
        <translation>الواجهة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="161"/>
        <source>Bitrate</source>
        <translation>معدل البت</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="225"/>
        <source>Flexible Data-Rate</source>
        <translation>معدل البيانات المرن</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="245"/>
        <source>Loopback</source>
        <translation>الحلقة الراجعة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="265"/>
        <source>Listen-Only</source>
        <translation>الاستماع فقط</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="285"/>
        <source>DBC Database</source>
        <translation>قاعدة بيانات DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="289"/>
        <source>Import DBC File…</source>
        <translation>استيراد ملف DBC…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="322"/>
        <source>No CAN Interfaces Found</source>
        <translation>لم يتم العثور على واجهات CAN</translation>
    </message>
</context>
<context>
    <name>CSV::Player</name>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="206"/>
        <source>Select CSV file</source>
        <translation>اختيار ملف CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="208"/>
        <source>CSV files (*.csv)</source>
        <translation>ملفات CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="366"/>
        <source>Device Connection Active</source>
        <translation>اتصال الجهاز نشط</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="367"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>لاستخدام هذه الميزة، يجب قطع الاتصال بالجهاز. هل تريد المتابعة؟</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Cannot read CSV file</source>
        <translation>تعذرت قراءة ملف CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Check file permissions and location</source>
        <translation>تحقق من أذونات الملف وموقعه</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="411"/>
        <source>Insufficient Data in CSV File</source>
        <translation>بيانات غير كافية في ملف CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="412"/>
        <source>The CSV file must contain at least one data row to proceed. Check the file and try again.</source>
        <translation>يجب أن يحتوي ملف CSV على صف بيانات واحد على الأقل للمتابعة. تحقق من الملف وحاول مرة أخرى.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="642"/>
        <source>Invalid CSV</source>
        <translation>CSV غير صالح</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="643"/>
        <source>The CSV file does not contain any data or headers.</source>
        <translation>ملف CSV لا يحتوي على أي بيانات أو رؤوس.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <source>Select a date/time column</source>
        <translation>تحديد عمود التاريخ/الوقت</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <location filename="../../src/CSV/Player.cpp" line="664"/>
        <source>Set interval manually</source>
        <translation>تعيين الفاصل الزمني يدويًا</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="654"/>
        <source>CSV Date/Time Selection</source>
        <translation>تحديد التاريخ/الوقت في CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="655"/>
        <source>Choose how to handle the date/time data:</source>
        <translation>اختر كيفية معالجة بيانات التاريخ/الوقت:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="667"/>
        <source>Set Interval</source>
        <translation>تعيين الفاصل الزمني</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="668"/>
        <source>Please enter the interval between rows in milliseconds:</source>
        <translation>أدخل الفاصل الزمني بين الصفوف بالميلي ثانية:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="684"/>
        <source>Select Date/Time Column</source>
        <translation>تحديد عمود التاريخ/الوقت</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="685"/>
        <source>Please select the column that contains the date/time data:</source>
        <translation>يرجى تحديد العمود الذي يحتوي على بيانات التاريخ/الوقت:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>Invalid Selection</source>
        <translation>تحديد غير صالح</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>The selected column is not valid.</source>
        <translation>العمود المحدد غير صالح.</translation>
    </message>
</context>
<context>
    <name>Console</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Console.qml" line="32"/>
        <source>Console</source>
        <translation>وحدة التحكم</translation>
    </message>
</context>
<context>
    <name>Console::Export</name>
    <message>
        <location filename="../../src/Console/Export.cpp" line="331"/>
        <source>Console Export is a Pro feature.</source>
        <translation>تصدير وحدة التحكم ميزة Pro.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="332"/>
        <source>This feature requires a license. Please purchase one to enable console export.</source>
        <translation>تتطلب هذه الميزة ترخيصًا. يرجى شراء ترخيص لتفعيل تصدير وحدة التحكم.</translation>
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
        <translation>بدون نهاية سطر</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="255"/>
        <source>New Line</source>
        <translation>سطر جديد</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="256"/>
        <source>Carriage Return</source>
        <translation>إرجاع السطر</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="257"/>
        <source>CR + NL</source>
        <translation>CR + NL</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="267"/>
        <source>Plain Text</source>
        <translation>نص عادي</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="268"/>
        <source>Hexadecimal</source>
        <translation>سداسي عشري</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="290"/>
        <source>No Checksum</source>
        <translation>بدون مجموع تدقيق</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="926"/>
        <source>Device %1</source>
        <translation>الجهاز %1</translation>
    </message>
</context>
<context>
    <name>ConstantsLibraryDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="44"/>
        <source>Insert Constant</source>
        <translation>إدراج ثابت</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Fundamental</source>
        <translation>أساسي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <source>Speed of light in vacuum</source>
        <translation>سرعة الضوء في الفراغ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <source>Planck constant</source>
        <translation>ثابت بلانك</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <source>Elementary charge</source>
        <translation>الشحنة الأولية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <source>Avogadro constant</source>
        <translation>ثابت أفوجادرو</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <source>Boltzmann constant</source>
        <translation>ثابت بولتزمان</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Stefan-Boltzmann constant</source>
        <translation>ثابت ستيفان-بولتزمان</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Mechanics</source>
        <translation>الميكانيكا</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <source>Standard gravity</source>
        <translation>الجاذبية القياسية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Gravitational constant</source>
        <translation>ثابت الجذب العام</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Pressure</source>
        <translation>الضغط</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <source>Standard atmosphere</source>
        <translation>الضغط الجوي القياسي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Sea-level barometric pressure</source>
        <translation>ضغط مستوى سطح البحر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Temperature</source>
        <translation>درجة الحرارة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <source>Absolute zero (Celsius)</source>
        <translation>الصفر المطلق (مئوية)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <source>Water freezing point</source>
        <translation>نقطة تجمد الماء</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Water boiling point (1 atm)</source>
        <translation>نقطة غليان الماء (1 ضغط جوي)</translation>
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
        <translation>الغازات والسوائل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <source>Universal gas constant</source>
        <translation>ثابت الغاز العام</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <source>Specific gas constant (dry air)</source>
        <translation>ثابت الغاز النوعي (هواء جاف)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <source>Specific gas constant (water vapor)</source>
        <translation>ثابت الغاز النوعي (بخار الماء)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <source>Air density (sea level, 15°C)</source>
        <translation>كثافة الهواء (مستوى سطح البحر، 15 درجة مئوية)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <source>Water density (4°C)</source>
        <translation>كثافة الماء (4 درجات مئوية)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <source>Speed of sound in air (20°C)</source>
        <translation>سرعة الصوت في الهواء (20 درجة مئوية)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Heat capacity ratio (dry air)</source>
        <translation>نسبة السعة الحرارية (الهواء الجاف)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Electromagnetism</source>
        <translation>الكهرومغناطيسية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <source>Vacuum permittivity</source>
        <translation>سماحية الفراغ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Vacuum permeability</source>
        <translation>نفاذية الفراغ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Math</source>
        <translation>الرياضيات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <source>Pi</source>
        <translation>باي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <source>Euler's number</source>
        <translation>عدد أويلر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Golden ratio</source>
        <translation>النسبة الذهبية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="212"/>
        <source>Physics Constants</source>
        <translation>ثوابت الفيزياء</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="221"/>
        <source>SI-unit preset values. Click a row to insert it into %1.</source>
        <translation>قيم مُعدّة مسبقاً لوحدات النظام الدولي. انقر على صف لإدراجه في %1.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="231"/>
        <source>Search</source>
        <translation>بحث</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="250"/>
        <source>Symbol</source>
        <translation>الرمز</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="251"/>
        <source>Name</source>
        <translation>الاسم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="252"/>
        <source>Value</source>
        <translation>القيمة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="253"/>
        <source>Category</source>
        <translation>الفئة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="357"/>
        <source>No constants match.</source>
        <translation>لا توجد ثوابت مطابقة.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="378"/>
        <source>%1 constants</source>
        <translation>%1 ثابت</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="379"/>
        <source>%1 of %2 constants</source>
        <translation>%1 من %2 ثابت</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="383"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
</context>
<context>
    <name>ControlScriptView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="33"/>
        <source>Control Script</source>
        <translation>سكريبت التحكم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="45"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="168"/>
        <source>Undo</source>
        <translation>تراجع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="52"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="179"/>
        <source>Redo</source>
        <translation>إعادة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="60"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="198"/>
        <source>Cut</source>
        <translation>قص</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="61"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="208"/>
        <source>Copy</source>
        <translation>نسخ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="62"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="218"/>
        <source>Paste</source>
        <translation>لصق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="67"/>
        <source>Select All</source>
        <translation>تحديد الكل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="77"/>
        <source>Format Document</source>
        <translation>تنسيق المستند</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="84"/>
        <source>Format Selection</source>
        <translation>تنسيق التحديد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="148"/>
        <source>Reset</source>
        <translation>إعادة تعيين</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="153"/>
        <source>Reset to the default control script</source>
        <translation>إعادة تعيين سكريبت التحكم إلى الافتراضي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="158"/>
        <source>Open</source>
        <translation>فتح</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="163"/>
        <source>Import a control script file</source>
        <translation>استيراد ملف سكريبت تحكم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="173"/>
        <source>Undo the last code edit</source>
        <translation>التراجع عن آخر تعديل للكود</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="185"/>
        <source>Redo the previously undone edit</source>
        <translation>إعادة التعديل الذي تم التراجع عنه</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="203"/>
        <source>Cut selected code to clipboard</source>
        <translation>قص الكود المحدد إلى الحافظة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="213"/>
        <source>Copy selected code to clipboard</source>
        <translation>نسخ الكود المحدد إلى الحافظة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="222"/>
        <source>Paste code from clipboard</source>
        <translation>لصق الكود من الحافظة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="236"/>
        <source>Help</source>
        <translation>مساعدة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="241"/>
        <source>Open the control script documentation</source>
        <translation>فتح توثيق سكريبت التحكم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="251"/>
        <source>Validate</source>
        <translation>التحقق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="255"/>
        <source>Verify that the script compiles correctly</source>
        <translation>التحقق من أن السكريبت يُترجم بشكل صحيح</translation>
    </message>
</context>
<context>
    <name>CrashRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="31"/>
        <source>Recovery Options</source>
        <translation>خيارات الاستعادة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="57"/>
        <source>Serial Studio has closed unexpectedly several times in a row.</source>
        <translation>تم إغلاق Serial Studio بشكل غير متوقع عدة مرات متتالية.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="82"/>
        <source>Consecutive crashes: %1</source>
        <translation>الأعطال المتتالية: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="90"/>
        <source>Last reported stage: %1</source>
        <translation>المرحلة الأخيرة المبلغ عنها: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="98"/>
        <source>Detected at: %1</source>
        <translation>تم الاكتشاف في: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="113"/>
        <source>Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.</source>
        <translation>اختر إجراء استعادة. سيتم إنهاء Serial Studio بعد تطبيقه ليبدأ التشغيل التالي بشكل نظيف.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="127"/>
        <source>Reset Rendering Backend to Default</source>
        <translation>إعادة تعيين محرك العرض إلى الوضع الافتراضي</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="135"/>
        <source>Skip Restoring the Last Opened Project</source>
        <translation>تخطي استعادة آخر مشروع تم فتحه</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="142"/>
        <source>Reset all Preferences</source>
        <translation>إعادة تعيين جميع التفضيلات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="160"/>
        <source>Continue Anyway</source>
        <translation>المتابعة على أي حال</translation>
    </message>
</context>
<context>
    <name>CsvPlayer</name>
    <message>
        <location filename="../../qml/Dialogs/CsvPlayer.qml" line="36"/>
        <source>CSV Player</source>
        <translation>مشغل CSV</translation>
    </message>
</context>
<context>
    <name>DBCPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="44"/>
        <source>DBC File Preview</source>
        <translation>معاينة ملف DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="169"/>
        <source>DBC File: %1</source>
        <translation>ملف DBC: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="177"/>
        <source>Review the CAN messages and signals to import into a new Serial Studio project.</source>
        <translation>مراجعة رسائل CAN والإشارات لاستيرادها إلى مشروع Serial Studio جديد.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="185"/>
        <source>Messages</source>
        <translation>الرسائل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="219"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="229"/>
        <source>Message Name</source>
        <translation>اسم الرسالة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="235"/>
        <source>CAN ID</source>
        <translation>معرف CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="242"/>
        <source>Signals</source>
        <translation>الإشارات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="323"/>
        <source>No messages found in DBC file.</source>
        <translation>لم يتم العثور على رسائل في ملف DBC.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="341"/>
        <source>Total: %1 messages, %2 signals</source>
        <translation>الإجمالي: %1 رسالة، %2 إشارة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="348"/>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="359"/>
        <source>Create Project</source>
        <translation>إنشاء مشروع</translation>
    </message>
</context>
<context>
    <name>Dashboard</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard.qml" line="127"/>
        <source>Dashboard %1</source>
        <translation>لوحة القيادة %1</translation>
    </message>
</context>
<context>
    <name>DashboardButton</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="40"/>
        <source>Send</source>
        <translation>إرسال</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="64"/>
        <source>No transmit function defined</source>
        <translation>لم يتم تعريف دالة إرسال</translation>
    </message>
</context>
<context>
    <name>DashboardCanvas</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="283"/>
        <source>Set Wallpaper…</source>
        <translation>تعيين خلفية…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="289"/>
        <source>Clear Wallpaper</source>
        <translation>مسح الخلفية</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="299"/>
        <source>Tile Windows</source>
        <translation>ترتيب النوافذ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="318"/>
        <source>Pro features detected in this project.</source>
        <translation>تم اكتشاف ميزات Pro في هذا المشروع.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="320"/>
        <source>Fallback widgets are active. Purchase a license for full functionality.</source>
        <translation>الودجات الاحتياطية نشطة. اشترِ ترخيصًا للحصول على الوظائف الكاملة.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="438"/>
        <source>Empty Workspace</source>
        <translation>مساحة العمل فارغة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="452"/>
        <source>Use the search bar to find and add widgets, or right-click a widget in another workspace to add it here.</source>
        <translation>استخدم شريط البحث للعثور على الودجات وإضافتها، أو انقر بزر الماوس الأيمن على ودجة في مساحة عمل أخرى لإضافتها هنا.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="467"/>
        <source>Search Widgets</source>
        <translation>البحث عن الودجات</translation>
    </message>
</context>
<context>
    <name>DashboardLayout</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="37"/>
        <source>Dashboard</source>
        <translation>لوحة المعلومات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="206"/>
        <source>API Server Active (%1)</source>
        <translation>خادم API نشط (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="207"/>
        <source>API Server Ready</source>
        <translation>خادم API جاهز</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="208"/>
        <source>API Server Off</source>
        <translation>خادم API متوقف</translation>
    </message>
</context>
<context>
    <name>DashboardOutputPanel</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="123"/>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="275"/>
        <source>Send</source>
        <translation>إرسال</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="263"/>
        <source>Enter command…</source>
        <translation>أدخل الأمر…</translation>
    </message>
</context>
<context>
    <name>DashboardSlider</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardSlider.qml" line="90"/>
        <source>No transmit function defined</source>
        <translation>لم يتم تعريف دالة الإرسال</translation>
    </message>
</context>
<context>
    <name>DashboardTextField</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="47"/>
        <source>Enter command…</source>
        <translation>أدخل الأمر…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="57"/>
        <source>Send</source>
        <translation>إرسال</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="76"/>
        <source>No transmit function defined</source>
        <translation>لم يتم تعريف دالة الإرسال</translation>
    </message>
</context>
<context>
    <name>DashboardToggle</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="57"/>
        <source>ON</source>
        <translation>تشغيل</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="59"/>
        <source>OFF</source>
        <translation>إيقاف</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="70"/>
        <source>No transmit function defined</source>
        <translation>لم يتم تعريف دالة الإرسال</translation>
    </message>
</context>
<context>
    <name>DataGrid</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="86"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="87"/>
        <source>Pause</source>
        <translation>إيقاف مؤقت</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="86"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="87"/>
        <source>Resume</source>
        <translation>استئناف</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="298"/>
        <source>Awaiting data…</source>
        <translation>في انتظار البيانات…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="352"/>
        <source>Open %1 in a separate window</source>
        <translation>فتح %1 في نافذة منفصلة</translation>
    </message>
</context>
<context>
    <name>DataModel::ControlScriptEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="282"/>
        <source>Select Javascript file to import</source>
        <translation>اختر ملف JavaScript للاستيراد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="348"/>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="359"/>
        <source>Code Validation Failed</source>
        <translation>فشل التحقق من صحة الكود</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="349"/>
        <source>Line %1: %2</source>
        <translation>السطر %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="360"/>
        <source>The script must define a setup() and/or loop() function.</source>
        <translation>يجب أن يُعرّف السكريبت دالة ()setup و/أو دالة ()loop.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="365"/>
        <source>Code Validation Successful</source>
        <translation>نجح التحقق من صحة الكود</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="366"/>
        <source>No syntax errors detected in the control script.</source>
        <translation>لم يتم اكتشاف أخطاء في بناء الجملة في سكريبت التحكم.</translation>
    </message>
</context>
<context>
    <name>DataModel::DBCImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>Import DBC File</source>
        <translation>استيراد ملف DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>DBC Files (*.dbc);;All Files (*)</source>
        <translation>ملفات DBC (*.DBC);;جميع الملفات (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="160"/>
        <source>Failed to parse DBC file: %1</source>
        <translation>فشل تحليل ملف DBC: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="161"/>
        <source>Verify the file format and try again.</source>
        <translation>تحقق من تنسيق الملف وحاول مرة أخرى.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="163"/>
        <source>DBC Import Error</source>
        <translation>خطأ في استيراد DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="171"/>
        <source>DBC file contains no messages</source>
        <translation>ملف DBC لا يحتوي على رسائل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="172"/>
        <source>The selected file does not contain any CAN message definitions.</source>
        <translation>الملف المحدد لا يحتوي على أي تعريفات رسائل CAN.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="174"/>
        <source>DBC Import Warning</source>
        <translation>تحذير استيراد DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="269"/>
        <source>Overview</source>
        <translation>نظرة عامة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="326"/>
        <source>Active</source>
        <translation>نشط</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">فشل تحميل المشروع المستورد</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">تعذر تحميل JSON للمشروع المُنشأ.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="218"/>
        <source>The project editor is now open for customization.</source>
        <translation>محرر المشروع مفتوح الآن للتخصيص.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="220"/>
        <source> Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); only simple multiplexing is supported.</source>
        <translation>تم تخطي %1 إشارة/إشارات تستخدم الإرسال المتعدد الموسع (SG_MUL_VAL_)؛ الإرسال المتعدد البسيط فقط مدعوم.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="225"/>
        <source>Successfully imported DBC file with %1 messages and %2 signals.</source>
        <translation>تم استيراد ملف DBC بنجاح مع %1 رسالة و %2 إشارة.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="230"/>
        <source>DBC Import Complete</source>
        <translation>اكتمل استيراد DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="254"/>
        <source>CAN Bus</source>
        <translation>ناقل CAN</translation>
    </message>
</context>
<context>
    <name>DataModel::DatasetTransformEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="59"/>
        <source>Dataset Value Transform</source>
        <translation>تحويل قيمة مجموعة البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="96"/>
        <source>Lua</source>
        <translation>Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="96"/>
        <source>JavaScript</source>
        <translation>JavaScript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="102"/>
        <source>Enter raw value (e.g., 1024)</source>
        <translation>أدخل القيمة الخام (مثال: 1024)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="107"/>
        <source>Test</source>
        <translation>اختبار</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="108"/>
        <source>Clear</source>
        <translation>مسح</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="110"/>
        <source>Apply</source>
        <translation>تطبيق</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="111"/>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="120"/>
        <source>Language:</source>
        <translation>اللغة:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="123"/>
        <source>Template:</source>
        <translation>القالب:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="134"/>
        <source>Input:</source>
        <translation>الإدخال:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="137"/>
        <source>Output:</source>
        <translation>الإخراج:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="215"/>
        <source>Transform — %1</source>
        <translation>التحويل — %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="297"/>
        <source>Enter a value</source>
        <translation>أدخل قيمة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="304"/>
        <source>Invalid number</source>
        <translation>رقم غير صالح</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="373"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>تنسيق المستند	Ctrl+Shift+I</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="374"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>تحديد التنسيق	Ctrl+I</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="485"/>
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
-- عرّف دالة transform(value) تستقبل قراءة مجموعة البيانات
-- المباشرة وتُرجع رقماً محوّلاً. إذا لم تُعرّف دالة
-- transform()، تُحفظ القيمة الخام ولا يُحفظ شيء مع المشروع.
--
-- مثال:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- المتغيرات العامة المُعلنة في المستوى الأعلى تستمر بين
-- الإطارات، وهذا مفيد للمرشحات والمراكمات وأي تحويل آخر
-- ذي حالة، تماماً مثل سكريبت محلل الإطارات:
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
-- استخدم قائمة القالب للحصول على أمثلة جاهزة، أو
-- انقر اختبار لتجربة دالتك.
--

</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="513"/>
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
 * عرّف دالة transform(value) تستقبل قراءة مجموعة البيانات
 * المباشرة وتُرجع رقماً محوّلاً. إذا لم تُعرّف دالة
 * transform()، تُحفظ القيمة الخام ولا يُحفظ شيء مع المشروع.
 *
 * مثال:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * المتغيرات العامة المُعلنة في المستوى الأعلى تستمر بين
 * الإطارات، وهذا مفيد للمرشحات والمراكمات وأي تحويل آخر
 * ذي حالة -- تماماً مثل سكريبت محلل الإطارات:
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
 * استخدم قائمة القالب للحصول على أمثلة جاهزة، أو
 * انقر اختبار لتجربة دالتك.
 */
</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="647"/>
        <source>Engine error</source>
        <translation>خطأ في المحرك</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="670"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="683"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="700"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="711"/>
        <source>Error: %1</source>
        <translation>خطأ: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="676"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="704"/>
        <source>Error: transform() not defined</source>
        <translation>خطأ: ()transform غير معرّفة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="688"/>
        <source>Error: transform() must return a number</source>
        <translation>خطأ: يجب أن تُرجع ()transform رقماً</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="756"/>
        <source>Select Template…</source>
        <translation>اختيار قالب…</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameBuilder</name>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1299"/>
        <source>JavaScript transform exceeded budget</source>
        <translation>تم تجاوز حد تحويل JavaScript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1300"/>
        <source>A dataset transform took longer than %1 ms; remaining datasets in the frame fell back to raw values until the next frame. Profile or simplify the transform code.</source>
        <translation>استغرق تحويل مجموعة بيانات وقتًا أطول من %1 مللي ثانية؛ تم استخدام القيم الخام للمجموعات المتبقية في الإطار حتى الإطار التالي. قم بتحليل أو تبسيط كود التحويل.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="199"/>
        <source>Frame pool exhausted</source>
        <translation>تم استنفاد مخزن الإطارات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="201"/>
        <source>A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API subscriber) is not draining frames fast enough. Serial Studio is falling back to per-frame allocations until the backlog clears. Disable a heavy consumer or reduce the data rate.</source>
        <translation>أحد المستهلكين في المسار (لوحة البيانات، تصدير CSV/MDF4، قاعدة بيانات الجلسات، أو مشترك API) لا يفرغ الإطارات بالسرعة الكافية. يقوم Serial Studio بالتحول إلى تخصيصات لكل إطار حتى يتم تصفية التراكم. قم بإيقاف مستهلك ثقيل أو خفّض معدل البيانات.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1531"/>
        <source>Device A</source>
        <translation>الجهاز أ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1570"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1684"/>
        <source>Channel %1</source>
        <translation>القناة %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1579"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1700"/>
        <source>Quick Plot</source>
        <translation>رسم بياني سريع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1586"/>
        <source>Quick Plot Data</source>
        <translation>بيانات الرسم البياني السريع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1598"/>
        <source>Multiple Plots</source>
        <translation>رسوم بيانية متعددة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1695"/>
        <source>Audio Input</source>
        <translation>دخل الصوت</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserModel</name>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Plain text (UTF-8)</source>
        <translation>نص عادي (UTF-8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Hexadecimal</source>
        <translation>سداسي عشري</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Binary (raw bytes)</source>
        <translation>ثنائي (بايتات خام)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="265"/>
        <source>End delimiter only</source>
        <translation>محدد النهاية فقط</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="266"/>
        <source>Start + end delimiters</source>
        <translation>محدد البداية + النهاية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="267"/>
        <source>Start delimiter only</source>
        <translation>محدد البداية فقط</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="268"/>
        <source>No delimiters (whole chunk)</source>
        <translation>بدون محددات (الجزء كاملاً)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="279"/>
        <source>No Checksum</source>
        <translation>بدون مجموع تدقيق</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="310"/>
        <source>Select Frame Parser Template</source>
        <translation>اختر قالب محلل الإطارات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="311"/>
        <source>Choose a template to load:</source>
        <translation>اختر قالباً لتحميله:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="494"/>
        <source>Invalid hexadecimal input.</source>
        <translation>إدخال سداسي عشري غير صالح.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="521"/>
        <source>No template selected.</source>
        <translation>لم يتم اختيار قالب.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="561"/>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation>تم استخراج %1 إطار | تم استهلاك %2 بايت | تم تخزين %3 بايت مؤقتًا | تم إسقاط %4</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="632"/>
        <source>Invalid JSON: %1</source>
        <translation>JSON غير صالح: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="728"/>
        <source>Parameters</source>
        <translation>المعاملات</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserTestDialog</name>
    <message>
        <source>None</source>
        <translation type="vanished">بدون</translation>
    </message>
    <message>
        <source>Invalid Hex Input</source>
        <translation type="vanished">إدخال HEX غير صالح</translation>
    </message>
    <message>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation type="vanished">الرجاء إدخال بايتات سداسية عشرية صالحة.

التنسيق الصالح: 01 A2 FF 3C</translation>
    </message>
    <message>
        <source>(no frames)</source>
        <translation type="vanished">(لا توجد إطارات)</translation>
    </message>
    <message>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation type="vanished">لم ينتج الاستخراج إطارًا كاملاً. تحقق من محددات البداية / النهاية ووضع الكشف.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">تم استخراج %1 إطار | تم استهلاك %2 بايت | تم تخزين %3 بايت مؤقتًا | تم إسقاط %4</translation>
    </message>
    <message>
        <source>Pipeline Configuration</source>
        <translation type="vanished">إعدادات خط المعالجة</translation>
    </message>
    <message>
        <source>Pipeline Results</source>
        <translation type="vanished">نتائج المسار</translation>
    </message>
    <message>
        <source>Detection</source>
        <translation type="vanished">الكشف</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">فك الترميز</translation>
    </message>
    <message>
        <source>Checksum</source>
        <translation type="vanished">المجموع الاختباري</translation>
    </message>
    <message>
        <source>Start Delimiter</source>
        <translation type="vanished">محدد البداية</translation>
    </message>
    <message>
        <source>End Delimiter</source>
        <translation type="vanished">محدد النهاية</translation>
    </message>
    <message>
        <source>Hex Delimiters</source>
        <translation type="vanished">المحددات السداسية عشرية</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">محدد النهاية فقط</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">محدد البداية + النهاية</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">محدد البداية فقط</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">بدون محددات (الجزء كاملاً)</translation>
    </message>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">نص عادي (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">سداسي عشري</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">ثنائي (بايتات خام)</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">مسح</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">تقييم</translation>
    </message>
    <message>
        <source>Enter raw stream bytes here...</source>
        <translation type="vanished">أدخل بايتات التدفق الخام هنا...</translation>
    </message>
    <message>
        <source>Stage</source>
        <translation type="vanished">مرحلة</translation>
    </message>
    <message>
        <source>Frame Data Input</source>
        <translation type="vanished">إدخال بيانات الإطار</translation>
    </message>
    <message>
        <source>Frame Parser Results</source>
        <translation type="vanished">نتائج محلل الإطارات</translation>
    </message>
    <message>
        <source>Enter frame data here…</source>
        <translation type="vanished">أدخل بيانات الإطار هنا…</translation>
    </message>
    <message>
        <source>Dataset Index</source>
        <translation type="vanished">فهرس مجموعة البيانات</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="vanished">القيمة</translation>
    </message>
    <message>
        <source>Enter frame data above, enable HEX mode if needed, then click "Evaluate" to run the frame parser.

Example (Text): a,b,c,d,e,f
Example (HEX):  48 65 6C 6C 6F</source>
        <translation type="vanished">أدخل بيانات الإطار أعلاه، فعّل وضع HEX إذا لزم الأمر، ثم انقر "تقييم" لتشغيل محلل الإطارات.

مثال (نص): a,b,c,d,e,f
مثال (HEX):  48 65 6C 6C 6F</translation>
    </message>
    <message>
        <source>Test Frame Parser</source>
        <translation type="vanished">اختبار محلل الإطارات</translation>
    </message>
    <message>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation type="vanished">أدخل بايتات سداسية عشرية (مثال: 01 A2 FF)</translation>
    </message>
    <message>
        <source>(empty)</source>
        <translation type="vanished">(فارغ)</translation>
    </message>
    <message>
        <source>No data returned</source>
        <translation type="vanished">لم يتم إرجاع بيانات</translation>
    </message>
</context>
<context>
    <name>DataModel::JsCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="218"/>
        <source>Change Scripting Language?</source>
        <translation>تغيير لغة البرمجة النصية؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="219"/>
        <source>Switching the scripting language replaces the current parser code with the equivalent template in the new language.

Any unsaved changes are lost. Continue?</source>
        <translation>تبديل لغة البرمجة النصية يستبدل كود المحلل الحالي بالقالب المكافئ في اللغة الجديدة.

سيتم فقدان أي تغييرات غير محفوظة. هل تريد المتابعة؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="381"/>
        <source>Select Lua file to import</source>
        <translation>اختر ملف Lua للاستيراد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="381"/>
        <source>Select Javascript file to import</source>
        <translation>اختر ملف Javascript للاستيراد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="413"/>
        <source>Code Validation Successful</source>
        <translation>نجح التحقق من صحة الكود</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="414"/>
        <source>No syntax errors detected in the parser code.</source>
        <translation>لم يتم اكتشاف أخطاء في بناء جملة كود المحلل.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="524"/>
        <source>Select Frame Parser Template</source>
        <translation>اختر قالب محلل الإطارات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="525"/>
        <source>Choose a template to load:</source>
        <translation>اختر قالباً لتحميله:</translation>
    </message>
</context>
<context>
    <name>DataModel::ModbusMapImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="299"/>
        <source>Import Modbus Register Map</source>
        <translation>استيراد خريطة سجلات Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="303"/>
        <source>Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files (*.xml);;JSON Files (*.json);;All Files (*)</source>
        <translation>خرائط سجلات Modbus (*.CSV *.XML *.JSON);;ملفات CSV (*.CSV);;ملفات XML (*.XML);;ملفات JSON (*.JSON);;جميع الملفات (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="340"/>
        <source>No registers found</source>
        <translation>لم يتم العثور على سجلات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="341"/>
        <source>The file could not be parsed or contains no register definitions.</source>
        <translation>لا يمكن تحليل الملف أو أنه لا يحتوي على تعريفات سجلات.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="343"/>
        <source>Modbus Import</source>
        <translation>استيراد Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="787"/>
        <source>Overview</source>
        <translation>نظرة عامة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="813"/>
        <source>On</source>
        <translation>مُشغّل</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">فشل تحميل المشروع المستورد</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">تعذر تحميل JSON المشروع المُنشأ.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="388"/>
        <source>Successfully imported %1 registers in %2 groups.</source>
        <translation>تم استيراد %1 سجل في %2 مجموعة بنجاح.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="390"/>
        <source>The project editor is now open for customization.</source>
        <translation>محرر المشروع مفتوح الآن للتخصيص.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="392"/>
        <source>Modbus Import Complete</source>
        <translation>اكتمل استيراد Modbus</translation>
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
        <translation type="vanished">نص عادي (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">سداسي عشري</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">ثنائي (بايتات خام)</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">محدد النهاية فقط</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">محدد البداية + النهاية</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">محدد البداية فقط</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">بدون محددات (الجزء كاملاً)</translation>
    </message>
    <message>
        <source>No Checksum</source>
        <translation type="vanished">بدون مجموع تدقيق</translation>
    </message>
    <message>
        <source>Invalid hexadecimal input.</source>
        <translation type="vanished">إدخال سداسي عشري غير صالح.</translation>
    </message>
    <message>
        <source>No template selected.</source>
        <translation type="vanished">لم يتم تحديد قالب.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">تم استخراج %1 إطار | تم استهلاك %2 بايت | تم تخزين %3 بايت مؤقتًا | تم إسقاط %4</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation type="vanished">المعاملات</translation>
    </message>
</context>
<context>
    <name>DataModel::OutputCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="288"/>
        <source>Select Javascript file to import</source>
        <translation>اختيار ملف JavaScript للاستيراد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="346"/>
        <source>Select Output Widget Template</source>
        <translation>اختيار قالب عنصر الإخراج</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="347"/>
        <source>Choose a template to load:</source>
        <translation>اختر قالبًا للتحميل:</translation>
    </message>
</context>
<context>
    <name>DataModel::PainterCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="300"/>
        <source>Select Javascript file to import</source>
        <translation>حدد ملف JavaScript للاستيراد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="387"/>
        <source>Select Painter Widget Template</source>
        <translation>حدد قالب عنصر الرسام</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="388"/>
        <source>Choose a template to load:</source>
        <translation>اختر قالبًا للتحميل:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="432"/>
        <source>Add datasets for this template?</source>
        <translation>إضافة مجموعات بيانات لهذا القالب؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="433"/>
        <source>"%1" expects %2 dataset(s); the current group has %3.

Add %4 dataset(s) using the template's defaults?</source>
        <translation>يتوقع "%1" %2 مجموعة بيانات؛ المجموعة الحالية تحتوي على %3.

إضافة %4 مجموعة بيانات باستخدام إعدادات القالب الافتراضية؟</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectEditor</name>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1289"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1292"/>
        <source>Frame Parser</source>
        <translation>محلل الإطارات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1431"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1432"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1474"/>
        <source>Groups</source>
        <translation>المجموعات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1504"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1517"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1518"/>
        <source>Shared Memory</source>
        <translation>الذاكرة المشتركة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1504"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1523"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1524"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4959"/>
        <source>Dataset Values</source>
        <translation>قيم مجموعة البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1566"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1579"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1580"/>
        <source>Workspaces</source>
        <translation>مساحات العمل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1618"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1621"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1622"/>
        <source>MQTT Publisher</source>
        <translation>ناشر MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1642"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1645"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1646"/>
        <source>Control Script</source>
        <translation>سكريبت التحكم</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1695"/>
        <source>Publishing</source>
        <translation>النشر</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1705"/>
        <source>Enable Publishing</source>
        <translation>تمكين النشر</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1706"/>
        <source>Broadcast frames, raw bytes and notifications to the broker</source>
        <translation>بث الإطارات والبايتات الخام والإشعارات إلى الوسيط</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1717"/>
        <source>Payload</source>
        <translation>الحمولة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1718"/>
        <source>Selects what gets published: parsed dashboard data or raw RX bytes</source>
        <translation>يحدد ما يتم نشره: بيانات لوحة المعلومات المحللة أو بايتات الاستقبال الخام</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1728"/>
        <source>Publish Rate (Hz)</source>
        <translation>معدل النشر (Hz)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1729"/>
        <source>How many times per second to publish (1-30 Hz). Higher rates increase broker load; dashboard data is rate-limited so a slow broker never blocks frame parsing.</source>
        <translation>عدد مرات النشر في الثانية (1-30 Hz). المعدلات الأعلى تزيد الحمل على الوسيط؛ بيانات لوحة المعلومات محدودة المعدل لذا الوسيط البطيء لا يعطل تحليل الإطارات أبدًا.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1741"/>
        <source>Topic Base</source>
        <translation>قاعدة الموضوع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1742"/>
        <source>serial-studio/device</source>
        <translation>serial-studio/device</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1743"/>
        <source>Base topic used for frame and raw-byte publishing</source>
        <translation>الموضوع الأساسي المستخدم لنشر الإطارات والبايتات الخام</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1753"/>
        <source>Script Topic</source>
        <translation>موضوع السكريبت</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1754"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1778"/>
        <source>Defaults to Topic Base when empty</source>
        <translation>يستخدم الموضوع الأساسي افتراضيًا عند تركه فارغًا</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1755"/>
        <source>Topic the user script publishes to</source>
        <translation>الموضوع الذي ينشر إليه سكريبت المستخدم</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1765"/>
        <source>Publish Notifications</source>
        <translation>نشر الإشعارات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1766"/>
        <source>Mirror dashboard notifications to a dedicated topic</source>
        <translation>عكس إشعارات لوحة المعلومات إلى موضوع مخصص</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1777"/>
        <source>Notification Topic</source>
        <translation>موضوع الإشعارات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1779"/>
        <source>Topic where dashboard notifications are mirrored</source>
        <translation>الموضوع الذي تُعكس إليه إشعارات لوحة المعلومات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1792"/>
        <source>Broker</source>
        <translation>Broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1802"/>
        <source>Hostname</source>
        <translation>اسم المضيف</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1803"/>
        <source>broker.hivemq.com</source>
        <translation>broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1804"/>
        <source>Hostname or IP address of the MQTT broker</source>
        <translation>اسم المضيف أو عنوان IP الخاص بوسيط MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1813"/>
        <source>Port</source>
        <translation>المنفذ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1814"/>
        <source>TCP port exposed by the broker (1883 plain, 8883 TLS)</source>
        <translation>منفذ TCP المكشوف بواسطة الوسيط (1883 عادي، 8883 TLS)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1824"/>
        <source>Custom Client ID</source>
        <translation>معرّف العميل المخصص</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1826"/>
        <source>Off: a fresh random id is generated on every project load. On: use the id below.</source>
        <translation>إيقاف: يتم إنشاء معرّف عشوائي جديد عند كل تحميل للمشروع. تشغيل: استخدام المعرّف أدناه.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1837"/>
        <source>Client ID</source>
        <translation>معرّف العميل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1838"/>
        <source>Identifier sent to the broker on CONNECT</source>
        <translation>المعرّف المُرسل إلى الوسيط عند CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1851"/>
        <source>Protocol Version</source>
        <translation>إصدار البروتوكول</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1852"/>
        <source>MQTT protocol revision used on CONNECT</source>
        <translation>إصدار بروتوكول MQTT المستخدم عند الاتصال (CONNECT)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1861"/>
        <source>Keep Alive (s)</source>
        <translation>الإبقاء على الاتصال (ث)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1862"/>
        <source>Seconds between PINGREQ packets when idle</source>
        <translation>الثواني بين حزم PINGREQ عند عدم النشاط</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1871"/>
        <source>Clean Session</source>
        <translation>جلسة نظيفة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1872"/>
        <source>Discard any persistent session state on CONNECT</source>
        <translation>تجاهل أي حالة جلسة مستمرة عند الاتصال (CONNECT)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1887"/>
        <source>Username</source>
        <translation>اسم المستخدم</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1888"/>
        <source>Username for broker authentication (leave empty for anonymous)</source>
        <translation>اسم المستخدم للمصادقة مع الوسيط (اتركه فارغًا للدخول المجهول)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1898"/>
        <source>Password</source>
        <translation>كلمة المرور</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1899"/>
        <source>Password for broker authentication</source>
        <translation>كلمة المرور للمصادقة مع الوسيط</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1910"/>
        <source>SSL / TLS</source>
        <translation>SSL / TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1920"/>
        <source>Use SSL/TLS</source>
        <translation>استخدام SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1921"/>
        <source>Tunnel the broker connection over TLS</source>
        <translation>نفق اتصال الوسيط عبر TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1934"/>
        <source>Protocol</source>
        <translation>البروتوكول</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1935"/>
        <source>Negotiated TLS protocol family</source>
        <translation>عائلة بروتوكول TLS المتفاوض عليها</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1945"/>
        <source>Peer Verify</source>
        <translation>التحقق من النظير</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1946"/>
        <source>How strictly the broker's certificate chain is validated</source>
        <translation>مدى صرامة التحقق من سلسلة شهادات الوسيط</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1956"/>
        <source>Verify Depth</source>
        <translation>عمق التحقق</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1957"/>
        <source>Maximum certificate chain length accepted (0 = unlimited)</source>
        <translation>الحد الأقصى لطول سلسلة الشهادات المقبول (0 = غير محدود)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2169"/>
        <source>Project Information</source>
        <translation>معلومات المشروع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2179"/>
        <source>Project Title</source>
        <translation>عنوان المشروع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2180"/>
        <source>Untitled Project</source>
        <translation>مشروع بدون عنوان</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2181"/>
        <source>Name or description of the project</source>
        <translation>اسم أو وصف المشروع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2199"/>
        <source>Group Information</source>
        <translation>معلومات المجموعة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2209"/>
        <source>Group Title</source>
        <translation>عنوان المجموعة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2210"/>
        <source>Untitled Group</source>
        <translation>مجموعة بدون عنوان</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2211"/>
        <source>Title or description of this dataset group</source>
        <translation>عنوان أو وصف مجموعة مجموعات البيانات هذه</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2226"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Device %1</source>
        <translation>جهاز %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2244"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2363"/>
        <source>Input Device</source>
        <translation>جهاز الإدخال</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2245"/>
        <source>Select which connected device provides data for this group</source>
        <translation>اختيار الجهاز المتصل الذي يوفر البيانات لهذه المجموعة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2261"/>
        <source>Image Configuration</source>
        <translation>إعدادات الصورة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2274"/>
        <source>Detection Mode</source>
        <translation>وضع الكشف</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2276"/>
        <source>Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences</source>
        <translation>الكشف التلقائي يقرأ البايتات السحرية لـ JPEG/PNG؛ اليدوي يستخدم تسلسلات بداية/نهاية صريحة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2286"/>
        <source>Start Sequence (Hex)</source>
        <translation>تسلسل البداية (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2287"/>
        <source>e.g. FF D8 FF</source>
        <translation>مثال: FF D8 FF</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2288"/>
        <source>Hex bytes marking the start of an image frame</source>
        <translation>بايتات سداسية عشرية تحدد بداية إطار الصورة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2297"/>
        <source>End Sequence (Hex)</source>
        <translation>تسلسل النهاية (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2298"/>
        <source>e.g. FF D9</source>
        <translation>مثال: FF D9</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2299"/>
        <source>Hex bytes marking the end of an image frame</source>
        <translation>بايتات سداسية عشرية تحدد نهاية إطار الصورة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2342"/>
        <source>Composite Widget</source>
        <translation>عنصر واجهة مركب</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2343"/>
        <source>Select how this group of datasets should be visualized (optional)</source>
        <translation>اختر كيفية عرض مجموعة مجموعات البيانات هذه (اختياري)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2373"/>
        <source>Device Name</source>
        <translation>اسم الجهاز</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2374"/>
        <source>Device 1</source>
        <translation>الجهاز 1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2375"/>
        <source>Human-readable name for this input device</source>
        <translation>اسم مقروء لجهاز الإدخال هذا</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2384"/>
        <source>Bus Type</source>
        <translation>نوع الناقل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2385"/>
        <source>Select the hardware interface for this input device</source>
        <translation>اختر واجهة الأجهزة لجهاز الإدخال هذا</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2387"/>
        <source>Serial Port</source>
        <translation>المنفذ التسلسلي</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2387"/>
        <source>Network Socket</source>
        <translation>مقبس الشبكة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2387"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2389"/>
        <source>Audio Input</source>
        <translation>إدخال الصوت</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2389"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2389"/>
        <source>CAN Bus</source>
        <translation>ناقل CAN</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2389"/>
        <source>Raw USB</source>
        <translation>USB الخام</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2390"/>
        <source>HID Device</source>
        <translation>جهاز HID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2390"/>
        <source>Process</source>
        <translation>العملية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2390"/>
        <source>MQTT Subscriber</source>
        <translation>مشترك MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2404"/>
        <source>Frame Detection</source>
        <translation>كشف الإطار</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2418"/>
        <source>Frame Detection Method</source>
        <translation>طريقة كشف الإطار</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2419"/>
        <source>Select how incoming data frames are identified</source>
        <translation>تحديد كيفية التعرف على إطارات البيانات الواردة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2429"/>
        <source>Hexadecimal Delimiters</source>
        <translation>محددات سداسية عشرية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2430"/>
        <source>Enter frame start/end sequences as hexadecimal values</source>
        <translation>إدخال تسلسلات بداية/نهاية الإطار كقيم سداسية عشرية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2446"/>
        <source>Frame Start Delimiter</source>
        <translation>محدد بداية الإطار</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2447"/>
        <source>e.g. /*</source>
        <translation>مثال: /*</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2448"/>
        <source>Sequence that marks the beginning of a data frame</source>
        <translation>التسلسل الذي يحدد بداية إطار البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2460"/>
        <source>Frame End Delimiter</source>
        <translation>محدد نهاية الإطار</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2461"/>
        <source>e.g. */</source>
        <translation>مثال: */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2462"/>
        <source>Sequence that marks the end of a data frame</source>
        <translation>التسلسل الذي يحدد نهاية إطار البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2468"/>
        <source>Payload Processing &amp; Validation</source>
        <translation>معالجة الحمولة والتحقق من الصحة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2479"/>
        <source>Data Conversion Method</source>
        <translation>طريقة تحويل البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2480"/>
        <source>Select how incoming binary data is decoded before parsing</source>
        <translation>اختر كيفية فك تشفير البيانات الثنائية الواردة قبل التحليل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2496"/>
        <source>Checksum Algorithm</source>
        <translation>خوارزمية المجموع الاختباري</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2497"/>
        <source>Select the checksum algorithm used to validate frames</source>
        <translation>حدد خوارزمية المجموع الاختباري المستخدمة للتحقق من صحة الإطارات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2545"/>
        <source>Connection Settings</source>
        <translation>إعدادات الاتصال</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2782"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3052"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4655"/>
        <source>General Information</source>
        <translation>معلومات عامة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2791"/>
        <source>Action Title</source>
        <translation>عنوان الإجراء</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2793"/>
        <source>Untitled Action</source>
        <translation>إجراء بدون عنوان</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2794"/>
        <source>Name or description of this action</source>
        <translation>اسم أو وصف هذا الإجراء</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2803"/>
        <source>Action Icon</source>
        <translation>أيقونة الإجراء</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2804"/>
        <source>Default Icon</source>
        <translation>الأيقونة الافتراضية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2805"/>
        <source>Icon displayed for this action in the dashboard</source>
        <translation>الأيقونة المعروضة لهذا الإجراء في لوحة المعلومات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2832"/>
        <source>Target Device</source>
        <translation>الجهاز المستهدف</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2833"/>
        <source>Select which connected device this action sends data to</source>
        <translation>حدد الجهاز المتصل الذي سيرسل إليه هذا الإجراء البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2845"/>
        <source>Data Payload</source>
        <translation>حمولة البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2856"/>
        <source>Send as Binary</source>
        <translation>إرسال كبيانات ثنائية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2857"/>
        <source>Send raw binary data when this action is triggered</source>
        <translation>إرسال بيانات ثنائية خام عند تفعيل هذا الإجراء</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2868"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2880"/>
        <source>Command</source>
        <translation>الأمر</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2869"/>
        <source>Transmit Data (Hex)</source>
        <translation>إرسال البيانات (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2870"/>
        <source>Hexadecimal payload to send when the action is triggered</source>
        <translation>الحمولة السداسية عشرية المراد إرسالها عند تفعيل الإجراء</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2881"/>
        <source>Transmit Data</source>
        <translation>إرسال البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2882"/>
        <source>Text payload to send when the action is triggered</source>
        <translation>الحمولة النصية المراد إرسالها عند تفعيل الإجراء</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2893"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4714"/>
        <source>Text Encoding</source>
        <translation>ترميز النص</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2894"/>
        <source>Character encoding used to serialize the text payload</source>
        <translation>ترميز الأحرف المستخدم لتسلسل الحمولة النصية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2918"/>
        <source>End-of-Line Sequence</source>
        <translation>تسلسل نهاية السطر</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2919"/>
        <source>EOL characters to append to the message (e.g. \n, \r\n)</source>
        <translation>أحرف نهاية السطر المُلحقة بالرسالة (مثل </translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2931"/>
        <source>Execution Behavior</source>
        <translation>سلوك التنفيذ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2942"/>
        <source>Auto-Execute on Connect</source>
        <translation>التنفيذ التلقائي عند الاتصال</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2943"/>
        <source>Automatically trigger this action when the device connects</source>
        <translation>تشغيل هذا الإجراء تلقائيًا عند اتصال الجهاز</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2949"/>
        <source>Timer Behavior</source>
        <translation>سلوك المؤقت</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2958"/>
        <source>Timer Mode</source>
        <translation>وضع المؤقت</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2961"/>
        <source>Choose when and how this action should repeat automatically</source>
        <translation>اختيار متى وكيف يتكرر هذا الإجراء تلقائيًا</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2968"/>
        <source>Interval (ms)</source>
        <translation>الفاصل الزمني (ميلي ثانية)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2972"/>
        <source>Timer Interval (ms)</source>
        <translation>الفاصل الزمني للمؤقت (مللي ثانية)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2973"/>
        <source>Milliseconds between each repeated trigger of this action</source>
        <translation>المللي ثانية بين كل تشغيل متكرر لهذا الإجراء</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2980"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2984"/>
        <source>Repeat Count</source>
        <translation>عدد التكرارات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2985"/>
        <source>Number of times to send the command on each trigger</source>
        <translation>عدد مرات إرسال الأمر عند كل تشغيل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3062"/>
        <source>Untitled Dataset</source>
        <translation>مجموعة بيانات بدون عنوان</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3063"/>
        <source>Dataset Title</source>
        <translation>عنوان مجموعة البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3064"/>
        <source>Name of the dataset, used for labeling and identification</source>
        <translation>اسم مجموعة البيانات، يُستخدم للتسمية والتعريف</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3076"/>
        <source>Virtual Dataset</source>
        <translation>مجموعة بيانات افتراضية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3077"/>
        <source>Virtual datasets compute their value from transforms and data tables, they do not require a frame index</source>
        <translation>مجموعات البيانات الافتراضية تحسب قيمتها من التحويلات وجداول البيانات، ولا تتطلب فهرس إطار</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3093"/>
        <source>Hide on Dashboard</source>
        <translation>إخفاء في لوحة المعلومات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3094"/>
        <source>Suppress this dataset's standalone dashboard tile; the painter widget can still read its values</source>
        <translation>إخفاء بلاطة لوحة المعلومات المستقلة لمجموعة البيانات هذه؛ يمكن لعنصر الرسم قراءة قيمها</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3107"/>
        <source>Frame Index</source>
        <translation>فهرس الإطار</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3108"/>
        <source>Frame position used for aligning datasets in time</source>
        <translation>موضع الإطار المستخدم لمحاذاة مجموعات البيانات زمنياً</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3117"/>
        <source>Measurement Unit</source>
        <translation>وحدة القياس</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3118"/>
        <source>Volts, Amps, etc.</source>
        <translation>فولت، أمبير، إلخ.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3119"/>
        <source>Unit of measurement, such as volts or amps (optional)</source>
        <translation>وحدة القياس، مثل الفولت أو الأمبير (اختياري)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3140"/>
        <source>Lower bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>الحد الأدنى لنطاق قيمة مجموعة البيانات؛ تعود إليه الأدوات وFFT عندما يُترك نطاقها الخاص غير محدد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3153"/>
        <source>Upper bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>الحد الأقصى لنطاق قيمة مجموعة البيانات؛ تعود إليه الأدوات وFFT عندما يُترك نطاقها الخاص غير محدد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3166"/>
        <source>Plot Settings</source>
        <translation>إعدادات الرسم البياني</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3189"/>
        <source>Enable Plot Widget</source>
        <translation>تفعيل عنصر الرسم البياني</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3191"/>
        <source>Plot data in real-time</source>
        <translation>رسم البيانات في الوقت الفعلي</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3210"/>
        <source>X-Axis Source</source>
        <translation>مصدر المحور X</translation>
    </message>
    <message>
        <source>Choose which dataset to use for the X-Axis in plots</source>
        <translation type="vanished">اختر مجموعة البيانات المستخدمة للمحور السيني في الرسوم البيانية</translation>
    </message>
    <message>
        <source>Minimum Plot Value (optional)</source>
        <translation type="vanished">القيمة الدنيا للرسم البياني (اختياري)</translation>
    </message>
    <message>
        <source>Lower bound for plot display range</source>
        <translation type="vanished">الحد الأدنى لنطاق عرض الرسم البياني</translation>
    </message>
    <message>
        <source>Maximum Plot Value (optional)</source>
        <translation type="vanished">القيمة القصوى للرسم البياني (اختياري)</translation>
    </message>
    <message>
        <source>Upper bound for plot display range</source>
        <translation type="vanished">الحد الأقصى لنطاق عرض الرسم البياني</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3224"/>
        <source>Frequency Analysis</source>
        <translation>تحليل التردد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3235"/>
        <source>Enable FFT Analysis</source>
        <translation>تفعيل تحليل FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3236"/>
        <source>Perform frequency-domain analysis of the dataset</source>
        <translation>إجراء تحليل المجال الترددي لمجموعة البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3246"/>
        <source>Enable Waterfall Plot</source>
        <translation>تفعيل رسم الشلال البياني</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3247"/>
        <source>Show a scrolling spectrogram of frequency content over time (Pro)</source>
        <translation>عرض مخطط طيفي متحرك لمحتوى التردد عبر الزمن (Pro)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3270"/>
        <source>Waterfall Y Axis</source>
        <translation>محور Y الشلالي</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3271"/>
        <source>Choose Time (default) or any dataset whose value drives the Y axis -- produces a Campbell diagram when bound to e.g. RPM</source>
        <translation>اختر الوقت (افتراضي) أو أي مجموعة بيانات تتحكم قيمتها في محور Y -- ينتج مخطط Campbell عند الربط بـ RPM مثلاً</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3298"/>
        <source>FFT Window Size</source>
        <translation>حجم نافذة FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3299"/>
        <source>Number of samples used for each FFT calculation window</source>
        <translation>عدد العينات المستخدمة لكل نافذة حساب FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3310"/>
        <source>FFT Sampling Rate (Hz, required)</source>
        <translation>معدل عينات FFT (Hz، مطلوب)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3311"/>
        <source>Sampling frequency used for FFT (in Hz)</source>
        <translation>تردد العينات المستخدم لـ FFT (بالـ Hz)</translation>
    </message>
    <message>
        <source>Minimum Value (recommended)</source>
        <translation type="vanished">القيمة الدنيا (موصى به)</translation>
    </message>
    <message>
        <source>Lower bound for data normalization</source>
        <translation type="vanished">الحد الأدنى لتطبيع البيانات</translation>
    </message>
    <message>
        <source>Maximum Value (recommended)</source>
        <translation type="vanished">القيمة القصوى (موصى به)</translation>
    </message>
    <message>
        <source>Upper bound for data normalization</source>
        <translation type="vanished">الحد الأقصى لتطبيع البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3360"/>
        <source>Widget Settings</source>
        <translation>إعدادات الأداة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3383"/>
        <source>Widget</source>
        <translation>الأداة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3384"/>
        <source>Select the visual widget used to display this dataset</source>
        <translation>اختر الأداة المرئية المستخدمة لعرض مجموعة البيانات هذه</translation>
    </message>
    <message>
        <source>Minimum Display Value (required)</source>
        <translation type="vanished">الحد الأدنى لقيمة العرض (مطلوب)</translation>
    </message>
    <message>
        <source>Lower bound of the gauge or bar display range</source>
        <translation type="vanished">الحد الأدنى لنطاق عرض المقياس أو الشريط</translation>
    </message>
    <message>
        <source>Maximum Display Value (required)</source>
        <translation type="vanished">الحد الأقصى لقيمة العرض (مطلوب)</translation>
    </message>
    <message>
        <source>Upper bound of the gauge or bar display range</source>
        <translation type="vanished">الحد الأقصى لنطاق عرض المقياس أو الشريط</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3440"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3621"/>
        <source>Auto</source>
        <translation>تلقائي</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3441"/>
        <source>Tick Count</source>
        <translation>عدد العلامات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3445"/>
        <source>Major-tick count on the dial scale (0 = auto-fit to widget size)</source>
        <translation>عدد العلامات الرئيسية على مقياس القرص (0 = ملاءمة تلقائية لحجم الواجهة)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3464"/>
        <source>Label Format</source>
        <translation>تنسيق التسمية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3465"/>
        <source>Decimal places or notation used on tick labels and the value display</source>
        <translation>عدد المنازل العشرية أو الترميز المستخدم على تسميات العلامات وعرض القيمة</translation>
    </message>
    <message>
        <source>Show Value Indicator</source>
        <translation type="vanished">إظهار مؤشر القيمة</translation>
    </message>
    <message>
        <source>Toggle the boxed numeric readout that sits below or beside the widget</source>
        <translation type="vanished">تبديل العرض الرقمي المؤطر الذي يظهر أسفل أو بجانب الواجهة</translation>
    </message>
    <message>
        <source>Alarm Settings</source>
        <translation type="vanished">إعدادات الإنذار</translation>
    </message>
    <message>
        <source>Enable Alarms</source>
        <translation type="vanished">تفعيل الإنذارات</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds alarm thresholds</source>
        <translation type="vanished">يُطلق إنذاراً مرئياً عندما تتجاوز القيمة عتبات الإنذار</translation>
    </message>
    <message>
        <source>Low Threshold</source>
        <translation type="vanished">الحد الأدنى</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value drops below this threshold</source>
        <translation type="vanished">يُفعّل تنبيهاً بصرياً عندما تنخفض القيمة عن هذا الحد</translation>
    </message>
    <message>
        <source>High Threshold</source>
        <translation type="vanished">الحد الأقصى</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds this threshold</source>
        <translation type="vanished">يُفعّل تنبيهاً بصرياً عندما تتجاوز القيمة هذا الحد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3526"/>
        <source>LED Display Settings</source>
        <translation>إعدادات شاشة LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3537"/>
        <source>Show in LED Panel</source>
        <translation>إظهار في لوحة LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3538"/>
        <source>Enable visual status monitoring using an LED display</source>
        <translation>تفعيل مراقبة الحالة البصرية باستخدام شاشة LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3552"/>
        <source>LED On Threshold (required)</source>
        <translation>حد تشغيل LED (مطلوب)</translation>
    </message>
    <message>
        <source>LED lights up when value meets or exceeds this threshold</source>
        <translation type="vanished">يضيء LED عندما تساوي القيمة هذا الحد أو تتجاوزه</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3573"/>
        <source>Off</source>
        <translation>إيقاف</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3573"/>
        <source>Auto Start</source>
        <translation>بدء تلقائي</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3573"/>
        <source>Start on Trigger</source>
        <translation>بدء عند المشغل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3573"/>
        <source>Toggle on Trigger</source>
        <translation>تبديل عند المشغل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3574"/>
        <source>Repeat N Times</source>
        <translation>تكرار N مرة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3577"/>
        <source>Plain Text (UTF8)</source>
        <translation>نص عادي (UTF8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3577"/>
        <source>Hexadecimal</source>
        <translation>سداسي عشري</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3577"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3578"/>
        <source>Binary (Direct)</source>
        <translation>ثنائي (مباشر)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3583"/>
        <source>No Checksum</source>
        <translation>بدون مجموع تدقيق</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3587"/>
        <source>End Delimiter Only</source>
        <translation>محدد النهاية فقط</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3587"/>
        <source>Start Delimiter Only</source>
        <translation>محدد البداية فقط</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3588"/>
        <source>Start + End Delimiter</source>
        <translation>محدد البداية + النهاية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3588"/>
        <source>No Delimiters</source>
        <translation>بدون محددات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3595"/>
        <source>Auto-detect</source>
        <translation>كشف تلقائي</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3595"/>
        <source>Manual Delimiters</source>
        <translation>محددات يدوية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3598"/>
        <source>Button</source>
        <translation>زر</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3598"/>
        <source>Slider</source>
        <translation>شريط تمرير</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3598"/>
        <source>Toggle</source>
        <translation>مفتاح تبديل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3598"/>
        <source>Text Field</source>
        <translation>حقل نص</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3599"/>
        <source>Knob</source>
        <translation>مقبض دوار</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3603"/>
        <source>Data Grid</source>
        <translation>شبكة البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3604"/>
        <source>GPS Map</source>
        <translation>خريطة GPS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3605"/>
        <source>Gyroscope</source>
        <translation>جيروسكوب</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3606"/>
        <source>Multiple Plot</source>
        <translation>رسم بياني متعدد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3607"/>
        <source>Accelerometer</source>
        <translation>مقياس التسارع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3608"/>
        <source>3D Plot</source>
        <translation>رسم بياني ثلاثي الأبعاد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3609"/>
        <source>Image View</source>
        <translation>عارض الصور</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3610"/>
        <source>Painter Widget</source>
        <translation>أداة الرسم</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3611"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3614"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3629"/>
        <source>None</source>
        <translation>بدون</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3615"/>
        <source>Bar</source>
        <translation>أعمدة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3616"/>
        <source>Gauge</source>
        <translation>مقياس</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3617"/>
        <source>Compass</source>
        <translation>بوصلة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3618"/>
        <source>Meter</source>
        <translation>عداد</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">مقياس حرارة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3622"/>
        <source>Integer (0 decimals)</source>
        <translation>عدد صحيح (0 منازل عشرية)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3623"/>
        <source>1 decimal</source>
        <translation>منزلة عشرية واحدة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3624"/>
        <source>2 decimals</source>
        <translation>منزلتان عشريتان</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3625"/>
        <source>3 decimals</source>
        <translation>3 منازل عشرية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3626"/>
        <source>Scientific</source>
        <translation>علمي</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3630"/>
        <source>New Line (\n)</source>
        <translation>سطر جديد (</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3631"/>
        <source>Carriage Return (\r)</source>
        <translation>إرجاع الحرف (\r)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3632"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3635"/>
        <source>No</source>
        <translation>لا</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3636"/>
        <source>Yes</source>
        <translation>نعم</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4665"/>
        <source>Label</source>
        <translation>تسمية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4666"/>
        <source>Display label</source>
        <translation>عرض التسمية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4676"/>
        <source>Button Icon</source>
        <translation>أيقونة الزر</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4685"/>
        <source>Colorize Icon</source>
        <translation>تلوين الأيقونة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4686"/>
        <source>Tint the icon with the button color</source>
        <translation>صبغ الأيقونة بلون الزر</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4703"/>
        <source>Initial Value</source>
        <translation>القيمة الابتدائية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4715"/>
        <source>Character encoding used when transmit() returns a string value</source>
        <translation>ترميز الأحرف المستخدم عندما ترجع ()transmit قيمة نصية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4733"/>
        <source>Value Range</source>
        <translation>نطاق القيمة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3139"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4743"/>
        <source>Minimum Value</source>
        <translation>القيمة الدنيا</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3152"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4752"/>
        <source>Maximum Value</source>
        <translation>القيمة القصوى</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3211"/>
        <source>Choose Time or a dataset to drive the X-Axis in plots</source>
        <translation>اختر الوقت أو مجموعة بيانات لتشغيل المحور السيني في الرسوم البيانية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3321"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3410"/>
        <source>Minimum Value (optional)</source>
        <translation>القيمة الدنيا (اختياري)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3322"/>
        <source>Lower bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>الحد الأدنى لتطبيع البيانات؛ يعود إلى نطاق قيم مجموعة البيانات عند تركه غير محدد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3334"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3423"/>
        <source>Maximum Value (optional)</source>
        <translation>القيمة القصوى (اختياري)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3335"/>
        <source>Upper bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>الحد الأقصى لتطبيع البيانات؛ يعود إلى نطاق قيم مجموعة البيانات عند تركه غير محدد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3411"/>
        <source>Lower bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>الحد الأدنى لنطاق المقياس أو الشريط؛ يعود إلى نطاق قيم مجموعة البيانات عند تركه غير محدد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3424"/>
        <source>Upper bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>الحد الأقصى لنطاق المقياس أو الشريط؛ يعود إلى نطاق قيم مجموعة البيانات عند تركه غير محدد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3509"/>
        <source>On</source>
        <translation>تشغيل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3553"/>
        <source>LED lights up when value meets or exceeds this threshold; define alarm bands for multi-state colors</source>
        <translation>يضيء LED عندما تساوي القيمة هذا الحد أو تتجاوزه؛ حدد نطاقات الإنذار لألوان متعددة الحالات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4761"/>
        <source>Step Size</source>
        <translation>حجم الخطوة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4960"/>
        <source>Raw and transformed values for every dataset (read-only)</source>
        <translation>القيم الخام والمحولة لكل مجموعة بيانات (للقراءة فقط)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4969"/>
        <source>Shared table defined in this project</source>
        <translation>الجدول المشترك المعرّف في هذا المشروع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5328"/>
        <source>Remove 1 widget reference whose target group or dataset no longer exists?</source>
        <translation>إزالة مرجع عنصر واجهة واحد لم تعد مجموعته أو مجموعة بياناته موجودة؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5329"/>
        <source>Remove %1 widget references whose target groups or datasets no longer exist?</source>
        <translation>إزالة %1 مرجع عنصر واجهة لم تعد مجموعاتها أو مجموعات بياناتها موجودة؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5334"/>
        <source>This will only affect workspace tile placement; no groups, datasets, or data are deleted.</source>
        <translation>سيؤثر هذا فقط على موضع المربعات في مساحة العمل؛ لن يتم حذف أي مجموعات أو مجموعات بيانات أو بيانات.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5337"/>
        <source>Clean Up Workspaces</source>
        <translation>تنظيف مساحات العمل</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectModel</name>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="390"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="399"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="412"/>
        <source>Project error</source>
        <translation>خطأ في المشروع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="390"/>
        <source>Project title cannot be empty!</source>
        <translation>لا يمكن أن يكون عنوان المشروع فارغاً!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="399"/>
        <source>You need to add at least one group!</source>
        <translation>يجب إضافة مجموعة واحدة على الأقل!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="412"/>
        <source>You need to add at least one dataset!</source>
        <translation>يجب إضافة مجموعة بيانات واحدة على الأقل!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="459"/>
        <source>Your project needs a title</source>
        <translation>مشروعك يحتاج إلى عنوان</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="461"/>
        <source>Add a group to get started</source>
        <translation>أضف مجموعة للبدء</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="463"/>
        <source>Add a dataset to a group</source>
        <translation>إضافة مجموعة بيانات إلى مجموعة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="477"/>
        <source>Open the Project view at the top of the tree and enter a name. You can rename the project at any time.</source>
        <translation>افتح عرض المشروع في أعلى الشجرة وأدخل اسمًا. يمكنك إعادة تسمية المشروع في أي وقت.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="480"/>
        <source>Groups organize datasets into dashboard widgets. Use the Group button in the toolbar above to create one, then add datasets to it.</source>
        <translation>تنظم المجموعات مجموعات البيانات في عناصر لوحة المعلومات. استخدم زر المجموعة في شريط الأدوات أعلاه لإنشاء واحدة، ثم أضف مجموعات البيانات إليها.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="484"/>
        <source>Datasets are the values that appear on the dashboard. Select a group in the tree and use the Dataset button in the toolbar to add one.</source>
        <translation>مجموعات البيانات هي القيم التي تظهر على لوحة المعلومات. حدد مجموعة في الشجرة واستخدم زر مجموعة البيانات في شريط الأدوات لإضافة واحدة.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="518"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="527"/>
        <source>Lock Project</source>
        <translation>قفل المشروع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="519"/>
        <source>Choose a password to lock the project:</source>
        <translation>اختر كلمة مرور لقفل المشروع:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="527"/>
        <source>Confirm the password:</source>
        <translation>تأكيد كلمة المرور:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="532"/>
        <source>Passwords do not match</source>
        <translation>كلمات المرور غير متطابقة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="533"/>
        <source>The two passwords you entered do not match. The project was not locked.</source>
        <translation>كلمتا المرور اللتان أدخلتهما غير متطابقتين. لم يتم قفل المشروع.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="567"/>
        <source>Unlock Project</source>
        <translation>فتح قفل المشروع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="568"/>
        <source>Enter the project password:</source>
        <translation>أدخل كلمة مرور المشروع:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="578"/>
        <source>Incorrect password</source>
        <translation>كلمة المرور غير صحيحة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="579"/>
        <source>The password you entered does not match the one stored in the project file.</source>
        <translation>كلمة المرور التي أدخلتها لا تطابق الكلمة المحفوظة في ملف المشروع.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="610"/>
        <source>New Project</source>
        <translation>مشروع جديد</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">العينات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="762"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="809"/>
        <source>Time</source>
        <translation>الوقت</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1218"/>
        <source>Multiple data sources require a Pro license</source>
        <translation>مصادر البيانات المتعددة تتطلب ترخيص Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1219"/>
        <source>Serial Studio Pro allows connecting to multiple devices simultaneously. Please upgrade to unlock this feature.</source>
        <translation>يتيح Serial Studio Pro الاتصال بأجهزة متعددة في نفس الوقت. يرجى الترقية لفتح هذه الميزة.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1231"/>
        <source>Device %1</source>
        <translation>الجهاز %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1263"/>
        <source>Do you want to delete data source "%1"?</source>
        <translation>هل تريد حذف مصدر البيانات "%1"؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1264"/>
        <source>Groups using this source will move to the default source. This action cannot be undone.</source>
        <translation>المجموعات التي تستخدم هذا المصدر ستنتقل إلى المصدر الافتراضي. لا يمكن التراجع عن هذا الإجراء.</translation>
    </message>
    <message>
        <source> (Copy)</source>
        <translation type="vanished">(نسخة)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1494"/>
        <source>Do you want to save your changes?</source>
        <translation>حفظ التغييرات؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1495"/>
        <source>You have unsaved modifications in this project!</source>
        <translation>يوجد تعديلات غير محفوظة في هذا المشروع!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1534"/>
        <source>Save Serial Studio Project</source>
        <translation>حفظ مشروع Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1536"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2156"/>
        <source>Serial Studio Project Files (*.ssproj)</source>
        <translation>ملفات مشروع Serial Studio (*.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1557"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1776"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2147"/>
        <source>Untitled Project</source>
        <translation>مشروع بدون عنوان</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1788"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2302"/>
        <source>Device A</source>
        <translation>الجهاز A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1986"/>
        <source>Select Project File</source>
        <translation>اختيار ملف المشروع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1988"/>
        <source>Project Files (*.json *.ssproj)</source>
        <translation>ملفات المشروع (*.json *.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2032"/>
        <source>JSON validation error</source>
        <translation>خطأ في التحقق من صحة JSON</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2121"/>
        <source>Project upgraded from an earlier file format</source>
        <translation>تمت ترقية المشروع من تنسيق ملف أقدم</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2122"/>
        <source>This project was saved with schema version %1; the current version is %2. Defaults have been applied to any new fields. Save the project to lock in the upgrade.</source>
        <translation>تم حفظ هذا المشروع بإصدار مخطط %1؛ الإصدار الحالي هو %2. تم تطبيق القيم الافتراضية على أي حقول جديدة. احفظ المشروع لتثبيت الترقية.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2154"/>
        <source>Save Imported Project</source>
        <translation>حفظ المشروع المستورد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2346"/>
        <source>Multi-source projects require a Pro license</source>
        <translation>المشاريع متعددة المصادر تتطلب ترخيص Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2347"/>
        <source>This project contains multiple data sources. Only the first source has been loaded. A Serial Studio Pro license is required to use multi-source projects.</source>
        <translation>يحتوي هذا المشروع على مصادر بيانات متعددة. تم تحميل المصدر الأول فقط. يلزم ترخيص Serial Studio Pro لاستخدام المشاريع متعددة المصادر.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2580"/>
        <source>Workspace IDs remapped on load</source>
        <translation>تم إعادة تعيين معرفات مساحة العمل عند التحميل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2581"/>
        <source>%1 custom workspace ID(s) overlapped the new reserved auto range and were moved into the user range. Save the project to make the remap permanent.</source>
        <translation>%1 من معرفات مساحات العمل المخصصة تداخلت مع النطاق التلقائي المحجوز الجديد وتم نقلها إلى نطاق المستخدم. احفظ المشروع لجعل إعادة التعيين دائمة.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2723"/>
        <source>Legacy frame parser function updated</source>
        <translation>تم تحديث دالة محلل الإطارات القديمة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2724"/>
        <source>Your project used a legacy frame parser function with a 'separator' argument. It has been automatically migrated to the new format.</source>
        <translation>استخدم مشروعك دالة محلل إطارات قديمة مع معامل 'separator'. تم ترحيلها تلقائياً إلى التنسيق الجديد.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2922"/>
        <source>Do you want to delete group "%1"?</source>
        <translation>هل تريد حذف المجموعة "%1"؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2923"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2968"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3000"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3738"/>
        <source>This action cannot be undone. Do you wish to proceed?</source>
        <translation>لا يمكن التراجع عن هذا الإجراء. هل تريد المتابعة؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2967"/>
        <source>Do you want to delete action "%1"?</source>
        <translation>هل تريد حذف الإجراء "%1"؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2999"/>
        <source>Do you want to delete dataset "%1"?</source>
        <translation>هل تريد حذف مجموعة البيانات "%1"؟</translation>
    </message>
    <message>
        <source>%1 (Copy)</source>
        <translation type="vanished">%1 (نسخة)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3650"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3686"/>
        <source>Output Controls</source>
        <translation>عناصر التحكم في الإخراج</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3698"/>
        <source>New Button</source>
        <translation>زر جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3701"/>
        <source>New Slider</source>
        <translation>شريط تمرير جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3704"/>
        <source>New Toggle</source>
        <translation>مفتاح تبديل جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3707"/>
        <source>New Text Field</source>
        <translation>حقل نص جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3710"/>
        <source>New Knob</source>
        <translation>مقبض دوار جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3737"/>
        <source>Do you want to delete output widget "%1"?</source>
        <translation>هل تريد حذف عنصر الإخراج "%1"؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3902"/>
        <source>Group</source>
        <translation>مجموعة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3920"/>
        <source>New Dataset</source>
        <translation>مجموعة بيانات جديدة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3923"/>
        <source>New Plot</source>
        <translation>رسم بياني جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3927"/>
        <source>New FFT Plot</source>
        <translation>مخطط FFT جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3931"/>
        <source>New Level Indicator</source>
        <translation>مؤشر مستوى جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3935"/>
        <source>New Gauge</source>
        <translation>مقياس جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3939"/>
        <source>New Compass</source>
        <translation>بوصلة جديدة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3945"/>
        <source>New Meter</source>
        <translation>مقياس جديد</translation>
    </message>
    <message>
        <source>New Thermometer</source>
        <translation type="vanished">مقياس حرارة جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3949"/>
        <source>New LED Indicator</source>
        <translation>مؤشر LED جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3953"/>
        <source>New Waterfall</source>
        <translation>شلال جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4021"/>
        <source>Channel %1</source>
        <translation>القناة %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4092"/>
        <source>New Action</source>
        <translation>إجراء جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4230"/>
        <source>Are you sure you want to change the group-level widget?</source>
        <translation>هل أنت متأكد من تغيير عنصر واجهة مستوى المجموعة؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4232"/>
        <source>Existing datasets for this group are deleted</source>
        <translation>سيتم حذف مجموعات البيانات الموجودة لهذه المجموعة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4295"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4296"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4297"/>
        <source>Accelerometer %1</source>
        <translation>مقياس التسارع %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4312"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4312"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4312"/>
        <source>Gyro %1</source>
        <translation>الجيروسكوب %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4327"/>
        <source>Latitude</source>
        <translation>خط العرض</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4327"/>
        <source>Longitude</source>
        <translation>خط الطول</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4327"/>
        <source>Altitude</source>
        <translation>الارتفاع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4342"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4356"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4342"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4356"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4342"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4356"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4614"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5486"/>
        <source>Workspace</source>
        <translation>مساحة العمل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4779"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4981"/>
        <source>Shared Table</source>
        <translation>الجدول المشترك</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4857"/>
        <source>register</source>
        <translation>سجل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4981"/>
        <source>New Shared Table</source>
        <translation>جدول مشترك جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4981"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4998"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5017"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5041"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5068"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5087"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5109"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5131"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5486"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5507"/>
        <source>Name:</source>
        <translation>الاسم:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4998"/>
        <source>Rename Table</source>
        <translation>إعادة تسمية الجدول</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5017"/>
        <source>Rename Group</source>
        <translation>إعادة تسمية المجموعة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5041"/>
        <source>Rename Dataset</source>
        <translation>إعادة تسمية مجموعة البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5068"/>
        <source>Rename Data Source</source>
        <translation>إعادة تسمية مصدر البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5087"/>
        <source>Rename Action</source>
        <translation>إعادة تسمية الإجراء</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5108"/>
        <source>New Register</source>
        <translation>سجل جديد</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5131"/>
        <source>Rename Register</source>
        <translation>إعادة تسمية السجل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5168"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5193"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6009"/>
        <source>This action cannot be undone.</source>
        <translation>لا يمكن التراجع عن هذا الإجراء.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5169"/>
        <source>This removes %1 register(s) along with the table. This action cannot be undone.</source>
        <translation>سيؤدي هذا إلى إزالة %1 سجل مع الجدول. لا يمكن التراجع عن هذا الإجراء.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5172"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5192"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6008"/>
        <source>Delete "%1"?</source>
        <translation>حذف "%1"؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5175"/>
        <source>Delete Table</source>
        <translation>حذف الجدول</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5195"/>
        <source>Delete Register</source>
        <translation>حذف السجل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5217"/>
        <source>Export Table</source>
        <translation>تصدير الجدول</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5219"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5259"/>
        <source>CSV files (*.csv)</source>
        <translation>ملفات CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5257"/>
        <source>Import Table</source>
        <translation>استيراد الجدول</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5486"/>
        <source>New Workspace</source>
        <translation>مساحة عمل جديدة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5507"/>
        <source>Rename Workspace</source>
        <translation>إعادة تسمية مساحة العمل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5590"/>
        <source>Overview</source>
        <translation>نظرة عامة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5599"/>
        <source>All Data</source>
        <translation>جميع البيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5777"/>
        <source>Discard workspace customisations?</source>
        <translation>تجاهل تخصيصات مساحة العمل؟</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5778"/>
        <source>Switching off Customize discards your edits and rebuilds the workspace list from the project's groups.</source>
        <translation>إيقاف التخصيص يتجاهل التعديلات ويعيد بناء قائمة مساحات العمل من مجموعات المشروع.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5781"/>
        <source>Customize Workspaces</source>
        <translation>تخصيص مساحات العمل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6011"/>
        <source>Delete Workspace</source>
        <translation>حذف مساحة العمل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6338"/>
        <source>File save error</source>
        <translation>خطأ في حفظ الملف</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2190"/>
        <source>File open error</source>
        <translation>خطأ في فتح الملف</translation>
    </message>
</context>
<context>
    <name>DataModel::ProtoImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="902"/>
        <source>Import Protocol Buffers File</source>
        <translation>استيراد ملف Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="904"/>
        <source>Proto Files (*.proto);;All Files (*)</source>
        <translation>ملفات Proto (*.proto);;جميع الملفات (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="938"/>
        <source>Failed to open proto file: %1</source>
        <translation>فشل فتح ملف proto: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="939"/>
        <source>Verify the file path and read permissions, then try again.</source>
        <translation>تحقق من مسار الملف وأذونات القراءة، ثم حاول مرة أخرى.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="941"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="950"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="968"/>
        <source>Protobuf Import Error</source>
        <translation>خطأ في استيراد Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="947"/>
        <source>Proto file is too large (the limit is 10 MB).</source>
        <translation>ملف Proto كبير جدًا (الحد الأقصى 10 ميجابايت).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="948"/>
        <source>Verify you selected the correct .proto definition file.</source>
        <translation>تحقق من اختيارك للملف الصحيح لتعريف .proto.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="965"/>
        <source>Failed to parse proto file at line %1: %2</source>
        <translation>فشل تحليل ملف proto في السطر %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="966"/>
        <source>Only proto3 syntax is supported. Verify the file format and try again.</source>
        <translation>يتم دعم صيغة proto3 فقط. تحقق من تنسيق الملف وحاول مرة أخرى.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="973"/>
        <source>Proto file contains no message definitions</source>
        <translation>ملف Proto لا يحتوي على تعريفات رسائل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="974"/>
        <source>The selected file has no `message` blocks to import.</source>
        <translation>الملف المحدد لا يحتوي على كتل `message` لاستيرادها.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="976"/>
        <source>Protobuf Import Warning</source>
        <translation>تحذير استيراد Protobuf</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">فشل تحميل المشروع المستورد</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">تعذر تحميل JSON للمشروع المُنشأ.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1014"/>
        <source>Successfully imported %1 message(s) and %2 field(s) from the proto file.</source>
        <translation>تم استيراد %1 رسالة و %2 حقل من ملف proto بنجاح.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1017"/>
        <source>The project editor is now open for customization.</source>
        <translation>محرر المشروع مفتوح الآن للتخصيص.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1019"/>
        <source>Protobuf Import Complete</source>
        <translation>اكتمل استيراد Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1053"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
</context>
<context>
    <name>DataModel::TransmitTestDialog</name>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="154"/>
        <source>Invalid Hex Input</source>
        <translation>إدخال HEX غير صالح</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="155"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>يُرجى إدخال بايتات سداسية عشرية صالحة.

التنسيق الصالح: 01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="160"/>
        <source>No transmit function code to evaluate.</source>
        <translation>لا يوجد كود دالة إرسال للتقييم.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="177"/>
        <source>transmit function is not callable</source>
        <translation>دالة الإرسال transmit غير قابلة للاستدعاء</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="238"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="239"/>
        <source>Clear</source>
        <translation>مسح</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="240"/>
        <source>Evaluate</source>
        <translation>تقييم</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="241"/>
        <source>Input Value</source>
        <translation>قيمة الإدخال</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="242"/>
        <source>Transmit Function Output</source>
        <translation>مخرجات دالة الإرسال</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="243"/>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="267"/>
        <source>Enter value to transmit…</source>
        <translation>أدخل القيمة المراد إرسالها…</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="244"/>
        <source>Raw string output appears here</source>
        <translation>يظهر هنا مخرج النص الخام</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="245"/>
        <source>Hex byte output appears here</source>
        <translation>يظهر هنا مخرج البايتات بصيغة Hex</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="248"/>
        <source>Test Transmit Function</source>
        <translation>اختبار دالة الإرسال</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="261"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>أدخل بايتات بصيغة hex (مثال: 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="367"/>
        <source>(empty) No data returned</source>
        <translation>(فارغ) لم يتم إرجاع بيانات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="369"/>
        <source>0 bytes</source>
        <translation>0 بايت</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="408"/>
        <source>%1 byte(s)</source>
        <translation>%1 بايت</translation>
    </message>
</context>
<context>
    <name>DataTablesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="33"/>
        <source>Shared Memory</source>
        <translation>الذاكرة المشتركة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="149"/>
        <source>Add Shared Table</source>
        <translation>إضافة جدول مشترك</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="151"/>
        <source>Add shared table</source>
        <translation>إضافة جدول مشترك</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="160"/>
        <source>Help</source>
        <translation>مساعدة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="165"/>
        <source>Open help documentation for shared memory</source>
        <translation>فتح وثائق المساعدة للذاكرة المشتركة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="174"/>
        <source>Name</source>
        <translation>الاسم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="175"/>
        <source>Description</source>
        <translation>الوصف</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="176"/>
        <source>Entries</source>
        <translation>الإدخالات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="274"/>
        <source>No shared tables.</source>
        <translation>لا توجد جداول مشتركة.</translation>
    </message>
</context>
<context>
    <name>DatabaseExplorer</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="35"/>
        <source>Sessions</source>
        <translation>الجلسات</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="218"/>
        <source>Open</source>
        <translation>فتح</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="220"/>
        <source>Open a session file</source>
        <translation>فتح ملف جلسة</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="226"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="229"/>
        <source>Close session file</source>
        <translation>إغلاق ملف الجلسة</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="242"/>
        <source>Replay</source>
        <translation>إعادة تشغيل</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="246"/>
        <source>Replay selected session on the dashboard</source>
        <translation>إعادة تشغيل الجلسة المحددة على لوحة المعلومات</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="252"/>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="258"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>إلغاء قفل ملف الجلسة لحذف الجلسات</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="259"/>
        <source>Delete the selected session</source>
        <translation>حذف الجلسة المحددة</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="276"/>
        <source>Unlock</source>
        <translation>إلغاء القفل</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="277"/>
        <source>Lock</source>
        <translation>قفل</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="282"/>
        <source>Unlock the session file to allow deletions</source>
        <translation>إلغاء قفل ملف الجلسة للسماح بالحذف</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="283"/>
        <source>Set a password to prevent session deletions</source>
        <translation>تعيين كلمة مرور لمنع حذف الجلسات</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="298"/>
        <source>Export CSV</source>
        <translation>تصدير CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="303"/>
        <source>Export selected session to CSV</source>
        <translation>تصدير الجلسة المحددة إلى CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="310"/>
        <source>Export PDF</source>
        <translation>تصدير PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="315"/>
        <source>Generate a PDF report for the selected session</source>
        <translation>إنشاء تقرير PDF للجلسة المحددة</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="329"/>
        <source>Restore Project</source>
        <translation>استعادة المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="333"/>
        <source>Restore the project file from this session file</source>
        <translation>استعادة ملف المشروع من ملف الجلسة هذا</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="402"/>
        <source>Loading session…</source>
        <translation>جارٍ تحميل الجلسة…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="403"/>
        <source>Working…</source>
        <translation>جارٍ العمل…</translation>
    </message>
</context>
<context>
    <name>DatasetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="109"/>
        <source>Pro features detected in this project.</source>
        <translation>تم اكتشاف ميزات Pro في هذا المشروع.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="111"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>استخدام عناصر واجهة احتياطية. اشترِ ترخيصًا لفتح الوظائف الكاملة.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="139"/>
        <source>Plots</source>
        <translation>الرسوم البيانية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="144"/>
        <source>Plot</source>
        <translation>رسم بياني</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="148"/>
        <source>Toggle 2D plot visualization for this dataset</source>
        <translation>تبديل تصور الرسم البياني ثنائي الأبعاد لمجموعة البيانات هذه</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="160"/>
        <source>FFT Plot</source>
        <translation>رسم بياني FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="163"/>
        <source>Toggle FFT plot to visualize frequency content</source>
        <translation>تبديل رسم FFT البياني لتصور محتوى التردد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="175"/>
        <source>Waterfall</source>
        <translation>مخطط شلالي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="179"/>
        <source>Toggle waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>تبديل المخطط الشلالي (مخطط طيفي) — يستخدم إعدادات FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="196"/>
        <source>Widgets</source>
        <translation>عناصر الواجهة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="202"/>
        <source>Bar/Level</source>
        <translation>شريط/مستوى</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="206"/>
        <source>Toggle bar/level indicator for this dataset</source>
        <translation>تبديل مؤشر الشريط/المستوى لمجموعة البيانات هذه</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="217"/>
        <source>Gauge</source>
        <translation>مقياس</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="222"/>
        <source>Toggle gauge widget for analog-style display</source>
        <translation>تبديل عنصر المقياس للعرض بنمط تناظري</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="234"/>
        <source>Compass</source>
        <translation>بوصلة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="238"/>
        <source>Toggle compass widget for directional data</source>
        <translation>تبديل عنصر البوصلة لبيانات الاتجاه</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="249"/>
        <source>Meter</source>
        <translation>مقياس</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="254"/>
        <source>Toggle analog meter (half-arc) widget</source>
        <translation>تبديل عنصر المقياس التناظري (نصف قوس)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="305"/>
        <source>Define colored value ranges with severity tiers for this dataset's gauge or LED.</source>
        <translation>تحديد نطاقات قيم ملونة بمستويات خطورة لمقياس أو LED مجموعة البيانات هذه.</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">مقياس حرارة</translation>
    </message>
    <message>
        <source>Toggle thermometer widget for temperature data</source>
        <translation type="vanished">تبديل عنصر مقياس الحرارة لبيانات درجة الحرارة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="265"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="270"/>
        <source>Toggle LED indicator for binary or thresholded values</source>
        <translation>تبديل مؤشر LED للقيم الثنائية أو ذات العتبة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="290"/>
        <source>Behavior</source>
        <translation>السلوك</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="295"/>
        <source>Alarm Bands</source>
        <translation>نطاقات الإنذار</translation>
    </message>
    <message>
        <source>Define colored value ranges with severity tiers for this dataset's gauge.</source>
        <translation type="vanished">تحديد نطاقات قيم ملونة بمستويات خطورة لمقياس مجموعة البيانات هذه.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="311"/>
        <source>Transform</source>
        <translation>تحويل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="315"/>
        <source>Edit a value transform expression for calibration, filtering, or unit conversion</source>
        <translation>تحرير تعبير تحويل القيمة للمعايرة أو الترشيح أو تحويل الوحدات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="328"/>
        <source>Duplicate</source>
        <translation>تكرار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="333"/>
        <source>Duplicate this dataset with the same configuration</source>
        <translation>تكرار مجموعة البيانات هذه بنفس الإعدادات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="338"/>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="341"/>
        <source>Delete this dataset from the group</source>
        <translation>حذف مجموعة البيانات هذه من المجموعة</translation>
    </message>
</context>
<context>
    <name>Donate</name>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="37"/>
        <source>Support Serial Studio</source>
        <translation>دعم Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="86"/>
        <source>Support the development of %1!</source>
        <translation>دعم تطوير %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="97"/>
        <source>Serial Studio is free &amp; open-source software supported by volunteers. Consider donating or obtaining a Pro license to support development efforts :)</source>
        <translation>Serial Studio برنامج مجاني ومفتوح المصدر يدعمه متطوعون. يُرجى التبرع أو الحصول على ترخيص Pro لدعم جهود التطوير :)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="110"/>
        <source>You can also support this project by sharing it, reporting bugs and proposing new features!</source>
        <translation>يمكنك أيضًا دعم هذا المشروع بمشاركته والإبلاغ عن الأخطاء واقتراح ميزات جديدة!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="124"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="135"/>
        <source>Donate</source>
        <translation>تبرع</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="150"/>
        <source>Get Serial Studio Pro</source>
        <translation>احصل على Serial Studio Pro</translation>
    </message>
</context>
<context>
    <name>Downloader</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="127"/>
        <source>Stop</source>
        <translation>إيقاف</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="128"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="362"/>
        <source>Downloading updates</source>
        <translation>تنزيل التحديثات</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="129"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="455"/>
        <source>Time remaining</source>
        <translation>الوقت المتبقي</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="129"/>
        <source>unknown</source>
        <translation>غير معروف</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="228"/>
        <source>Error</source>
        <translation>خطأ</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="228"/>
        <source>Cannot find downloaded update!</source>
        <translation>تعذر العثور على التحديث المُنزَّل!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="244"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="245"/>
        <source>Download complete!</source>
        <translation>اكتمل التنزيل!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="246"/>
        <source>The installer opens separately</source>
        <translation>يفتح المثبِّت بشكل منفصل</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="253"/>
        <source>Click "OK" to begin installing the update</source>
        <translation>انقر "موافق" لبدء تثبيت التحديث</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="255"/>
        <source>In order to install the update, you may need to quit the application.</source>
        <translation>لتثبيت التحديث، قد تحتاج إلى إنهاء التطبيق.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="259"/>
        <source>In order to install the update, you may need to quit the application. This is a mandatory update, exiting now will close the application.</source>
        <translation>لتثبيت التحديث، قد تحتاج إلى إنهاء التطبيق. هذا تحديث إلزامي، الخروج الآن سيغلق التطبيق.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="275"/>
        <source>Click the "Open" button to apply the update</source>
        <translation>انقر على زر "فتح" لتطبيق التحديث</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="288"/>
        <source>Updater</source>
        <translation>المُحدِّث</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="292"/>
        <source>Are you sure you want to cancel the download?</source>
        <translation>هل تريد بالتأكيد إلغاء التنزيل؟</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="294"/>
        <source>Are you sure you want to cancel the download? This is a mandatory update, exiting now will close the application</source>
        <translation>هل تريد بالتأكيد إلغاء التنزيل؟ هذا تحديث إلزامي، الخروج الآن سيغلق التطبيق</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="349"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="356"/>
        <source>%1 bytes</source>
        <translation>%1 بايت</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="351"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="358"/>
        <source>%1 KB</source>
        <translation>%1 كيلوبايت</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="353"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="360"/>
        <source>%1 MB</source>
        <translation>%1 ميغابايت</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="362"/>
        <source>of</source>
        <translation>من</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="406"/>
        <source>Downloading Updates</source>
        <translation>تنزيل التحديثات</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="407"/>
        <source>Time Remaining</source>
        <translation>الوقت المتبقي</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="407"/>
        <source>Unknown</source>
        <translation>غير معروف</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="431"/>
        <source>about %1 hours</source>
        <translation>حوالي %1 ساعة</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="433"/>
        <source>about one hour</source>
        <translation>حوالي ساعة واحدة</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="441"/>
        <source>%1 minutes</source>
        <translation>%1 دقيقة</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="443"/>
        <source>1 minute</source>
        <translation>دقيقة واحدة</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="450"/>
        <source>%1 seconds</source>
        <translation>%1 ثانية</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="452"/>
        <source>1 second</source>
        <translation>ثانية واحدة</translation>
    </message>
    <message>
        <source>Time remaining: 0 minutes</source>
        <translation type="vanished">الوقت المتبقي: 0 دقيقة</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">فتح</translation>
    </message>
</context>
<context>
    <name>ExamplesBrowser</name>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="34"/>
        <source>Examples Browser</source>
        <translation>متصفح الأمثلة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="91"/>
        <source>Search in Examples…</source>
        <translation>البحث في الأمثلة…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="120"/>
        <source>Back</source>
        <translation>رجوع</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="152"/>
        <source>Pro</source>
        <translation>Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="173"/>
        <source>Download &amp;&amp; Open</source>
        <translation>تنزيل &amp;&amp; فتح</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="188"/>
        <source>View on GitHub</source>
        <translation>عرض على GitHub</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="245"/>
        <source>Fetching examples…</source>
        <translation>جارٍ جلب الأمثلة…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="568"/>
        <source>Loading...</source>
        <translation>جارٍ التحميل...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="569"/>
        <source>No README available.</source>
        <translation>لا يوجد ملف README متاح.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="609"/>
        <source>Copied to Clipboard</source>
        <translation>تم النسخ إلى الحافظة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="672"/>
        <source>No screenshot available</source>
        <translation>لا توجد لقطة شاشة متاحة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="704"/>
        <source>Details</source>
        <translation>التفاصيل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="733"/>
        <source>Info</source>
        <translation>معلومات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="756"/>
        <source>Category:</source>
        <translation>الفئة:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="769"/>
        <source>Difficulty:</source>
        <translation>الصعوبة:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="787"/>
        <source>Project:</source>
        <translation>المشروع:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="829"/>
        <source>No Results Found</source>
        <translation>لم يتم العثور على نتائج</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="840"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>تحقق من الإملاء أو جرّب مصطلح بحث مختلف.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="855"/>
        <source>%1 examples</source>
        <translation>%1 مثال</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="864"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
</context>
<context>
    <name>ExtensionManager</name>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="32"/>
        <source>Extension Manager</source>
        <translation>مدير الإضافات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="78"/>
        <source>Search extensions…</source>
        <translation>البحث عن إضافات…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="119"/>
        <source>Refresh</source>
        <translation>تحديث</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="133"/>
        <source>Repos</source>
        <translation>المستودعات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="163"/>
        <source>Repository Settings</source>
        <translation>إعدادات المستودع</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="175"/>
        <source>Back</source>
        <translation>رجوع</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="216"/>
        <source>Install</source>
        <translation>تثبيت</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="233"/>
        <source>Uninstall</source>
        <translation>إلغاء التثبيت</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="260"/>
        <source>Run</source>
        <translation>تشغيل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="284"/>
        <source>Stop</source>
        <translation>إيقاف</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="318"/>
        <source>Reset</source>
        <translation>إعادة تعيين</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="368"/>
        <source>Fetching extensions…</source>
        <translation>جارٍ جلب الإضافات…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="605"/>
        <source>Running</source>
        <translation>قيد التشغيل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Update</source>
        <translation>تحديث</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Installed</source>
        <translation>مُثبَّت</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="644"/>
        <source>Unavailable</source>
        <translation>غير متاح</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="823"/>
        <source>No description available.</source>
        <translation>لا يوجد وصف متاح.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="864"/>
        <source>Details</source>
        <translation>التفاصيل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="885"/>
        <source>Type:</source>
        <translation>النوع:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="898"/>
        <source>Author:</source>
        <translation>المؤلف:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="910"/>
        <source>Version:</source>
        <translation>الإصدار:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="922"/>
        <source>License:</source>
        <translation>الترخيص:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="983"/>
        <source>No preview</source>
        <translation>لا توجد معاينة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1012"/>
        <source>  PLUGIN OUTPUT</source>
        <translation>مخرجات الإضافة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1042"/>
        <source>No output yet. Run the plugin to see its log here.</source>
        <translation>لا توجد مخرجات بعد. قم بتشغيل الإضافة لرؤية سجلها هنا.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1077"/>
        <source>No preview available</source>
        <translation>المعاينة غير متاحة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1121"/>
        <source>Repositories</source>
        <translation>المستودعات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1134"/>
        <source>Add URLs to remote repositories or local folder paths.</source>
        <translation>أضف عناوين URL لمستودعات بعيدة أو مسارات مجلدات محلية.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1171"/>
        <source>LOCAL</source>
        <translation>محلي</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1228"/>
        <source>URL or local path…</source>
        <translation>عنوان URL أو مسار محلي…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1236"/>
        <source>Add</source>
        <translation>إضافة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1259"/>
        <source>Browse…</source>
        <translation>استعراض…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1296"/>
        <source>No Results Found</source>
        <translation>لم يتم العثور على نتائج</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1307"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>تحقق من الإملاء أو جرّب مصطلح بحث مختلف.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1331"/>
        <source>No Extensions Available</source>
        <translation>لا توجد إضافات متاحة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1342"/>
        <source>Add a repository URL or local path in the Repos settings, then refresh.</source>
        <translation>أضف عنوان URL لمستودع أو مسار محلي في إعدادات المستودعات، ثم حدّث.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1357"/>
        <source>%1 extensions</source>
        <translation>%1 إضافة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1366"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
</context>
<context>
    <name>FFTPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="157"/>
        <source>Interpolation: %1</source>
        <translation>الاستيفاء: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="185"/>
        <source>Show Area Under Plot</source>
        <translation>إظهار المنطقة أسفل الرسم</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="203"/>
        <source>Show X Axis Label</source>
        <translation>إظهار تسمية المحور X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="215"/>
        <source>Show Y Axis Label</source>
        <translation>إظهار تسمية المحور Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="233"/>
        <source>Show Crosshair</source>
        <translation>إظهار الشعيرات المتقاطعة</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="240"/>
        <source>Pause</source>
        <translation>إيقاف مؤقت</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="240"/>
        <source>Resume</source>
        <translation>استئناف</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="259"/>
        <source>Reset View</source>
        <translation>إعادة تعيين العرض</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="265"/>
        <source>Axis Range Settings</source>
        <translation>إعدادات نطاق المحاور</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="294"/>
        <source>Magnitude (dB)</source>
        <translation>المقدار (dB)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="295"/>
        <source>Frequency (Hz)</source>
        <translation>التردد (Hz)</translation>
    </message>
</context>
<context>
    <name>FileDropArea</name>
    <message>
        <location filename="../../qml/Widgets/FileDropArea.qml" line="130"/>
        <source>Drop Projects and CSV files here</source>
        <translation>إسقاط ملفات المشاريع وملفات CSV هنا</translation>
    </message>
</context>
<context>
    <name>FileTransmission</name>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="34"/>
        <source>File Transmission</source>
        <translation>نقل الملفات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="102"/>
        <source>Transfer Protocol:</source>
        <translation>بروتوكول النقل:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="135"/>
        <source>File Selection:</source>
        <translation>اختيار الملف:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="152"/>
        <source>Select File…</source>
        <translation>اختيار ملف…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="170"/>
        <source>Transmission Interval:</source>
        <translation>فاصل الإرسال:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="196"/>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="272"/>
        <source>msecs</source>
        <translation>ميلي ثانية</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="206"/>
        <source>Block Size:</source>
        <translation>حجم الكتلة:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="234"/>
        <source>bytes</source>
        <translation>بايت</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="244"/>
        <source>Timeout:</source>
        <translation>المهلة:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="282"/>
        <source>Max Retries:</source>
        <translation>الحد الأقصى لإعادة المحاولة:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="340"/>
        <source>Progress: %1%</source>
        <translation>التقدم: %1%</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="373"/>
        <source>%1 / %2 bytes</source>
        <translation>%1 / %2 بايت</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="381"/>
        <source>Errors: %1</source>
        <translation>الأخطاء: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="419"/>
        <source>Pause Transmission</source>
        <translation>إيقاف الإرسال مؤقتاً</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="420"/>
        <source>Resume Transmission</source>
        <translation>استئناف الإرسال</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="423"/>
        <source>Stop Transmission</source>
        <translation>إيقاف الإرسال</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="424"/>
        <source>Begin Transmission</source>
        <translation>بدء الإرسال</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="461"/>
        <source>Activity Log</source>
        <translation>سجل النشاط</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="465"/>
        <source>Clear</source>
        <translation>مسح</translation>
    </message>
</context>
<context>
    <name>FlowDiagram</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="159"/>
        <source>Frame Parser</source>
        <translation>محلل الإطارات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="166"/>
        <source>Device %1</source>
        <translation>الجهاز %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="222"/>
        <source>Group</source>
        <translation>مجموعة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="345"/>
        <source>Action</source>
        <translation>إجراء</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="395"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1430"/>
        <source>Output Panel</source>
        <translation>لوحة الإخراج</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="439"/>
        <source>Control</source>
        <translation>عنصر تحكم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="462"/>
        <source>Table</source>
        <translation>جدول</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="478"/>
        <source>%1 regs</source>
        <translation>%1 سجلات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="479"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="501"/>
        <source>empty</source>
        <translation>فارغ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="499"/>
        <source>Control Script</source>
        <translation>سكريبت التحكم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="524"/>
        <source>MQTT Publisher</source>
        <translation>ناشر MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="927"/>
        <source>Open the transform code editor for this dataset.</source>
        <translation>فتح محرر كود التحويل لمجموعة البيانات هذه.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1212"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1215"/>
        <source>Dataset Container</source>
        <translation>حاوية مجموعة البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1224"/>
        <source>Multi-Plot</source>
        <translation>رسم بياني متعدد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1227"/>
        <source>Multiple Plot</source>
        <translation>رسم بياني متعدد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1236"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1239"/>
        <source>Accelerometer</source>
        <translation>مقياس التسارع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1248"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1251"/>
        <source>Gyroscope</source>
        <translation>الجيروسكوب</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1260"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1263"/>
        <source>GPS Map</source>
        <translation>خريطة GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1271"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1274"/>
        <source>3D Plot</source>
        <translation>رسم بياني ثلاثي الأبعاد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1282"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1285"/>
        <source>Image View</source>
        <translation>عارض الصور</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1294"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1297"/>
        <source>Painter Widget</source>
        <translation>أداة الرسم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1306"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1309"/>
        <source>Data Grid</source>
        <translation>شبكة البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1322"/>
        <source>Generic</source>
        <translation>عام</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1335"/>
        <source>Plot</source>
        <translation>رسم بياني</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1348"/>
        <source>FFT Plot</source>
        <translation>رسم بياني FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1361"/>
        <source>Gauge</source>
        <translation>مقياس</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1374"/>
        <source>Level Indicator</source>
        <translation>مؤشر المستوى</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1387"/>
        <source>Compass</source>
        <translation>بوصلة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1400"/>
        <source>Meter</source>
        <translation>مقياس</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2107"/>
        <source>Edit Control Script…</source>
        <translation>تحرير سكريبت التحكم…</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">مقياس حرارة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1413"/>
        <source>LED Indicator</source>
        <translation>مؤشر LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1442"/>
        <source>Slider</source>
        <translation>شريط التمرير</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1455"/>
        <source>Toggle</source>
        <translation>مفتاح تبديل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1468"/>
        <source>Knob</source>
        <translation>مقبض</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1481"/>
        <source>Text Field</source>
        <translation>حقل نصي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1494"/>
        <source>Button</source>
        <translation>زر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1518"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1593"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1680"/>
        <source>Add Group</source>
        <translation>إضافة مجموعة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1533"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1608"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1695"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1740"/>
        <source>Add Dataset</source>
        <translation>إضافة مجموعة بيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1547"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1622"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1709"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1754"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1961"/>
        <source>Add Output</source>
        <translation>إضافة مخرج</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1563"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1635"/>
        <source>Add Action</source>
        <translation>إضافة إجراء</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1572"/>
        <source>Add Data Source</source>
        <translation>إضافة مصدر بيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1579"/>
        <source>Add Data Table</source>
        <translation>إضافة جدول بيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1646"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1781"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1848"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1976"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2010"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2066"/>
        <source>Rename…</source>
        <translation>إعادة تسمية…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1654"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1811"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1881"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1933"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1984"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2040"/>
        <source>Duplicate</source>
        <translation>تكرار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1665"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1822"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1893"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1945"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1995"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2051"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2077"/>
        <source>Delete…</source>
        <translation>حذف…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1725"/>
        <source>Edit Frame Parser…</source>
        <translation>تحرير محلل الإطارات…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1767"/>
        <source>Edit Painter Code…</source>
        <translation>تحرير كود الرسام…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1789"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1857"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1909"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2018"/>
        <source>Move Up</source>
        <translation>نقل لأعلى</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1800"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1869"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1921"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2029"/>
        <source>Move Down</source>
        <translation>نقل لأسفل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1837"/>
        <source>Edit Transform Code…</source>
        <translation>تحرير كود التحويل…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2092"/>
        <source>Edit Code…</source>
        <translation>تحرير الكود…</translation>
    </message>
</context>
<context>
    <name>FrameParserTest</name>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="39"/>
        <source>Test Frame Parser</source>
        <translation>اختبار محلل الإطارات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="96"/>
        <source>Frame %1</source>
        <translation>الإطار %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="98"/>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="209"/>
        <source>Decoder</source>
        <translation>فك الترميز</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="100"/>
        <source>Rows</source>
        <translation>الصفوف</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="101"/>
        <source>%1 row(s)</source>
        <translation>%1 صف (صفوف)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="105"/>
        <source>Row %1</source>
        <translation>الصف %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="164"/>
        <source>Pipeline Configuration</source>
        <translation>إعدادات خط المعالجة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="186"/>
        <source>Detection</source>
        <translation>الكشف</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="232"/>
        <source>Start</source>
        <translation>البداية</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="253"/>
        <source>End</source>
        <translation>نهاية</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="274"/>
        <source>Checksum</source>
        <translation>المجموع الاختباري</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="299"/>
        <source>Hex Delimiters</source>
        <translation>المحددات السداسية عشرية</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="315"/>
        <source>Frame Data Input</source>
        <translation>إدخال بيانات الإطار</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="342"/>
        <source>Enter hex bytes (e.g. 01 A2 FF)</source>
        <translation>أدخل بايتات سداسية عشرية (مثال: 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="343"/>
        <source>Enter raw stream bytes here...</source>
        <translation>أدخل بايتات التدفق الخام هنا...</translation>
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
        <translation>العينة لا تحتوي على محددات الإطار المُكوَّنة، لذا لن يتم استخراج أي إطار. اكتبها في العينة (مثال: 
 لسطر جديد) أو اضبط وضع الكشف.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="407"/>
        <source>Pipeline Results</source>
        <translation>نتائج المسار</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="480"/>
        <source>Stage</source>
        <translation>مرحلة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="487"/>
        <source>Value</source>
        <translation>القيمة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="530"/>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation>الاستخراج لم ينتج إطارًا كاملاً. تحقق من محددات البداية / النهاية ووضع الكشف.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="532"/>
        <source>Enter sample data above and press Evaluate to preview the parsed output</source>
        <translation>أدخل بيانات عينة أعلاه واضغط تقييم لمعاينة المخرجات المحللة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="614"/>
        <source>Clear</source>
        <translation>مسح</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="625"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="632"/>
        <source>Evaluate</source>
        <translation>تقييم</translation>
    </message>
</context>
<context>
    <name>FrameParserView</name>
    <message>
        <source>Undo</source>
        <translation type="vanished">تراجع</translation>
    </message>
    <message>
        <source>Redo</source>
        <translation type="vanished">إعادة</translation>
    </message>
    <message>
        <source>Cut</source>
        <translation type="vanished">قص</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="vanished">نسخ</translation>
    </message>
    <message>
        <source>Paste</source>
        <translation type="vanished">لصق</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation type="vanished">تحديد الكل</translation>
    </message>
    <message>
        <source>Format Document</source>
        <translation type="vanished">تنسيق المستند</translation>
    </message>
    <message>
        <source>Format Selection</source>
        <translation type="vanished">تنسيق التحديد</translation>
    </message>
    <message>
        <source>Reset</source>
        <translation type="vanished">إعادة تعيين</translation>
    </message>
    <message>
        <source>Reset to the default parsing script</source>
        <translation type="vanished">إعادة تعيين إلى نص التحليل الافتراضي</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">فتح</translation>
    </message>
    <message>
        <source>Import a script file for data parsing</source>
        <translation type="vanished">استيراد ملف نص برمجي لتحليل البيانات</translation>
    </message>
    <message>
        <source>Undo the last code edit</source>
        <translation type="vanished">التراجع عن آخر تعديل للكود</translation>
    </message>
    <message>
        <source>Redo the previously undone edit</source>
        <translation type="vanished">إعادة التعديل الذي تم التراجع عنه</translation>
    </message>
    <message>
        <source>Cut selected code to clipboard</source>
        <translation type="vanished">قص الكود المحدد إلى الحافظة</translation>
    </message>
    <message>
        <source>Copy selected code to clipboard</source>
        <translation type="vanished">نسخ الكود المحدد إلى الحافظة</translation>
    </message>
    <message>
        <source>Paste code from clipboard</source>
        <translation type="vanished">لصق الكود من الحافظة</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="vanished">مساعدة</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">فتح وثائق المساعدة لتحليل البيانات</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">اللغة:</translation>
    </message>
    <message>
        <source>Select Template…</source>
        <translation type="vanished">اختيار قالب…</translation>
    </message>
    <message>
        <source>Test With Sample Data</source>
        <translation type="vanished">اختبار ببيانات نموذجية</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">تقييم</translation>
    </message>
</context>
<context>
    <name>GPS</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="105"/>
        <source>Auto Center</source>
        <translation>توسيط تلقائي</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="121"/>
        <source>Plot Trajectory</source>
        <translation>رسم المسار</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="138"/>
        <source>Zoom In</source>
        <translation>تكبير</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="149"/>
        <source>Zoom Out</source>
        <translation>تصغير</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="173"/>
        <source>Show Weather</source>
        <translation>إظهار الطقس</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="191"/>
        <source>NASA Weather Overlay</source>
        <translation>طبقة طقس NASA</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="223"/>
        <source>Base Map: %1</source>
        <translation>الخريطة الأساسية: %1</translation>
    </message>
</context>
<context>
    <name>GroupView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="97"/>
        <source>Pro features detected in this project.</source>
        <translation>تم اكتشاف ميزات Pro في هذا المشروع.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="99"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>استخدام عناصر بديلة. اشترِ ترخيصًا لفتح الوظائف الكاملة.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="129"/>
        <source>Add Dataset</source>
        <translation>إضافة مجموعة بيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="136"/>
        <source>Dataset</source>
        <translation>مجموعة البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="139"/>
        <source>Add a generic dataset to the current group</source>
        <translation>إضافة مجموعة بيانات عامة إلى المجموعة الحالية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="146"/>
        <source>Plot</source>
        <translation>رسم بياني</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="150"/>
        <source>Add a 2D plot to visualize numeric data</source>
        <translation>إضافة رسم بياني ثنائي الأبعاد لتصور البيانات الرقمية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="158"/>
        <source>FFT Plot</source>
        <translation>رسم بياني FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="163"/>
        <source>Add an FFT plot for frequency domain visualization</source>
        <translation>إضافة رسم بياني FFT لتصور مجال التردد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="169"/>
        <source>Waterfall</source>
        <translation>مخطط شلالي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="174"/>
        <source>Add a waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>إضافة مخطط شلالي (مخطط طيفي) — يستخدم إعدادات FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="180"/>
        <source>Bar/Level</source>
        <translation>شريط/مستوى</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="184"/>
        <source>Add a bar or level indicator for scaled values</source>
        <translation>إضافة مؤشر شريط أو مستوى للقيم المقاسة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="190"/>
        <source>Gauge</source>
        <translation>مقياس</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="195"/>
        <source>Add a gauge widget for analog-style visualization</source>
        <translation>إضافة عنصر مقياس للتصور بنمط تناظري</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="202"/>
        <source>Compass</source>
        <translation>بوصلة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="206"/>
        <source>Add a compass to display directional or angular data</source>
        <translation>إضافة بوصلة لعرض البيانات الاتجاهية أو الزاوية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="212"/>
        <source>Meter</source>
        <translation>مقياس</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="216"/>
        <source>Add an analog meter (half-arc) widget</source>
        <translation>إضافة عنصر مقياس تناظري (نصف قوس)</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">مقياس حرارة</translation>
    </message>
    <message>
        <source>Add a thermometer widget for temperature data</source>
        <translation type="vanished">إضافة عنصر مقياس حرارة لبيانات درجة الحرارة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="223"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="228"/>
        <source>Add an LED indicator for binary status signals</source>
        <translation>إضافة مؤشر LED لإشارات الحالة الثنائية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="241"/>
        <source>Add Control</source>
        <translation>إضافة عنصر تحكم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="247"/>
        <source>Button</source>
        <translation>زر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="251"/>
        <source>Add a button that sends a command on click</source>
        <translation>إضافة زر يرسل أمرًا عند النقر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="257"/>
        <source>Slider</source>
        <translation>شريط تمرير</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="261"/>
        <source>Add a slider for sending scaled numeric values</source>
        <translation>إضافة شريط تمرير لإرسال قيم رقمية متدرجة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="267"/>
        <source>Toggle</source>
        <translation>مفتاح تبديل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="271"/>
        <source>Add a toggle switch for on/off commands</source>
        <translation>إضافة مفتاح تبديل لأوامر التشغيل/الإيقاف</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="278"/>
        <source>Text Field</source>
        <translation>حقل نصي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="281"/>
        <source>Add a text field for typing and sending commands</source>
        <translation>إضافة حقل نصي لكتابة وإرسال الأوامر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="287"/>
        <source>Knob</source>
        <translation>مقبض دوار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="291"/>
        <source>Add a rotary knob for setpoint control</source>
        <translation>إضافة مقبض دوار للتحكم في نقطة الضبط</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="310"/>
        <source>Edit Code</source>
        <translation>تحرير الكود</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="314"/>
        <source>Edit the JavaScript that draws this painter widget</source>
        <translation>تحرير كود JavaScript الذي يرسم عنصر الرسام هذا</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="327"/>
        <source>Duplicate</source>
        <translation>تكرار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="331"/>
        <source>Duplicate the current group and its contents</source>
        <translation>تكرار المجموعة الحالية ومحتوياتها</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="336"/>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="341"/>
        <source>Delete the current group and all contained datasets</source>
        <translation>حذف المجموعة الحالية وجميع مجموعات البيانات المحتواة</translation>
    </message>
</context>
<context>
    <name>Gyroscope</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="376"/>
        <source>ROLL ↔</source>
        <translation>الدوران ↔</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="404"/>
        <source>YAW ↻</source>
        <translation>الانعراج ↻</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="432"/>
        <source>PITCH ↕</source>
        <translation>الميلان ↕</translation>
    </message>
</context>
<context>
    <name>HID</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="50"/>
        <source>HID Device</source>
        <translation>جهاز HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="80"/>
        <source>Usage Page</source>
        <translation>صفحة الاستخدام</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="96"/>
        <source>Usage</source>
        <translation>الاستخدام</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="137"/>
        <source>Connect gamepads, joysticks, steering wheels, flight controllers, and other HID-class USB devices.</source>
        <translation>توصيل أذرع التحكم والعصي والمقود وأجهزة التحكم بالطيران وأجهزة USB الأخرى من فئة HID.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="145"/>
        <source>HID Usage Tables (USB.org)</source>
        <translation>جداول استخدام HID (USB.org)</translation>
    </message>
</context>
<context>
    <name>HelpCenter</name>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="33"/>
        <source>Help Center</source>
        <translation>مركز المساعدة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="95"/>
        <source>Fetching help pages…</source>
        <translation>جارٍ جلب صفحات المساعدة…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="129"/>
        <source>Search…</source>
        <translation>بحث…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="247"/>
        <source>Loading…</source>
        <translation>جارٍ التحميل…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="291"/>
        <source>Select a page from the sidebar</source>
        <translation>اختر صفحة من الشريط الجانبي</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="321"/>
        <source>Copied to Clipboard</source>
        <translation>تم النسخ إلى الحافظة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="353"/>
        <source>View Online</source>
        <translation>عرض عبر الإنترنت</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="372"/>
        <source>%1 pages</source>
        <translation>%1 صفحة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="381"/>
        <source>Close</source>
        <translation>إغلاق</translation>
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
        <translation>مقبس الشبكة</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="269"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="271"/>
        <source>Audio</source>
        <translation>الصوت</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="272"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="273"/>
        <source>CAN Bus</source>
        <translation>ناقل CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="274"/>
        <source>USB Device</source>
        <translation>جهاز USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="275"/>
        <source>HID Device</source>
        <translation>جهاز HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="276"/>
        <source>Process</source>
        <translation>العملية</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="277"/>
        <source>MQTT Subscriber</source>
        <translation>مشترك MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="639"/>
        <source>Your trial period has ended.</source>
        <translation>انتهت فترة التجربة.</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="640"/>
        <source>To continue using Serial Studio, please activate your license.</source>
        <translation>لمتابعة استخدام Serial Studio، يُرجى تفعيل الترخيص.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Audio</name>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="743"/>
        <source>channels</source>
        <translation>القنوات</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="796"/>
        <source> channels</source>
        <translation>القنوات</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="944"/>
        <source>Unsigned 8-bit</source>
        <translation>8 بت بدون إشارة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="945"/>
        <source>Signed 16-bit</source>
        <translation>16 بت بإشارة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="946"/>
        <source>Signed 24-bit</source>
        <translation>24 بت بإشارة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="947"/>
        <source>Signed 32-bit</source>
        <translation>32 بت بإشارة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="948"/>
        <source>Float 32-bit</source>
        <translation>32 بت عائم</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="951"/>
        <source>Mono</source>
        <translation>أحادي</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="952"/>
        <source>Stereo</source>
        <translation>ستيريو</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1332"/>
        <source>Input Device</source>
        <translation>جهاز الإدخال</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1340"/>
        <source>Sample Rate</source>
        <translation>معدل العينة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1348"/>
        <source>Sample Format</source>
        <translation>تنسيق العينة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1356"/>
        <source>Channels</source>
        <translation>القنوات</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::BluetoothLE</name>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="456"/>
        <source>BLE I/O Module Error</source>
        <translation>خطأ في وحدة الإدخال/الإخراج BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="751"/>
        <source>Select Device</source>
        <translation>اختيار الجهاز</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="762"/>
        <source>Select Service</source>
        <translation>اختيار الخدمة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="773"/>
        <source>Select Characteristic</source>
        <translation>اختيار الخاصية</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="972"/>
        <source>Error while configuring BLE service</source>
        <translation>خطأ أثناء تكوين خدمة BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1190"/>
        <source>Operation error</source>
        <translation>خطأ في العملية</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1193"/>
        <source>Characteristic write error</source>
        <translation>خطأ في كتابة الخاصية</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1196"/>
        <source>Descriptor write error</source>
        <translation>خطأ في كتابة الواصف</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1199"/>
        <source>Unknown error</source>
        <translation>خطأ غير معروف</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1202"/>
        <source>Characteristic read error</source>
        <translation>خطأ في قراءة الخاصية</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1205"/>
        <source>Descriptor read error</source>
        <translation>خطأ في قراءة الواصف</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1440"/>
        <source>BLE Device</source>
        <translation>جهاز BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1448"/>
        <source>Service</source>
        <translation>الخدمة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1455"/>
        <source>Notify Characteristic</source>
        <translation>خاصية الإشعار</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1462"/>
        <source>Characteristic</source>
        <translation>الخاصية</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::CANBus</name>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="273"/>
        <source>CAN Device Creation Failed</source>
        <translation>فشل إنشاء جهاز CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="274"/>
        <source>Unable to create CAN bus device. Check your hardware and drivers.</source>
        <translation>تعذر إنشاء جهاز ناقل CAN. تحقق من العتاد وبرامج التشغيل.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="297"/>
        <source>CAN Connection Failed</source>
        <translation>فشل الاتصال بـ CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="299"/>
        <source>Unable to connect to CAN bus device. Check your hardware connection and settings.</source>
        <translation>تعذر الاتصال بجهاز ناقل CAN. تحقق من توصيل العتاد والإعدادات.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="316"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="322"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="328"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="333"/>
        <source>CAN Bus Not Available</source>
        <translation>ناقل CAN غير متاح</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="317"/>
        <source>No CAN bus plugins found on this system.

On Linux, ensure SocketCAN kernel modules are loaded.</source>
        <translation>لم يتم العثور على إضافات ناقل CAN على هذا النظام.

على Linux، تأكد من تحميل وحدات نواة SOCKETCAN.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="323"/>
        <source>No CAN bus plugins found on this system.

On Windows, install CAN hardware drivers (PEAK, Vector, etc.).</source>
        <translation>لم يتم العثور على إضافات ناقل CAN على هذا النظام.

على Windows، قم بتثبيت برامج تشغيل أجهزة CAN (PEAK، VECTOR، إلخ).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="329"/>
        <source>No CAN bus plugins found on this system.

CAN bus support on macOS is limited and may require third-party hardware drivers.</source>
        <translation>لم يتم العثور على إضافات ناقل CAN على هذا النظام.

دعم ناقل CAN على macOS محدود وقد يتطلب برامج تشغيل أجهزة من جهات خارجية.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="334"/>
        <source>No CAN bus plugins are available on this platform.</source>
        <translation>لا توجد إضافات ناقل CAN متاحة على هذه المنصة.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="346"/>
        <source>Invalid CAN Configuration</source>
        <translation>إعداد CAN غير صالح</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="347"/>
        <source>The CAN bus configuration is incomplete. Select a valid plugin and interface.</source>
        <translation>إعداد ناقل CAN غير مكتمل. حدد إضافة وواجهة صالحة.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="354"/>
        <source>Invalid Selection</source>
        <translation>تحديد غير صالح</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="355"/>
        <source>The selected plugin or interface is no longer available. Refresh the lists and try again.</source>
        <translation>الإضافة أو الواجهة المحددة لم تعد متاحة. قم بتحديث القوائم وحاول مرة أخرى.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="363"/>
        <source>No Devices Available</source>
        <translation>لا توجد أجهزة متاحة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="364"/>
        <source>The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected.</source>
        <translation>قائمة الإضافات أو الواجهات فارغة. حدّث القوائم وتأكد من توصيل عتاد CAN.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="687"/>
        <source>CAN Bus Error</source>
        <translation>خطأ في ناقل CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="688"/>
        <source>An error occurred but the CAN device is no longer available.</source>
        <translation>حدث خطأ لكن جهاز CAN لم يعد متاحًا.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="695"/>
        <source>Error code: %1</source>
        <translation>رمز الخطأ: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="698"/>
        <source>CAN Bus Communication Error</source>
        <translation>خطأ في اتصال ناقل CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="713"/>
        <source>Connect a %1 adapter, then refresh</source>
        <translation>قم بتوصيل محول %1، ثم قم بالتحديث</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="717"/>
        <source>Load SocketCAN kernel modules first</source>
        <translation>حمّل وحدات نواة SOCKETCAN أولاً</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="720"/>
        <source>Set up a virtual CAN interface first</source>
        <translation>أنشئ واجهة CAN افتراضية أولاً</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="722"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="742"/>
        <source>No interfaces found for %1</source>
        <translation>لم يُعثر على واجهات لـ %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="726"/>
        <source>Install &lt;a href='https://www.peak-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN drivers&lt;/a&gt;</source>
        <translation>ثبّت &lt;a href='https://www.PEAK-system.com/Drivers.523.0.html?&amp;L=1'&gt;تعريفات PEAK CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="730"/>
        <source>Install &lt;a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'&gt;Vector CAN drivers&lt;/a&gt;</source>
        <translation>ثبّت &lt;a href='https://www.VECTOR.com/us/en/products/products-a-z/libraries-drivers/'&gt;تعريفات VECTOR CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="734"/>
        <source>Install &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN drivers&lt;/a&gt;</source>
        <translation>تثبيت &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;برامج تشغيل SysTec CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="737"/>
        <source>Install %1 drivers</source>
        <translation>تثبيت برامج تشغيل %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="740"/>
        <source>Install %1 drivers for macOS</source>
        <translation>تثبيت برامج تشغيل %1 لنظام macOS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="755"/>
        <source>No CAN driver selected</source>
        <translation>لم يتم اختيار برنامج تشغيل CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="836"/>
        <source>Plugin</source>
        <translation>الإضافة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="844"/>
        <source>Interface</source>
        <translation>الواجهة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="852"/>
        <source>Bitrate</source>
        <translation>معدل البت</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="861"/>
        <source>CAN FD</source>
        <translation>CAN FD</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="868"/>
        <source>Loopback</source>
        <translation>الحلقة الراجعة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="875"/>
        <source>Listen-Only</source>
        <translation>الاستماع فقط</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::GsUsbCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="381"/>
        <source>Failed to initialize libusb for the CANable adapter.</source>
        <translation>فشل تهيئة libusb لمحول CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="409"/>
        <source>Unable to enumerate USB devices.</source>
        <translation>تعذر تعداد أجهزة USB.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="429"/>
        <source>The selected CANable adapter is no longer connected, or another application has it open. On Windows the device must use the WinUSB driver (candleLight installs it automatically).</source>
        <translation>محول CANable المحدد لم يعد متصلاً، أو تطبيق آخر يستخدمه. على Windows يجب أن يستخدم الجهاز برنامج تشغيل WinUSB (candleLight يثبته تلقائياً).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="442"/>
        <source>Could not claim the CANable USB interface.</source>
        <translation>تعذر المطالبة بواجهة USB الخاصة بـ CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="509"/>
        <source>CANable adapter is not open for writing.</source>
        <translation>محول CANable غير مفتوح للكتابة.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="544"/>
        <source>Failed to transmit CAN frame to the adapter.</source>
        <translation>فشل إرسال إطار CAN إلى المحول.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="560"/>
        <source>CAN bus error reported by the CANable adapter.</source>
        <translation>خطأ في ناقل CAN أبلغ عنه محول CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="615"/>
        <source>A CAN frame was not acknowledged on the bus.</source>
        <translation>لم يتم الإقرار بإطار CAN على الناقل.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="707"/>
        <source>CANable adapter rejected the host-format handshake.</source>
        <translation>محول CANable رفض مصافحة تنسيق المضيف.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="714"/>
        <source>Could not read CANable timing constants.</source>
        <translation>تعذرت قراءة ثوابت التوقيت الخاصة بـ CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="720"/>
        <source>The bitrate %1 bps is not supported by this CANable adapter.</source>
        <translation>معدل البت %1 bps غير مدعوم من قبل محول CANable هذا.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="726"/>
        <source>CANable adapter rejected the requested bitrate.</source>
        <translation>محول CANable رفض معدل البت المطلوب.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="740"/>
        <source>Could not start the CANable channel.</source>
        <translation>تعذر بدء قناة CANable.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::HID</name>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="173"/>
        <source>Unknown error</source>
        <translation>خطأ غير معروف</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="176"/>
        <source>

Check that your user is in the 'plugdev' group or that a udev rule grants access to this device.</source>
        <translation>تحقق من أن المستخدم ضمن مجموعة 'plugdev' أو أن قاعدة udev تمنح الوصول إلى هذا الجهاز.

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="180"/>
        <source>Failed to open "%1"</source>
        <translation>فشل فتح "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="285"/>
        <source>HID Device Error</source>
        <translation>خطأ في جهاز HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="286"/>
        <source>The HID device was disconnected or encountered a fatal read error.</source>
        <translation>تم فصل جهاز HID أو حدث خطأ قراءة فادح.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="395"/>
        <source>Select Device</source>
        <translation>اختيار الجهاز</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="537"/>
        <source>HID Device</source>
        <translation>جهاز HID</translation>
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
        <translation>TLS 1.3 أو أحدث</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="62"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 أو أحدث</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="63"/>
        <source>Any Protocol</source>
        <translation>أي بروتوكول</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="64"/>
        <source>Secure Protocols Only</source>
        <translation>البروتوكولات الآمنة فقط</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="66"/>
        <source>None</source>
        <translation>بدون</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="67"/>
        <source>Query Peer</source>
        <translation>استعلام النظير</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="68"/>
        <source>Verify Peer</source>
        <translation>التحقق من النظير</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="69"/>
        <source>Auto Verify Peer</source>
        <translation>التحقق التلقائي من النظير</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="167"/>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation>ميزة MQTT تتطلب ترخيصاً تجارياً</translation>
    </message>
    <message>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio commercial license (Hobbyist tier or above).</source>
        <translation type="vanished">الاشتراك في وسيط MQTT متاح فقط مع ترخيص Serial Studio تجاري صالح (مستوى Hobbyist أو أعلى).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="168"/>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio license or an active trial.</source>
        <translation>الاشتراك في وسيط MQTT متاح فقط مع ترخيص Serial Studio صالح أو فترة تجريبية نشطة.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="393"/>
        <source>Use System Database</source>
        <translation>استخدام قاعدة بيانات النظام</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="394"/>
        <source>Load From Folder…</source>
        <translation>تحميل من مجلد…</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="427"/>
        <source>Select PEM Certificates Directory</source>
        <translation>تحديد دليل شهادات PEM</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="682"/>
        <source>Hostname</source>
        <translation>اسم المضيف</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="689"/>
        <source>Port</source>
        <translation>المنفذ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="698"/>
        <source>Topic Filter</source>
        <translation>مرشح الموضوع</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="705"/>
        <source>Client ID</source>
        <translation>معرف العميل</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="712"/>
        <source>Username</source>
        <translation>اسم المستخدم</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="719"/>
        <source>Password</source>
        <translation>كلمة المرور</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="726"/>
        <source>MQTT Version</source>
        <translation>إصدار MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="734"/>
        <source>Clean Session</source>
        <translation>جلسة نظيفة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="741"/>
        <source>Keep Alive (s)</source>
        <translation>الإبقاء على الاتصال (ث)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="750"/>
        <source>Auto Keep Alive</source>
        <translation>الإبقاء على الاتصال تلقائيًا</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="767"/>
        <source>SSL/TLS Enabled</source>
        <translation>SSL/TLS مُفعّل</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="777"/>
        <source>SSL Protocol</source>
        <translation>بروتوكول SSL</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="785"/>
        <source>Peer Verify Mode</source>
        <translation>وضع التحقق من النظير</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="793"/>
        <source>Peer Verify Depth</source>
        <translation>عمق التحقق من النظير</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="894"/>
        <source>MQTT Subscription Error</source>
        <translation>خطأ في اشتراك MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="895"/>
        <source>Failed to subscribe to topic "%1".</source>
        <translation>فشل الاشتراك في الموضوع "%1".</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="922"/>
        <source>Invalid MQTT Protocol Version</source>
        <translation>إصدار بروتوكول MQTT غير صالح</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="923"/>
        <source>The broker rejected the configured MQTT protocol version.</source>
        <translation>رفض الوسيط إصدار بروتوكول MQTT المُحدد.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="926"/>
        <source>Client ID Rejected</source>
        <translation>رُفض معرّف العميل</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="927"/>
        <source>The broker rejected the client ID. Try a different identifier.</source>
        <translation>رفض الوسيط معرّف العميل. جرّب معرّفاً مختلفاً.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="930"/>
        <source>MQTT Server Unavailable</source>
        <translation>خادم MQTT غير متاح</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="931"/>
        <source>The broker is currently unavailable. Retry later.</source>
        <translation>الوسيط غير متاح حالياً. أعد المحاولة لاحقاً.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="934"/>
        <source>Authentication Error</source>
        <translation>خطأ في المصادقة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="935"/>
        <source>The credentials provided were rejected by the broker.</source>
        <translation>رفض الوسيط بيانات الاعتماد المُقدمة.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="938"/>
        <source>Authorization Error</source>
        <translation>خطأ في التفويض</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="939"/>
        <source>Account lacks permission for this operation.</source>
        <translation>الحساب لا يملك صلاحية لهذه العملية.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="942"/>
        <source>Network or Transport Error</source>
        <translation>خطأ في الشبكة أو النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="943"/>
        <source>Network/transport layer issue while connecting to the broker.</source>
        <translation>مشكلة في طبقة الشبكة/النقل أثناء الاتصال بـ Broker.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="946"/>
        <source>MQTT Protocol Violation</source>
        <translation>انتهاك بروتوكول MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="947"/>
        <source>The broker reported a protocol violation and closed the connection.</source>
        <translation>أبلغ Broker عن انتهاك للبروتوكول وأغلق الاتصال.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="950"/>
        <source>MQTT 5 Error</source>
        <translation>خطأ MQTT 5</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="951"/>
        <source>An MQTT 5 protocol-level error occurred.</source>
        <translation>حدث خطأ على مستوى بروتوكول MQTT 5.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="954"/>
        <source>MQTT Error</source>
        <translation>خطأ MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="955"/>
        <source>An unexpected MQTT error occurred.</source>
        <translation>حدث خطأ MQTT غير متوقع.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Modbus</name>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="364"/>
        <source>Invalid Serial Port</source>
        <translation>منفذ تسلسلي غير صالح</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="365"/>
        <source>The selected serial port "%1" is no longer available. Refresh the port list and try again.</source>
        <translation>المنفذ التسلسلي المحدد "%1" لم يعد متاحاً. قم بتحديث قائمة المنافذ وحاول مرة أخرى.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="409"/>
        <source>Modbus Initialization Failed</source>
        <translation>فشل تهيئة Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="410"/>
        <source>Unable to create Modbus device. Check your system configuration and try again.</source>
        <translation>تعذر إنشاء جهاز Modbus. تحقق من إعدادات النظام وأعد المحاولة.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="435"/>
        <source>Modbus Connection Failed</source>
        <translation>فشل اتصال Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="437"/>
        <source>Unable to connect to "%1". Check your connection settings.</source>
        <translation>تعذر الاتصال بـ "%1". تحقق من إعدادات الاتصال.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="438"/>
        <source>"%1": %2</source>
        <translation>"%1": %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="559"/>
        <source>None</source>
        <translation>بدون</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="560"/>
        <source>Even</source>
        <translation>زوجي</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="561"/>
        <source>Odd</source>
        <translation>فردي</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="562"/>
        <source>Space</source>
        <translation>مسافة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="563"/>
        <source>Mark</source>
        <translation>علامة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="615"/>
        <source>Holding Registers (0x03)</source>
        <translation>سجلات الاحتفاظ (0x03)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="616"/>
        <source>Input Registers (0x04)</source>
        <translation>سجلات الإدخال (0x04)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="617"/>
        <source>Coils (0x01)</source>
        <translation>الملفات (0x01)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="618"/>
        <source>Discrete Inputs (0x02)</source>
        <translation>مداخل منفصلة (0x02)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="800"/>
        <source>No register groups configured</source>
        <translation>لم يتم تكوين مجموعات سجلات</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="801"/>
        <source>Add at least one register group before generating a project.</source>
        <translation>أضف مجموعة سجلات واحدة على الأقل قبل إنشاء المشروع.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="803"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="815"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="840"/>
        <source>Modbus Project Generator</source>
        <translation>مولّد مشروع Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="812"/>
        <source>Failed to load generated project</source>
        <translation>فشل تحميل المشروع المُنشأ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="813"/>
        <source>The generated project JSON could not be loaded.</source>
        <translation>تعذر تحميل JSON للمشروع المُنشأ.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="835"/>
        <source>Successfully generated project with %1 groups and %2 datasets.</source>
        <translation>تم إنشاء المشروع بنجاح بـ %1 مجموعة و %2 مجموعة بيانات.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="838"/>
        <source>The project editor is now open for customization.</source>
        <translation>محرر المشروع مفتوح الآن للتخصيص.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="853"/>
        <source>Modbus Project</source>
        <translation>مشروع Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="858"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="878"/>
        <source>Holding Registers</source>
        <translation>سجلات الاحتفاظ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="879"/>
        <source>Input Registers</source>
        <translation>سجلات الإدخال</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="880"/>
        <source>Coils</source>
        <translation>الملفات</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="881"/>
        <source>Discrete Inputs</source>
        <translation>المدخلات المنفصلة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="895"/>
        <source>Unknown</source>
        <translation>غير معروف</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="908"/>
        <source>Register %1</source>
        <translation>السجل %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="916"/>
        <source>Coil %1</source>
        <translation>الملف %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="916"/>
        <source>Discrete %1</source>
        <translation>المنفصل %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1322"/>
        <source>Error code: %1</source>
        <translation>رمز الخطأ: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1325"/>
        <source>Modbus Communication Error</source>
        <translation>خطأ اتصال Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1337"/>
        <source>Select Port</source>
        <translation>اختيار المنفذ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1490"/>
        <source>Protocol</source>
        <translation>البروتوكول</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1498"/>
        <source>Slave Address</source>
        <translation>عنوان الجهاز التابع</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1507"/>
        <source>Poll Interval (ms)</source>
        <translation>فترة الاستقصاء (ميلي ثانية)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1547"/>
        <source>Host / IP</source>
        <translation>المضيف / IP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1554"/>
        <source>Port</source>
        <translation>المنفذ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1569"/>
        <source>Serial Port</source>
        <translation>المنفذ التسلسلي</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1577"/>
        <source>Baud Rate</source>
        <translation>معدل البود</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1585"/>
        <source>Parity</source>
        <translation>التكافؤ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1593"/>
        <source>Data Bits</source>
        <translation>بتات البيانات</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1601"/>
        <source>Stop Bits</source>
        <translation>بتات الإيقاف</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Network</name>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="473"/>
        <source>Network socket error</source>
        <translation>خطأ في مقبس الشبكة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="489"/>
        <source>Socket Type</source>
        <translation>نوع المقبس</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="497"/>
        <source>Remote Address</source>
        <translation>العنوان البعيد</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="505"/>
        <source>TCP Port</source>
        <translation>منفذ TCP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="514"/>
        <source>UDP Local Port</source>
        <translation>المنفذ المحلي UDP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="523"/>
        <source>UDP Remote Port</source>
        <translation>المنفذ البعيد UDP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="532"/>
        <source>UDP Multicast</source>
        <translation>البث المتعدد UDP</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Process</name>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="183"/>
        <location filename="../../src/IO/Drivers/Process.cpp" line="224"/>
        <source>Failed to start process</source>
        <translation>فشل بدء العملية</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="184"/>
        <source>Executable "%1" not found in PATH.</source>
        <translation>الملف التنفيذي "%1" غير موجود في PATH.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="368"/>
        <source>Select Executable</source>
        <translation>اختيار الملف التنفيذي</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="392"/>
        <source>Select Working Directory</source>
        <translation>اختيار دليل العمل</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="417"/>
        <source>Select Named Pipe / FIFO</source>
        <translation>اختيار الأنبوب المسمى / FIFO</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="514"/>
        <source>The process crashed.</source>
        <translation>تعطلت العملية.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="515"/>
        <source>Exit code: %1</source>
        <translation>كود الخروج: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="518"/>
        <source>Process "%1" stopped</source>
        <translation>توقفت العملية "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="536"/>
        <source>Unknown error</source>
        <translation>خطأ غير معروف</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="537"/>
        <source>Process Error</source>
        <translation>خطأ في العملية</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Pipe Error</source>
        <translation>خطأ في الأنبوب</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Could not open named pipe: %1</source>
        <translation>تعذر فتح الأنبوب المسمى: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="763"/>
        <source>Mode</source>
        <translation>الوضع</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Launch Process</source>
        <translation>تشغيل العملية</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Named Pipe</source>
        <translation>أنبوب مسمى</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="771"/>
        <source>Executable</source>
        <translation>الملف التنفيذي</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="778"/>
        <source>Arguments</source>
        <translation>المعاملات</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="785"/>
        <source>Working Directory</source>
        <translation>دليل العمل</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="792"/>
        <source>Pipe Path</source>
        <translation>مسار الأنبوب</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SeeedCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="205"/>
        <source>The bitrate %1 bps is not supported by the USB-CAN Analyzer.</source>
        <translation>معدل البت %1 bps غير مدعوم من قبل محلل USB-CAN.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="215"/>
        <source>Could not open serial port %1: %2</source>
        <translation>تعذر فتح المنفذ التسلسلي %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="226"/>
        <source>Failed to initialize the USB-CAN Analyzer.</source>
        <translation>فشل تهيئة محلل USB-CAN.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="263"/>
        <source>USB-CAN Analyzer is not open for writing.</source>
        <translation>محلل USB-CAN غير مفتوح للكتابة.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="309"/>
        <source>CAN bus error reported by the USB-CAN Analyzer.</source>
        <translation>تم الإبلاغ عن خطأ في ناقل CAN من قبل محلل USB-CAN.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SlcanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="166"/>
        <source>The bitrate %1 bps is not a standard slcan rate.</source>
        <translation>معدل البت %1 bps ليس معدل slcan قياسي.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="176"/>
        <source>Could not open serial port %1: %2</source>
        <translation>تعذر فتح المنفذ التسلسلي %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="189"/>
        <source>Failed to open the slcan channel.</source>
        <translation>فشل فتح قناة slcan.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="228"/>
        <source>slcan adapter is not open for writing.</source>
        <translation>محول slcan غير مفتوح للكتابة.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="266"/>
        <source>CAN bus error reported by the slcan adapter.</source>
        <translation>تم الإبلاغ عن خطأ في ناقل CAN بواسطة محول slcan.</translation>
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
        <translation>بلا</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="245"/>
        <source>Failed to connect to serial port "%1"</source>
        <translation>فشل الاتصال بالمنفذ التسلسلي "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="333"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="717"/>
        <source>Select Port</source>
        <translation>اختيار المنفذ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="376"/>
        <source>Even</source>
        <translation>زوجي</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="377"/>
        <source>Odd</source>
        <translation>فردي</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="378"/>
        <source>Space</source>
        <translation>مسافة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="379"/>
        <source>Mark</source>
        <translation>علامة</translation>
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
        <translation>"%1" ليس مساراً صالحاً</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="547"/>
        <source>Please type another path to register a custom serial device</source>
        <translation>اكتب مساراً آخر لتسجيل جهاز تسلسلي مخصص</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="782"/>
        <source>Unknown</source>
        <translation>غير معروف</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="783"/>
        <source>Critical error on serial port "%1"</source>
        <translation>خطأ حرج في المنفذ التسلسلي "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="784"/>
        <source>Unknown error</source>
        <translation>خطأ غير معروف</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="806"/>
        <source>No error occurred.</source>
        <translation>لم يحدث خطأ.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="807"/>
        <source>The specified device could not be found. Check the connection and try again.</source>
        <translation>تعذر العثور على الجهاز المحدد. تحقق من الاتصال وأعد المحاولة.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="808"/>
        <source>Permission denied. Ensure the application has the necessary access rights to the device.</source>
        <translation>رُفض الإذن. تأكد من أن التطبيق لديه صلاحيات الوصول اللازمة للجهاز.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="809"/>
        <source>Failed to open the device. It may already be in use or unavailable.</source>
        <translation>فشل فتح الجهاز. قد يكون قيد الاستخدام أو غير متاح.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="810"/>
        <source>An error occurred while writing data to the device.</source>
        <translation>حدث خطأ أثناء كتابة البيانات إلى الجهاز.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="811"/>
        <source>An error occurred while reading data from the device.</source>
        <translation>حدث خطأ أثناء قراءة البيانات من الجهاز.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="812"/>
        <source>A critical resource error occurred. The device may have been disconnected or is no longer accessible.</source>
        <translation>حدث خطأ حرج في المورد. قد يكون الجهاز قد فُصل أو لم يعد متاحًا.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="813"/>
        <source>The requested operation is not supported on this device.</source>
        <translation>العملية المطلوبة غير مدعومة على هذا الجهاز.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="814"/>
        <source>An unknown error occurred. Check the device and try again.</source>
        <translation>حدث خطأ غير معروف. تحقق من الجهاز وحاول مرة أخرى.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="815"/>
        <source>The operation timed out. The device may not be responding.</source>
        <translation>انتهت مهلة العملية. قد لا يستجيب الجهاز.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="816"/>
        <source>The device is not open. Open the device before attempting this operation.</source>
        <translation>الجهاز غير مفتوح. افتح الجهاز قبل محاولة هذه العملية.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="978"/>
        <source>Serial Port</source>
        <translation>المنفذ التسلسلي</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="986"/>
        <source>Baud Rate</source>
        <translation>معدل البود</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="994"/>
        <source>Parity</source>
        <translation>التماثل</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1002"/>
        <source>Data Bits</source>
        <translation>بتات البيانات</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1010"/>
        <source>Stop Bits</source>
        <translation>بتات الإيقاف</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1018"/>
        <source>Flow Control</source>
        <translation>التحكم بالتدفق</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1026"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1033"/>
        <source>Auto-Reconnect</source>
        <translation>إعادة الاتصال التلقائي</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::USB</name>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="169"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="177"/>
        <source>USB Error</source>
        <translation>خطأ USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="170"/>
        <source>Failed to initialize the USB subsystem. Check that libusb is available on your system.</source>
        <translation>فشل تهيئة نظام USB الفرعي. تحقق من توفر libusb على نظامك.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="178"/>
        <source>No USB device selected. Select a device and try again.</source>
        <translation>لم يتم تحديد جهاز USB. حدد جهازًا وحاول مرة أخرى.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="184"/>
        <source>Unknown Device</source>
        <translation>جهاز غير معروف</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="189"/>
        <source>Failed to open "%1"</source>
        <translation>فشل فتح "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="190"/>
        <source>Could not open the USB device: %1.

On Linux, ensure you have read/write permission on the device node (add a udev rule or run as root). On macOS, the kernel driver may need to be detached first.</source>
        <translation>تعذر فتح جهاز USB: %1.

على Linux، تأكد من امتلاكك صلاحية القراءة/الكتابة على عقدة الجهاز (أضف قاعدة udev أو شغّل كمستخدم جذر). على macOS، قد يلزم فصل برنامج تشغيل النواة أولاً.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="212"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="227"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="612"/>
        <source>USB Device Error</source>
        <translation>خطأ في جهاز USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="228"/>
        <source>Could not claim interface %1 on the USB device.

Another driver or application may already have it open. On Linux, try unloading the kernel driver (e.g. cdc_acm) or adding a udev rule.</source>
        <translation>تعذر المطالبة بالواجهة %1 على جهاز USB.

قد يكون برنامج تشغيل أو تطبيق آخر قد فتحه بالفعل. على Linux، حاول إلغاء تحميل برنامج تشغيل النواة (مثل cdc_acm) أو إضافة قاعدة udev.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="386"/>
        <source>Select Device</source>
        <translation>اختيار الجهاز</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="405"/>
        <source>Select IN Endpoint</source>
        <translation>اختيار نقطة نهاية IN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="416"/>
        <source>None (Read-only)</source>
        <translation>لا شيء (قراءة فقط)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="490"/>
        <source>Enable Advanced USB Control Transfers?</source>
        <translation>تفعيل عمليات النقل التحكمية USB المتقدمة؟</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="491"/>
        <source>This enables control transfers in addition to bulk transfers. Sending incorrect control requests can crash or damage connected hardware. Only enable this if you know what you are doing.</source>
        <translation>يفعّل هذا عمليات النقل التحكمية بالإضافة إلى عمليات النقل الكتلية. إرسال طلبات تحكم غير صحيحة قد يتسبب في تعطل أو تلف الأجهزة المتصلة. فعّل هذا فقط إذا كنت تعرف ما تفعله.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="495"/>
        <source>Advanced USB Mode</source>
        <translation>وضع USB المتقدم</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="613"/>
        <source>The USB device was disconnected or encountered a fatal read error.</source>
        <translation>تم فصل جهاز USB أو واجه خطأ قراءة فادح.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="750"/>
        <source>No isochronous IN endpoint was found on this device, but bulk endpoints are available.

Switch the Transfer Mode to "Bulk Stream" and try again.</source>
        <translation>لم يتم العثور على نقطة نهاية IN متزامنة على هذا الجهاز، لكن نقاط نهاية كتلية متاحة.

بدّل وضع النقل إلى "تدفق كتلي" وحاول مرة أخرى.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="755"/>
        <source>No bulk IN endpoint was found on this device, but isochronous endpoints are available.

Switch the Transfer Mode to "Isochronous" and try again.</source>
        <translation>لم يتم العثور على نقطة نهاية IN كتلية على هذا الجهاز، لكن نقاط نهاية متزامنة متاحة.

بدّل وضع النقل إلى "متزامن" وحاول مرة أخرى.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="759"/>
        <source>No usable IN endpoint was found on this device.

The device may not expose data endpoints in its active configuration, or it may require a specific driver.</source>
        <translation>لم يتم العثور على نقطة نهاية IN قابلة للاستخدام على هذا الجهاز.

قد لا يكشف الجهاز عن نقاط نهاية بيانات في تكوينه النشط، أو قد يتطلب برنامج تشغيل محدد.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1185"/>
        <source>USB Device</source>
        <translation>جهاز USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1193"/>
        <source>Transfer Mode</source>
        <translation>وضع النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1196"/>
        <source>Bulk Stream</source>
        <translation>تدفق مجمّع</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1196"/>
        <source>Advanced Control</source>
        <translation>تحكم متقدم</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1196"/>
        <source>Isochronous</source>
        <translation>متزامن</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1201"/>
        <source>IN Endpoint</source>
        <translation>نقطة نهاية IN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1209"/>
        <source>OUT Endpoint</source>
        <translation>نقطة نهاية OUT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1217"/>
        <source>ISO Packet Size</source>
        <translation>حجم حزمة ISO</translation>
    </message>
</context>
<context>
    <name>IO::FileTransmission</name>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="211"/>
        <source>No file selected…</source>
        <translation>لم يتم تحديد ملف…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="246"/>
        <source>Plain Text</source>
        <translation>نص عادي</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="247"/>
        <source>Raw Binary</source>
        <translation>ثنائي خام</translation>
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
        <translation>تحديد الملف المراد إرساله</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="291"/>
        <source>File selected: %1 (%2 bytes)</source>
        <translation>الملف المحدد: %1 (%2 بايت)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="294"/>
        <source>Error opening file: %1</source>
        <translation>خطأ في فتح الملف: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="380"/>
        <source>Starting %1 transfer…</source>
        <translation>بدء نقل %1…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="608"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="628"/>
        <source>Transmission complete</source>
        <translation>اكتمل الإرسال</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="610"/>
        <source>Plain text transmission complete</source>
        <translation>اكتمل إرسال النص العادي</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="630"/>
        <source>Raw binary transmission complete (%1 bytes)</source>
        <translation>اكتمل إرسال البيانات الثنائية الخام (%1 بايت)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="654"/>
        <source>Transfer complete</source>
        <translation>اكتمل النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="655"/>
        <source>Transfer completed successfully (%1 bytes)</source>
        <translation>اكتمل النقل بنجاح (%1 بايت)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="657"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="658"/>
        <source>Transfer failed: %1</source>
        <translation>فشل النقل: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="742"/>
        <source>%1 B/s</source>
        <translation>%1 بايت/ث</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="744"/>
        <source>%1 KB/s</source>
        <translation>%1 كيلوبايت/ث</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="746"/>
        <source>%1 MB/s</source>
        <translation>%1 ميجابايت/ث</translation>
    </message>
</context>
<context>
    <name>IO::FrameReader</name>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="350"/>
        <source>Frames dropped</source>
        <translation>إطارات مُسقَطة</translation>
    </message>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="352"/>
        <source>Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) have been dropped. Reduce the data rate or disable a heavy consumer.</source>
        <translation>البيانات الواردة تصل أسرع من قدرة Serial Studio على معالجتها؛ تم إسقاط %1 إطار. قلل معدل البيانات أو عطّل مستهلكًا ثقيلًا.</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::XMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="83"/>
        <source>Cannot open file: %1</source>
        <translation>تعذر فتح الملف: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="93"/>
        <source>Waiting for receiver…</source>
        <translation>في انتظار المستقبِل…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="112"/>
        <source>Transfer cancelled</source>
        <translation>تم إلغاء النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="113"/>
        <source>Transfer cancelled by user</source>
        <translation>تم إلغاء النقل من قبل المستخدم</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="142"/>
        <source>Too many retries, transfer aborted</source>
        <translation>عدد كبير جداً من المحاولات، تم إيقاف النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="143"/>
        <source>Maximum retries exceeded</source>
        <translation>تم تجاوز الحد الأقصى للمحاولات</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="147"/>
        <source>NAK received, retrying block %1 (%2/%3)</source>
        <translation>تم استقبال NAK، إعادة محاولة الكتلة %1 (%2/%3)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="155"/>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="384"/>
        <source>Failed to seek in file</source>
        <translation>فشل التنقل في الملف</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="165"/>
        <source>Transfer cancelled by receiver</source>
        <translation>تم إلغاء النقل من قبل المستقبِل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="166"/>
        <source>Receiver cancelled the transfer</source>
        <translation>ألغى المستقبِل النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="180"/>
        <source>Transfer complete</source>
        <translation>اكتمل النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="207"/>
        <source>Receiver ready (CRC mode), sending data…</source>
        <translation>المستقبِل جاهز (وضع CRC)، جارٍ إرسال البيانات…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="302"/>
        <source>File read returned more data than requested</source>
        <translation>قراءة الملف أرجعت بيانات أكثر من المطلوب</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="316"/>
        <source>Sending block %1 (%2 bytes)</source>
        <translation>إرسال الكتلة %1 (%2 بايت)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="329"/>
        <source>Sending EOT…</source>
        <translation>إرسال EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="370"/>
        <source>Transfer timed out</source>
        <translation>انتهت مهلة النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="371"/>
        <source>Timeout: no response from receiver</source>
        <translation>انتهت المهلة: لا استجابة من المستقبِل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="375"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>انتهت المهلة، إعادة المحاولة (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::YMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="62"/>
        <source>Cannot open file: %1</source>
        <translation>تعذر فتح الملف: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="74"/>
        <source>Waiting for receiver…</source>
        <translation>انتظار المستقبِل…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="96"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="173"/>
        <source>Transfer cancelled by receiver</source>
        <translation>ألغى المستقبِل النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="97"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="174"/>
        <source>Receiver cancelled the transfer</source>
        <translation>ألغى المستقبِل النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="133"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="316"/>
        <source>Sending first EOT…</source>
        <translation>إرسال EOT الأول…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="151"/>
        <source>Too many retries, transfer aborted</source>
        <translation>محاولات كثيرة جدًا، تم إحباط النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="152"/>
        <source>Maximum retries exceeded</source>
        <translation>تم تجاوز الحد الأقصى للمحاولات</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="156"/>
        <source>NAK received, retrying block %1</source>
        <translation>تم استقبال NAK، إعادة محاولة الكتلة %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="161"/>
        <source>Failed to seek in file</source>
        <translation>فشل التنقل في الملف</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="220"/>
        <source>Sending second EOT…</source>
        <translation>إرسال EOT الثاني…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="242"/>
        <source>Transfer complete</source>
        <translation>اكتمل النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="284"/>
        <source>Sending file header: %1 (%2 bytes)</source>
        <translation>إرسال رأس الملف: %1 (%2 بايت)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="299"/>
        <source>Sending end-of-batch marker…</source>
        <translation>إرسال علامة نهاية الدفعة…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="330"/>
        <source>Sending block %1 (%2/%3 bytes)</source>
        <translation>إرسال الكتلة %1 (%2/%3 بايت)</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::ZMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="86"/>
        <source>Cannot open file: %1</source>
        <translation>لا يمكن فتح الملف: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="103"/>
        <source>File is too large for ZMODEM (%1 bytes, limit 4 GiB).</source>
        <translation>الملف كبير جدًا لبروتوكول ZMODEM (%1 بايت، الحد الأقصى 4 جيجابايت).</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="128"/>
        <source>Transfer cancelled</source>
        <translation>تم إلغاء النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="129"/>
        <source>Transfer cancelled by user</source>
        <translation>تم إلغاء النقل من قبل المستخدم</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="269"/>
        <source>Hex header CRC mismatch, dropping frame</source>
        <translation>عدم تطابق CRC لرأس Hex، تم إسقاط الإطار</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="420"/>
        <source>Sending ZRQINIT, waiting for receiver…</source>
        <translation>إرسال ZRQINIT، في انتظار المستقبِل…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="444"/>
        <source>Sending file info: %1 (%2 bytes)</source>
        <translation>إرسال معلومات الملف: %1 (%2 بايت)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="459"/>
        <source>Failed to seek to offset %1</source>
        <translation>فشل الانتقال إلى الموضع %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="488"/>
        <source>File read returned more data than requested</source>
        <translation>قراءة الملف أرجعت بيانات أكثر من المطلوب</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="514"/>
        <source>File data sent, waiting for confirmation…</source>
        <translation>تم إرسال بيانات الملف، في انتظار التأكيد…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="525"/>
        <source>Sending ZFIN…</source>
        <translation>إرسال ZFIN…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="561"/>
        <source>Receiver ready, sending file info…</source>
        <translation>المستقبِل جاهز، إرسال معلومات الملف…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="571"/>
        <source>Receiver requests data from offset %1</source>
        <translation>المستقبِل يطلب البيانات من الإزاحة %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="579"/>
        <source>Receiver skipped the file</source>
        <translation>المستقبِل تخطى الملف</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="591"/>
        <source>Too many errors, transfer aborted</source>
        <translation>أخطاء كثيرة جدًا، تم إيقاف النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="592"/>
        <source>Maximum retries exceeded</source>
        <translation>تم تجاوز الحد الأقصى لمحاولات الإعادة</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="596"/>
        <source>NAK received, retrying (%1/%2)…</source>
        <translation>تم استلام NAK، إعادة المحاولة (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="617"/>
        <source>Transfer complete</source>
        <translation>اكتمل النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="627"/>
        <source>Transfer cancelled by receiver</source>
        <translation>تم إلغاء النقل من قِبل المستقبِل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="628"/>
        <source>Receiver cancelled the transfer</source>
        <translation>المستقبِل ألغى النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="636"/>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="637"/>
        <source>Receiver reported a file error</source>
        <translation>أبلغ المستقبِل عن خطأ في الملف</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="835"/>
        <source>Transfer timed out</source>
        <translation>انتهت مهلة النقل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="836"/>
        <source>Timeout: no response from receiver</source>
        <translation>انتهت المهلة: لا استجابة من المستقبِل</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="840"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>انتهت المهلة، إعادة المحاولة (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="42"/>
        <source>Select Icon</source>
        <translation>اختيار الأيقونة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="119"/>
        <source>Search Online…</source>
        <translation>البحث عبر الإنترنت…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="132"/>
        <source>OK</source>
        <translation>موافق</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="144"/>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
</context>
<context>
    <name>ImageView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="68"/>
        <source>Normal</source>
        <translation>عادي</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="69"/>
        <source>Grayscale</source>
        <translation>تدرج رمادي</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="70"/>
        <source>High Contrast</source>
        <translation>تباين عالٍ</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="71"/>
        <source>Vivid</source>
        <translation>حيوي</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="72"/>
        <source>Night Vision</source>
        <translation>رؤية ليلية</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="73"/>
        <source>Infrared</source>
        <translation>الأشعة تحت الحمراء</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="74"/>
        <source>Deep Blue</source>
        <translation>أزرق عميق</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="75"/>
        <source>Amber</source>
        <translation>كهرماني</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="172"/>
        <source>Export Images</source>
        <translation>تصدير الصور</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="182"/>
        <source>Open Export Folder</source>
        <translation>فتح مجلد التصدير</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="198"/>
        <source>Zoom In</source>
        <translation>تكبير</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="211"/>
        <source>Zoom Out</source>
        <translation>تصغير</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="231"/>
        <source>Show Crosshair</source>
        <translation>إظهار الشعيرات المتصالبة</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="556"/>
        <source>Waiting for Image…</source>
        <translation>في انتظار الصورة…</translation>
    </message>
</context>
<context>
    <name>KeyManagerDialog</name>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="23"/>
        <source>API Keys</source>
        <translation>مفاتيح API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="47"/>
        <source>Anthropic Claude. The default is Claude Haiku 4.5 ($1 input / $5 output per million tokens). Sonnet 4.6 and Opus 4.7 are also available. Supports streaming, tool use, extended thinking, and prompt caching.</source>
        <translation>Anthropic Claude. الافتراضي هو Claude Haiku 4.5 ($1 إدخال / $5 إخراج لكل مليون رمز). Sonnet 4.6 و OPUS 4.7 متاحان أيضًا. يدعم البث، استخدام الأدوات، التفكير الموسع، والتخزين المؤقت للمطالبات.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="52"/>
        <source>OpenAI Chat Completions. The default is GPT-5 mini for fast, cost-conscious agentic work. GPT-5.2 is the stronger general-purpose option, and GPT-5.2 Chat tracks the model currently used in ChatGPT.</source>
        <translation>OpenAI Chat Completions. الافتراضي هو GPT-5 mini للعمل الوكيلي السريع والموفر للتكلفة. GPT-5.2 هو الخيار الأقوى للأغراض العامة، و GPT-5.2 Chat يتتبع النموذج المستخدم حاليًا في ChatGPT.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="57"/>
        <source>Google Gemini. The default is Gemini 2.0 Flash, which has a generous free tier (subject to rate limits). Gemini 1.5 Pro and Gemini 1.5 Flash are also available.</source>
        <translation>Google Gemini. الافتراضي هو Gemini 2.0 Flash، الذي يحتوي على مستوى مجاني سخي (يخضع لحدود المعدل). Gemini 1.5 Pro و Gemini 1.5 Flash متاحان أيضًا.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="61"/>
        <source>DeepSeek. OpenAI-compatible API. The default is deepseek-chat (V3). deepseek-reasoner (R1) is also available. Often the cheapest cloud option for tool use.</source>
        <translation>DeepSeek. واجهة API متوافقة مع OpenAI. الافتراضي هو deepseek-chat (V3). deepseek-reasoner (R1) متاح أيضًا. غالبًا الخيار السحابي الأرخص لاستخدام الأدوات.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="65"/>
        <source>OpenRouter. One key, ~200 models from Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier models (suffixed :free) are rate-limited but require no additional billing. Pay-as-you-go for the rest.</source>
        <translation>OpenRouter. مفتاح واحد، ~200 نموذج من Anthropic و OpenAI و Google و Meta و Mistral و DeepSeek و Qwen وآخرين. النماذج المجانية (بلاحقة :free) محدودة المعدل لكن لا تتطلب فوترة إضافية. الدفع حسب الاستخدام للباقي.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="70"/>
        <source>Groq. Hardware-accelerated inference (LPUs) for very fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen models. Generous free tier with daily token limits.</source>
        <translation>Groq. استنتاج معجل بالأجهزة (LPUs) لنماذج Llama و Mixtral و Gemma و DeepSeek-R1 distill و Qwen السريعة جدًا. مستوى مجاني سخي مع حدود رموز يومية.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="74"/>
        <source>Mistral. The default is Mistral Large. Codestral targets code completion, Pixtral handles vision, and the Ministral models are tuned for edge / low-latency use.</source>
        <translation>Mistral. الافتراضي هو Mistral Large. Codestral يستهدف إكمال الكود، Pixtral يتعامل مع الرؤية، ونماذج Ministral مضبوطة للاستخدام الطرفي / منخفض الكمون.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="78"/>
        <source>Local model server. Works with any OpenAI-compatible endpoint -- Ollama, llama.cpp's llama-server, LM Studio, or vLLM. Nothing leaves your machine. The model list is queried live from the server.</source>
        <translation>خادم نموذج محلي. يعمل مع أي نقطة نهاية متوافقة مع OpenAI -- Ollama أو llama-server من llama.cpp أو LM Studio أو vLLM. لا شيء يغادر جهازك. يتم الاستعلام عن قائمة النماذج مباشرة من الخادم.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="100"/>
        <source>Bring your own API keys. They are encrypted at rest with a per-machine key and never leave your computer except to communicate with the provider you select.</source>
        <translation>استخدم مفاتيح API الخاصة بك. يتم تشفيرها عند التخزين بمفتاح خاص بالجهاز ولا تغادر حاسوبك أبدًا إلا للاتصال بالمزود الذي تختاره.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="166"/>
        <source>Key set</source>
        <translation>تم تعيين المفتاح</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="167"/>
        <source>No key</source>
        <translation>لا يوجد مفتاح</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="204"/>
        <source>A key is on file -- paste a new one to replace it</source>
        <translation>يوجد مفتاح مسجل -- الصق مفتاحًا جديدًا لاستبداله</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="205"/>
        <source>Paste your API key here</source>
        <translation>الصق مفتاح API هنا</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="212"/>
        <source>Hide key</source>
        <translation>إخفاء المفتاح</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="213"/>
        <source>Show key while typing</source>
        <translation>إظهار المفتاح أثناء الكتابة</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="224"/>
        <source>Get key</source>
        <translation>الحصول على مفتاح</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="225"/>
        <source>Open the provider's console to create a new key</source>
        <translation>افتح وحدة تحكم المزود لإنشاء مفتاح جديد</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="236"/>
        <source>Save</source>
        <translation>حفظ</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="254"/>
        <source>Remove the stored key for %1</source>
        <translation>إزالة المفتاح المخزن لـ %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="278"/>
        <source>http://localhost:11434/v1</source>
        <translation>http://localhost:11434/v1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="282"/>
        <source>Install Ollama</source>
        <translation>تثبيت Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="283"/>
        <source>Open the Ollama download page</source>
        <translation>فتح صفحة تنزيل Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="294"/>
        <source>Apply</source>
        <translation>تطبيق</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="309"/>
        <source>Re-query the model list</source>
        <translation>إعادة الاستعلام عن قائمة النماذج</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="357"/>
        <source>No API keys configured yet. Add a key to get started.</source>
        <translation>لم يتم تكوين مفاتيح API بعد. أضف مفتاحاً للبدء.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="360"/>
        <source>One provider is ready.</source>
        <translation>مزود واحد جاهز.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="362"/>
        <source>%1 providers are ready.</source>
        <translation>%1 مزود جاهز.</translation>
    </message>
</context>
<context>
    <name>LicenseManagement</name>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="37"/>
        <source>Licensing</source>
        <translation>الترخيص</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="84"/>
        <source>Please wait…</source>
        <translation>يُرجى الانتظار…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="124"/>
        <source>Activate Serial Studio Pro</source>
        <translation>تفعيل Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="131"/>
        <source>Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.</source>
        <translation>الصق مفتاح الترخيص أدناه لفتح ميزات Pro مثل MQTT والرسوم ثلاثية الأبعاد والمزيد.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="138"/>
        <source>Your license includes 5 device activations.
Plans include Monthly, Yearly, and Lifetime options.</source>
        <translation>يتضمن الترخيص 5 تفعيلات للأجهزة.
تشمل الخطط خيارات شهرية وسنوية ومدى الحياة.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="151"/>
        <source>Paste your license key here…</source>
        <translation>الصق مفتاح الترخيص هنا…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="170"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="332"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="381"/>
        <source>Copy</source>
        <translation>نسخ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="176"/>
        <source>Paste</source>
        <translation>لصق</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="182"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="338"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="387"/>
        <source>Select All</source>
        <translation>تحديد الكل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="234"/>
        <source>Product</source>
        <translation>المنتج</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="241"/>
        <source>Serial Studio %1</source>
        <translation>Serial Studio %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="252"/>
        <source>Licensee</source>
        <translation>المرخص له</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="271"/>
        <source>Licensee E-Mail</source>
        <translation>البريد الإلكتروني للمرخص له</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="288"/>
        <source>Device Usage</source>
        <translation>استخدام الأجهزة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="296"/>
        <source>%1 devices in use (Unlimited plan)</source>
        <translation>%1 جهاز قيد الاستخدام (خطة غير محدودة)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="297"/>
        <source>%1 of %2 devices used</source>
        <translation>%1 من %2 جهاز مستخدم</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="307"/>
        <source>Device ID</source>
        <translation>معرف الجهاز</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="354"/>
        <source>License Key</source>
        <translation>مفتاح الترخيص</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="409"/>
        <source>Customer Portal</source>
        <translation>بوابة العملاء</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="420"/>
        <source>Buy License</source>
        <translation>شراء ترخيص</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="427"/>
        <source>Activate</source>
        <translation>تفعيل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="437"/>
        <source>Deactivate</source>
        <translation>إلغاء التفعيل</translation>
    </message>
</context>
<context>
    <name>Licensing::LemonSqueezy</name>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="517"/>
        <source>There was an issue validating your license.</source>
        <translation>حدثت مشكلة أثناء التحقق من الترخيص.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="535"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="716"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="835"/>
        <source>The license key you provided does not belong to Serial Studio.</source>
        <translation>مفتاح الترخيص المُدخل لا ينتمي إلى Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="536"/>
        <source>Please double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>تحقق من شراء الترخيص من متجر Serial Studio الرسمي.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="547"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="725"/>
        <source>This license key was activated on a different device.</source>
        <translation>مفتاح الترخيص مُفعّل على جهاز آخر.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="548"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="726"/>
        <source>Deactivate it there first or contact support for help.</source>
        <translation>ألغِ تفعيله هناك أولاً أو اتصل بالدعم للمساعدة.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="559"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="736"/>
        <source>This license is not currently active.</source>
        <translation>الترخيص غير نشط حالياً.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="560"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="737"/>
        <source>It may have expired or been deactivated (status: %1).</source>
        <translation>قد يكون منتهي الصلاحية أو مُلغى التفعيل (الحالة: %1).</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="570"/>
        <source>Something went wrong on the server.</source>
        <translation>حدث خطأ في الخادم.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="571"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="747"/>
        <source>No activation ID was returned.</source>
        <translation>لم يتم إرجاع معرّف التفعيل.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="581"/>
        <source>Could not validate your license at this time.</source>
        <translation>تعذّر التحقق من الترخيص في الوقت الحالي.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="582"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="756"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="845"/>
        <source>Try again later.</source>
        <translation>أعد المحاولة لاحقاً.</translation>
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
        <translation>تم تفعيل الترخيص بنجاح.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="654"/>
        <source>Thank you for supporting Serial Studio!
You now have access to all premium features.</source>
        <translation>شكراً لدعمك Serial Studio!
أصبح لديك الآن إمكانية الوصول إلى جميع الميزات المتقدمة.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="708"/>
        <source>There was an issue activating your license.</source>
        <translation>حدثت مشكلة أثناء تفعيل الترخيص.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="717"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="836"/>
        <source>Double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>تحقق من شراء الترخيص من متجر Serial Studio الرسمي.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="746"/>
        <source>Something went wrong on the server…</source>
        <translation>حدث خطأ في الخادم…</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="755"/>
        <source>Could not activate your license at this time.</source>
        <translation>تعذر تفعيل الترخيص في الوقت الحالي.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="827"/>
        <source>There was an issue deactivating your license.</source>
        <translation>حدثت مشكلة أثناء إلغاء تفعيل الترخيص.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="844"/>
        <source>Could not deactivate your license at this time.</source>
        <translation>تعذر إلغاء تفعيل الترخيص في الوقت الحالي.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="854"/>
        <source>Your license has been deactivated.</source>
        <translation>تم إلغاء تفعيل الترخيص.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="855"/>
        <source>Access to Pro features has been removed.
Thank you again for supporting Serial Studio!</source>
        <translation>تمت إزالة الوصول إلى ميزات Pro.
شكراً مجدداً لدعم Serial Studio!</translation>
    </message>
</context>
<context>
    <name>MDF4::Export</name>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="629"/>
        <source>MDF4 Export is a Pro feature.</source>
        <translation>تصدير MDF4 ميزة من ميزات Pro.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="630"/>
        <source>This feature requires a license. Please purchase one to enable MDF4 export.</source>
        <translation>تتطلب هذه الميزة ترخيصاً. يُرجى شراء ترخيص لتمكين تصدير MDF4.</translation>
    </message>
</context>
<context>
    <name>MDF4::Player</name>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="399"/>
        <source>Select MDF4 file</source>
        <translation>تحديد ملف MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="401"/>
        <source>MDF4 files (*.mf4 *.dat)</source>
        <translation>ملفات MDF4 (*.mf4 *.dat)</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="423"/>
        <source>Disconnect from device?</source>
        <translation>قطع الاتصال بالجهاز؟</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="424"/>
        <source>You must disconnect from the current device before opening a MDF4 file.</source>
        <translation>يجب قطع الاتصال عن الجهاز الحالي قبل فتح ملف MDF4.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="440"/>
        <source>Cannot open MDF4 file</source>
        <translation>تعذر فتح ملف MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="441"/>
        <source>The file may be corrupted or in an unsupported format.</source>
        <translation>قد يكون الملف تالفًا أو بتنسيق غير مدعوم.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="448"/>
        <source>Invalid MDF4 file</source>
        <translation>ملف MDF4 غير صالح</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="449"/>
        <source>Failed to read file structure. The file may be corrupted.</source>
        <translation>فشل قراءة بنية الملف. قد يكون الملف تالفًا.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="464"/>
        <source>No data in file</source>
        <translation>لا توجد بيانات في الملف</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="465"/>
        <source>The MDF4 file contains no measurement data.</source>
        <translation>ملف MDF4 لا يحتوي على بيانات قياس.</translation>
    </message>
</context>
<context>
    <name>MQTT</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="51"/>
        <source>Hostname</source>
        <translation>اسم المضيف</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="59"/>
        <source>e.g. broker.hivemq.com</source>
        <translation>مثال: broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="67"/>
        <source>Port</source>
        <translation>المنفذ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="87"/>
        <source>Topic Filter</source>
        <translation>مرشح الموضوع</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="95"/>
        <source>e.g. sensors/#</source>
        <translation>مثال: sensors/#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="103"/>
        <source>Client ID</source>
        <translation>معرف العميل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="120"/>
        <source>Regenerate</source>
        <translation>إعادة توليد</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="130"/>
        <source>Username</source>
        <translation>اسم المستخدم</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="145"/>
        <source>Password</source>
        <translation>كلمة المرور</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="161"/>
        <source>Version</source>
        <translation>الإصدار</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="187"/>
        <source>Keep Alive (s)</source>
        <translation>الإبقاء نشطاً (ث)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="206"/>
        <source>Clean Session</source>
        <translation>جلسة نظيفة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="232"/>
        <source>Use SSL/TLS</source>
        <translation>استخدام SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="258"/>
        <source>SSL Protocol</source>
        <translation>بروتوكول SSL</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="289"/>
        <source>Peer Verify</source>
        <translation>التحقق من النظير</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="320"/>
        <source>Verify Depth</source>
        <translation>عمق التحقق</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="341"/>
        <source>CA Certificates</source>
        <translation>شهادات CA</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="349"/>
        <source>Load From Folder…</source>
        <translation>تحميل من مجلد…</translation>
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
        <translation>TLS 1.3 أو أحدث</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="799"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 أو أحدث</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="800"/>
        <source>Any Protocol</source>
        <translation>أي بروتوكول</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="801"/>
        <source>Secure Protocols Only</source>
        <translation>البروتوكولات الآمنة فقط</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="803"/>
        <source>None</source>
        <translation>بدون</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="804"/>
        <source>Query Peer</source>
        <translation>استعلام النظير</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="805"/>
        <source>Verify Peer</source>
        <translation>التحقق من النظير</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="806"/>
        <source>Auto Verify Peer</source>
        <translation>التحقق التلقائي من النظير</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1123"/>
        <source>Raw RX Data</source>
        <translation>بيانات RX الخام</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1124"/>
        <source>Custom Script</source>
        <translation>سكريبت مخصص</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1125"/>
        <source>Dashboard Data (CSV)</source>
        <translation>بيانات لوحة القيادة (CSV)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1126"/>
        <source>Dashboard Data (JSON)</source>
        <translation>بيانات لوحة القيادة (JSON)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1281"/>
        <source>MQTT publisher unavailable</source>
        <translation>ناشر MQTT غير متاح</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1282"/>
        <source>A valid commercial license is required to use MQTT publishing.</source>
        <translation>يلزم ترخيص تجاري صالح لاستخدام النشر عبر MQTT.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1284"/>
        <location filename="../../src/MQTT/Publisher.cpp" line="1853"/>
        <source>MQTT Test Connection</source>
        <translation>اختبار اتصال MQTT</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1303"/>
        <source>Select PEM Certificates Directory</source>
        <translation>تحديد دليل شهادات PEM</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker reachable</source>
        <translation>وسيط MQTT قابل للوصول</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker unreachable</source>
        <translation>وسيط MQTT غير قابل للوصول</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT broker connection failed</source>
        <translation>فشل الاتصال بوسيط MQTT</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT Publisher</source>
        <translation>ناشر MQTT</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherScriptEditor</name>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="49"/>
        <source>MQTT Publisher Script</source>
        <translation>سكريبت ناشر MQTT</translation>
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
        <translation>بايتات الإطار النموذجية (نص أو سداسي عشري)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="97"/>
        <source>Hex</source>
        <translation>سداسي عشري</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="98"/>
        <source>Test</source>
        <translation>اختبار</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="99"/>
        <source>Clear</source>
        <translation>مسح</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="101"/>
        <source>Apply</source>
        <translation>تطبيق</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="102"/>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="111"/>
        <source>Language:</source>
        <translation>اللغة:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="114"/>
        <source>Template:</source>
        <translation>القالب:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="125"/>
        <source>Frame:</source>
        <translation>الإطار:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="129"/>
        <source>Output:</source>
        <translation>الإخراج:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="269"/>
        <source>Enter a frame</source>
        <translation>أدخل إطارًا</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="276"/>
        <source>Invalid hex</source>
        <translation>قيمة سداسية عشرية غير صالحة</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="359"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>تنسيق المستند	Ctrl+Shift+I</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="360"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>تنسيق التحديد	Ctrl+I</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="500"/>
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
-- عرّف دالة mqtt(frame) تستقبل البايتات الخام
-- لإطار واحد محلل وتُرجع الحمولة للنشر إلى
-- الوسيط، أو nil لتخطي هذا الإطار.
--
-- وسيط frame يطابق ما يراه سكريبت محلل الإطارات:
-- سلسلة Lua تحتوي على البايتات بين محددات
-- FrameReader.
--
-- مثال:
--   function mqtt(frame)
--     return frame    -- إعادة توجيه كما هو
--   end
--
-- استخدم قائمة القالب المنسدلة للحصول على أمثلة جاهزة.
--
</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="517"/>
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
 * عرّف دالة mqtt(frame) تستقبل البايتات الخام
 * لإطار واحد محلل وتُرجع الحمولة للنشر إلى
 * الوسيط، أو null لتخطي هذا الإطار.
 *
 * وسيط frame يطابق ما يراه سكريبت محلل الإطارات:
 * سلسلة تحتوي على البايتات بين محددات
 * FrameReader.
 *
 * مثال:
 *   function mqtt(frame) {
 *     return frame;   // إعادة توجيه كما هو
 *   }
 *
 * استخدم قائمة القالب المنسدلة للحصول على أمثلة جاهزة.
 */</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="614"/>
        <source>Script is empty</source>
        <translation>السكريبت فارغ</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="621"/>
        <source>Lua engine error</source>
        <translation>خطأ في محرك Lua</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="643"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="657"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="681"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="695"/>
        <source>Error: %1</source>
        <translation>خطأ: %1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="651"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="687"/>
        <source>mqtt() is not defined</source>
        <translation>()mqtt غير معرّف</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="668"/>
        <source>(nil -- frame skipped)</source>
        <translation>(nil -- تم تخطي الإطار)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="670"/>
        <source>(non-string return)</source>
        <translation>(قيمة مُرجعة غير نصية)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="700"/>
        <source>(null -- frame skipped)</source>
        <translation>(null -- تم تخطي الإطار)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="778"/>
        <source>Select Template…</source>
        <translation>اختيار قالب…</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherWorker</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="674"/>
        <source>Configure broker hostname and port before testing the connection.</source>
        <translation>قم بتكوين اسم مضيف الوسيط والمنفذ قبل اختبار الاتصال.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="710"/>
        <source>Successfully connected to %1:%2.</source>
        <translation>تم الاتصال بنجاح بـ %1:%2.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="721"/>
        <source>Timed out after 5 seconds without reaching the broker.</source>
        <translation>انتهت المهلة بعد 5 ثوانٍ دون الوصول إلى الوسيط.</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="191"/>
        <source>Console Only Mode</source>
        <translation>وضع وحدة التحكم فقط</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="194"/>
        <source>Quick Plot Mode</source>
        <translation>وضع الرسم السريع</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="201"/>
        <source>Empty Project</source>
        <translation>مشروع فارغ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="686"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="694"/>
        <source>Waiting for data…</source>
        <translation>في انتظار البيانات…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="695"/>
        <source>Connecting to device…</source>
        <translation>جارٍ الاتصال بالجهاز…</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="146"/>
        <source>Code sample</source>
        <translation>نموذج الكود</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="148"/>
        <source>Completer</source>
        <translation>المُكمِل التلقائي</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="150"/>
        <source>Highlighter</source>
        <translation>مُبرِز الأكواد</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="152"/>
        <source>Style</source>
        <translation>النمط</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="214"/>
        <source> spaces</source>
        <translation>مسافات</translation>
    </message>
</context>
<context>
    <name>MarkdownWebView</name>
    <message>
        <location filename="../../qml/Widgets/MarkdownWebView.qml" line="36"/>
        <source>Copied to Clipboard</source>
        <translation>تم النسخ إلى الحافظة</translation>
    </message>
</context>
<context>
    <name>Mdf4Player</name>
    <message>
        <location filename="../../qml/Dialogs/Mdf4Player.qml" line="24"/>
        <source>MDF4 Player</source>
        <translation>مشغل MDF4</translation>
    </message>
</context>
<context>
    <name>MessageBubble</name>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="97"/>
        <source>Error</source>
        <translation>خطأ</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>You</source>
        <translation>أنت</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>Assistant</source>
        <translation>المساعد</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="208"/>
        <source>Discovery</source>
        <translation>الاكتشاف</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="209"/>
        <source>Execution</source>
        <translation>التنفيذ</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="239"/>
        <source>Approve %1 actions in %2?</source>
        <translation>الموافقة على %1 إجراء في %2؟</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="249"/>
        <source>These calls will run together. Expand each card below to inspect arguments.</source>
        <translation>سيتم تشغيل هذه الاستدعاءات معًا. قم بتوسيع كل بطاقة أدناه لفحص المعاملات.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="260"/>
        <source>Approve all</source>
        <translation>الموافقة على الكل</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="266"/>
        <source>Deny all</source>
        <translation>رفض الكل</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="332"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="384"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="436"/>
        <source>Copy</source>
        <translation>نسخ</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="337"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="389"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="441"/>
        <source>Copy All</source>
        <translation>نسخ الكل</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="345"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="397"/>
        <source>Select All</source>
        <translation>تحديد الكل</translation>
    </message>
</context>
<context>
    <name>MessageWebView</name>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="63"/>
        <source>You</source>
        <translation>أنت</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="64"/>
        <source>Assistant</source>
        <translation>المساعد</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="65"/>
        <location filename="../../qml/AI/MessageWebView.qml" line="71"/>
        <source>Error</source>
        <translation>خطأ</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="66"/>
        <source>Discovery</source>
        <translation>الاكتشاف</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="67"/>
        <source>Execution</source>
        <translation>التنفيذ</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="68"/>
        <source>Running</source>
        <translation>قيد التشغيل</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="69"/>
        <source>Awaiting approval</source>
        <translation>في انتظار الموافقة</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="70"/>
        <source>Done</source>
        <translation>تم</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="72"/>
        <source>Denied</source>
        <translation>مرفوض</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="73"/>
        <source>Blocked</source>
        <translation>محظور</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="74"/>
        <source>Approve</source>
        <translation>موافقة</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="75"/>
        <source>Deny</source>
        <translation>رفض</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="76"/>
        <source>Approve all</source>
        <translation>الموافقة على الكل</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="77"/>
        <source>Deny all</source>
        <translation>رفض الكل</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="78"/>
        <source>Arguments</source>
        <translation>المعاملات</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="79"/>
        <source>Result</source>
        <translation>النتيجة</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="80"/>
        <source>Approve {n} actions in {family}?</source>
        <translation>الموافقة على {n} إجراء في {family}؟</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="81"/>
        <source>These calls will run together. Expand each card to inspect arguments.</source>
        <translation>سيتم تنفيذ هذه الاستدعاءات معًا. قم بتوسيع كل بطاقة لفحص المعاملات.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="82"/>
        <source>Copy</source>
        <translation>نسخ</translation>
    </message>
</context>
<context>
    <name>Misc::Examples</name>
    <message>
        <location filename="../../src/Misc/Examples.cpp" line="282"/>
        <source>Failed to load README: %1</source>
        <translation>فشل تحميل README: %1</translation>
    </message>
</context>
<context>
    <name>Misc::ExtensionManager</name>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="244"/>
        <source>Theme</source>
        <translation>السمة</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="247"/>
        <source>Frame Parser</source>
        <translation>محلل الإطارات</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="250"/>
        <source>Project Template</source>
        <translation>قالب المشروع</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="253"/>
        <source>Plugin</source>
        <translation>الإضافة</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="256"/>
        <source>All Types</source>
        <translation>جميع الأنواع</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="473"/>
        <source>Reset Extensions</source>
        <translation>إعادة تعيين الإضافات</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="474"/>
        <source>This uninstalls all extensions, removes all custom repositories, and restores the default settings. Continue?</source>
        <translation>سيؤدي هذا إلى إلغاء تثبيت جميع الإضافات وإزالة جميع المستودعات المخصصة واستعادة الإعدادات الافتراضية. هل تريد المتابعة؟</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="513"/>
        <source>Select Extension Repository Folder</source>
        <translation>تحديد مجلد مستودع الإضافات</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1014"/>
        <source>Installed (repository no longer available)</source>
        <translation>مثبت (المستودع لم يعد متاحاً)</translation>
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
        <translation>خطأ في المكون الإضافي</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1321"/>
        <source>Plugin "%1" is not installed.</source>
        <translation>المكون الإضافي "%1" غير مثبت.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1332"/>
        <source>Extension "%1" is not a plugin (type: %2).</source>
        <translation>الإضافة "%1" ليست مكوناً إضافياً (النوع: %2).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1353"/>
        <source>Cannot read plugin metadata file:
%1/info.json</source>
        <translation>تعذرت قراءة ملف بيانات المكون الإضافي:
%1/info.json</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1375"/>
        <source>Plugin "%1" requires gRPC but this build does not include gRPC support.</source>
        <translation>المكون الإضافي "%1" يتطلب GRPC لكن هذا الإصدار لا يتضمن دعم GRPC.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1384"/>
        <source>This plugin uses gRPC for high-performance data streaming. The API server needs to be enabled.

Would you like to enable it now?</source>
        <translation>يستخدم هذا المكون الإضافي GRPC لبث البيانات عالي الأداء. يجب تمكين خادم API.

هل تريد تمكينه الآن؟</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1387"/>
        <source>Plugins need the API server to communicate with Serial Studio. Would you like to enable it now?</source>
        <translation>تحتاج الإضافات إلى خادم API للتواصل مع Serial Studio. هل تريد تمكينه الآن؟</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1390"/>
        <source>API Server Required</source>
        <translation>خادم API مطلوب</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1419"/>
        <source>Plugin "%1" has no 'entry' field in info.json.</source>
        <translation>الإضافة "%1" لا تحتوي على حقل 'entry' في info.json.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1429"/>
        <source>Entry point not found:
%1</source>
        <translation>نقطة الدخول غير موجودة:
%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1438"/>
        <source>Plugin "%1" has an invalid entry point path.</source>
        <translation>الإضافة "%1" تحتوي على مسار نقطة دخول غير صالح.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1481"/>
        <source>Missing Dependency</source>
        <translation>اعتمادية مفقودة</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1482"/>
        <source>This plugin requires "%1" but it was not found on your system.

Would you like to open the download page?</source>
        <translation>تتطلب هذه الإضافة "%1" لكنها غير موجودة على نظامك.

هل تريد فتح صفحة التنزيل؟</translation>
    </message>
</context>
<context>
    <name>Misc::GraphicsBackend</name>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="246"/>
        <source>Restart Required</source>
        <translation>إعادة التشغيل مطلوبة</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="247"/>
        <source>The new rendering backend will take effect after restarting Serial Studio. Restart now to apply the change?</source>
        <translation>سيصبح محرك العرض الجديد فعالًا بعد إعادة تشغيل Serial Studio. هل تريد إعادة التشغيل الآن لتطبيق التغيير؟</translation>
    </message>
</context>
<context>
    <name>Misc::HelpCenter</name>
    <message>
        <location filename="../../src/Misc/HelpCenter.cpp" line="303"/>
        <source>Failed to load page: %1</source>
        <translation>فشل تحميل الصفحة: %1</translation>
    </message>
</context>
<context>
    <name>Misc::IconEngine</name>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="148"/>
        <source>Invalid icon identifier</source>
        <translation>معرّف أيقونة غير صالح</translation>
    </message>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="218"/>
        <source>Empty SVG data received</source>
        <translation>تم استلام بيانات SVG فارغة</translation>
    </message>
</context>
<context>
    <name>Misc::ShortcutGenerator</name>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="73"/>
        <source>Windows Shortcut (*.lnk)</source>
        <translation>اختصار Windows ‏(*.lnk)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="75"/>
        <source>macOS Application (*.app)</source>
        <translation>تطبيق macOS ‏(*.app)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="77"/>
        <source>Desktop Entry (*.desktop)</source>
        <translation>مدخل سطح المكتب (*.desktop)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="101"/>
        <source>Use a .icns icon for the sharpest result in Finder and the Dock.</source>
        <translation>استخدم أيقونة ‎.icns للحصول على أوضح نتيجة في Finder وشريط Dock.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="103"/>
        <source>Leave the icon empty to inherit the Serial Studio executable icon.</source>
        <translation>اترك الأيقونة فارغة لوراثة أيقونة Serial Studio القابلة للتنفيذ.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="105"/>
        <source>Place the file under ~/.local/share/applications/ to expose it in your application launcher.</source>
        <translation>ضع الملف في ‎~/.local/share/applications/‎ لإظهاره في مشغّل التطبيقات.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="116"/>
        <source>Apple Icon Image (*.icns)</source>
        <translation>صورة أيقونة Apple ‏(*.icns)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="118"/>
        <source>Windows Icon (*.ico)</source>
        <translation>أيقونة Windows ‏(*.ico)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="120"/>
        <source>Vector or Raster Image (*.svg *.png)</source>
        <translation>صورة متجهة أو نقطية (*.svg *.png)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="212"/>
        <source>A Pro license is required to generate shortcuts.</source>
        <translation>يلزم ترخيص Pro لإنشاء الاختصارات.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="217"/>
        <source>No output path was provided.</source>
        <translation>لم يتم توفير مسار الإخراج.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="258"/>
        <source>Failed to write shortcut file.</source>
        <translation>فشلت كتابة ملف الاختصار.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="125"/>
        <source>Could not write the bundle launcher: %1</source>
        <translation>تعذرت كتابة مشغّل الحزمة: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="144"/>
        <source>Could not mark the bundle launcher as executable.</source>
        <translation>تعذر وضع علامة قابل للتنفيذ على مشغّل الحزمة.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="164"/>
        <source>Could not write Info.plist: %1</source>
        <translation>تعذرت كتابة Info.plist: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="222"/>
        <source>Could not replace the existing shortcut at %1.</source>
        <translation>تعذر استبدال الاختصار الموجود في %1.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="232"/>
        <source>Could not create the .app bundle directory layout.</source>
        <translation>تعذر إنشاء تخطيط دليل حزمة .app.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="140"/>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="271"/>
        <source>Windows shortcut writer is not available on this platform.</source>
        <translation>كاتب اختصارات Windows غير متاح على هذه المنصة.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="285"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="199"/>
        <source>Linux shortcut writer is not available on this platform.</source>
        <translation>كاتب اختصارات Linux غير متاح على هذه المنصة.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="107"/>
        <source>Could not initialise COM (required to write .lnk shortcuts).</source>
        <translation>تعذر تهيئة COM (مطلوب لكتابة اختصارات .lnk).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="118"/>
        <source>CoCreateInstance(IShellLink) failed (HRESULT 0x%1).</source>
        <translation>فشل CoCreateInstance(IShellLink) (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="153"/>
        <source>QueryInterface(IPersistFile) failed (HRESULT 0x%1).</source>
        <translation>فشل QueryInterface(IPersistFile) (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="163"/>
        <source>Saving the .lnk file failed (HRESULT 0x%1).</source>
        <translation>فشل حفظ ملف .lnk (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="154"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="185"/>
        <source>macOS shortcut writer is not available on this platform.</source>
        <translation>كاتب اختصارات macOS غير متاح على هذه المنصة.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="86"/>
        <source>Could not open the shortcut path for writing: %1</source>
        <translation>تعذر فتح مسار الاختصار للكتابة: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="91"/>
        <source>Serial Studio shortcut</source>
        <translation>اختصار Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="112"/>
        <source>Could not mark the shortcut as executable.</source>
        <translation>تعذر وضع علامة قابل للتنفيذ على الاختصار.</translation>
    </message>
</context>
<context>
    <name>Misc::ThemeManager</name>
    <message>
        <location filename="../../src/Misc/ThemeManager.cpp" line="398"/>
        <source>System</source>
        <translation>النظام</translation>
    </message>
</context>
<context>
    <name>Misc::Utilities</name>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="161"/>
        <source>Ok</source>
        <translation>موافق</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="163"/>
        <source>Save</source>
        <translation>حفظ</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="165"/>
        <source>Save all</source>
        <translation>حفظ الكل</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="167"/>
        <source>Open</source>
        <translation>فتح</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="169"/>
        <source>Yes</source>
        <translation>نعم</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="171"/>
        <source>Yes to all</source>
        <translation>نعم للكل</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="173"/>
        <source>No</source>
        <translation>لا</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="175"/>
        <source>No to all</source>
        <translation>لا للكل</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="177"/>
        <source>Abort</source>
        <translation>إيقاف</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="179"/>
        <source>Retry</source>
        <translation>إعادة المحاولة</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="181"/>
        <source>Ignore</source>
        <translation>تجاهل</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="183"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="185"/>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="187"/>
        <source>Discard</source>
        <translation>تجاهل</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="189"/>
        <source>Help</source>
        <translation>مساعدة</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="191"/>
        <source>Apply</source>
        <translation>تطبيق</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="193"/>
        <source>Reset</source>
        <translation>إعادة تعيين</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="195"/>
        <source>Restore defaults</source>
        <translation>استعادة الإعدادات الافتراضية</translation>
    </message>
</context>
<context>
    <name>Misc::WorkspaceManager</name>
    <message>
        <location filename="../../src/Misc/WorkspaceManager.cpp" line="261"/>
        <source>Select Workspace Location</source>
        <translation>تحديد موقع مساحة العمل</translation>
    </message>
</context>
<context>
    <name>Modbus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="49"/>
        <source>Protocol</source>
        <translation>البروتوكول</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="72"/>
        <source>Serial Port</source>
        <translation>المنفذ التسلسلي</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="97"/>
        <source>Baud Rate</source>
        <translation>معدل البود</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="201"/>
        <source>Parity</source>
        <translation>التماثل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="222"/>
        <source>Data Bits</source>
        <translation>بتات البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="245"/>
        <source>Stop Bits</source>
        <translation>بتات الإيقاف</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="268"/>
        <source>Host</source>
        <translation>المضيف</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="278"/>
        <source>IP Address</source>
        <translation>عنوان IP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="292"/>
        <source>Port</source>
        <translation>المنفذ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="301"/>
        <source>TCP Port</source>
        <translation>منفذ TCP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="329"/>
        <source>Slave Address</source>
        <translation>عنوان العبد</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="334"/>
        <source>1-247</source>
        <translation>1-247</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="349"/>
        <source>Poll Interval (ms)</source>
        <translation>فترة الاستقصاء (ميلي ثانية)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="354"/>
        <source>Polling interval</source>
        <translation>فاصل الاستقصاء</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="382"/>
        <source>Configure Register Groups…</source>
        <translation>تكوين مجموعات السجلات…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="392"/>
        <source>Import Register Map…</source>
        <translation>استيراد خريطة السجلات…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="407"/>
        <source>%1 group(s) configured</source>
        <translation>تم تكوين %1 مجموعة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="408"/>
        <source>No groups configured</source>
        <translation>لا توجد مجموعات مكونة</translation>
    </message>
</context>
<context>
    <name>ModbusGroupsDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="45"/>
        <source>Modbus Register Groups</source>
        <translation>مجموعات سجلات Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="166"/>
        <source>Configure multiple register groups to poll different register types in sequence.</source>
        <translation>تكوين مجموعات سجلات متعددة لاستقصاء أنواع سجلات مختلفة بالتسلسل.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="174"/>
        <source>Add New Group</source>
        <translation>إضافة مجموعة جديدة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="198"/>
        <source>Register Type:</source>
        <translation>نوع السجل:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="210"/>
        <source>Start Address:</source>
        <translation>عنوان البداية:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="217"/>
        <source>0-65535</source>
        <translation>0-65535</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="223"/>
        <source>Register Count:</source>
        <translation>عدد السجلات:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="234"/>
        <source>1-125</source>
        <translation>1-125</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="239"/>
        <source>Add Group</source>
        <translation>إضافة مجموعة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="262"/>
        <source>Configured Groups</source>
        <translation>المجموعات المُهيأة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="296"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="303"/>
        <source>Type</source>
        <translation>النوع</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="311"/>
        <source>Start</source>
        <translation>البداية</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="318"/>
        <source>Count</source>
        <translation>العدد</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="325"/>
        <source>Action</source>
        <translation>الإجراء</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="400"/>
        <source>Remove</source>
        <translation>إزالة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="412"/>
        <source>No groups configured.
Add groups above to poll multiple register types.</source>
        <translation>لا توجد مجموعات مكوّنة.
أضف مجموعات أعلاه لاستقصاء أنواع سجلات متعددة.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="430"/>
        <source>Total groups: %1</source>
        <translation>إجمالي المجموعات: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="434"/>
        <source>Generate Project</source>
        <translation>إنشاء المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="440"/>
        <source>Clear All</source>
        <translation>مسح الكل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="446"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
</context>
<context>
    <name>ModbusPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="33"/>
        <source>Modbus Register Map Preview</source>
        <translation>معاينة خريطة سجلات Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="155"/>
        <source>File: %1</source>
        <translation>الملف: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="163"/>
        <source>Review the registers to import into a new Serial Studio project.</source>
        <translation>راجع السجلات لاستيرادها إلى مشروع Serial Studio جديد.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="171"/>
        <source>Registers</source>
        <translation>السجلات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="205"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="212"/>
        <source>Name</source>
        <translation>الاسم</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="221"/>
        <source>Address</source>
        <translation>العنوان</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="227"/>
        <source>Type</source>
        <translation>النوع</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="235"/>
        <source>Data Type</source>
        <translation>نوع البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="242"/>
        <source>Units</source>
        <translation>الوحدات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="343"/>
        <source>No registers found in file.</source>
        <translation>لم يتم العثور على سجلات في الملف.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="361"/>
        <source>Total: %1 registers in %2 groups</source>
        <translation>الإجمالي: %1 سجل في %2 مجموعة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="367"/>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="379"/>
        <source>Create Project</source>
        <translation>إنشاء مشروع</translation>
    </message>
</context>
<context>
    <name>MqttPublisherView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="33"/>
        <source>MQTT Publisher</source>
        <translation>ناشر MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="110"/>
        <source>Connected to broker</source>
        <translation>متصل بالوسيط</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="111"/>
        <source>Not connected</source>
        <translation>غير متصل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="124"/>
        <source>Test Connection</source>
        <translation>اختبار الاتصال</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="129"/>
        <source>Probe the broker with the current settings</source>
        <translation>فحص الوسيط بالإعدادات الحالية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="162"/>
        <source>Enable publishing first</source>
        <translation>تفعيل النشر أولاً</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="140"/>
        <source>Edit Script</source>
        <translation>تحرير السكريبت</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="146"/>
        <source>Edit the publisher script (Lua or JavaScript)</source>
        <translation>تحرير سكريبت الناشر (Lua أو JavaScript)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="158"/>
        <source>Load CA Certs</source>
        <translation>تحميل شهادات CA</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="164"/>
        <source>Add PEM certificates from a folder</source>
        <translation>إضافة شهادات PEM من مجلد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="165"/>
        <source>Enable SSL/TLS first</source>
        <translation>تمكين SSL/TLS أولاً</translation>
    </message>
</context>
<context>
    <name>MultiPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="261"/>
        <source>Interpolation: %1</source>
        <translation>الاستيفاء: %1</translation>
    </message>
    <message>
        <source>Show Legends</source>
        <translation type="vanished">إظهار وسائل الإيضاح</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="283"/>
        <source>Show X Axis Label</source>
        <translation>إظهار تسمية المحور X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="294"/>
        <source>Show Y Axis Label</source>
        <translation>إظهار تسمية المحور Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="306"/>
        <source>Show Crosshair</source>
        <translation>إظهار الشعيرات المتصالبة</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="313"/>
        <source>Pause</source>
        <translation>إيقاف مؤقت</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="313"/>
        <source>Resume</source>
        <translation>استئناف</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="330"/>
        <source>Sweep / Trigger Mode</source>
        <translation>وضع المسح / الزناد</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="342"/>
        <source>Trigger Settings</source>
        <translation>إعدادات الزناد</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="366"/>
        <source>Reset View</source>
        <translation>إعادة تعيين العرض</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="372"/>
        <source>Axis Range Settings</source>
        <translation>إعدادات نطاق المحور</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">العينات</translation>
    </message>
</context>
<context>
    <name>NativeTemplates</name>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="292"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="430"/>
        <source>Bytes per value</source>
        <translation>البايتات لكل قيمة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="293"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="431"/>
        <source>Number of bytes combined into each channel value.</source>
        <translation>عدد البايتات المدمجة في كل قيمة قناة.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="301"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="439"/>
        <source>Endianness</source>
        <translation>ترتيب البايتات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="302"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="440"/>
        <source>Byte order used when combining multi-byte values.</source>
        <translation>ترتيب البايتات المستخدم عند دمج القيم متعددة البايتات.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="310"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="448"/>
        <source>Signed values</source>
        <translation>القيم الموقعة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="311"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="449"/>
        <source>Interprets each value as two's-complement signed.</source>
        <translation>يفسر كل قيمة على أنها موقعة بمتمم الاثنين.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="651"/>
        <source>Tag routing table</source>
        <translation>جدول توجيه الوسوم</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="652"/>
        <source>Comma-separated tag:index entries, e.g. 1:0,2:1,3:2. Tags may be decimal or 0x-prefixed hex.</source>
        <translation>إدخالات وسم:فهرس مفصولة بفواصل، مثل 1:0,2:1,3:2. يمكن أن تكون الوسوم عشرية أو سداسية عشرية بادئة 0x.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1096"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1300"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1245"/>
        <source>Validate checksum</source>
        <translation>التحقق من المجموع الاختباري</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1097"/>
        <source>Rejects messages with an invalid Fletcher checksum.</source>
        <translation>يرفض الرسائل ذات المجموع الاختباري Fletcher غير الصالح.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1301"/>
        <source>Rejects messages with an invalid additive checksum.</source>
        <translation>يرفض الرسائل ذات المجموع الاختباري الإضافي غير الصالح.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1454"/>
        <source>Protocol version</source>
        <translation>إصدار البروتوكول</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1455"/>
        <source>Selects the expected start marker (0xFE for v1, 0xFD for v2).</source>
        <translation>يحدد علامة البداية المتوقعة (0xFE للإصدار 1، 0xFD للإصدار 2).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1883"/>
        <source>Validate CRC</source>
        <translation>التحقق من CRC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1884"/>
        <source>Rejects frames with an invalid CRC-24Q checksum.</source>
        <translation>يرفض الإطارات ذات المجموع الاختباري CRC-24Q غير الصالح.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2059"/>
        <source>Channel count</source>
        <translation>عدد القنوات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2060"/>
        <source>Number of output channels (registers or coils).</source>
        <translation>عدد قنوات الإخراج (السجلات أو الملفات).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2068"/>
        <source>Register offset</source>
        <translation>إزاحة السجل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2069"/>
        <source>Address offset subtracted from single-write echoes (40001 for legacy Modicon maps).</source>
        <translation>إزاحة العنوان المطروحة من أصداء الكتابة الفردية (40001 لخرائط Modicon القديمة).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2079"/>
        <source>Signed registers</source>
        <translation>السجلات الموقعة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2080"/>
        <source>Interprets 16-bit registers as two's-complement signed values.</source>
        <translation>يفسر السجلات 16-بت كقيم موقعة بمتمم الاثنين.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2386"/>
        <source>Payload layout</source>
        <translation>تخطيط الحمولة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2387"/>
        <source>Array emits every element in order; map routes keys through the key list.</source>
        <translation>المصفوفة تصدر كل عنصر بالترتيب؛ الخريطة توجه المفاتيح عبر قائمة المفاتيح.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2397"/>
        <source>Keys (map mode)</source>
        <translation>المفاتيح (وضع الخريطة)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2398"/>
        <source>Comma-separated map keys in channel order. Only used for the map layout.</source>
        <translation>مفاتيح الخريطة مفصولة بفواصل حسب ترتيب القنوات. تُستخدم فقط لتخطيط الخريطة.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="184"/>
        <source>Scalar fields</source>
        <translation>الحقول القياسية</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="185"/>
        <source>Comma-separated JSON fields repeated in every generated frame.</source>
        <translation>حقول JSON مفصولة بفواصل تتكرر في كل إطار مُنشأ.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="192"/>
        <source>Sample array field</source>
        <translation>حقل مصفوفة العينات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="193"/>
        <source>JSON field holding the batched sample array.</source>
        <translation>حقل JSON الذي يحتوي على مصفوفة العينات المجمّعة.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="334"/>
        <source>Records array field</source>
        <translation>حقل مصفوفة السجلات</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="335"/>
        <source>JSON field holding the array of record objects.</source>
        <translation>حقل JSON الذي يحتوي على مصفوفة كائنات السجلات.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="341"/>
        <source>Record fields (in channel order)</source>
        <translation>حقول السجل (حسب ترتيب القنوات)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="342"/>
        <source>Comma-separated record fields. The position of each field sets its channel index.</source>
        <translation>حقول السجل مفصولة بفواصل. يحدد موضع كل حقل فهرس قناته.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="605"/>
        <source>Column widths</source>
        <translation>عروض الأعمدة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="606"/>
        <source>Comma-separated character counts per field. Leave empty to split on whitespace.</source>
        <translation>عدد الأحرف المفصولة بفواصل لكل حقل. اتركه فارغًا للتقسيم على المسافات البيضاء.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="614"/>
        <source>Trim whitespace</source>
        <translation>إزالة المسافات البيضاء</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="615"/>
        <source>Removes padding around every sliced field.</source>
        <translation>يزيل الحشو حول كل حقل مقسّم.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="744"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="893"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1360"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1787"/>
        <source>Keys (in channel order)</source>
        <translation>المفاتيح (بترتيب القنوات)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="745"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="894"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1788"/>
        <source>Comma-separated key names. The position of each key sets its channel index.</source>
        <translation>أسماء المفاتيح المفصولة بفواصل. يحدد موضع كل مفتاح فهرس قناته.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="753"/>
        <source>Pair separator</source>
        <translation>فاصل الأزواج</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="754"/>
        <source>Character between key=value pairs.</source>
        <translation>الحرف بين أزواج مفتاح=قيمة.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="760"/>
        <source>Key-value separator</source>
        <translation>فاصل المفتاح-القيمة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="761"/>
        <source>Character between a key and its value.</source>
        <translation>الحرف بين المفتاح وقيمته.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="767"/>
        <source>Numeric values only</source>
        <translation>القيم الرقمية فقط</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="768"/>
        <source>Ignores pairs whose value is not a number.</source>
        <translation>يتجاهل الأزواج التي لا تكون قيمتها رقمًا.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1010"/>
        <source>Command routing table</source>
        <translation>جدول توجيه الأوامر</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1011"/>
        <source>Semicolon-separated entries of NAME:index list, e.g. CSQ:0,1;CREG:2,3;CGATT:4.</source>
        <translation>إدخالات مفصولة بفاصلة منقوطة من قائمة الاسم:فهرس، مثل CSQ:0,1;CREG:2,3;CGATT:4.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1236"/>
        <source>Talker prefix</source>
        <translation>بادئة المتحدث</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1237"/>
        <source>Two-letter talker id, e.g. GP for GPS or GN for multi-constellation receivers.</source>
        <translation>معرّف المتحدث المكون من حرفين، مثل GP لـ GPS أو GN لأجهزة الاستقبال متعددة الأبراج.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1246"/>
        <source>Rejects sentences whose *hh checksum does not match.</source>
        <translation>يرفض الجمل التي لا يتطابق فيها المجموع الاختباري *hh.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1361"/>
        <source>Comma-separated parameter names. The position of each key sets its channel index.</source>
        <translation>أسماء المعاملات مفصولة بفواصل. يحدد موضع كل مفتاح فهرس قناته.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1500"/>
        <source>Fields (in channel order)</source>
        <translation>الحقول (حسب ترتيب القناة)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1501"/>
        <source>Comma-separated field names. The position of each field sets its channel index.</source>
        <translation>أسماء الحقول مفصولة بفواصل. يحدد موضع كل حقل فهرس قناته.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1620"/>
        <source>Tags (in channel order)</source>
        <translation>العلامات (بترتيب القنوات)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1621"/>
        <source>Comma-separated tag names. The position of each tag sets its channel index.</source>
        <translation>أسماء العلامات مفصولة بفواصل. موضع كل علامة يحدد فهرس قناتها.</translation>
    </message>
    <message>
        <source>Register blocks</source>
        <translation type="vanished">كتل السجلات</translation>
    </message>
    <message>
        <source>Polled register blocks with typed, scaled entries. Written by the Modbus register map importer.</source>
        <translation type="vanished">كتل السجلات المستطلعة مع إدخالات مكتوبة ومقاسة. مكتوبة بواسطة مستورد خريطة سجلات Modbus.</translation>
    </message>
    <message>
        <source>Signal map</source>
        <translation type="vanished">خريطة الإشارات</translation>
    </message>
    <message>
        <source>CAN messages with their signal bit layouts and scaling. Written by the DBC importer.</source>
        <translation type="vanished">رسائل CAN مع تخطيطات بتات الإشارات والقياس الخاصة بها. مكتوبة بواسطة مستورد DBC.</translation>
    </message>
</context>
<context>
    <name>Network</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="78"/>
        <source>Socket Type</source>
        <translation>نوع المقبس</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="99"/>
        <source>Local Port</source>
        <translation>المنفذ المحلي</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="106"/>
        <source>Type 0 for automatic port</source>
        <translation>اكتب 0 لمنفذ تلقائي</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="132"/>
        <source>Remote Address</source>
        <translation>العنوان البعيد</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="156"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="189"/>
        <source>Remote Port</source>
        <translation>المنفذ البعيد</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="219"/>
        <source>Multicast</source>
        <translation>البث المتعدد</translation>
    </message>
</context>
<context>
    <name>NotificationLog</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="162"/>
        <source>Filter by channel…</source>
        <translation>تصفية حسب القناة…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="187"/>
        <source>Clear all notifications</source>
        <translation>مسح جميع الإشعارات</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="271"/>
        <source>(no title)</source>
        <translation>(بلا عنوان)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="329"/>
        <source>No notifications yet</source>
        <translation>لا توجد إشعارات بعد</translation>
    </message>
</context>
<context>
    <name>OnlineIconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="42"/>
        <source>Search Online Icons</source>
        <translation>البحث عن الأيقونات عبر الإنترنت</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="72"/>
        <source>Download failed: %1</source>
        <translation>فشل التنزيل: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="97"/>
        <source>Search icons (e.g. temperature, arrow, play)…</source>
        <translation>البحث عن أيقونات (مثل: درجة الحرارة، سهم، تشغيل)…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="109"/>
        <source>Search</source>
        <translation>بحث</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="148"/>
        <source>Search for icons above to get started</source>
        <translation>ابحث عن الأيقونات أعلاه للبدء</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="249"/>
        <source>OK</source>
        <translation>موافق</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="259"/>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
</context>
<context>
    <name>OutputWidgetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="91"/>
        <source>Output widgets require a Pro license.</source>
        <translation>تتطلب عناصر الإخراج ترخيص Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="93"/>
        <source>You can configure output widgets, but they only appear on the dashboard with a Pro license.</source>
        <translation>يمكنك تكوين عناصر الإخراج، لكنها تظهر على لوحة المعلومات فقط مع ترخيص Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="125"/>
        <source>Button</source>
        <translation>زر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="129"/>
        <source>Send a command on click</source>
        <translation>إرسال أمر عند النقر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="134"/>
        <source>Slider</source>
        <translation>شريط تمرير</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="138"/>
        <source>Send scaled numeric values</source>
        <translation>إرسال قيم رقمية مقاسة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="143"/>
        <source>Toggle</source>
        <translation>مفتاح تبديل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="147"/>
        <source>Send on/off commands</source>
        <translation>إرسال أوامر تشغيل/إيقاف</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="152"/>
        <source>Text Field</source>
        <translation>حقل نص</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="156"/>
        <source>Type and send arbitrary commands</source>
        <translation>كتابة وإرسال أوامر عشوائية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="160"/>
        <source>Knob</source>
        <translation>مقبض دوار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="165"/>
        <source>Rotary input for setpoints</source>
        <translation>إدخال دوار لنقاط الضبط</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="182"/>
        <source>Duplicate</source>
        <translation>تكرار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="185"/>
        <source>Duplicate this output widget</source>
        <translation>تكرار عنصر الإخراج هذا</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="195"/>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="197"/>
        <source>Delete this output widget</source>
        <translation>حذف عنصر الإخراج هذا</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="274"/>
        <source>Transmit Function</source>
        <translation>دالة الإرسال</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="284"/>
        <source>Import</source>
        <translation>استيراد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="290"/>
        <source>Import transmit function from a .js file</source>
        <translation>استيراد دالة الإرسال من ملف .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="297"/>
        <source>Template</source>
        <translation>قالب</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="301"/>
        <source>Select a pre-built transmit function template</source>
        <translation>اختيار قالب دالة إرسال جاهز</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="306"/>
        <source>Test</source>
        <translation>اختبار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="312"/>
        <source>Test the transmit function with sample input</source>
        <translation>اختبار وظيفة الإرسال بإدخال نموذجي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="353"/>
        <source>Undo</source>
        <translation>تراجع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="359"/>
        <source>Redo</source>
        <translation>إعادة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="367"/>
        <source>Cut</source>
        <translation>قص</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="372"/>
        <source>Copy</source>
        <translation>نسخ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="377"/>
        <source>Paste</source>
        <translation>لصق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="384"/>
        <source>Select All</source>
        <translation>تحديد الكل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="391"/>
        <source>Format Document</source>
        <translation>تنسيق المستند</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="396"/>
        <source>Format Selection</source>
        <translation>تنسيق التحديد</translation>
    </message>
</context>
<context>
    <name>Painter</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Painter.qml" line="56"/>
        <source>Painter Widget Error</source>
        <translation>خطأ في عنصر Painter</translation>
    </message>
</context>
<context>
    <name>PainterCodeDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="30"/>
        <source>Painter Widget Code Editor</source>
        <translation>محرر كود عنصر الرسام</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="76"/>
        <source>paint(ctx, w, h)</source>
        <translation>paint(ctx, w, h)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="86"/>
        <source>Import</source>
        <translation>استيراد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="92"/>
        <source>Import painter code from a .js file</source>
        <translation>استيراد كود الرسام من ملف .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="99"/>
        <source>Template</source>
        <translation>قالب</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="103"/>
        <source>Select a built-in painter template</source>
        <translation>اختيار قالب رسام مدمج</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="108"/>
        <source>Format</source>
        <translation>تنسيق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="113"/>
        <source>Reformat the painter code</source>
        <translation>إعادة تنسيق كود الرسام</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="119"/>
        <source>Test</source>
        <translation>اختبار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="124"/>
        <source>Open a live preview with simulated dataset values</source>
        <translation>فتح معاينة مباشرة بقيم مجموعة بيانات محاكاة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="127"/>
        <source>Preview</source>
        <translation>معاينة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="182"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="191"/>
        <source>Cut</source>
        <translation>قص</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="192"/>
        <source>Copy</source>
        <translation>نسخ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="193"/>
        <source>Paste</source>
        <translation>لصق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="194"/>
        <source>Select All</source>
        <translation>تحديد الكل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="196"/>
        <source>Undo</source>
        <translation>تراجع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="197"/>
        <source>Redo</source>
        <translation>إعادة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="199"/>
        <source>Format Document</source>
        <translation>تنسيق المستند</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="200"/>
        <source>Format Selection</source>
        <translation>تنسيق التحديد</translation>
    </message>
</context>
<context>
    <name>PainterTestDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="28"/>
        <source>Painter Live Preview</source>
        <translation>معاينة الرسام المباشرة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="32"/>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="37"/>
        <source>Preview</source>
        <translation>معاينة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="113"/>
        <source>Simulated datasets</source>
        <translation>مجموعات البيانات المحاكاة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="180"/>
        <source>Runtime OK</source>
        <translation>وقت التشغيل سليم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="181"/>
        <source>Awaiting first frame...</source>
        <translation>في انتظار الإطار الأول...</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="194"/>
        <source>Console</source>
        <translation>وحدة التحكم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="236"/>
        <source>Clear console</source>
        <translation>مسح وحدة التحكم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="245"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
</context>
<context>
    <name>Plot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="257"/>
        <source>Interpolation: %1</source>
        <translation>الاستيفاء: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="270"/>
        <source>Show Area Under Plot</source>
        <translation>إظهار المساحة أسفل الرسم</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="288"/>
        <source>Show X Axis Label</source>
        <translation>إظهار تسمية المحور X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="299"/>
        <source>Show Y Axis Label</source>
        <translation>إظهار تسمية المحور Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="311"/>
        <source>Show Crosshair</source>
        <translation>إظهار الشعيرات المتقاطعة</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="318"/>
        <source>Pause</source>
        <translation>إيقاف مؤقت</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="318"/>
        <source>Resume</source>
        <translation>استئناف</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="335"/>
        <source>Sweep / Trigger Mode</source>
        <translation>وضع المسح / الزناد</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="347"/>
        <source>Trigger Settings</source>
        <translation>إعدادات الزناد</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="371"/>
        <source>Reset View</source>
        <translation>إعادة تعيين العرض</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="377"/>
        <source>Axis Range Settings</source>
        <translation>إعدادات نطاق المحاور</translation>
    </message>
</context>
<context>
    <name>Plot3D</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="203"/>
        <source>Interpolate</source>
        <translation>استيفاء</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="221"/>
        <source>Orbit Navigation</source>
        <translation>التنقل المداري</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="231"/>
        <source>Pan Navigation</source>
        <translation>التنقل الأفقي</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="242"/>
        <source>Orthogonal View</source>
        <translation>عرض متعامد</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="248"/>
        <source>Top View</source>
        <translation>عرض علوي</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="254"/>
        <source>Left View</source>
        <translation>عرض أيسر</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="260"/>
        <source>Front View</source>
        <translation>عرض أمامي</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="277"/>
        <source>Auto Center</source>
        <translation>توسيط تلقائي</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="293"/>
        <source>Anaglyph 3D</source>
        <translation>ثلاثي الأبعاد تجسيمي</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="307"/>
        <source>Invert Eye Positions</source>
        <translation>عكس مواضع العين</translation>
    </message>
</context>
<context>
    <name>PlotCommon</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="59"/>
        <source>None</source>
        <translation>بدون</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="62"/>
        <source>ZOH</source>
        <translation>ZOH</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="65"/>
        <source>Stem</source>
        <translation>Stem</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="67"/>
        <source>Linear</source>
        <translation>خطي</translation>
    </message>
</context>
<context>
    <name>PlotWidget</name>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1326"/>
        <source>Time</source>
        <translation>الوقت</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1349"/>
        <source>ΔX: %1  ΔY: %2 — Drag to move, right-click to clear</source>
        <translation>ΔX: %1  ΔY: %2 — اسحب للتحريك، انقر بالزر الأيمن للمسح</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1352"/>
        <source>Click to place cursor</source>
        <translation>انقر لوضع المؤشر</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1354"/>
        <source>Click to place second cursor — Drag to move</source>
        <translation>انقر لوضع المؤشر الثاني — اسحب للتحريك</translation>
    </message>
</context>
<context>
    <name>ProNotice</name>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="119"/>
        <source>Visit Website</source>
        <translation>زيارة الموقع</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="127"/>
        <source>Buy License</source>
        <translation>شراء الترخيص</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="140"/>
        <source>Activate</source>
        <translation>تفعيل</translation>
    </message>
</context>
<context>
    <name>ProUpgradeNotice</name>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="26"/>
        <source>Assistant — Pro feature</source>
        <translation>المساعد — ميزة Pro</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="44"/>
        <source>The Assistant is a Serial Studio Pro feature. Activate your license to unlock it.</source>
        <translation>المساعد هو ميزة من Serial Studio Pro. فعّل ترخيصك لإلغاء قفله.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="52"/>
        <source>Activate</source>
        <translation>تفعيل</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="66"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
</context>
<context>
    <name>Process</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="69"/>
        <source>Mode</source>
        <translation>الوضع</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Launch Process</source>
        <translation>تشغيل العملية</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Named Pipe</source>
        <translation>أنبوب مسمى</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="101"/>
        <source>Executable</source>
        <translation>الملف التنفيذي</translation>
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
        <translation>استعراض</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="145"/>
        <source>Arguments</source>
        <translation>المعاملات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="156"/>
        <source>--arg1 value1 --arg2 value2</source>
        <translation>--arg1 value1 --arg2 value2</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="177"/>
        <source>Working Dir</source>
        <translation>دليل العمل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="192"/>
        <source>(optional) /working/directory</source>
        <translation>(اختياري) /دليل/العمل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="223"/>
        <source>Pipe Path</source>
        <translation>مسار الأنبوب</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="273"/>
        <source>Pick Running Process…</source>
        <translation>اختيار عملية قيد التشغيل…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="311"/>
        <source>Launch a child process and capture its stdout, or connect to a named pipe written by an existing process.</source>
        <translation>تشغيل عملية فرعية والتقاط stdout الخاص بها، أو الاتصال بأنبوب مسمى تكتبه عملية موجودة.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="319"/>
        <source>Learn about named pipes</source>
        <translation>معرفة المزيد عن الأنابيب المسماة</translation>
    </message>
</context>
<context>
    <name>ProcessPicker</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="60"/>
        <source>Select Running Process</source>
        <translation>تحديد عملية قيد التشغيل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="211"/>
        <source>Select a running process to derive a named-pipe path suggestion.</source>
        <translation>تحديد عملية قيد التشغيل لاشتقاق اقتراح مسار أنبوب مسمى.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="217"/>
        <source>Filter Processes</source>
        <translation>تصفية العمليات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="231"/>
        <source>Type to filter by name…</source>
        <translation>اكتب للتصفية حسب الاسم…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="235"/>
        <source>Refresh</source>
        <translation>تحديث</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="243"/>
        <source>Running Processes</source>
        <translation>العمليات قيد التشغيل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="281"/>
        <source>Process</source>
        <translation>العملية</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="288"/>
        <source>PID</source>
        <translation>PID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="383"/>
        <source>No processes match the filter.</source>
        <translation>لا توجد عمليات مطابقة للمرشح.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="384"/>
        <source>No running processes found.
Click Refresh to update the list.</source>
        <translation>لم يتم العثور على عمليات قيد التشغيل.
انقر على تحديث لتحديث القائمة.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="400"/>
        <source>%1 process(es)</source>
        <translation>%1 عملية</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="404"/>
        <source>Select</source>
        <translation>تحديد</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="410"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
</context>
<context>
    <name>ProjectEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="43"/>
        <source>modified</source>
        <translation>معدّل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="362"/>
        <source>This project is password protected</source>
        <translation>هذا المشروع محمي بكلمة مرور</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="363"/>
        <source>Editing is available in Project mode</source>
        <translation>التحرير متاح في وضع المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="374"/>
        <source>Enter the password to make changes, or open a different project.</source>
        <translation>أدخل كلمة المرور لإجراء التغييرات، أو افتح مشروعًا آخر.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="375"/>
        <source>Switch to Project mode to load and edit a project.</source>
        <translation>انتقل إلى وضع المشروع لتحميل وتحرير مشروع.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="397"/>
        <source>Unlock</source>
        <translation>إلغاء القفل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="398"/>
        <source>Switch to Project Mode</source>
        <translation>الانتقال إلى وضع المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="417"/>
        <source>Open Other Project</source>
        <translation>فتح مشروع آخر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="418"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="434"/>
        <source>Create New Project</source>
        <translation>إنشاء مشروع جديد</translation>
    </message>
</context>
<context>
    <name>ProjectStructure</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="32"/>
        <source>Project Structure</source>
        <translation>بنية المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="71"/>
        <source>Search</source>
        <translation>بحث</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="169"/>
        <source>Move Up</source>
        <translation>تحريك لأعلى</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="174"/>
        <source>Move Down</source>
        <translation>تحريك لأسفل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="185"/>
        <source>Duplicate Selected (%1)</source>
        <translation>تكرار المحدد (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="186"/>
        <source>Duplicate</source>
        <translation>تكرار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="199"/>
        <source>Delete Selected (%1)</source>
        <translation>حذف المحدد (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="200"/>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
</context>
<context>
    <name>ProjectToolbar</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="142"/>
        <source>New</source>
        <translation>جديد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="145"/>
        <source>Create a new JSON project</source>
        <translation>إنشاء مشروع JSON جديد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="158"/>
        <source>Open</source>
        <translation>فتح</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="162"/>
        <source>Open an existing JSON project</source>
        <translation>فتح مشروع JSON موجود</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="168"/>
        <source>Save</source>
        <translation>حفظ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="175"/>
        <source>Save the current project</source>
        <translation>حفظ المشروع الحالي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="181"/>
        <source>Save As</source>
        <translation>حفظ باسم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="188"/>
        <source>Save the current project under a new name</source>
        <translation>حفظ المشروع الحالي باسم جديد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="200"/>
        <source>Import</source>
        <translation>استيراد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="204"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="208"/>
        <source>Generate a project from a Protocol Buffers (.proto) schema</source>
        <translation>إنشاء مشروع من مخطط Protocol Buffers ‏(.proto)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="230"/>
        <source>Lock</source>
        <translation>قفل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="234"/>
        <source>Set a password and lock the Project Editor</source>
        <translation>تعيين كلمة مرور وقفل محرر المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="245"/>
        <source>Add Device</source>
        <translation>إضافة جهاز</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="249"/>
        <source>Add a new data source (device) to the project</source>
        <translation>إضافة مصدر بيانات (جهاز) جديد إلى المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="272"/>
        <source>Action</source>
        <translation>إجراء</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="275"/>
        <source>Add a new action to the project</source>
        <translation>إضافة إجراء جديد إلى المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="260"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="264"/>
        <source>Output</source>
        <translation>الإخراج</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="218"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="222"/>
        <source>Restore</source>
        <translation>استعادة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="226"/>
        <source>Restore a recent automatic snapshot of the current project</source>
        <translation>استعادة لقطة تلقائية حديثة للمشروع الحالي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="267"/>
        <source>Add a new output control panel with a button</source>
        <translation>إضافة لوحة تحكم إخراج جديدة مع زر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="288"/>
        <source>Slider</source>
        <translation>منزلق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="291"/>
        <source>Add an output slider control</source>
        <translation>إضافة عنصر تحكم منزلق للإخراج</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="298"/>
        <source>Toggle</source>
        <translation>مفتاح تبديل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="301"/>
        <source>Add an output toggle control</source>
        <translation>إضافة عنصر تحكم مفتاح تبديل للإخراج</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="308"/>
        <source>Knob</source>
        <translation>مقبض دوار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="311"/>
        <source>Add an output knob control</source>
        <translation>إضافة عنصر تحكم مقبض دوار للإخراج</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="319"/>
        <source>Text Field</source>
        <translation>حقل نصي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="321"/>
        <source>Add an output text field control</source>
        <translation>إضافة عنصر تحكم حقل نص إخراج</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="328"/>
        <source>Button</source>
        <translation>زر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="331"/>
        <source>Add an output button control</source>
        <translation>إضافة عنصر تحكم زر إخراج</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="344"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="348"/>
        <source>Dataset</source>
        <translation>مجموعة بيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="350"/>
        <source>Add a generic dataset</source>
        <translation>إضافة مجموعة بيانات عامة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="364"/>
        <source>Plot</source>
        <translation>رسم بياني</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="367"/>
        <source>Add a 2D plot dataset</source>
        <translation>إضافة مجموعة بيانات رسم بياني ثنائي الأبعاد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="374"/>
        <source>FFT Plot</source>
        <translation>رسم بياني FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="377"/>
        <source>Add a Fast Fourier Transform plot</source>
        <translation>إضافة رسم بياني لتحويل فورييه السريع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="384"/>
        <source>Gauge</source>
        <translation>مقياس</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="387"/>
        <source>Add a gauge widget for numeric data</source>
        <translation>إضافة عنصر مقياس للبيانات الرقمية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="395"/>
        <source>Level Indicator</source>
        <translation>مؤشر المستوى</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="397"/>
        <source>Add a vertical bar level indicator</source>
        <translation>إضافة مؤشر مستوى شريطي عمودي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="404"/>
        <source>Compass</source>
        <translation>البوصلة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="407"/>
        <source>Add a compass widget for directional data</source>
        <translation>إضافة عنصر بوصلة لبيانات الاتجاه</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="415"/>
        <source>LED Indicator</source>
        <translation>مؤشر LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="417"/>
        <source>Add an LED-style status indicator</source>
        <translation>إضافة مؤشر حالة بنمط LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="430"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="434"/>
        <source>Group</source>
        <translation>المجموعة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="436"/>
        <source>Add a dataset container group</source>
        <translation>إضافة مجموعة حاوية لمجموعات البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="438"/>
        <source>Dataset Container</source>
        <translation>حاوية مجموعات البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="442"/>
        <source>Image</source>
        <translation>صورة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="444"/>
        <source>Add an image/video stream viewer</source>
        <translation>إضافة عارض لتدفق الصور/الفيديو</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="446"/>
        <source>Image View</source>
        <translation>عارض الصور</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="454"/>
        <source>Painter</source>
        <translation>رسّام</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="458"/>
        <source>Add a custom JavaScript-rendered painter widget</source>
        <translation>إضافة أداة رسّام مخصصة بلغة JavaScript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="459"/>
        <source>Painter widgets require a Pro license — adding one will fall back to a data grid</source>
        <translation>أدوات الرسّام تتطلب ترخيص Pro — إضافة واحدة سيتم التراجع بها إلى شبكة بيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="460"/>
        <source>Painter Widget</source>
        <translation>أداة الرسّام</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="472"/>
        <source>Table</source>
        <translation>جدول</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="475"/>
        <source>Add a data table view</source>
        <translation>إضافة عرض جدول بيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="477"/>
        <source>Data Grid</source>
        <translation>شبكة البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="483"/>
        <source>Multi-Plot</source>
        <translation>رسم بياني متعدد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="485"/>
        <source>Add a 2D plot with multiple signals</source>
        <translation>إضافة رسم بياني ثنائي الأبعاد بإشارات متعددة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="487"/>
        <source>Multiple Plot</source>
        <translation>رسم بياني متعدد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="492"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="497"/>
        <source>3D Plot</source>
        <translation>رسم بياني ثلاثي الأبعاد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="495"/>
        <source>Add a 3D plot visualization</source>
        <translation>إضافة تصور رسم بياني ثلاثي الأبعاد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="503"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="507"/>
        <source>Accelerometer</source>
        <translation>مقياس التسارع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="505"/>
        <source>Add a group for 3-axis accelerometer data</source>
        <translation>إضافة مجموعة لبيانات مقياس التسارع ثلاثي المحاور</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="513"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="516"/>
        <source>Gyroscope</source>
        <translation>الجيروسكوب</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="517"/>
        <source>Add a group for 3-axis gyroscope data</source>
        <translation>إضافة مجموعة لبيانات الجيروسكوب ثلاثي المحاور</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="522"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="527"/>
        <source>GPS Map</source>
        <translation>خريطة GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="525"/>
        <source>Add a map widget for GPS data</source>
        <translation>إضافة عنصر خريطة لبيانات GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="539"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="543"/>
        <source>Assistant</source>
        <translation>المساعد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="546"/>
        <source>Open the Assistant</source>
        <translation>فتح المساعد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="552"/>
        <source>Help Center</source>
        <translation>مركز المساعدة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="556"/>
        <source>Open the Project Editor documentation</source>
        <translation>فتح توثيق محرر المشروع</translation>
    </message>
</context>
<context>
    <name>ProjectView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="34"/>
        <source>Project Summary</source>
        <translation>ملخص المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="81"/>
        <source>Pro features detected in this project.</source>
        <translation>تم اكتشاف ميزات Pro في هذا المشروع.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="83"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>يتم استخدام عناصر بديلة. اشترِ ترخيصًا لفتح الوظائف الكاملة.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="118"/>
        <source>Project Title:</source>
        <translation>عنوان المشروع:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="129"/>
        <source>Untitled Project</source>
        <translation>مشروع بدون عنوان</translation>
    </message>
    <message>
        <source>Points:</source>
        <translation type="vanished">النقاط</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="149"/>
        <source>Time Range:</source>
        <translation>نطاق الوقت:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="235"/>
        <source>Source</source>
        <translation>المصدر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="236"/>
        <source>Sources</source>
        <translation>المصادر</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="241"/>
        <source>Group</source>
        <translation>المجموعة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="242"/>
        <source>Groups</source>
        <translation>المجموعات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="247"/>
        <source>Dataset</source>
        <translation>مجموعة البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="248"/>
        <source>Datasets</source>
        <translation>مجموعات البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="253"/>
        <source>Action</source>
        <translation>الإجراء</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="254"/>
        <source>Actions</source>
        <translation>الإجراءات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="342"/>
        <source>Double-click a block to edit it. Right-click anywhere to add a group, dataset, action, data table, or device.</source>
        <translation>انقر نقرًا مزدوجًا على كتلة لتحريرها. انقر بزر الماوس الأيمن في أي مكان لإضافة مجموعة أو مجموعة بيانات أو إجراء أو جدول بيانات أو جهاز.</translation>
    </message>
</context>
<context>
    <name>ProtoPreviewDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="41"/>
        <source>Protocol Buffers File Preview</source>
        <translation>معاينة ملف Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="165"/>
        <source>Proto File: %1</source>
        <translation>ملف Proto: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="173"/>
        <source>Browse the messages below, then create the project. Every message becomes a dashboard group; matching-type channel blocks get a MultiPlot and mixed-type messages get a DataGrid.</source>
        <translation>تصفح الرسائل أدناه، ثم أنشئ المشروع. كل رسالة تصبح مجموعة لوحة؛ كتل القنوات المتطابقة في النوع تحصل على MultiPlot والرسائل المختلطة في النوع تحصل على DataGrid.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="183"/>
        <source>Show fields for</source>
        <translation>إظهار الحقول لـ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="209"/>
        <source>Fields</source>
        <translation>الحقول</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="243"/>
        <source>Tag</source>
        <translation>الوسم</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="253"/>
        <source>Field Name</source>
        <translation>اسم الحقل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="258"/>
        <source>Type</source>
        <translation>النوع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="345"/>
        <source>No fields in the selected message.</source>
        <translation>لا توجد حقول في الرسالة المحددة.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="363"/>
        <source>Total: %1 messages, %2 fields</source>
        <translation>الإجمالي: %1 رسالة، %2 حقل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="370"/>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="381"/>
        <source>Create Project</source>
        <translation>إنشاء مشروع</translation>
    </message>
</context>
<context>
    <name>Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="71"/>
        <source>No error</source>
        <translation>لا يوجد خطأ</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="73"/>
        <source>The broker rejected the connection due to an unsupported protocol version. Match the broker's MQTT version and try again.</source>
        <translation>رفض الوسيط الاتصال بسبب إصدار بروتوكول غير مدعوم. طابق إصدار MQTT الخاص بالوسيط وأعد المحاولة.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="76"/>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Regenerate it and try again.</source>
        <translation>رفض الوسيط معرّف العميل. قد يكون مشوهًا أو طويلًا جدًا أو قيد الاستخدام بالفعل. أعد إنشاءه وحاول مرة أخرى.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="79"/>
        <source>The network reached the broker, but the broker is currently unavailable. Verify its status and try again later.</source>
        <translation>وصلت الشبكة إلى الوسيط، لكن الوسيط غير متاح حاليًا. تحقق من حالته وأعد المحاولة لاحقًا.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="82"/>
        <source>The username or password is incorrect or malformed. Double-check the credentials and try again.</source>
        <translation>اسم المستخدم أو كلمة المرور غير صحيحة أو مشوهة. راجع بيانات الاعتماد وأعد المحاولة.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="85"/>
        <source>The broker denied the connection due to insufficient permissions. Verify that the account has the required ACLs.</source>
        <translation>رفض الوسيط الاتصال بسبب أذونات غير كافية. تحقق من أن الحساب يمتلك قوائم ACL المطلوبة.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="88"/>
        <source>A network or transport-layer issue prevented the connection. Check connectivity, ports, and TLS configuration.</source>
        <translation>منعت مشكلة في الشبكة أو طبقة النقل الاتصال. تحقق من الاتصال والمنافذ وإعدادات TLS.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="91"/>
        <source>The client detected an MQTT protocol violation and closed the connection. Verify broker and client compatibility.</source>
        <translation>اكتشف العميل انتهاكًا لبروتوكول MQTT وأغلق الاتصال. تحقق من توافق الوسيط والعميل.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="94"/>
        <source>An unexpected error occurred. Check the broker logs and the application console for details.</source>
        <translation>حدث خطأ غير متوقع. تحقق من سجلات الوسيط ووحدة التحكم بالتطبيق للحصول على التفاصيل.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="97"/>
        <source>An MQTT 5 protocol-level error occurred. Inspect the broker's reason code for details.</source>
        <translation>حدث خطأ على مستوى بروتوكول MQTT 5. افحص رمز السبب الخاص بالوسيط للحصول على التفاصيل.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="101"/>
        <source>Unspecified MQTT error (code %1).</source>
        <translation>خطأ MQTT غير محدد (الرمز %1).</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../src/Misc/Translator.cpp" line="231"/>
        <source>Failed to load welcome text :(</source>
        <translation>فشل تحميل نص الترحيب :(</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="183"/>
        <source>Critical</source>
        <translation>حرج</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="183"/>
        <source>Warning</source>
        <translation>تحذير</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="572"/>
        <source>Project file not found</source>
        <translation>ملف المشروع غير موجود</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="573"/>
        <source>The project file referenced by this shortcut could not be found:

%1</source>
        <translation>تعذر العثور على ملف المشروع المشار إليه بواسطة هذا الاختصار:

%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="576"/>
        <source>Would you like to delete this shortcut?</source>
        <translation>هل تريد حذف هذا الاختصار؟</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="580"/>
        <source>Delete Shortcut</source>
        <translation>حذف الاختصار</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="582"/>
        <source>Quit</source>
        <translation>إنهاء</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1051"/>
        <source>Time (s)</source>
        <translation>الوقت (ث)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1109"/>
        <source>Freq: %1</source>
        <translation>التردد: %1</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1112"/>
        <source>Time: −%1</source>
        <translation>الوقت: −%1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1311"/>
        <source>Invalid Bluetooth adapter!</source>
        <translation>محول Bluetooth غير صالح!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1314"/>
        <source>Unsuported platform or operating system</source>
        <translation>نظام تشغيل أو منصة غير مدعومة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1317"/>
        <source>Unsupported discovery method</source>
        <translation>طريقة اكتشاف غير مدعومة</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1320"/>
        <source>General I/O error</source>
        <translation>خطأ إدخال/إخراج عام</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/LocalProvider.cpp" line="319"/>
        <source>No local model server URL configured. Open Manage Keys to set one.</source>
        <translation>لم يتم تكوين عنوان URL لخادم النموذج المحلي. افتح إدارة المفاتيح لتعيين واحد.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/DeepSeekProvider.cpp" line="146"/>
        <source>No DeepSeek API key set. Open Manage Keys to add one.</source>
        <translation>لم يتم تعيين مفتاح API لـ DeepSeek. افتح إدارة المفاتيح لإضافة واحد.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/MistralProvider.cpp" line="168"/>
        <source>No Mistral API key set. Open Manage Keys to add one.</source>
        <translation>لم يتم تعيين مفتاح Mistral API. افتح إدارة المفاتيح لإضافة واحد.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIProvider.cpp" line="362"/>
        <source>No OpenAI API key set. Open Manage Keys to add one.</source>
        <translation>لم يتم تعيين مفتاح OpenAI API. افتح إدارة المفاتيح لإضافة واحد.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenRouterProvider.cpp" line="181"/>
        <source>No OpenRouter API key set. Open Manage Keys to add one.</source>
        <translation>لم يتم تعيين مفتاح OpenRouter API. افتح إدارة المفاتيح لإضافة واحد.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GroqProvider.cpp" line="152"/>
        <source>No Groq API key set. Open Manage Keys to add one.</source>
        <translation>لم يتم تعيين مفتاح Groq API. افتح إدارة المفاتيح لإضافة واحد.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicProvider.cpp" line="234"/>
        <source>No Anthropic API key set. Open Manage Keys to add one.</source>
        <translation>لم يتم تعيين مفتاح Anthropic API. افتح إدارة المفاتيح لإضافة واحد.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiProvider.cpp" line="285"/>
        <source>No Gemini API key set. Open Manage Keys to add one.</source>
        <translation>لم يتم تعيين مفتاح Gemini API. افتح إدارة المفاتيح لإضافة واحد.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="251"/>
        <source>Network error</source>
        <translation>خطأ في الشبكة</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="254"/>
        <location filename="../../src/Licensing/Trial.cpp" line="270"/>
        <location filename="../../src/Licensing/Trial.cpp" line="302"/>
        <source>Trial Activation Error</source>
        <translation>خطأ في تفعيل الفترة التجريبية</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="267"/>
        <source>Invalid server response</source>
        <translation>استجابة خادم غير صالحة</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="268"/>
        <source>The server returned malformed data: %1</source>
        <translation>أرجع الخادم بيانات مشوهة: %1</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="299"/>
        <source>Unexpected server response</source>
        <translation>استجابة غير متوقعة من الخادم</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="300"/>
        <source>The server response is missing required fields.</source>
        <translation>استجابة الخادم تفتقد حقولاً مطلوبة.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1057"/>
        <source>The frame parser is using more than %1% of CPU time.</source>
        <translation>محلل الإطارات يستخدم أكثر من %1% من وقت المعالج.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1059"/>
        <source>Serial Studio is dropping frames to keep the application responsive. Please simplify or optimize the frame parser script to reduce its workload.</source>
        <translation>Serial Studio يتجاهل إطارات للحفاظ على استجابة التطبيق. يُرجى تبسيط أو تحسين سكريبت محلل الإطارات لتقليل عبء العمل.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="252"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="273"/>
        <source>Frame Parser Disabled</source>
        <translation>محلل الإطارات معطّل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="274"/>
        <source>The Lua frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>محلل الإطارات Lua للمصدر %1 تجاوز المهلة لـ %2 إطار متتالي وتم تعطيله للحفاظ على استجابة Serial Studio.

السبب الأرجح: حلقة لا نهائية أو عملية بطيئة للغاية في نص السكريبت. أصلح السكريبت وأعد تحميل المشروع لإعادة تفعيل التحليل.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="322"/>
        <source>Lua Syntax Error</source>
        <translation>خطأ صياغة Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="323"/>
        <source>The parser code contains an error:

%1</source>
        <translation>كود المحلل يحتوي على خطأ:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="371"/>
        <source>Lua Runtime Error</source>
        <translation>خطأ تشغيل Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="372"/>
        <source>The parser code triggered an error:

%1</source>
        <translation>كود المحلل تسبب في خطأ:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="478"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="393"/>
        <source>Missing Parse Function</source>
        <translation>دالة التحليل مفقودة</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="394"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) ... end</source>
        <translation>دالة 'parse' غير معرّفة في السكريبت.

يُرجى التأكد من أن الكود يتضمن:
function parse(frame) ... end</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="530"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="456"/>
        <source>Parse Function Runtime Error</source>
        <translation>خطأ تنفيذ دالة التحليل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="457"/>
        <source>The parse function contains an error:

%1

Please fix the error in the function body.</source>
        <translation>دالة التحليل تحتوي على خطأ:

%1

يُرجى إصلاح الخطأ في جسم الدالة.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="253"/>
        <source>The JavaScript frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>محلل الإطارات JavaScript للمصدر %1 تجاوز المهلة لـ %2 إطار متتالي وتم تعطيله للحفاظ على استجابة Serial Studio.

السبب الأرجح: حلقة لا نهائية أو عملية بطيئة للغاية في جسم السكريبت. أصلح السكريبت وأعد تحميل المشروع لإعادة تفعيل التحليل.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="419"/>
        <source>JavaScript Timed Out</source>
        <translation>انتهت مهلة JavaScript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="420"/>
        <source>The parser code did not finish evaluating within %1 ms and was interrupted.

Most likely cause: an infinite loop at the top level of the script.</source>
        <translation>لم ينته كود المحلل من التقييم خلال %1 مللي ثانية وتم إيقافه.

السبب الأكثر احتمالاً: حلقة لا نهائية في المستوى الأعلى من النص البرمجي.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="437"/>
        <source>JavaScript Syntax Error</source>
        <translation>خطأ صياغة JavaScript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="438"/>
        <source>The parser code contains a syntax error at line %1:

%2</source>
        <translation>كود المحلل يحتوي على خطأ صياغة في السطر %1:

%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="452"/>
        <source>JavaScript Exception Occurred</source>
        <translation>حدث استثناء JavaScript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="453"/>
        <source>The parser code triggered the following exceptions:

%1</source>
        <translation>كود المحلل أثار الاستثناءات التالية:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="479"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) { ... }</source>
        <translation>دالة 'parse' غير معرّفة في السكريبت.

يُرجى التأكد من أن الكود يتضمن:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="531"/>
        <source>The parse function contains an error at line %1:

%2

Please fix the error in the function body.</source>
        <translation>تحتوي دالة التحليل على خطأ في السطر %1:

%2

يُرجى إصلاح الخطأ في نص الدالة.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="631"/>
        <source>Invalid Function Declaration</source>
        <translation>تصريح دالة غير صالح</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="632"/>
        <source>No callable 'parse' export found.

Define one of:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</source>
        <translation>لم يتم العثور على تصدير قابل للاستدعاء باسم 'parse'.

عرّف أحد الخيارات التالية:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="648"/>
        <source>The 'parse' function must accept at least one parameter (the frame payload).</source>
        <translation>يجب أن تقبل دالة 'parse' معاملًا واحدًا على الأقل (حمولة الـ Frame).</translation>
    </message>
    <message>
        <source>No valid 'parse' function declaration found.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">لم يتم العثور على تصريح دالة 'parse' صالح.

التنسيق المتوقع:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="647"/>
        <source>Invalid Function Parameter</source>
        <translation>معامل دالة غير صالح</translation>
    </message>
    <message>
        <source>The 'parse' function must have at least one parameter.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">يجب أن تحتوي دالة 'parse' على معامل واحد على الأقل.

التنسيق المتوقع:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="613"/>
        <source>Deprecated Function Signature</source>
        <translation>توقيع دالة مُهمل</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="614"/>
        <source>The 'parse' function uses the old two-parameter format: parse(%1, %2)

This format is no longer supported. Please update to the new single-parameter format:
function parse(%1) { ... }

The separator parameter is no longer needed.</source>
        <translation>تستخدم دالة 'parse' تنسيق المعاملين القديم: parse(%1, %2)

هذا التنسيق لم يعد مدعومًا. يُرجى التحديث إلى تنسيق المعامل الواحد الجديد:
function parse(%1) { ... }

لم تعد هناك حاجة لمعامل الفاصل.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="386"/>
        <source>Expected %1, got '%2'</source>
        <translation>متوقع %1، تم الحصول على '%2'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="435"/>
        <source>Expected enum name after 'enum'</source>
        <translation>متوقع اسم enum بعد 'enum'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="449"/>
        <source>Expected oneof name</source>
        <translation>متوقع اسم oneof</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="476"/>
        <source>Field tag '%1' out of range (1..%2)</source>
        <translation>وسم الحقل '%1' خارج النطاق (1..%2)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="494"/>
        <source>Expected key type in map&lt;&gt;</source>
        <translation>نوع المفتاح المتوقع في map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="502"/>
        <source>Expected value type in map&lt;&gt;</source>
        <translation>نوع القيمة المتوقع في map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="510"/>
        <source>Expected map field name</source>
        <translation>اسم حقل map المتوقع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="522"/>
        <source>Expected map field tag</source>
        <translation>وسم حقل map المتوقع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="554"/>
        <source>Expected field type, got '%1'</source>
        <translation>نوع الحقل المتوقع، تم الحصول على '%1'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="573"/>
        <source>Expected field name after type</source>
        <translation>اسم الحقل المتوقع بعد النوع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="583"/>
        <source>Expected field tag number</source>
        <translation>رقم وسم الحقل المتوقع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="630"/>
        <source>Message nesting too deep (limit %1)</source>
        <translation>تداخل الرسائل عميق جدًا (الحد الأقصى %1)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="635"/>
        <source>Expected message name</source>
        <translation>اسم الرسالة المتوقع</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="717"/>
        <source>Unexpected token '%1' at file scope</source>
        <translation>رمز غير متوقع '%1' في نطاق الملف</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="763"/>
        <source>Unsupported top-level keyword '%1'</source>
        <translation>كلمة مفتاحية '%1' غير مدعومة في المستوى الأعلى</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="162"/>
        <source>Console Output File Error</source>
        <translation>خطأ في ملف إخراج وحدة التحكم</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="163"/>
        <source>Cannot open file for writing!</source>
        <translation>تعذر فتح الملف للكتابة!</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="294"/>
        <source>Automatic (Platform Default)</source>
        <translation>تلقائي (افتراضي النظام)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="299"/>
        <source>Software (Fallback)</source>
        <translation>برمجي (بديل)</translation>
    </message>
    <message>
        <source>Row %1</source>
        <translation type="vanished">الصف %1</translation>
    </message>
    <message>
        <source>[%1]</source>
        <translation type="vanished">[%1]</translation>
    </message>
    <message>
        <source>Frame %1</source>
        <translation type="vanished">الإطار %1</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">وحدة فك الترميز</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation type="vanished">الصفوف</translation>
    </message>
    <message>
        <source>%1 row(s)</source>
        <translation type="vanished">%1 صف (صفوف)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="186"/>
        <source>The native parser configuration is not a valid JSON object.</source>
        <translation>إعداد المحلل الأصلي ليس كائن JSON صالح.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="196"/>
        <source>Unknown native parser template: "%1".</source>
        <translation>قالب محلل أصلي غير معروف: "%1".</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="330"/>
        <source>Built-In Parser Error</source>
        <translation>خطأ في المحلل المدمج</translation>
    </message>
    <message>
        <source>Native Parser Error</source>
        <translation type="vanished">خطأ في المحلل الأصلي</translation>
    </message>
</context>
<context>
    <name>QuaGzipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="60"/>
        <source>QIODevice::Append is not supported for GZIP</source>
        <translation>QIODevice::Append غير مدعوم لـ GZIP</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="66"/>
        <source>Opening gzip for both reading and writing is not supported</source>
        <translation>فتح gzip للقراءة والكتابة معاً غير مدعوم</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="75"/>
        <source>You can open a gzip either for reading or for writing. Which is it?</source>
        <translation>يمكنك فتح gzip إما للقراءة أو للكتابة. أيهما تريد؟</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="81"/>
        <source>Could not gzopen() file</source>
        <translation>تعذر تنفيذ gzopen() على الملف</translation>
    </message>
</context>
<context>
    <name>QuaZIODevice</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="178"/>
        <source>QIODevice::Append is not supported for QuaZIODevice</source>
        <translation>QIODevice::Append غير مدعوم لـ QuaZIODevice</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="183"/>
        <source>QIODevice::ReadWrite is not supported for QuaZIODevice</source>
        <translation>QIODevice::ReadWrite غير مدعوم لـ QuaZIODevice</translation>
    </message>
</context>
<context>
    <name>QuaZipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quazipfile.cpp" line="251"/>
        <source>ZIP/UNZIP API error %1</source>
        <translation>خطأ في واجهة ZIP/UNZIP البرمجية %1</translation>
    </message>
</context>
<context>
    <name>ReportOptionsDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate PDF Report</source>
        <translation>إنشاء تقرير PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate Report</source>
        <translation>إنشاء تقرير</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="61"/>
        <source>Solid</source>
        <translation>صلب</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="62"/>
        <source>Dashed</source>
        <translation>متقطع</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="63"/>
        <source>Dotted</source>
        <translation>منقط</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="80"/>
        <source>A4 (210 × 297 mm)</source>
        <translation>A4 (210 × 297 مم)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="81"/>
        <source>A3 (297 × 420 mm)</source>
        <translation>A3 (297 × 420 مم)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="82"/>
        <source>A2 (420 × 594 mm)</source>
        <translation>A2 (420 × 594 مم)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="83"/>
        <source>A1 (594 × 841 mm)</source>
        <translation>A1 (594 × 841 مم)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="84"/>
        <source>A0 (841 × 1189 mm)</source>
        <translation>A0 (841 × 1189 مم)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="85"/>
        <source>A5 (148 × 210 mm)</source>
        <translation>A5 (148 × 210 مم)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="86"/>
        <source>A6 (105 × 148 mm)</source>
        <translation>A6 (105 × 148 مم)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="87"/>
        <source>B4 (250 × 353 mm)</source>
        <translation>B4 (250 × 353 مم)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="88"/>
        <source>B5 (176 × 250 mm)</source>
        <translation>B5 (176 × 250 مم)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="89"/>
        <source>Letter (8.5 × 11 in)</source>
        <translation>Letter (8.5 × 11 بوصة)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="90"/>
        <source>Legal (8.5 × 14 in)</source>
        <translation>Legal (8.5 × 14 بوصة)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="91"/>
        <source>Executive (7.25 × 10.5 in)</source>
        <translation>Executive (7.25 × 10.5 بوصة)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="92"/>
        <source>Tabloid (11 × 17 in)</source>
        <translation>Tabloid (11 × 17 بوصة)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="93"/>
        <source>Ledger (17 × 11 in)</source>
        <translation>Ledger (17 × 11 بوصة)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="103"/>
        <source>%1 — Session Report</source>
        <translation>%1 — تقرير الجلسة</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="105"/>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="279"/>
        <source>Session Report</source>
        <translation>تقرير الجلسة</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="187"/>
        <source>Branding</source>
        <translation>العلامة التجارية</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="193"/>
        <source>Page</source>
        <translation>الصفحة</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="199"/>
        <source>Sections</source>
        <translation>الأقسام</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="247"/>
        <source>Identity</source>
        <translation>الهوية</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="261"/>
        <source>Company</source>
        <translation>الشركة</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="268"/>
        <source>e.g. Acme Test Systems</source>
        <translation>مثال: أنظمة اختبار أكمي</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="272"/>
        <source>Document title</source>
        <translation>عنوان المستند</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="283"/>
        <source>Author</source>
        <translation>المؤلف</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="290"/>
        <source>Prepared by (optional)</source>
        <translation>أعده (اختياري)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="299"/>
        <source>Logo</source>
        <translation>الشعار</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="312"/>
        <source>File</source>
        <translation>ملف</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="323"/>
        <source>PNG, JPG or SVG (optional)</source>
        <translation>PNG أو JPG أو SVG (اختياري)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="325"/>
        <source>Browse…</source>
        <translation>استعراض…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="328"/>
        <source>Clear</source>
        <translation>مسح</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="369"/>
        <source>Paper</source>
        <translation>ورق</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="381"/>
        <source>Page size</source>
        <translation>حجم الصفحة</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="396"/>
        <source>Plot appearance</source>
        <translation>مظهر الرسم البياني</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="410"/>
        <source>Line width</source>
        <translation>عرض الخط</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="442"/>
        <source>Line style</source>
        <translation>نمط الخط</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="482"/>
        <source>Include</source>
        <translation>تضمين</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="497"/>
        <source>Cover page (logo, document title, test subtitle)</source>
        <translation>صفحة الغلاف (الشعار، عنوان المستند، العنوان الفرعي للاختبار)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="500"/>
        <source>Test information (project, timestamps, classification and notes)</source>
        <translation>معلومات الاختبار (المشروع، الطوابع الزمنية، التصنيف والملاحظات)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="503"/>
        <source>Measurement summary (min, max, mean, std. deviation per parameter)</source>
        <translation>ملخص القياسات (الحد الأدنى، الحد الأقصى، المتوسط، الانحراف المعياري لكل معامل)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="506"/>
        <source>Parameter trends (time-series chart per numeric parameter)</source>
        <translation>اتجاهات المعاملات (مخطط السلاسل الزمنية لكل معامل رقمي)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="509"/>
        <source>Annotate min, max, and mean values on plots</source>
        <translation>إضافة تعليقات توضيحية لقيم الحد الأدنى والأقصى والمتوسط على المخططات</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="532"/>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="557"/>
        <source>Export PDF</source>
        <translation>تصدير PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="557"/>
        <source>Export HTML</source>
        <translation>تصدير HTML</translation>
    </message>
</context>
<context>
    <name>ReportProgressDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="20"/>
        <source>Generating Report</source>
        <translation>إنشاء التقرير</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="69"/>
        <source>Working…</source>
        <translation>جارٍ العمل…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="86"/>
        <source>This can take a few seconds for sessions with many parameters. The window closes automatically when the report is ready.</source>
        <translation>قد يستغرق هذا بضع ثوانٍ للجلسات ذات المعاملات الكثيرة. تُغلق النافذة تلقائيًا عند جاهزية التقرير.</translation>
    </message>
</context>
<context>
    <name>RuntimeReconfigure</name>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="40"/>
        <source>Connection Lost</source>
        <translation>فُقد الاتصال</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="41"/>
        <source>Device Unavailable</source>
        <translation>الجهاز غير متاح</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="95"/>
        <source>The connection to your device was lost.</source>
        <translation>فُقد الاتصال بجهازك.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="96"/>
        <source>Serial Studio couldn't reach your device.</source>
        <translation>تعذر على Serial Studio الوصول إلى جهازك.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="105"/>
        <source>Check the cable, power, and that no other application has taken over the device. You can try reconnecting, switch to a different device, or quit.</source>
        <translation>تحقق من الكابل والطاقة وأنه لا يوجد تطبيق آخر استحوذ على الجهاز. يمكن محاولة إعادة الاتصال أو التبديل إلى جهاز مختلف أو الإنهاء.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="108"/>
        <source>Make sure it's plugged in, powered on, and not already in use by another app. You can try again, pick a different device, or quit.</source>
        <translation>تأكد من توصيله وتشغيله وأنه غير مستخدم بالفعل من تطبيق آخر. يمكن المحاولة مجددًا أو اختيار جهاز مختلف أو الإنهاء.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="120"/>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="189"/>
        <source>Quit</source>
        <translation>إنهاء</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="130"/>
        <source>Pick Different Device</source>
        <translation>اختيار جهاز مختلف</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Reconnect</source>
        <translation>إعادة الاتصال</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Try Again</source>
        <translation>إعادة المحاولة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="157"/>
        <source>Pick the correct device, then press Connect.</source>
        <translation>اختر الجهاز الصحيح، ثم اضغط اتصال.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="166"/>
        <source>I/O Interface: %1</source>
        <translation>واجهة الإدخال/الإخراج: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="199"/>
        <source>Connect</source>
        <translation>اتصال</translation>
    </message>
</context>
<context>
    <name>SerialStudio</name>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="334"/>
        <source>Data Grids</source>
        <translation>شبكات البيانات</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="337"/>
        <source>Multiple Data Plots</source>
        <translation>رسوم بيانية متعددة للبيانات</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="340"/>
        <source>Accelerometers</source>
        <translation>مقاييس التسارع</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="343"/>
        <source>Gyroscopes</source>
        <translation>الجيروسكوبات</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="346"/>
        <source>GPS</source>
        <translation>GPS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="349"/>
        <source>FFT Plots</source>
        <translation>رسوم FFT البيانية</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="352"/>
        <source>LED Panels</source>
        <translation>لوحات LED</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="355"/>
        <source>Data Plots</source>
        <translation>رسوم البيانات</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="358"/>
        <source>Bars</source>
        <translation>الأشرطة</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="361"/>
        <source>Gauges</source>
        <translation>المقاييس</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="364"/>
        <source>Terminal</source>
        <translation>الطرفية</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="367"/>
        <source>Clock</source>
        <translation>ساعة</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="370"/>
        <source>Stopwatch</source>
        <translation>ساعة إيقاف</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="373"/>
        <source>Compasses</source>
        <translation>البوصلات</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="376"/>
        <source>Meters</source>
        <translation>المقاييس</translation>
    </message>
    <message>
        <source>Thermometers</source>
        <translation type="vanished">مقاييس الحرارة</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="379"/>
        <source>3D Plots</source>
        <translation>رسوم 3D</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="383"/>
        <source>Image Views</source>
        <translation>عارضات الصور</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="386"/>
        <source>Output Panels</source>
        <translation>لوحات الإخراج</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="389"/>
        <source>Notifications</source>
        <translation>الإشعارات</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="392"/>
        <source>Waterfalls</source>
        <translation>الشلالات</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="395"/>
        <source>Painter Widgets</source>
        <translation>عناصر الرسام</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="934"/>
        <source>UTF-8</source>
        <translation>UTF-8</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="935"/>
        <source>UTF-16 LE</source>
        <translation>UTF-16 LE</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="936"/>
        <source>UTF-16 BE</source>
        <translation>UTF-16 BE</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="937"/>
        <source>Latin-1</source>
        <translation>Latin-1</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="938"/>
        <source>System</source>
        <translation>النظام</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="939"/>
        <source>GBK</source>
        <translation>GBK</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="940"/>
        <source>GB18030</source>
        <translation>GB18030</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="941"/>
        <source>Big5</source>
        <translation>Big5</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="942"/>
        <source>Shift-JIS</source>
        <translation>Shift-JIS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="943"/>
        <source>EUC-JP</source>
        <translation>EUC-JP</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="944"/>
        <source>EUC-KR</source>
        <translation>EUC-KR</translation>
    </message>
</context>
<context>
    <name>SessionDetail</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="19"/>
        <source>Session Details</source>
        <translation>تفاصيل الجلسة</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="88"/>
        <source>Select a session to view details.</source>
        <translation>اختر جلسة لعرض التفاصيل.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="130"/>
        <source>Project:</source>
        <translation>المشروع:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="143"/>
        <source>Started:</source>
        <translation>البداية:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="156"/>
        <source>Ended:</source>
        <translation>الانتهاء:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="162"/>
        <source>(in progress)</source>
        <translation>(قيد التقدم)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="169"/>
        <source>Frames:</source>
        <translation>الإطارات:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="185"/>
        <source>Notes</source>
        <translation>ملاحظات</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="200"/>
        <source>Add session notes…</source>
        <translation>إضافة ملاحظات الجلسة…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="201"/>
        <source>Notes are read-only for completed sessions.</source>
        <translation>الملاحظات للقراءة فقط للجلسات المكتملة.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="222"/>
        <source>Tags</source>
        <translation>وسوم</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="286"/>
        <source>New tag…</source>
        <translation>وسم جديد…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="293"/>
        <source>Add</source>
        <translation>إضافة</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="330"/>
        <source>Replay</source>
        <translation>إعادة تشغيل</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="338"/>
        <source>Export CSV</source>
        <translation>تصدير CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="345"/>
        <source>Generate Report</source>
        <translation>إنشاء تقرير</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="356"/>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="362"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>فتح قفل ملف الجلسة لحذف الجلسات</translation>
    </message>
</context>
<context>
    <name>SessionList</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="19"/>
        <source>Sessions</source>
        <translation>الجلسات</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="71"/>
        <source>Search</source>
        <translation>بحث</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="91"/>
        <source>Date</source>
        <translation>التاريخ</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="92"/>
        <source>Frames</source>
        <translation>الإطارات</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="93"/>
        <source>Tags</source>
        <translation>الوسوم</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="193"/>
        <source>No sessions found.</source>
        <translation>لم يتم العثور على جلسات.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="194"/>
        <source>No session file open.</source>
        <translation>لا يوجد ملف جلسة مفتوح.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseManager</name>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="426"/>
        <source>Open Session File</source>
        <translation>فتح ملف جلسة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="428"/>
        <source>Session files (*.db)</source>
        <translation>ملفات الجلسات (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="484"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="493"/>
        <source>Lock Session File</source>
        <translation>قفل ملف الجلسة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="485"/>
        <source>Choose a password to lock the session file:</source>
        <translation>اختر كلمة مرور لقفل ملف الجلسة:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="494"/>
        <source>Confirm the password:</source>
        <translation>تأكيد كلمة المرور:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="502"/>
        <source>Passwords do not match</source>
        <translation>كلمات المرور غير متطابقة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="503"/>
        <source>The two passwords you entered do not match. The session file was not locked.</source>
        <translation>كلمتا المرور المُدخلتان غير متطابقتين. لم يتم قفل ملف الجلسة.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="539"/>
        <source>Unlock Session File</source>
        <translation>فتح قفل ملف الجلسة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="540"/>
        <source>Enter the session file password:</source>
        <translation>أدخل كلمة مرور ملف الجلسة:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="550"/>
        <source>Incorrect password</source>
        <translation>كلمة مرور غير صحيحة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="551"/>
        <source>The password you entered does not match the one stored in the session file.</source>
        <translation>كلمة المرور المُدخلة لا تطابق المحفوظة في ملف الجلسة.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="643"/>
        <source>Session file locked</source>
        <translation>تم قفل ملف الجلسة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="644"/>
        <source>Unlock the session file before deleting recorded sessions.</source>
        <translation>فتح قفل ملف الجلسة قبل حذف الجلسات المسجلة.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="653"/>
        <source>Delete session from %1?</source>
        <translation>حذف الجلسة من %1؟</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="654"/>
        <source>All readings and raw data for this session are permanently removed.</source>
        <translation>سيتم حذف جميع القراءات والبيانات الخام لهذه الجلسة نهائياً.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="656"/>
        <source>Delete Session</source>
        <translation>حذف الجلسة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="778"/>
        <source>Export Session to CSV</source>
        <translation>تصدير الجلسة إلى CSV</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="778"/>
        <source>CSV files (*.csv)</source>
        <translation>ملفات CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="853"/>
        <source>Loading session data…</source>
        <translation>تحميل بيانات الجلسة…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="873"/>
        <source>Save PDF Report</source>
        <translation>حفظ تقرير PDF</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="873"/>
        <source>Save HTML Report</source>
        <translation>حفظ تقرير HTML</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="874"/>
        <source>PDF files (*.pdf)</source>
        <translation>ملفات PDF (*.PDF)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="874"/>
        <source>HTML files (*.html)</source>
        <translation>ملفات HTML (*.HTML)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="937"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="973"/>
        <source>Failed</source>
        <translation>فشل</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="943"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="983"/>
        <source>Report Failed</source>
        <translation>فشل التقرير</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="945"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="984"/>
        <source>Could not generate the report. Check the output path and try again.</source>
        <translation>تعذر إنشاء التقرير. تحقق من مسار الإخراج وحاول مرة أخرى.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="973"/>
        <source>Done</source>
        <translation>تم</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1001"/>
        <source>Select logo image</source>
        <translation>اختر صورة الشعار</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1003"/>
        <source>Images (*.png *.jpg *.jpeg *.svg)</source>
        <translation>صور (*.png *.jpg *.jpeg *.svg)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1061"/>
        <source>No project data</source>
        <translation>لا توجد بيانات مشروع</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1062"/>
        <source>This session file does not contain an embedded project.</source>
        <translation>ملف الجلسة هذا لا يحتوي على مشروع مضمّن.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1071"/>
        <source>Invalid project data</source>
        <translation>بيانات مشروع غير صالحة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1072"/>
        <source>The embedded project JSON is malformed and cannot be restored.</source>
        <translation>ملف JSON الخاص بالمشروع المضمّن تالف ولا يمكن استعادته.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1082"/>
        <source>Restore Project</source>
        <translation>استعادة المشروع</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1082"/>
        <source>Serial Studio projects (*.ssproj *.json)</source>
        <translation>مشاريع Serial Studio (*.ssproj *.json)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1090"/>
        <source>Cannot write file</source>
        <translation>تعذّرت كتابة الملف</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1090"/>
        <source>Check file permissions and try again.</source>
        <translation>تحقق من أذونات الملف وحاول مرة أخرى.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1200"/>
        <source>Cannot open session file</source>
        <translation>تعذّر فتح ملف الجلسة</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseWorker</name>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="76"/>
        <source>Empty file path</source>
        <translation>مسار الملف فارغ</translation>
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
        <translation>قاعدة البيانات غير مفتوحة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="262"/>
        <source>Database not open or empty label</source>
        <translation>قاعدة البيانات غير مفتوحة أو التسمية فارغة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="330"/>
        <source>Invalid label</source>
        <translation>تسمية غير صالحة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="597"/>
        <source>Cancelled</source>
        <translation>ملغى</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="710"/>
        <source>Could not load session data</source>
        <translation>تعذر تحميل بيانات الجلسة</translation>
    </message>
</context>
<context>
    <name>Sessions::HtmlReport</name>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="209"/>
        <source>Assembling report…</source>
        <translation>تجميع التقرير…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="217"/>
        <source>Writing output…</source>
        <translation>كتابة المخرجات…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="282"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="342"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="700"/>
        <source>Session Report</source>
        <translation>تقرير الجلسة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="345"/>
        <source>Untitled project</source>
        <translation>مشروع بدون عنوان</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="352"/>
        <source>Prepared by</source>
        <translation>أعده</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="355"/>
        <source>Generated on %1</source>
        <translation>تم الإنشاء في %1</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="377"/>
        <source>Test ID</source>
        <translation>معرّف الاختبار</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="379"/>
        <source>Duration</source>
        <translation>المدة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="381"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="493"/>
        <source>Samples</source>
        <translation>العينات</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="383"/>
        <source>Parameters</source>
        <translation>المعاملات</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="385"/>
        <source>Started</source>
        <translation>البداية</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="387"/>
        <source>Ended</source>
        <translation>النهاية</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="423"/>
        <source>Project</source>
        <translation>المشروع</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="425"/>
        <source>Test identifier</source>
        <translation>معرّف الاختبار</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="426"/>
        <source>Start time</source>
        <translation>وقت البدء</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="427"/>
        <source>End time</source>
        <translation>وقت الانتهاء</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="428"/>
        <source>Total duration</source>
        <translation>المدة الإجمالية</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="429"/>
        <source>Samples acquired</source>
        <translation>العينات المُكتسبة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="430"/>
        <source>Parameters logged</source>
        <translation>المعاملات المسجلة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="446"/>
        <source>Classification</source>
        <translation>التصنيف</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="453"/>
        <source>Notes</source>
        <translation>الملاحظات</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="461"/>
        <source>Test Information</source>
        <translation>معلومات الاختبار</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="482"/>
        <source>Parameter</source>
        <translation>المعامل</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="485"/>
        <source>Units</source>
        <translation>الوحدات</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="494"/>
        <source>Minimum</source>
        <translation>الحد الأدنى</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="495"/>
        <source>Maximum</source>
        <translation>الحد الأقصى</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="496"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="652"/>
        <source>Mean</source>
        <translation>المتوسط</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="497"/>
        <source>Std. Deviation</source>
        <translation>الانحراف المعياري</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="542"/>
        <source>Measurement Summary</source>
        <translation>ملخص القياسات</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="543"/>
        <source>click a column to sort</source>
        <translation>انقر على عمود للترتيب</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="568"/>
        <source>%1 samples over %2 seconds</source>
        <translation>%1 عينة على مدى %2 ثانية</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="586"/>
        <source>Combined Parameter View</source>
        <translation>عرض المعاملات المجمّع</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="587"/>
        <source>click legend items to toggle signals</source>
        <translation>انقر على عناصر وسيلة الإيضاح لتبديل الإشارات</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="595"/>
        <source>Parameter Trends</source>
        <translation>اتجاهات المعاملات</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="650"/>
        <source>Min</source>
        <translation>الأدنى</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="651"/>
        <source>Max</source>
        <translation>الأقصى</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="724"/>
        <source>Page %1 of %2</source>
        <translation>صفحة %1 من %2</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="794"/>
        <source>Loading rendering engine…</source>
        <translation>تحميل محرك العرض…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="814"/>
        <source>Rendering charts…</source>
        <translation>جارٍ عرض المخططات…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="858"/>
        <source>Generating PDF…</source>
        <translation>جارٍ إنشاء PDF…</translation>
    </message>
</context>
<context>
    <name>Sessions::Player</name>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="261"/>
        <source>Open Session File</source>
        <translation>فتح ملف الجلسة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="263"/>
        <source>Session files (*.db)</source>
        <translation>ملفات الجلسات (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="334"/>
        <source>Device Connection Active</source>
        <translation>اتصال الجهاز نشط</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="335"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>لاستخدام هذه الميزة، يجب قطع الاتصال بالجهاز. هل تريد المتابعة؟</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="383"/>
        <location filename="../../src/Sessions/Player.cpp" line="462"/>
        <source>Cannot open session file</source>
        <translation>تعذر فتح ملف الجلسة</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="384"/>
        <source>Unknown error</source>
        <translation>خطأ غير معروف</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="400"/>
        <source>No project data</source>
        <translation>لا توجد بيانات مشروع</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="401"/>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation>لا تحتوي هذه الجلسة على ملف مشروع مضمّن — تستخدم لوحة المعلومات تخطيط رسم سريع بدلاً من ذلك.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="463"/>
        <source>Check file permissions and try again.</source>
        <translation>تحقق من أذونات الملف وحاول مرة أخرى.</translation>
    </message>
</context>
<context>
    <name>Sessions::PlayerLoaderWorker</name>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="69"/>
        <source>This file does not contain any recording sessions.</source>
        <translation>لا يحتوي هذا الملف على أي جلسات مسجلة.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="144"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="205"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="224"/>
        <source>Cancelled</source>
        <translation>ملغى</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="168"/>
        <source>Empty file path</source>
        <translation>مسار الملف فارغ</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="218"/>
        <source>The selected session is missing its column definitions.</source>
        <translation>الجلسة المحددة تفتقد تعريفات الأعمدة الخاصة بها.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="235"/>
        <source>The selected session does not contain any frames.</source>
        <translation>الجلسة المحددة لا تحتوي على أي إطارات.</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="34"/>
        <source>Preferences</source>
        <translation>التفضيلات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="61"/>
        <source>General</source>
        <translation>عام</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="67"/>
        <source>Dashboard</source>
        <translation>لوحة المعلومات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="73"/>
        <source>Taskbar</source>
        <translation>شريط المهام</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="79"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="849"/>
        <source>Console</source>
        <translation>وحدة التحكم</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="86"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="850"/>
        <source>Notifications</source>
        <translation>الإشعارات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="144"/>
        <source>Appearance</source>
        <translation>المظهر</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="159"/>
        <source>Language</source>
        <translation>اللغة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="175"/>
        <source>Theme</source>
        <translation>السمة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="194"/>
        <source>Files &amp; Updates</source>
        <translation>الملفات والتحديثات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="210"/>
        <source>Workspace Folder</source>
        <translation>مجلد مساحة العمل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="237"/>
        <source>Automatically Check for Updates</source>
        <translation>التحقق من التحديثات تلقائيًا</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="255"/>
        <source>Advanced</source>
        <translation>متقدم</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="270"/>
        <source>Rendering Backend</source>
        <translation>محرك العرض</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="303"/>
        <source>Auto-Hide Toolbar</source>
        <translation>إخفاء شريط الأدوات تلقائيًا</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="320"/>
        <source>Enable API Server (Port 7777)</source>
        <translation>تفعيل خادم API (المنفذ 7777)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="338"/>
        <source>Allow External API Connections</source>
        <translation>السماح باتصالات API خارجية</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="354"/>
        <source>API Access Token</source>
        <translation>رمز الوصول لـ API</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="384"/>
        <source>Export Protobuf File</source>
        <translation>تصدير ملف Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="386"/>
        <source>Export…</source>
        <translation>تصدير…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="435"/>
        <source>Data Plotting</source>
        <translation>رسم البيانات</translation>
    </message>
    <message>
        <source>Point Count</source>
        <translation type="vanished">عدد النقاط</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="450"/>
        <source>Time Range</source>
        <translation>النطاق الزمني</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="500"/>
        <source>UI Refresh Rate (Hz)</source>
        <translation>معدل تحديث الواجهة (Hz)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="522"/>
        <source>Dashboard Font</source>
        <translation>خط لوحة المعلومات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="537"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1012"/>
        <source>Font Family</source>
        <translation>عائلة الخط</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="552"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1027"/>
        <source>Font Size</source>
        <translation>حجم الخط</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Small</source>
        <translation>صغير</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Normal</source>
        <translation>عادي</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Large</source>
        <translation>كبير</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Extra Large</source>
        <translation>كبير جداً</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Custom</source>
        <translation>مخصص</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="609"/>
        <source>Layout</source>
        <translation>التخطيط</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="624"/>
        <source>Show Actions Panel</source>
        <translation>إظهار لوحة الإجراءات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="647"/>
        <source>Video Export</source>
        <translation>تصدير الفيديو</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="665"/>
        <source>Save Videos by Default</source>
        <translation>حفظ الفيديوهات افتراضياً</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="716"/>
        <source>Behavior</source>
        <translation>السلوك</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="737"/>
        <source>Always Show Taskbar Buttons</source>
        <translation>إظهار أزرار شريط المهام دائمًا</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="752"/>
        <source>Show Search Field</source>
        <translation>إظهار حقل البحث</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="767"/>
        <source>Auto-hide Taskbar</source>
        <translation>إخفاء شريط المهام تلقائيًا</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="785"/>
        <source>Hide Delay (ms)</source>
        <translation>تأخير الإخفاء (ميلي ثانية)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="809"/>
        <source>Pinned Buttons</source>
        <translation>الأزرار المثبتة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="827"/>
        <source>Drag a pinned button on the taskbar to reorder it.</source>
        <translation>اسحب زرًا مثبتًا على شريط المهام لإعادة ترتيبه.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="848"/>
        <source>Settings</source>
        <translation>الإعدادات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="851"/>
        <source>Clock</source>
        <translation>ساعة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="852"/>
        <source>Stopwatch</source>
        <translation>ساعة إيقاف</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="853"/>
        <source>Pause / Resume</source>
        <translation>إيقاف مؤقت / استئناف</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="854"/>
        <source>File Transmission</source>
        <translation>نقل الملفات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="855"/>
        <source>AI Assistant</source>
        <translation>مساعد الذكاء الاصطناعي</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="984"/>
        <source>Display</source>
        <translation>العرض</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="999"/>
        <source>Display Mode</source>
        <translation>وضع العرض</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1044"/>
        <source>Show Timestamps</source>
        <translation>إظهار الطوابع الزمنية</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1063"/>
        <source>Data Transmission</source>
        <translation>إرسال البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1078"/>
        <source>Line Ending</source>
        <translation>نهاية السطر</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1091"/>
        <source>Input Mode</source>
        <translation>وضع الإدخال</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1104"/>
        <source>Text Encoding</source>
        <translation>ترميز النص</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1117"/>
        <source>Checksum</source>
        <translation>المجموع الاختباري</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1130"/>
        <source>Echo Sent Data</source>
        <translation>عكس البيانات المُرسَلة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1149"/>
        <source>Escape Codes</source>
        <translation>أكواد الهروب</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1164"/>
        <source>VT100 Emulation</source>
        <translation>محاكاة VT100</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1183"/>
        <source>ANSI Colors</source>
        <translation>ألوان ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1241"/>
        <source>Delivery</source>
        <translation>التسليم</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1256"/>
        <source>System Notifications</source>
        <translation>إشعارات النظام</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1277"/>
        <source>Show Warning/Critical events as OS desktop notifications when Serial Studio is not the foreground window.</source>
        <translation>عرض أحداث التحذير/الحرجة كإشعارات سطح المكتب عندما لا يكون Serial Studio في النافذة النشطة.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1287"/>
        <source>Application Logs</source>
        <translation>سجلات التطبيق</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1302"/>
        <source>Route Warnings to Notifications</source>
        <translation>توجيه التحذيرات إلى الإشعارات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1323"/>
        <source>Off by default — Qt and QML emit warnings frequently and enabling this can drown out real alarms. Critical messages are always routed regardless of this setting.</source>
        <translation>معطل افتراضياً — تصدر QT و QML تحذيرات بشكل متكرر وتفعيل هذا قد يطغى على التنبيهات الحقيقية. الرسائل الحرجة يتم توجيهها دائماً بغض النظر عن هذا الإعداد.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1342"/>
        <source>Reset</source>
        <translation>إعادة تعيين</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1377"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1385"/>
        <source>Apply</source>
        <translation>تطبيق</translation>
    </message>
</context>
<context>
    <name>Setup</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="35"/>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="380"/>
        <source>Device Setup</source>
        <translation>إعداد الجهاز</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="126"/>
        <source>API Server Active (%1)</source>
        <translation>خادم API نشط (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="127"/>
        <source>API Server Ready</source>
        <translation>خادم API جاهز</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="128"/>
        <source>API Server Off</source>
        <translation>خادم API متوقف</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="188"/>
        <source>Frame Parsing</source>
        <translation>تحليل الإطارات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="198"/>
        <source>Console Only (No Parsing)</source>
        <translation>وحدة التحكم فقط (بدون تحليل)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="211"/>
        <source>Quick Plot (Comma Separated Values)</source>
        <translation>رسم سريع (قيم مفصولة بفواصل)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="222"/>
        <source>Parse via Project File</source>
        <translation>تحليل عبر ملف المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="245"/>
        <source>Change Project File (%1)</source>
        <translation>تغيير ملف المشروع (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="246"/>
        <source>Select Project File</source>
        <translation>تحديد ملف المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="261"/>
        <source>Data Export</source>
        <translation>تصدير البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="285"/>
        <source>CSV Spreadsheet</source>
        <translation>جدول بيانات CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="303"/>
        <source>Session Recording</source>
        <translation>تسجيل الجلسة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="324"/>
        <source>MDF4 Recording</source>
        <translation>تسجيل MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="340"/>
        <source>Console Log</source>
        <translation>سجل وحدة التحكم</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="392"/>
        <source>I/O Interface: %1</source>
        <translation>واجهة الإدخال/الإخراج: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="461"/>
        <source>Multi-Device Project</source>
        <translation>مشروع متعدد الأجهزة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="474"/>
        <source>This project streams data from %1 independent devices.</source>
        <translation>يبث هذا المشروع البيانات من %1 جهاز مستقل.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="487"/>
        <source>Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.</source>
        <translation>لكل جهاز إعدادات اتصال خاصة به. يمكن تكوينها في محرر المشروع ضمن علامة تبويب المصادر.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="506"/>
        <source>Open Project Editor</source>
        <translation>فتح محرر المشروع</translation>
    </message>
</context>
<context>
    <name>ShortcutGenerator</name>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="22"/>
        <source>New Deployment</source>
        <translation>نشر جديد</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="93"/>
        <source>Choose an Icon</source>
        <translation>اختيار أيقونة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="106"/>
        <source>Save Deployment</source>
        <translation>حفظ النشر</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="146"/>
        <source>General</source>
        <translation>عام</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="152"/>
        <source>Taskbar</source>
        <translation>شريط المهام</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="158"/>
        <source>Logging</source>
        <translation>التسجيل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="215"/>
        <source>Identity</source>
        <translation>الهوية</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="271"/>
        <source>Click to choose an icon</source>
        <translation>انقر لاختيار أيقونة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="280"/>
        <source>Name:</source>
        <translation>الاسم:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="289"/>
        <source>Deployment Name</source>
        <translation>اسم النشر</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="298"/>
        <source>Change Icon…</source>
        <translation>تغيير الأيقونة…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="315"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="333"/>
        <source>Project</source>
        <translation>المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="343"/>
        <source>Choose a project file to begin</source>
        <translation>اختر ملف مشروع للبدء</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="365"/>
        <source>Behavior</source>
        <translation>السلوك</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="380"/>
        <source>Theme</source>
        <translation>المظهر</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="390"/>
        <source>Same as Serial Studio</source>
        <translation>نفس Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="402"/>
        <source>Fullscreen</source>
        <translation>ملء الشاشة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="412"/>
        <source>Actions Panel</source>
        <translation>لوحة الإجراءات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="423"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="632"/>
        <source>File Transmission</source>
        <translation>نقل الملفات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="439"/>
        <source>Double-clicking this deployment takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation>النقر المزدوج على هذا النشر ينقل مباشرة إلى لوحة المعلومات المباشرة لهذا المشروع. لا يوجد شريط أدوات أو لوحة إعداد، فقط البيانات، ويُغلق Serial Studio بمجرد قطع اتصال الجهاز.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="485"/>
        <source>Visibility</source>
        <translation>الرؤية</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="500"/>
        <source>Mode</source>
        <translation>الوضع</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="509"/>
        <source>Always shown</source>
        <translation>مرئي دائمًا</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="510"/>
        <source>Auto-hide</source>
        <translation>إخفاء تلقائي</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="511"/>
        <source>Hidden</source>
        <translation>مخفي</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="526"/>
        <source>Hiding the taskbar removes window minimize/maximize/close buttons and forces auto-layout, so the dashboard always fills the available area.</source>
        <translation>إخفاء شريط المهام يزيل أزرار تصغير/تكبير/إغلاق النافذة ويفرض التخطيط التلقائي، بحيث تملأ لوحة المعلومات المساحة المتاحة دائمًا.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="530"/>
        <source>The taskbar slides in when the user moves the cursor near the bottom edge.</source>
        <translation>ينزلق شريط المهام عندما يحرك المستخدم المؤشر بالقرب من الحافة السفلية.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="532"/>
        <source>The taskbar is permanently visible at the bottom of the dashboard.</source>
        <translation>شريط المهام مرئي بشكل دائم في أسفل لوحة المعلومات.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="545"/>
        <source>Pinned Buttons</source>
        <translation>الأزرار المثبتة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="562"/>
        <source>Console</source>
        <translation>وحدة التحكم</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="576"/>
        <source>Notifications</source>
        <translation>الإشعارات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="590"/>
        <source>Clock</source>
        <translation>ساعة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="604"/>
        <source>Stopwatch</source>
        <translation>ساعة إيقاف</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="618"/>
        <source>Pause</source>
        <translation>إيقاف مؤقت</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="682"/>
        <source>Recorders</source>
        <translation>المسجلات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="697"/>
        <source>CSV File</source>
        <translation>ملف CSV</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="707"/>
        <source>MDF4 File</source>
        <translation>ملف MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="717"/>
        <source>Session Database</source>
        <translation>قاعدة بيانات الجلسة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="727"/>
        <source>Console Log</source>
        <translation>سجل وحدة التحكم</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="743"/>
        <source>Recordings are saved in the Serial Studio workspace folder</source>
        <translation>يتم حفظ التسجيلات في مجلد مساحة عمل Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="772"/>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="781"/>
        <source>Save</source>
        <translation>حفظ</translation>
    </message>
</context>
<context>
    <name>SourceFrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="80"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="317"/>
        <source>Undo</source>
        <translation>تراجع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="87"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="328"/>
        <source>Redo</source>
        <translation>إعادة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="96"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="347"/>
        <source>Cut</source>
        <translation>قص</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="101"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="357"/>
        <source>Copy</source>
        <translation>نسخ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="106"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="367"/>
        <source>Paste</source>
        <translation>لصق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="113"/>
        <source>Select All</source>
        <translation>تحديد الكل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="123"/>
        <source>Format Document</source>
        <translation>تنسيق المستند</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="130"/>
        <source>Format Selection</source>
        <translation>تنسيق التحديد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="297"/>
        <source>Reset</source>
        <translation>إعادة تعيين</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="382"/>
        <source>Validate</source>
        <translation>التحقق</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="386"/>
        <source>Verify that the script compiles correctly</source>
        <translation>التحقق من أن السكريبت يُترجم بشكل صحيح</translation>
    </message>
    <message>
        <source>Reset template parameters to their defaults</source>
        <translation type="vanished">إعادة تعيين معاملات القالب إلى قيمها الافتراضية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="302"/>
        <source>Reset to the default parsing script</source>
        <translation>إعادة تعيين إلى نص التحليل الافتراضي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="307"/>
        <source>Open</source>
        <translation>فتح</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="312"/>
        <source>Import a script file for data parsing</source>
        <translation>استيراد ملف نص لتحليل البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="322"/>
        <source>Undo the last code edit</source>
        <translation>التراجع عن آخر تعديل للكود</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="334"/>
        <source>Redo the previously undone edit</source>
        <translation>إعادة التعديل الذي تم التراجع عنه</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="352"/>
        <source>Cut selected code to clipboard</source>
        <translation>قص الكود المحدد إلى الحافظة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="362"/>
        <source>Copy selected code to clipboard</source>
        <translation>نسخ الكود المحدد إلى الحافظة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="371"/>
        <source>Paste code from clipboard</source>
        <translation>لصق الكود من الحافظة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="247"/>
        <source>Help</source>
        <translation>مساعدة</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">فتح وثائق المساعدة لتحليل البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="196"/>
        <source>Platform:</source>
        <translation>المنصة:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="204"/>
        <source>Built-In</source>
        <translation>مدمج</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">اللغة:</translation>
    </message>
    <message>
        <source>Native</source>
        <translation type="vanished">أصلي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="222"/>
        <source>Select Template…</source>
        <translation>اختيار قالب…</translation>
    </message>
    <message>
        <source>Template:</source>
        <translation type="vanished">القالب:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="238"/>
        <source>Test With Sample Data</source>
        <translation>اختبار باستخدام بيانات عينة</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">تقييم</translation>
    </message>
</context>
<context>
    <name>SourceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="107"/>
        <source>Duplicate</source>
        <translation>تكرار</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="109"/>
        <source>Create a copy of this data source</source>
        <translation>إنشاء نسخة من مصدر البيانات هذا</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="121"/>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="126"/>
        <source>Remove this data source</source>
        <translation>إزالة مصدر البيانات هذا</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="127"/>
        <source>The primary data source cannot be removed</source>
        <translation>لا يمكن إزالة مصدر البيانات الأساسي</translation>
    </message>
</context>
<context>
    <name>SqlitePlayer</name>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="25"/>
        <source>Session Player</source>
        <translation>مشغل الجلسة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="92"/>
        <source>Loading session…</source>
        <translation>جارٍ تحميل الجلسة…</translation>
    </message>
</context>
<context>
    <name>StartMenu</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="99"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="543"/>
        <source>Auto Layout</source>
        <translation>تخطيط تلقائي</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="107"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="555"/>
        <source>Full Screen</source>
        <translation>ملء الشاشة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="113"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="568"/>
        <source>Add External Window</source>
        <translation>إضافة نافذة خارجية</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="119"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="740"/>
        <source>Console</source>
        <translation>وحدة التحكم</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="125"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="749"/>
        <source>Notifications</source>
        <translation>الإشعارات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="757"/>
        <source>Clock</source>
        <translation>ساعة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="141"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="764"/>
        <source>Stopwatch</source>
        <translation>ساعة إيقاف</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="149"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="772"/>
        <source>Preferences</source>
        <translation>التفضيلات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="155"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="853"/>
        <source>Help Center</source>
        <translation>مركز المساعدة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="781"/>
        <source>Sessions</source>
        <translation>الجلسات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="168"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="790"/>
        <source>File Transmission</source>
        <translation>نقل الملفات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="175"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="798"/>
        <source>AI Assistant</source>
        <translation>مساعد الذكاء الاصطناعي</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="276"/>
        <source>Workspaces</source>
        <translation>مساحات العمل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="343"/>
        <source>Show "%1"</source>
        <translation>إظهار "%1"</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="348"/>
        <source>Show All Hidden Workspaces</source>
        <translation>إظهار جميع مساحات العمل المخفية</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="357"/>
        <source>New Workspace…</source>
        <translation>مساحة عمل جديدة…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="372"/>
        <source>No Workspaces Available</source>
        <translation>لا توجد مساحات عمل متاحة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="406"/>
        <source>Actions</source>
        <translation>الإجراءات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="429"/>
        <source>No Actions Available</source>
        <translation>لا توجد إجراءات متاحة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="459"/>
        <source>Plugins</source>
        <translation>الإضافات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="497"/>
        <source>Manage Plugins…</source>
        <translation>إدارة الإضافات…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="507"/>
        <source>No Plugins Installed</source>
        <translation>لا توجد إضافات مثبتة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="588"/>
        <source>Export</source>
        <translation>تصدير</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="619"/>
        <source>CSV File</source>
        <translation>ملف CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="625"/>
        <source>MDF4 File</source>
        <translation>ملف MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="631"/>
        <source>Console Transcript</source>
        <translation>نسخة وحدة التحكم</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="640"/>
        <source>Session Database</source>
        <translation>قاعدة بيانات الجلسة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="654"/>
        <source>No Export Formats Available</source>
        <translation>لا توجد صيغ تصدير متاحة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="684"/>
        <source>Tools</source>
        <translation>أدوات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="813"/>
        <source>No Tools Available</source>
        <translation>لا توجد أدوات متاحة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="874"/>
        <source>Resume</source>
        <translation>استئناف</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="875"/>
        <source>Pause</source>
        <translation>إيقاف مؤقت</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="881"/>
        <source>Reset</source>
        <translation>إعادة تعيين</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="902"/>
        <source>Quit</source>
        <translation>إنهاء</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="902"/>
        <source>Disconnect</source>
        <translation>قطع الاتصال</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="928"/>
        <source>Edit…</source>
        <translation>تحرير…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="939"/>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="940"/>
        <source>Hide</source>
        <translation>إخفاء</translation>
    </message>
</context>
<context>
    <name>Stopwatch</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Stop</source>
        <translation>إيقاف</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Start</source>
        <translation>البداية</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="210"/>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="288"/>
        <source>Lap</source>
        <translation>لفة</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="226"/>
        <source>Reset</source>
        <translation>إعادة تعيين</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="279"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="297"/>
        <source>Total</source>
        <translation>الإجمالي</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="393"/>
        <source>No laps recorded</source>
        <translation>لا توجد لفات مسجلة</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="401"/>
        <source>Press Lap while the stopwatch is running</source>
        <translation>اضغط على لفة أثناء تشغيل ساعة الإيقاف</translation>
    </message>
</context>
<context>
    <name>SubMenuCombo</name>
    <message>
        <location filename="../../qml/Widgets/SubMenuCombo.qml" line="86"/>
        <source>No Data Available</source>
        <translation>لا توجد بيانات متاحة</translation>
    </message>
</context>
<context>
    <name>SystemDatasetsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="33"/>
        <source>Dataset Values</source>
        <translation>قيم مجموعة البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="158"/>
        <source>Search</source>
        <translation>بحث</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="179"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="180"/>
        <source>Group</source>
        <translation>المجموعة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="181"/>
        <source>Dataset</source>
        <translation>مجموعة البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="182"/>
        <source>Units</source>
        <translation>الوحدات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="252"/>
        <source>(virtual)</source>
        <translation>(افتراضي)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="298"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>نسخ رمز الوصول %1 إلى الحافظة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="323"/>
        <source>No datasets defined in this project.</source>
        <translation>لا توجد مجموعات بيانات معرّفة في هذا المشروع.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="324"/>
        <source>No datasets match your search.</source>
        <translation>لا توجد مجموعات بيانات تطابق البحث.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="374"/>
        <source>Dataset access code copied</source>
        <translation>تم نسخ رمز وصول مجموعة البيانات</translation>
    </message>
</context>
<context>
    <name>TableDelegate</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="130"/>
        <source>Parameter</source>
        <translation>المعامل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="151"/>
        <source>Value</source>
        <translation>القيمة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="514"/>
        <source>(Custom Icon)</source>
        <translation>(أيقونة مخصصة)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="639"/>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="645"/>
        <source>Auto</source>
        <translation>تلقائي</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="811"/>
        <source>No</source>
        <translation>لا</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="811"/>
        <source>Yes</source>
        <translation>نعم</translation>
    </message>
</context>
<context>
    <name>Taskbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="66"/>
        <source>Start Menu</source>
        <translation>قائمة البدء</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="194"/>
        <source>Menu</source>
        <translation>القائمة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="230"/>
        <source>Search…</source>
        <translation>بحث…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="496"/>
        <source>Settings</source>
        <translation>الإعدادات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="497"/>
        <source>Console</source>
        <translation>وحدة التحكم</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="498"/>
        <source>Notifications</source>
        <translation>الإشعارات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="499"/>
        <source>Clock</source>
        <translation>ساعة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="500"/>
        <source>Stopwatch</source>
        <translation>ساعة إيقاف</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="501"/>
        <source>File Transmission</source>
        <translation>نقل الملفات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="502"/>
        <source>AI Assistant</source>
        <translation>مساعد الذكاء الاصطناعي</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Resume</source>
        <translation>استئناف</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Pause</source>
        <translation>إيقاف مؤقت</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="949"/>
        <source>MQTT: Connected to %1</source>
        <translation>MQTT: متصل بـ %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="950"/>
        <source>MQTT: Not connected</source>
        <translation>MQTT: غير متصل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="974"/>
        <source>MQTT Publisher</source>
        <translation>ناشر MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="984"/>
        <source>Status:</source>
        <translation>الحالة:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="992"/>
        <source>Connected</source>
        <translation>متصل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="993"/>
        <source>Disconnected</source>
        <translation>غير متصل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1000"/>
        <source>Broker:</source>
        <translation>Broker:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1013"/>
        <source>Mode:</source>
        <translation>الوضع:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1026"/>
        <source>Messages sent:</source>
        <translation>الرسائل المُرسَلة:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1040"/>
        <source>Open MQTT Settings</source>
        <translation>فتح إعدادات MQTT</translation>
    </message>
</context>
<context>
    <name>Terminal</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="141"/>
        <source>Copy</source>
        <translation>نسخ</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="149"/>
        <source>Select all</source>
        <translation>تحديد الكل</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="155"/>
        <source>Clear</source>
        <translation>مسح</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="253"/>
        <source>Send Data to Device</source>
        <translation>إرسال البيانات إلى الجهاز</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="428"/>
        <source>Show Timestamp</source>
        <translation>إظهار الطابع الزمني</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="436"/>
        <source>Echo</source>
        <translation>صدى</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="453"/>
        <source>Emulate VT-100</source>
        <translation>محاكاة VT-100</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="466"/>
        <source>ANSI Colors</source>
        <translation>ألوان ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="486"/>
        <source>Display: %1</source>
        <translation>العرض: %1</translation>
    </message>
</context>
<context>
    <name>Tips</name>
    <message>
        <source>Did You Know?</source>
        <translation type="vanished">هل تعلم؟</translation>
    </message>
    <message>
        <source>Keep your firmware simple by sending raw data and letting Serial Studio parse it in JavaScript, Lua, or code-free Built-In templates.</source>
        <translation type="vanished">حافظ على بساطة البرنامج الثابت بإرسال البيانات الخام ودع Serial Studio يحللها بواسطة JavaScript أو Lua أو القوالب المدمجة بدون كود.</translation>
    </message>
    <message>
        <source>Give each channel its own function to calibrate, filter, or convert units. Offload the math to Serial Studio and keep your firmware lean.</source>
        <translation type="vanished">امنح كل قناة وظيفتها الخاصة للمعايرة أو الترشيح أو تحويل الوحدات. انقل العمليات الحسابية إلى Serial Studio واحتفظ ببرنامجك الثابت خفيفاً.</translation>
    </message>
    <message>
        <source>Need a value your device never sends? A virtual dataset computes its own channel, like power from voltage and current, plotted and logged as data.</source>
        <translation type="vanished">تحتاج قيمة لا يرسلها جهازك أبداً؟ مجموعة البيانات الافتراضية تحسب قناتها الخاصة، مثل الطاقة من الجهد والتيار، وتُرسم وتُسجل كبيانات.</translation>
    </message>
    <message>
        <source>Catch glitches like a bench scope. Time-axis plots have a sweep and trigger mode, and you can drag the trigger level right on the plot.</source>
        <translation type="vanished">التقط الأعطال مثل الأوسيلوسكوب المكتبي. تحتوي الرسوم البيانية لمحور الزمن على وضع المسح ووضع الزناد، ويمكنك سحب مستوى الزناد مباشرة على الرسم البياني.</translation>
    </message>
    <message>
        <source>Stop scrolling to find the right widget. Group them into your own workspaces and jump between them from the taskbar search.</source>
        <translation type="vanished">توقف عن التمرير للعثور على الأداة المناسبة. اجمعها في مساحات العمل الخاصة بك وانتقل بينها من بحث شريط المهام.</translation>
    </message>
    <message>
        <source>Never lose a test run again. Record sessions to a local database, then browse, tag, and replay them whenever you need them.</source>
        <translation type="vanished">لا تفقد أي تشغيل اختبار مرة أخرى. سجل الجلسات في قاعدة بيانات محلية، ثم تصفحها ووسمها وأعد تشغيلها متى احتجتها.</translation>
    </message>
    <message>
        <source>Hand a polished report to your team in seconds. Export any session to HTML or PDF, complete with charts and min/max/mean stats.</source>
        <translation type="vanished">سلّم تقريراً مصقولاً لفريقك في ثوانٍ. صدّر أي جلسة إلى HTML أو PDF، كاملة مع الرسوم البيانية وإحصائيات الحد الأدنى/الأقصى/المتوسط.</translation>
    </message>
    <message>
        <source>Close the loop without extra tooling. Output Controls let you send commands back to your device straight from the dashboard.</source>
        <translation type="vanished">أغلق الحلقة دون أدوات إضافية. تتيح لك عناصر التحكم في الإخراج إرسال الأوامر مباشرة إلى جهازك من لوحة المعلومات.</translation>
    </message>
    <message>
        <source>Build a visualization nobody else has. The Painter widget runs your own script to draw fully custom graphics from incoming data.</source>
        <translation type="vanished">ابنِ تصورًا لا يملكه أحد غيرك. تقوم عنصر الرسام بتشغيل البرنامج النصي الخاص بك لرسم رسومات مخصصة بالكامل من البيانات الواردة.</translation>
    </message>
    <message>
        <source>One tool for every link. Serial Studio reads from UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT, and Process I/O.</source>
        <translation type="vanished">أداة واحدة لكل رابط. يقرأ Serial Studio من UART وTCP/UDP وBluetooth LE وModbus وCAN Bus والصوت وUSB وHID وMQTT وProcess I/O.</translation>
    </message>
    <message>
        <source>Skip the terminal dance. Send and receive files over your serial link with the built-in XMODEM, YMODEM, and ZMODEM protocols.</source>
        <translation type="vanished">تخطَّ رقصة الطرفية. أرسل واستقبل الملفات عبر رابط التسلسل الخاص بك باستخدام بروتوكولات XMODEM وYMODEM وZMODEM المدمجة.</translation>
    </message>
    <message>
        <source>Already have a Modbus register map or a DBC file? Generate a ready-to-use project from it automatically instead of building one by hand.</source>
        <translation type="vanished">هل لديك بالفعل خريطة سجلات Modbus أو ملف DBC؟ أنشئ مشروعًا جاهزًا للاستخدام منه تلقائيًا بدلاً من بنائه يدويًا.</translation>
    </message>
    <message>
        <source>Describe what you want and let the AI Assistant build it. It can create and edit projects for you across eight model providers.</source>
        <translation type="vanished">صِف ما تريد ودع المساعد الذكي يبنيه. يمكنه إنشاء وتحرير المشاريع نيابة عنك عبر ثمانية مزودي نماذج.</translation>
    </message>
    <message>
        <source>Tip %1 of %2</source>
        <translation type="vanished">نصيحة %1 من %2</translation>
    </message>
    <message>
        <source>Learn More</source>
        <translation type="vanished">معرفة المزيد</translation>
    </message>
    <message>
        <source>Show Tips on Startup</source>
        <translation type="vanished">إظهار النصائح عند بدء التشغيل</translation>
    </message>
    <message>
        <source>Previous</source>
        <translation type="vanished">السابق</translation>
    </message>
    <message>
        <source>Next</source>
        <translation type="vanished">التالي</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">إغلاق</translation>
    </message>
</context>
<context>
    <name>ToolCallCard</name>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="47"/>
        <source>Awaiting approval</source>
        <translation>في انتظار الموافقة</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="48"/>
        <source>Done</source>
        <translation>تم</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="49"/>
        <source>Error</source>
        <translation>خطأ</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="50"/>
        <source>Denied</source>
        <translation>مرفوض</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="51"/>
        <source>Blocked</source>
        <translation>محظور</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="52"/>
        <source>Running</source>
        <translation>قيد التشغيل</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="152"/>
        <source>Approve</source>
        <translation>موافقة</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="158"/>
        <source>Deny</source>
        <translation>رفض</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="175"/>
        <source>Arguments</source>
        <translation>المعاملات</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="212"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="272"/>
        <source>Copy</source>
        <translation>نسخ</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="217"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="277"/>
        <source>Copy All</source>
        <translation>نسخ الكل</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="225"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="285"/>
        <source>Select All</source>
        <translation>تحديد الكل</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="233"/>
        <source>Result</source>
        <translation>النتيجة</translation>
    </message>
</context>
<context>
    <name>Toolbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="197"/>
        <source>Project Editor</source>
        <translation>محرر المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="200"/>
        <source>Open the Project Editor to create or modify your JSON layout</source>
        <translation>افتح محرر المشروع لإنشاء أو تعديل تخطيط JSON الخاص بك</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="214"/>
        <source>Open Project</source>
        <translation>فتح مشروع</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="217"/>
        <source>Open an existing JSON project</source>
        <translation>فتح مشروع JSON موجود</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="226"/>
        <source>Open CSV</source>
        <translation>فتح CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="232"/>
        <source>Play a CSV file as if it were live sensor data</source>
        <translation>تشغيل ملف CSV كما لو كان بيانات حساس مباشرة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="238"/>
        <source>Open MDF4</source>
        <translation>فتح MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="243"/>
        <source>Play an MDF4 file as if it were live sensor data (Pro)</source>
        <translation>تشغيل ملف MDF4 كما لو كان بيانات حساس مباشرة (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <source>Assistant</source>
        <translation>المساعد</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="302"/>
        <source>Extensions</source>
        <translation>الإضافات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="264"/>
        <source>Chat with an AI to build and edit your project</source>
        <translation>محادثة مع ذكاء اصطناعي لبناء وتحرير المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="265"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="305"/>
        <source>Browse and install extensions</source>
        <translation>استعراض وتثبيت الإضافات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="281"/>
        <source>Deploy</source>
        <translation>النشر</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="286"/>
        <source>Build an operator app for the current project</source>
        <translation>بناء تطبيق تشغيل للمشروع الحالي</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="292"/>
        <source>Sessions</source>
        <translation>الجلسات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="296"/>
        <source>Browse, replay, and export recorded sessions</source>
        <translation>استعراض وإعادة تشغيل وتصدير الجلسات المسجلة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="319"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="323"/>
        <source>Preferences</source>
        <translation>التفضيلات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="327"/>
        <source>Open application settings and preferences</source>
        <translation>فتح إعدادات وتفضيلات التطبيق</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="345"/>
        <source>UART</source>
        <translation>UART</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="351"/>
        <source>Select Serial port (UART) communication</source>
        <translation>تحديد اتصال المنفذ التسلسلي (UART)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="362"/>
        <source>Audio</source>
        <translation>الصوت</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="366"/>
        <source>Select audio input device (Pro)</source>
        <translation>تحديد جهاز إدخال الصوت (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="381"/>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="386"/>
        <source>Select raw USB communication (Pro)</source>
        <translation>تحديد اتصال USB الخام (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="395"/>
        <source>Network</source>
        <translation>الشبكة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="400"/>
        <source>Select TCP/UDP network communication</source>
        <translation>اختيار اتصال شبكة TCP/UDP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="412"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="417"/>
        <source>Select MODBUS communication (Pro)</source>
        <translation>اختيار اتصال MODBUS ‏(Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="431"/>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="436"/>
        <source>Select HID device communication (Pro)</source>
        <translation>اختيار اتصال جهاز HID ‏(Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="446"/>
        <source>Bluetooth</source>
        <translation>Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="450"/>
        <source>Select Bluetooth Low Energy communication</source>
        <translation>اختيار اتصال Bluetooth Low Energy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="462"/>
        <source>CAN Bus</source>
        <translation>ناقل CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="467"/>
        <source>Select CAN Bus communication (Pro)</source>
        <translation>اختيار اتصال ناقل CAN ‏(Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="481"/>
        <source>Process</source>
        <translation>العملية</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="486"/>
        <source>Select process pipe communication (Pro)</source>
        <translation>اختيار اتصال أنبوب العملية (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="502"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="506"/>
        <source>About</source>
        <translation>حول</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="510"/>
        <source>Show application info and license details</source>
        <translation>عرض معلومات التطبيق وتفاصيل الترخيص</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="522"/>
        <source>Examples</source>
        <translation>الأمثلة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="525"/>
        <source>Browse example projects</source>
        <translation>استعراض المشاريع النموذجية</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="533"/>
        <source>Help Center</source>
        <translation>مركز المساعدة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="537"/>
        <source>Browse documentation, FAQ, and wiki</source>
        <translation>استعراض الوثائق والأسئلة الشائعة والويكي</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="543"/>
        <source>AI Wiki &amp; Chat</source>
        <translation>ويكي ودردشة AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="546"/>
        <source>View detailed documentation and ask questions on DeepWiki</source>
        <translation>عرض الوثائق التفصيلية وطرح الأسئلة على DeepWiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="586"/>
        <source>Activate</source>
        <translation>تفعيل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="590"/>
        <source>Manage license and activate Serial Studio Pro</source>
        <translation>إدارة الترخيص وتفعيل Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="627"/>
        <source>Disconnect</source>
        <translation>قطع الاتصال</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <source>Connect</source>
        <translation>اتصال</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="607"/>
        <source>Connect or disconnect from the configured device</source>
        <translation>الاتصال أو قطع الاتصال بالجهاز المُهيأ</translation>
    </message>
</context>
<context>
    <name>TriggerDialog</name>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="50"/>
        <source>Trigger Settings</source>
        <translation>إعدادات الزناد</translation>
    </message>
    <message>
        <source>Hold the waveform stationary by aligning each sweep to a trigger event.</source>
        <translation type="vanished">تثبيت الموجة عن طريق محاذاة كل مسح مع حدث الزناد.</translation>
    </message>
    <message>
        <source>Lock a repeating signal in place, like the Auto button on an oscilloscope. Each sweep starts at the same point on the waveform, so it holds still instead of scrolling past.</source>
        <translation type="vanished">قفل إشارة متكررة في مكانها، مثل زر Auto في راسم الذبذبات. تبدأ كل مسحة عند نفس النقطة على الموجة، لذا تبقى ثابتة بدلاً من التمرير.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="173"/>
        <source>Trigger</source>
        <translation>المشغل</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="vanished">الوضع:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="110"/>
        <source>Mode</source>
        <translation>الوضع</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Auto</source>
        <translation>تلقائي</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Normal</source>
        <translation>عادي</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Single</source>
        <translation>مفرد</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="158"/>
        <source>Auto free-runs when nothing crosses the level.</source>
        <translation>يعمل التلقائي بحرية عندما لا يعبر شيء المستوى.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="159"/>
        <source>Normal waits for a crossing.</source>
        <translation>ينتظر العادي حدوث عبور.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="160"/>
        <source>Single captures one sweep, then stops.</source>
        <translation>يلتقط الفردي مسحة واحدة، ثم يتوقف.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="238"/>
        <source>Slope:</source>
        <translation>الميل:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="270"/>
        <source>Trigger on a downward crossing</source>
        <translation>التشغيل عند عبور هابط</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="316"/>
        <source>Timebase:</source>
        <translation>القاعدة الزمنية:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="381"/>
        <source>Leave timebase empty to use the plot's time range; lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each.</source>
        <translation>اترك القاعدة الزمنية فارغة لاستخدام النطاق الزمني للرسم؛ اخفضها للتكبير على إشارة سريعة. يتجاهل Holdoff المشغلات الجديدة للحظة بعد كل واحدة.</translation>
    </message>
    <message>
        <source>Signal:</source>
        <translation type="vanished">الإشارة:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="230"/>
        <source>Value to cross</source>
        <translation>القيمة المراد عبورها</translation>
    </message>
    <message>
        <source>Edge:</source>
        <translation type="vanished">الحافة:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="251"/>
        <source>Rising</source>
        <translation>صاعد</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="255"/>
        <source>Trigger on an upward crossing</source>
        <translation>التشغيل عند عبور صاعد</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="266"/>
        <source>Falling</source>
        <translation>هابط</translation>
    </message>
    <message>
        <source>A new sweep begins each time the signal crosses the level in the chosen direction. Auto also free-runs when no crossing is found.</source>
        <translation type="vanished">تبدأ مسحة جديدة في كل مرة تعبر فيها الإشارة المستوى في الاتجاه المختار. يعمل Auto أيضاً بحرية عندما لا يتم العثور على عبور.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="289"/>
        <source>Timing</source>
        <translation>التوقيت</translation>
    </message>
    <message>
        <source>Timebase (ms):</source>
        <translation type="vanished">القاعدة الزمنية (ميلي ثانية):</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="329"/>
        <source>Match time range</source>
        <translation>مطابقة النطاق الزمني</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="341"/>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="368"/>
        <source>ms</source>
        <translation>ميلي ثانية</translation>
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
        <translation type="vanished">تحدد القاعدة الزمنية مقدار الوقت الذي تعرضه مسحة واحدة؛ اتركها فارغة لاستخدام النطاق الزمني للرسم. اخفضها للتكبير على إشارة سريعة. يتجاهل Holdoff المشغلات الجديدة للحظة بعد كل واحدة.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="396"/>
        <source>Capture Next</source>
        <translation>التقاط التالي</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="398"/>
        <source>Arm for one more single-shot capture</source>
        <translation>التسليح لالتقاط لقطة واحدة إضافية</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="217"/>
        <source>Level:</source>
        <translation>المستوى:</translation>
    </message>
    <message>
        <source>Trigger level</source>
        <translation type="vanished">مستوى التشغيل</translation>
    </message>
    <message>
        <source>Holdoff (ms):</source>
        <translation type="vanished">وقت الانتظار (ميلي ثانية):</translation>
    </message>
    <message>
        <source>Holdoff time</source>
        <translation type="vanished">مدة الانتظار</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="197"/>
        <source>Source:</source>
        <translation>المصدر:</translation>
    </message>
    <message>
        <source>Re-arm</source>
        <translation type="vanished">إعادة التسليح</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="411"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
</context>
<context>
    <name>UART</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="52"/>
        <source>COM Port</source>
        <translation>منفذ COM</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="81"/>
        <source>Baud Rate</source>
        <translation>معدل البود</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="174"/>
        <source>Data Bits</source>
        <translation>بتات البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="195"/>
        <source>Parity</source>
        <translation>التماثل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="216"/>
        <source>Stop Bits</source>
        <translation>بتات الإيقاف</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="237"/>
        <source>Flow Control</source>
        <translation>التحكم في التدفق</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="269"/>
        <source>Auto Reconnect</source>
        <translation>إعادة الاتصال التلقائي</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="287"/>
        <source>Send DTR Signal</source>
        <translation>إرسال إشارة DTR</translation>
    </message>
</context>
<context>
    <name>UI::AlarmMonitor</name>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="195"/>
        <source>Alarm</source>
        <translation>إنذار</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>critical</source>
        <translation>حرج</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>warning</source>
        <translation>تحذير</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="200"/>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation>دخلت القيمة %1%2 نطاق %3 (%4–%5).</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="205"/>
        <source>Alarms</source>
        <translation>الإنذارات</translation>
    </message>
</context>
<context>
    <name>UI::Dashboard</name>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1620"/>
        <source>Console</source>
        <translation>وحدة التحكم</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1628"/>
        <source>Notifications</source>
        <translation>الإشعارات</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1636"/>
        <source>Clock</source>
        <translation>ساعة</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1643"/>
        <source>Stopwatch</source>
        <translation>ساعة إيقاف</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1689"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1704"/>
        <source>%1 (Fallback)</source>
        <translation>%1 (احتياطي)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1726"/>
        <source>LED Panel (%1)</source>
        <translation>لوحة LED ‏(%1)</translation>
    </message>
</context>
<context>
    <name>UI::DashboardWidget</name>
    <message>
        <location filename="../../src/UI/DashboardWidget.cpp" line="148"/>
        <source>Invalid</source>
        <translation>غير صالح</translation>
    </message>
</context>
<context>
    <name>UI::WindowManager</name>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1057"/>
        <source>Select Background Image</source>
        <translation>تحديد صورة الخلفية</translation>
    </message>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1059"/>
        <source>Images (*.png *.jpg *.jpeg *.bmp)</source>
        <translation>الصور (*.png *.jpg *.jpeg *.bmp)</translation>
    </message>
</context>
<context>
    <name>USB</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="50"/>
        <source>USB Device</source>
        <translation>جهاز USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="80"/>
        <source>Transfer Mode</source>
        <translation>وضع النقل</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="89"/>
        <source>Bulk Stream</source>
        <translation>تدفق مجمّع</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="90"/>
        <source>Advanced (Bulk + Control)</source>
        <translation>متقدم (مجمّع + تحكم)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="91"/>
        <source>Isochronous</source>
        <translation>متزامن</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="143"/>
        <source>Connect to USB devices using bulk, control, or isochronous transfers. Suitable for data loggers, custom firmware devices, and USB instruments.</source>
        <translation>الاتصال بأجهزة USB باستخدام عمليات نقل مجمّعة أو تحكم أو متزامنة. مناسب لمسجلات البيانات وأجهزة البرامج الثابتة المخصصة وأدوات USB.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="152"/>
        <source>USB specifications (USB.org)</source>
        <translation>مواصفات USB (USB.org)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="169"/>
        <source>IN Endpoint</source>
        <translation>نقطة نهاية الإدخال</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="205"/>
        <source>OUT Endpoint</source>
        <translation>نقطة نهاية الإخراج</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="241"/>
        <source>Max Packet Size</source>
        <translation>الحد الأقصى لحجم الحزمة</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="301"/>
        <source>Control Transfers Enabled</source>
        <translation>تمكين نقل التحكم</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="310"/>
        <source>Sending incorrect control requests may crash or damage connected hardware. Use with caution.</source>
        <translation>إرسال طلبات تحكم غير صحيحة قد يتسبب في تعطل أو تلف الأجهزة المتصلة. استخدم بحذر.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="317"/>
        <source>Learn about USB control transfers</source>
        <translation>تعرّف على نقل التحكم USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="351"/>
        <source>Packet size should match the maximum transfer size reported by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).</source>
        <translation>يجب أن يتطابق حجم الحزمة مع الحد الأقصى لحجم النقل المُبلّغ عنه من قبل نقطة النهاية. القيم النموذجية: 192 بايت (صوت FS)، 1024 بايت (HS).</translation>
    </message>
</context>
<context>
    <name>Updater</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="477"/>
        <source>Would you like to download the update now?</source>
        <translation>هل تريد تنزيل التحديث الآن؟</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="479"/>
        <source>Would you like to download the update now?&lt;br /&gt;This is a mandatory update, exiting now will close the application.</source>
        <translation>هل تريد تنزيل التحديث الآن؟&lt;br /&gt;هذا تحديث إلزامي، الخروج الآن سيغلق التطبيق.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="485"/>
        <source>&lt;strong&gt;Change log:&lt;/strong&gt;&lt;br/&gt;%1</source>
        <translation>&lt;strong&gt;سجل التغييرات:&lt;/strong&gt;&lt;br/&gt;%1</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="488"/>
        <source>Version %1 of %2 has been released!</source>
        <translation>تم إصدار الإصدار %1 من %2!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="520"/>
        <source>No updates are available for the moment</source>
        <translation>لا توجد تحديثات متاحة في الوقت الحالي</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="522"/>
        <source>Congratulations! You are running the latest version of %1</source>
        <translation>تهانينا! أنت تستخدم أحدث إصدار من %1</translation>
    </message>
</context>
<context>
    <name>UserTableView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="166"/>
        <source>Add Register</source>
        <translation>إضافة سجل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="169"/>
        <source>Add register</source>
        <translation>إضافة سجل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="176"/>
        <source>Insert Constant</source>
        <translation>إدراج ثابت</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="179"/>
        <source>Insert constant</source>
        <translation>إدراج ثابت</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="186"/>
        <source>Import</source>
        <translation>استيراد</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="189"/>
        <source>Import registers from CSV</source>
        <translation>استيراد السجلات من CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="196"/>
        <source>Export</source>
        <translation>تصدير</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="199"/>
        <source>Export registers to CSV</source>
        <translation>تصدير السجلات إلى CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="211"/>
        <source>Rename</source>
        <translation>إعادة تسمية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="214"/>
        <source>Rename table</source>
        <translation>إعادة تسمية الجدول</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="221"/>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="224"/>
        <source>Delete table</source>
        <translation>حذف الجدول</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="238"/>
        <source>Help</source>
        <translation>مساعدة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="243"/>
        <source>Open help documentation for shared memory</source>
        <translation>فتح وثائق المساعدة للذاكرة المشتركة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="283"/>
        <source>Permissions</source>
        <translation>الأذونات</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="284"/>
        <source>Register Name</source>
        <translation>اسم السجل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="285"/>
        <source>Default Value</source>
        <translation>القيمة الافتراضية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="324"/>
        <source>Read-Only</source>
        <translation>للقراءة فقط</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="324"/>
        <source>Read/Write</source>
        <translation>قراءة/كتابة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="462"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>نسخ رمز الوصول %1 إلى الحافظة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="495"/>
        <source>Delete register</source>
        <translation>حذف السجل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="512"/>
        <source>No registers.</source>
        <translation>لا توجد سجلات.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="562"/>
        <source>Register access code copied</source>
        <translation>تم نسخ كود الوصول للسجل</translation>
    </message>
</context>
<context>
    <name>Waterfall</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="232"/>
        <source>Show Colorbar</source>
        <translation>إظهار شريط الألوان</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="245"/>
        <source>Show Axes &amp; Grid</source>
        <translation>إظهار المحاور والشبكة</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="258"/>
        <source>Show Crosshair</source>
        <translation>إظهار الشعيرات المتقاطعة</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="272"/>
        <source>Pause</source>
        <translation>إيقاف مؤقت</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="272"/>
        <source>Resume</source>
        <translation>استئناف</translation>
    </message>
</context>
<context>
    <name>Welcome</name>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="177"/>
        <source>Welcome to %1!</source>
        <translation>مرحباً بك في %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="188"/>
        <source>Serial Studio is a powerful real-time visualization tool, built for engineers, students, and makers.</source>
        <translation>Serial Studio أداة تصور قوية في الوقت الفعلي، مصممة للمهندسين والطلاب والصناع.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="199"/>
        <source>You can start a fully-functional 14-day trial, activate it with your license key, or download and compile the GPLv3 source code yourself.</source>
        <translation>يمكن بدء فترة تجريبية كاملة الوظائف لمدة 14 يومًا، أو تفعيلها بمفتاح الترخيص، أو تنزيل كود المصدر GPLv3 وترجمته يدويًا.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="209"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="394"/>
        <source>Buying Pro supports the author directly and helps fund future development.</source>
        <translation>شراء Pro يدعم المطور مباشرة ويساعد في تمويل التطوير المستقبلي.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="217"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="402"/>
        <source>Building the GPLv3 version yourself helps grow the community and encourages technical contributions.</source>
        <translation>بناء إصدار GPLv3 يدويًا يساعد في نمو المجتمع ويشجع المساهمات التقنية.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="239"/>
        <source>Please wait…</source>
        <translation>يُرجى الانتظار…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="275"/>
        <source>%1 days remaining in your trial.</source>
        <translation>%1 يوم متبقي في الفترة التجريبية.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="285"/>
        <source>You're currently using the fully-featured trial of %1 Pro. It's valid for 14 days of personal, non-commercial use.</source>
        <translation>تستخدم حاليًا النسخة التجريبية كاملة المزايا من %1 Pro. صالحة لمدة 14 يومًا للاستخدام الشخصي غير التجاري.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="296"/>
        <source>Upgrade to a paid plan to keep using Serial Studio Pro.</source>
        <translation>الترقية إلى خطة مدفوعة لمواصلة استخدام Serial Studio Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="304"/>
        <source>Or, compile the GPLv3 source code to use it for free.</source>
        <translation>أو ترجمة كود المصدر GPLv3 لاستخدامه مجانًا.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="312"/>
        <source>To see available subscription plans, click "Upgrade Now" below.</source>
        <translation>لعرض خطط الاشتراك المتاحة، انقر "الترقية الآن" أدناه.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="333"/>
        <source>Don't nag me about the trial.
I understand that when it ends, I'll need to buy a license or build the GPLv3 version.</source>
        <translation>لا تذكرني بالفترة التجريبية.
أدرك أنه عند انتهائها، سأحتاج إلى شراء ترخيص أو بناء إصدار GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="364"/>
        <source>Your %1 trial has expired.</source>
        <translation>انتهت صلاحية النسخة التجريبية من %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="374"/>
        <source>Your trial period has ended. To continue using %1 with all Pro features, please upgrade to a paid plan.</source>
        <translation>انتهت فترة التجربة. لمواصلة استخدام %1 بجميع ميزات Pro، يُرجى الترقية إلى خطة مدفوعة.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="385"/>
        <source>If you prefer, you can also compile the open-source version under the GPLv3 license.</source>
        <translation>يمكنك أيضًا تجميع الإصدار مفتوح المصدر بموجب ترخيص GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="413"/>
        <source>Thank you for trying %1!</source>
        <translation>شكرًا لتجربة %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="455"/>
        <source>Upgrade Now</source>
        <translation>الترقية الآن</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="464"/>
        <source>Activate</source>
        <translation>تفعيل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Open in Limited Mode</source>
        <translation>فتح في الوضع المحدود</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Continue</source>
        <translation>متابعة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Start Trial</source>
        <translation>بدء التجربة</translation>
    </message>
</context>
<context>
    <name>WhatsNew</name>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="31"/>
        <source>What's New in %1</source>
        <translation>الجديد في %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="58"/>
        <source>Lua &amp; Built-In Parsers</source>
        <translation>Lua والمحللات المدمجة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="59"/>
        <source>Parse frames in Lua 5.4 or with code-free Built-In templates, alongside JavaScript.</source>
        <translation>تحليل الإطارات في Lua 5.4 أو باستخدام قوالب مدمجة بدون كود، إلى جانب JavaScript.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="67"/>
        <source>AI Assistant</source>
        <translation>مساعد الذكاء الاصطناعي</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="68"/>
        <source>An in-app assistant across eight providers that can build and edit projects for you.</source>
        <translation>مساعد داخل التطبيق عبر ثمانية مزودين يمكنه بناء وتحرير المشاريع نيابة عنك.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="76"/>
        <source>Oscilloscope Sweep &amp; Trigger</source>
        <translation>مسح ومشغل الأوسيلوسكوب</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="77"/>
        <source>Scope-style sweep with an animated trigger cursor you can drag on the plot.</source>
        <translation>مسح بأسلوب الأوسيلوسكوب مع مؤشر زناد متحرك يمكنك سحبه على الرسم البياني.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="85"/>
        <source>Output Controls</source>
        <translation>عناصر التحكم في الإخراج</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="86"/>
        <source>Transmit back to your device with control widgets and a protocol-helper engine.</source>
        <translation>أرسل البيانات إلى جهازك باستخدام عناصر التحكم ومحرك مساعد البروتوكول.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="94"/>
        <source>Dashboard Workspaces</source>
        <translation>مساحات عمل لوحة المعلومات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="95"/>
        <source>Group widgets into your own dashboards and find them from the taskbar search.</source>
        <translation>جمّع العناصر في لوحات معلومات خاصة بك وابحث عنها من شريط المهام.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="103"/>
        <source>Session Database &amp; Reports</source>
        <translation>قاعدة بيانات الجلسة والتقارير</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="104"/>
        <source>Record sessions to SQLite, replay them, and export HTML or PDF reports.</source>
        <translation>سجّل الجلسات في SQLITE وأعد تشغيلها وصدّر تقارير HTML أو PDF.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="112"/>
        <source>Operator Deployments</source>
        <translation>عمليات نشر المشغّل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="113"/>
        <source>Ship a locked-down, single-purpose build to operators with a runtime-only mode.</source>
        <translation>وزّع نسخة محدودة الوظائف للمشغّلين بوضع تشغيل فقط.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="121"/>
        <source>New Dashboard Widgets</source>
        <translation>عناصر جديدة للوحة المعلومات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="122"/>
        <source>Gauge and Meter faces with live readouts, plus Clock, Stopwatch, and Waterfall.</source>
        <translation>واجهات مقياس وعداد مع قراءات مباشرة، بالإضافة إلى ساعة وساعة إيقاف وشلال.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="130"/>
        <source>Dataset Transforms</source>
        <translation>تحويلات مجموعة البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="131"/>
        <source>Calibrate, filter, and convert each channel with a per-dataset function, or add virtual datasets that compute new channels.</source>
        <translation>معايرة وتصفية وتحويل كل قناة باستخدام دالة لكل مجموعة بيانات، أو إضافة مجموعات بيانات افتراضية تحسب قنوات جديدة.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="139"/>
        <source>Painter Widget</source>
        <translation>أداة الرسّام</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="140"/>
        <source>Draw fully custom graphics from incoming data with your own drawing script.</source>
        <translation>ارسم رسومات مخصصة بالكامل من البيانات الواردة باستخدام نص الرسم الخاص بك.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="148"/>
        <source>Data Tables</source>
        <translation>جداول البيانات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="149"/>
        <source>Live register-style tables with virtual datasets and editable cells.</source>
        <translation>جداول مباشرة بنمط السجلات مع مجموعات بيانات افتراضية وخلايا قابلة للتحرير.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="157"/>
        <source>Image Widget</source>
        <translation>أداة الصور</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="158"/>
        <source>Decode and display image frames streamed straight from your device.</source>
        <translation>فك تشفير وعرض إطارات الصور المنقولة مباشرة من جهازك.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="166"/>
        <source>Notifications &amp; Alarms</source>
        <translation>الإشعارات والتنبيهات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="167"/>
        <source>Multi-band dataset alarms with severity tiers, routed to the Notification Center.</source>
        <translation>تنبيهات مجموعات البيانات متعددة النطاقات مع مستويات خطورة، موجهة إلى مركز الإشعارات.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="175"/>
        <source>Project Lock</source>
        <translation>قفل المشروع</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="176"/>
        <source>Lock a project to separate operator use from editing, with an access code.</source>
        <translation>قفل المشروع لفصل استخدام المشغل عن التحرير، باستخدام رمز وصول.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="184"/>
        <source>MQTT, Protobuf &amp; File Transfer</source>
        <translation>MQTT وProtobuf ونقل الملفات</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="185"/>
        <source>MQTT input and publishing, Protobuf import, and XMODEM / YMODEM / ZMODEM transfers.</source>
        <translation>إدخال ونشر MQTT، واستيراد Protobuf، وعمليات نقل XMODEM / YMODEM / ZMODEM.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="231"/>
        <source>Welcome to %1!</source>
        <translation>مرحباً بك في %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="241"/>
        <source>Here's what's new in version %1.</source>
        <translation>إليك الجديد في الإصدار %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="414"/>
        <source>Show on Startup</source>
        <translation>إظهار عند بدء التشغيل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="421"/>
        <source>Close</source>
        <translation>إغلاق</translation>
    </message>
</context>
<context>
    <name>WidgetDelegate</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml" line="338"/>
        <source>Device Disconnected</source>
        <translation>الجهاز غير متصل</translation>
    </message>
</context>
<context>
    <name>Widgets::Bar</name>
    <message>
        <source>Alarm</source>
        <translation type="vanished">إنذار</translation>
    </message>
    <message>
        <source>critical</source>
        <translation type="vanished">حرج</translation>
    </message>
    <message>
        <source>warning</source>
        <translation type="vanished">تحذير</translation>
    </message>
    <message>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation type="vanished">القيمة %1%2 دخلت النطاق %3 (%4–%5).</translation>
    </message>
    <message>
        <source>Value %1%2 reached the high alarm %3%2</source>
        <translation type="vanished">القيمة %1%2 بلغت حد الإنذار العلوي %3%2</translation>
    </message>
    <message>
        <source>Value %1%2 reached the low alarm %3%2</source>
        <translation type="vanished">القيمة %1%2 بلغت حد الإنذار السفلي %3%2</translation>
    </message>
    <message>
        <source>Alarms</source>
        <translation type="vanished">الإنذارات</translation>
    </message>
</context>
<context>
    <name>Widgets::Compass</name>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="157"/>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="180"/>
        <source>N</source>
        <translation>ش</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="160"/>
        <source>NE</source>
        <translation>ش ق</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="163"/>
        <source>E</source>
        <translation>ق</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="166"/>
        <source>SE</source>
        <translation>ج ق</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="169"/>
        <source>S</source>
        <translation>ج</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="172"/>
        <source>SW</source>
        <translation>ج غ</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="175"/>
        <source>W</source>
        <translation>غ</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="178"/>
        <source>NW</source>
        <translation>ش غ</translation>
    </message>
</context>
<context>
    <name>Widgets::DataGrid</name>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="132"/>
        <source>Title</source>
        <translation>العنوان</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="133"/>
        <source>Value</source>
        <translation>القيمة</translation>
    </message>
    <message>
        <source>Awaiting data…</source>
        <translation type="vanished">في انتظار البيانات…</translation>
    </message>
</context>
<context>
    <name>Widgets::GPS</name>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery</source>
        <translation>صور الأقمار الصناعية</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery with Labels</source>
        <translation>صور الأقمار الصناعية مع التسميات</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Street Map</source>
        <translation>خريطة الشوارع</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Topographic Map</source>
        <translation>خريطة طبوغرافية</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Terrain</source>
        <translation>التضاريس</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Light Gray Canvas</source>
        <translation>لوحة رمادية فاتحة</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>Dark Gray Canvas</source>
        <translation>لوحة رمادية داكنة</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>National Geographic</source>
        <translation>National Geographic</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="373"/>
        <source>Additional map layers are available only for Pro users.</source>
        <translation>طبقات الخرائط الإضافية متاحة فقط لمستخدمي Pro.</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="374"/>
        <source>We can't offer unrestricted access because the ArcGIS API key incurs real costs.</source>
        <translation>لا يمكننا تقديم وصول غير مقيد لأن مفتاح API الخاص بـ ArcGIS يتكبد تكاليف فعلية.</translation>
    </message>
</context>
<context>
    <name>Widgets::MultiPlot</name>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="101"/>
        <source>Time (s)</source>
        <translation>الوقت (ث)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="101"/>
        <source>Samples</source>
        <translation>العينات</translation>
    </message>
</context>
<context>
    <name>Widgets::Output::Base</name>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="168"/>
        <source>Transmit script timed out after %1 ms</source>
        <translation>انتهت مهلة نص الإرسال بعد %1 مللي ثانية</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="184"/>
        <source>Payload exceeds maximum size</source>
        <translation>الحمولة تتجاوز الحجم الأقصى</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="86"/>
        <source>Time (s)</source>
        <translation>الوقت (ث)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="112"/>
        <source>Samples</source>
        <translation>العينات</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot3D</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot3D.cpp" line="959"/>
        <source>Grid Interval: %1 unit(s)</source>
        <translation>فاصل الشبكة: %1 وحدة</translation>
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
        <translation>نفاث</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="407"/>
        <source>Hot</source>
        <translation>ساخن</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="409"/>
        <source>Grayscale</source>
        <translation>تدرج رمادي</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="411"/>
        <source>Unknown</source>
        <translation>غير معروف</translation>
    </message>
</context>
<context>
    <name>WorkspaceDialog</name>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="51"/>
        <source>Edit Workspace</source>
        <translation>تحرير مساحة العمل</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="52"/>
        <source>New Workspace</source>
        <translation>مساحة عمل جديدة</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="157"/>
        <source>Name:</source>
        <translation>الاسم:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="166"/>
        <source>My Workspace</source>
        <translation>مساحة العمل الخاصة بي</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="181"/>
        <source>Select widgets to include:</source>
        <translation>اختر الودجات المراد تضمينها:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="187"/>
        <source>Filter widgets…</source>
        <translation>تصفية الودجات…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="302"/>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="309"/>
        <source>OK</source>
        <translation>موافق</translation>
    </message>
</context>
<context>
    <name>WorkspaceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="41"/>
        <source>Workspace</source>
        <translation>مساحة العمل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="129"/>
        <source>Add Widget</source>
        <translation>إضافة عنصر واجهة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="131"/>
        <source>Add widget to workspace</source>
        <translation>إضافة عنصر واجهة إلى مساحة العمل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="142"/>
        <source>Rename</source>
        <translation>إعادة تسمية</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="144"/>
        <source>Rename workspace</source>
        <translation>إعادة تسمية مساحة العمل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="153"/>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="155"/>
        <source>Delete workspace</source>
        <translation>حذف مساحة العمل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="177"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="183"/>
        <source>Group</source>
        <translation>مجموعة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="178"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="184"/>
        <source>Widget</source>
        <translation>عنصر واجهة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="179"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="185"/>
        <source>Type</source>
        <translation>النوع</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="229"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="267"/>
        <source>(unknown)</source>
        <translation>(غير معروف)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="247"/>
        <source>(group widget)</source>
        <translation>(عنصر واجهة مجموعة)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="297"/>
        <source>Remove widget from workspace</source>
        <translation>إزالة عنصر الواجهة من مساحة العمل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="317"/>
        <source>No widgets in this workspace.</source>
        <translation>لا توجد عناصر واجهة في مساحة العمل هذه.</translation>
    </message>
</context>
<context>
    <name>WorkspacesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="33"/>
        <source>Workspaces</source>
        <translation>مساحات العمل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="127"/>
        <source>Customize</source>
        <translation>تخصيص</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="129"/>
        <source>Edit workspaces manually</source>
        <translation>تحرير مساحات العمل يدويًا</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="145"/>
        <source>Move Up</source>
        <translation>نقل لأعلى</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="147"/>
        <source>Move the selected workspace up</source>
        <translation>نقل مساحة العمل المحددة لأعلى</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="158"/>
        <source>Move Down</source>
        <translation>نقل لأسفل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="160"/>
        <source>Move the selected workspace down</source>
        <translation>نقل مساحة العمل المحددة لأسفل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="171"/>
        <source>Add Workspace</source>
        <translation>إضافة مساحة عمل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="173"/>
        <source>Add workspace</source>
        <translation>إضافة مساحة عمل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="182"/>
        <source>Cleanup</source>
        <translation>تنظيف</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="185"/>
        <source>Remove %1 widget reference(s) whose target group or dataset no longer exists</source>
        <translation>إزالة %L1 مرجع(مراجع) عنصر واجهة لم تعد مجموعتها أو مجموعة بياناتها المستهدفة موجودة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="188"/>
        <source>No stale widget references in any workspace</source>
        <translation>لا توجد مراجع عناصر واجهة قديمة في أي مساحة عمل</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="203"/>
        <source>Title</source>
        <translation>العنوان</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="204"/>
        <source>Widgets</source>
        <translation>عناصر الواجهة</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="276"/>
        <source>No workspaces. Add one with the toolbar above, or reset to the auto layout.</source>
        <translation>لا توجد مساحات عمل. أضف واحدة باستخدام شريط الأدوات أعلاه، أو أعد التعيين إلى التخطيط التلقائي.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="278"/>
        <source>Project has no eligible groups -- add a group with widgets to populate workspaces.</source>
        <translation>المشروع لا يحتوي على مجموعات مؤهلة -- أضف مجموعة تحتوي على عناصر واجهة لملء مساحات العمل.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="284"/>
        <source>Reset to Auto Layout</source>
        <translation>إعادة التعيين إلى التخطيط التلقائي</translation>
    </message>
</context>
</TS>