<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="sv_SE" sourcelanguage="en_US">
<context>
    <name>AI::AnthropicReply</name>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="164"/>
        <source>Anthropic error</source>
        <translation>Anthropic-fel</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="310"/>
        <source>Stream parse error: %1</source>
        <translation>Fel vid tolkning av ström: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="359"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="362"/>
        <source>Invalid API key (%1)</source>
        <translation>Ogiltig API-nyckel (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="364"/>
        <source>Rate limited: %1</source>
        <translation>Hastighetsbegränsad: %1</translation>
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
        <translation>Tillåt AI-enhetskontroll?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="192"/>
        <source>This lets the AI assistant configure devices, open and close connections, and send data to your hardware.

Every device action still requires your explicit per-call approval in the chat, even when auto-approve is enabled. Only enable this if you trust the configured AI provider with hardware access.</source>
        <translation>Detta låter AI-assistenten konfigurera enheter, öppna och stänga anslutningar samt skicka data till din hårdvara.

Varje enhetsåtgärd kräver fortfarande ditt uttryckliga godkännande per anrop i chatten, även när automatiskt godkännande är aktiverat. Aktivera endast detta om du litar på den konfigurerade AI-leverantören med hårdvaruåtkomst.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="390"/>
        <source>Switch AI provider?</source>
        <translation>Byt AI-leverantör?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="391"/>
        <source>Switching to a different provider clears the current conversation. Do you want to continue?</source>
        <translation>Byte till en annan leverantör rensar den aktuella konversationen. Vill du fortsätta?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="394"/>
        <source>Assistant</source>
        <translation>Assistent</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="431"/>
        <source>AI Assistant is not available in this build</source>
        <translation>AI-assistent är inte tillgänglig i denna version</translation>
    </message>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">AI-assistenten kräver en Pro-licens</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="436"/>
        <source>Set an API key first</source>
        <translation>Ange en API-nyckel först</translation>
    </message>
</context>
<context>
    <name>AI::Conversation</name>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">AI-assistent kräver en Pro-licens</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="161"/>
        <source>AI Assistant is not available in this build</source>
        <translation>AI-assistent är inte tillgänglig i denna version</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="167"/>
        <source>AI subsystem not initialized</source>
        <translation>AI-undersystem ej initierat</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="173"/>
        <source>Already busy with a previous request</source>
        <translation>Upptagen med en tidigare förfrågan</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="484"/>
        <source>Tool-call budget reached for this turn; no further tools will run.</source>
        <translation>Verktygsanropsbudgeten nådd för denna omgång; inga fler verktyg kommer att köras.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1824"/>
        <source>You have reached the tool-call budget for this turn. Do not request more tools. Summarize what you found so far, and if the task is incomplete, say which steps remain so the user can tell you to continue.</source>
        <translation>Du har nått verktygsanropsbudgeten för denna omgång. Begär inte fler verktyg. Sammanfatta vad du hittade hittills, och om uppgiften är ofullständig, ange vilka steg som återstår så att användaren kan be dig fortsätta.</translation>
    </message>
    <message>
        <source>Tool-call budget exceeded</source>
        <translation type="vanished">Budget för verktygsanrop överskriden</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="927"/>
        <source>(The model returned an empty response. Try rephrasing, switching to a different model, or checking that the request is allowed by the provider's safety filters.)</source>
        <translation>(Modellen returnerade ett tomt svar. Försök omformulera, byt till en annan modell eller kontrollera att förfrågan tillåts av leverantörens säkerhetsfilter.)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1071"/>
        <source>Sending request to %1...</source>
        <translation>Skickar förfrågan till %1...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1082"/>
        <source>Provider returned no reply</source>
        <translation>Leverantör returnerade inget svar</translation>
    </message>
</context>
<context>
    <name>AI::GeminiReply</name>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="145"/>
        <source>Prompt blocked by Gemini safety filter: %1</source>
        <translation>Prompt blockerad av Geminis säkerhetsfilter: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="199"/>
        <source>Gemini stopped without producing a response: %1</source>
        <translation>Gemini stoppade utan att producera ett svar: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="261"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="264"/>
        <source>Invalid API key (%1)</source>
        <translation>Ogiltig API-nyckel (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="266"/>
        <source>Rate limited: %1</source>
        <translation>Hastighetsbegränsad: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="268"/>
        <source>Invalid API key</source>
        <translation>Ogiltig API-nyckel</translation>
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
        <translation>Ogiltig API-nyckel (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="345"/>
        <source>Rate limited: %1</source>
        <translation>Hastighetsbegränsad: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="347"/>
        <source>%1 %2: %3</source>
        <translation>%1 %2: %3</translation>
    </message>
    <message>
        <source>OpenAI %1: %2</source>
        <translation type="vanished">Openai %1: %2</translation>
    </message>
</context>
<context>
    <name>API::GRPC::GRPCServer</name>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="421"/>
        <source>Export Protobuf File</source>
        <translation>Exportera Protobuf-fil</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="423"/>
        <source>Protocol Buffers (*.proto)</source>
        <translation>Protocol Buffers (*.proto)</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="469"/>
        <source>Unable to start gRPC server</source>
        <translation>Kan inte starta GRPC-server</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="470"/>
        <source>Failed to bind to %1</source>
        <translation>Misslyckades att binda till %1</translation>
    </message>
</context>
<context>
    <name>API::Server</name>
    <message>
        <location filename="../../src/API/Server.cpp" line="434"/>
        <source>Unable to start API TCP server</source>
        <translation>Kan inte starta API TCP-server</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="478"/>
        <source>Allow External API Connections?</source>
        <translation>Tillåt Externa API-anslutningar?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="479"/>
        <source>Exposing the API server to external hosts allows other devices on your network to connect to Serial Studio on port 7777.

Only enable this on trusted networks. Untrusted clients may read live data or send commands to your device.</source>
        <translation>Att exponera API-servern för externa värdar tillåter andra enheter i nätverket att ansluta till Serial Studio på port 7777.

Aktivera endast detta i betrodda nätverk. Ej betrodda klienter kan läsa livedata eller skicka kommandon till enheten.</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="513"/>
        <source>Unable to restart API TCP server</source>
        <translation>Kan inte starta om API TCP-server</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="600"/>
        <source>Allow API device control?</source>
        <translation>Tillåt API-enhetskontroll?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="601"/>
        <source>A program using Serial Studio's local API is requesting to send data to the connected device. Allow API clients to write to the device?</source>
        <translation>Ett program som använder Serial Studios lokala API begär att skicka data till den anslutna enheten. Tillåt API-klienter att skriva till enheten?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="604"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1242"/>
        <source>API server</source>
        <translation>API-server</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1242"/>
        <source>Invalid pending connection</source>
        <translation>Ogiltig väntande anslutning</translation>
    </message>
</context>
<context>
    <name>About</name>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="39"/>
        <source>About</source>
        <translation>Om</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="96"/>
        <source>Version %1</source>
        <translation>Version %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="106"/>
        <source>Copyright © %1 %2</source>
        <translation>Copyright © %1 %2</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="112"/>
        <source>All Rights Reserved</source>
        <translation>Alla Rättigheter Förbehållna</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="127"/>
        <source>%1 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

%1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</source>
        <translation>%1 är fri programvara: du kan distribuera och/eller modifiera den under villkoren i GNU General Public License som publicerats av Free Software Foundation; antingen version 3 av licensen, eller (efter eget val) någon senare version.

%1 distribueras i hopp om att den ska vara användbar, men UTAN NÅGON GARANTI; utan ens underförstådd garanti om SÄLJBARHET eller LÄMPLIGHET FÖR ETT VISST ÄNDAMÅL. Se GNU General Public License för mer information.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="146"/>
        <source>This configuration is licensed for commercial and proprietary use. It may be used in closed-source and commercial applications, subject to the terms of the commercial license.</source>
        <translation>Denna konfiguration är licensierad för kommersiellt och proprietärt bruk. Den får användas i slutna och kommersiella applikationer, i enlighet med villkoren i den kommersiella licensen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="160"/>
        <source>This configuration is for personal and evaluation purposes only. Commercial use is prohibited unless a valid commercial license is activated.</source>
        <translation>Denna konfiguration är endast för personligt bruk och utvärdering. Kommersiellt bruk är förbjudet om inte en giltig kommersiell licens är aktiverad.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="174"/>
        <source>This software is provided 'as is' without warranty of any kind, express or implied, including but not limited to the warranties of merchantability or fitness for a particular purpose. In no event shall the author be liable for any damages arising from the use of this software.</source>
        <translation>Denna programvara tillhandahålls 'i befintligt skick' utan garanti av något slag, vare sig uttrycklig eller underförstådd, inklusive men inte begränsat till garantier om säljbarhet eller lämplighet för ett visst ändamål. Under inga omständigheter ska upphovsmannen hållas ansvarig för skador som uppstår vid användning av denna programvara.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="195"/>
        <source>Manage License</source>
        <translation>Hantera Licens</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="203"/>
        <source>Donate</source>
        <translation>Donera</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="214"/>
        <source>Check for Updates</source>
        <translation>Sök efter Uppdateringar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="223"/>
        <source>What's New</source>
        <translation>Vad Är Nytt</translation>
    </message>
    <message>
        <source>Tips &amp;&amp; Tricks</source>
        <translation type="vanished">Tips &amp; Tricks</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="232"/>
        <source>License Agreement</source>
        <translation>Licensavtal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="241"/>
        <source>Report Bug</source>
        <translation>Rapportera Bugg</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="250"/>
        <source>Acknowledgements</source>
        <translation>Erkännanden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="259"/>
        <source>Benchmark</source>
        <translation>Benchmark</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="267"/>
        <source>Website</source>
        <translation>Webbplats</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="283"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
</context>
<context>
    <name>Accelerometer</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="181"/>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="182"/>
        <source>Settings</source>
        <translation>Inställningar</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="240"/>
        <source>G-FORCE</source>
        <translation>G-KRAFT</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="276"/>
        <source>PITCH ↕</source>
        <translation>PITCH ↕</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="311"/>
        <source>ROLL ↔</source>
        <translation>ROLL ↔</translation>
    </message>
</context>
<context>
    <name>AccelerometerConfigDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="35"/>
        <source>Accelerometer Configuration</source>
        <translation>Accelerometerkonfiguration</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="95"/>
        <source>Configure the accelerometer display range and input units.</source>
        <translation>Konfigurera accelerometerns visningsområde och indataenheter.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="109"/>
        <source>Display Range</source>
        <translation>Visningsområde</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="130"/>
        <source>Max G:</source>
        <translation>Max G:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="144"/>
        <source>Enter max G value</source>
        <translation>Ange max G-värde</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="164"/>
        <source>Input Configuration</source>
        <translation>Ingångskonfiguration</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="184"/>
        <source>Input already in G</source>
        <translation>Ingång redan i G</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="218"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
</context>
<context>
    <name>Acknowledgements</name>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="35"/>
        <source>Acknowledgements</source>
        <translation>Erkännanden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="76"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="87"/>
        <source>About Qt…</source>
        <translation>Om QT…</translation>
    </message>
</context>
<context>
    <name>ActionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="136"/>
        <source>Change Icon</source>
        <translation>Ändra Ikon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="138"/>
        <source>Change the icon used for this action</source>
        <translation>Ändra ikonen som används för denna åtgärd</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="156"/>
        <source>Duplicate</source>
        <translation>Duplicera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="160"/>
        <source>Duplicate this action with all its settings</source>
        <translation>Duplicera denna åtgärd med alla dess inställningar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="169"/>
        <source>Delete</source>
        <translation>Ta Bort</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="171"/>
        <source>Delete this action from the project</source>
        <translation>Ta bort denna åtgärd från projektet</translation>
    </message>
</context>
<context>
    <name>AddWidgetDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="44"/>
        <source>Add Widget</source>
        <translation>Lägg till Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="211"/>
        <source>Available Widgets</source>
        <translation>Tillgängliga Widgetar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="220"/>
        <source>Click a row to add it to the workspace.</source>
        <translation>Klicka på en rad för att lägga till den i arbetsytan.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="228"/>
        <source>Search</source>
        <translation>Sök</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="247"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="248"/>
        <source>Group</source>
        <translation>Grupp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="249"/>
        <source>Name</source>
        <translation>Namn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="316"/>
        <source>(entire group)</source>
        <translation>(hela gruppen)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="351"/>
        <source>Already in workspace</source>
        <translation>Finns redan i arbetsytan</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="352"/>
        <source>Add to workspace</source>
        <translation>Lägg till i arbetsytan</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="381"/>
        <source>No widgets available.</source>
        <translation>Inga widgetar tillgängliga.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="382"/>
        <source>No widgets match.</source>
        <translation>Inga widgetar matchar.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="399"/>
        <source>%1 widgets</source>
        <translation>%1 widgetar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="400"/>
        <source>%1 of %2 widgets</source>
        <translation>%1 av %2 widgetar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="404"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
</context>
<context>
    <name>AlarmBandsEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="35"/>
        <source>Alarm Bands</source>
        <translation>Larmband</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="70"/>
        <source>Info</source>
        <translation>Info</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="71"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="151"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="72"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="152"/>
        <source>Warning</source>
        <translation>Varning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="73"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="153"/>
        <source>Critical</source>
        <translation>Kritiskt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="82"/>
        <source>Tachometer</source>
        <translation>Varvräknare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="84"/>
        <source>Idle</source>
        <translation>Tomgång</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="85"/>
        <source>Operating</source>
        <translation>Drift</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="86"/>
        <source>Caution</source>
        <translation>Försiktighet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="87"/>
        <source>Redline</source>
        <translation>Röd Zon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="91"/>
        <source>Speedometer</source>
        <translation>Hastighetsmätare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="93"/>
        <source>Cruise</source>
        <translation>Marschfart</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="94"/>
        <source>Fast</source>
        <translation>Snabb</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="95"/>
        <source>Top Speed</source>
        <translation>Toppfart</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="99"/>
        <source>Engine Temperature</source>
        <translation>Motortemperatur</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="101"/>
        <source>Cold</source>
        <translation>Kall</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="102"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="111"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="143"/>
        <source>Normal</source>
        <translation>Normal</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="103"/>
        <source>Warm</source>
        <translation>Varm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="104"/>
        <source>Overheat</source>
        <translation>Överhettning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="108"/>
        <source>Pressure</source>
        <translation>Tryck</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="110"/>
        <source>Vacuum</source>
        <translation>Vakuum</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="112"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="121"/>
        <source>High</source>
        <translation>Hög</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="113"/>
        <source>Burst</source>
        <translation>Sprängning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="117"/>
        <source>Battery Voltage</source>
        <translation>Batterispänning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="119"/>
        <source>Low</source>
        <translation>Låg</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="120"/>
        <source>Nominal</source>
        <translation>Nominell</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="125"/>
        <source>Fuel Level</source>
        <translation>Bränslenivå</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="127"/>
        <source>Empty</source>
        <translation>Tom</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="128"/>
        <source>Reserve</source>
        <translation>Reserv</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="133"/>
        <source>Signal Strength</source>
        <translation>Signalstyrka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="135"/>
        <source>No Signal</source>
        <translation>Ingen Signal</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="136"/>
        <source>Weak</source>
        <translation>Svag</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="137"/>
        <source>Good</source>
        <translation>Bra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="141"/>
        <source>CPU / System Load</source>
        <translation>CPU / Systembelastning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="144"/>
        <source>Busy</source>
        <translation>Upptagen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="145"/>
        <source>Overload</source>
        <translation>Överbelastad</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="149"/>
        <source>OK / Warning / Critical</source>
        <translation>OK / Varning / Kritiskt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="270"/>
        <source>Choose Band Color</source>
        <translation>Välj Bandfärg</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="297"/>
        <source>Presets</source>
        <translation>Förinställningar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="320"/>
        <source>Preset</source>
        <translation>Förinställning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="335"/>
        <source>Choose preset…</source>
        <translation>Välj förinställning…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="557"/>
        <source>Reset to severity default</source>
        <translation>Återställ till allvarlighetsgradstandard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="571"/>
        <source>Click to choose a color. Right-click to reset to severity default.</source>
        <translation>Klicka för att välja en färg. Högerklicka för att återställa till allvarlighetsgradstandard.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="572"/>
        <source>Click to choose a custom color.</source>
        <translation>Klicka för att välja en anpassad färg.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="657"/>
        <source>No bands defined. Pick a preset above or add a band to get started.</source>
        <translation>Inga band definierade. Välj en förinställning ovan eller lägg till ett band för att komma igång.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="781"/>
        <source>Apply</source>
        <translation>Tillämpa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="784"/>
        <source>Apply changes to the dataset.</source>
        <translation>Tillämpa ändringar på datasetet.</translation>
    </message>
    <message>
        <source>Apply Preset</source>
        <translation type="vanished">Tillämpa Förinställning</translation>
    </message>
    <message>
        <source>Replace the current bands with the selected preset, scaled to this dataset's range.</source>
        <translation type="vanished">Ersätt de aktuella banden med den valda förinställningen, skalad till detta datasets område.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="346"/>
        <source>Range</source>
        <translation>Område</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="374"/>
        <source>Bands</source>
        <translation>Band</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="385"/>
        <source>Add Band</source>
        <translation>Lägg till Band</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="389"/>
        <source>Add a new band continuing from the last one.</source>
        <translation>Lägg till ett nytt band som fortsätter från det senaste.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="420"/>
        <source>Min</source>
        <translation>Min</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="426"/>
        <source>Max</source>
        <translation>Max</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="432"/>
        <source>Severity</source>
        <translation>Allvarlighetsgrad</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="438"/>
        <source>Color</source>
        <translation>Färg</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="445"/>
        <source>Label</source>
        <translation>Etikett</translation>
    </message>
    <message>
        <source>Choose a custom color.</source>
        <translation type="vanished">Välj en anpassad färg.</translation>
    </message>
    <message>
        <source>auto</source>
        <translation type="vanished">auto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="594"/>
        <source>(optional)</source>
        <translation>(valfritt)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="611"/>
        <source>Move up.</source>
        <translation>Flytta upp.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="630"/>
        <source>Move down.</source>
        <translation>Flytta ner.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="643"/>
        <source>Remove this band.</source>
        <translation>Ta bort detta band.</translation>
    </message>
    <message>
        <source>No bands defined. Apply a preset or add a band to get started.</source>
        <translation type="vanished">Inga band definierade. Använd en förinställning eller lägg till ett band för att komma igång.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="674"/>
        <source>Preview</source>
        <translation>Förhandsgranskning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="770"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="772"/>
        <source>Discard changes.</source>
        <translation>Kassera ändringar.</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="vanished">Spara</translation>
    </message>
    <message>
        <source>Save changes to the dataset.</source>
        <translation type="vanished">Spara ändringar i datasetet.</translation>
    </message>
</context>
<context>
    <name>AssistantPanel</name>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="31"/>
        <source>Assistant</source>
        <translation>Assistent</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="129"/>
        <source>CSV vs MDF4 export - what is the difference?</source>
        <translation>CSV vs MDF4-export - vad är skillnaden?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="132"/>
        <source>Plot, Bar, and Gauge - when to use each?</source>
        <translation>Plot, Bar och Gauge - när ska man använda varje?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="204"/>
        <source>How can I help with your project?</source>
        <translation>Hur kan jag hjälpa till med ditt projekt?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="205"/>
        <source>Set up your API key to get started</source>
        <translation>Ställ in din API-nyckel för att komma igång</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="217"/>
        <source>Describe what you would like to build, and I will configure the sources, groups, datasets, frame parsers, and transforms for you.</source>
        <translation>Beskriv vad du vill bygga, så konfigurerar jag källor, grupper, dataset, frame-parsers och transformationer åt dig.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="220"/>
        <source>To start chatting, paste an API key for the selected provider. Keys are encrypted on this machine and never leave your computer except to talk to the provider you choose.</source>
        <translation>För att börja chatta, klistra in en API-nyckel för den valda leverantören. Nycklar krypteras på denna maskin och lämnar aldrig din dator förutom för att kommunicera med den leverantör du väljer.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="241"/>
        <source>Open API Key Setup</source>
        <translation>Öppna API-nyckelinställningar</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="251"/>
        <source>Get a key from %1</source>
        <translation>Hämta en nyckel från %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="125"/>
        <source>List the sources in this project</source>
        <translation>Lista källorna i detta projekt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="122"/>
        <source>Help me discover Serial Studio's features</source>
        <translation>Hjälp mig upptäcka Serial Studios funktioner</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="123"/>
        <source>What can this app do for my telemetry?</source>
        <translation>Vad kan den här appen göra för min telemetri?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="124"/>
        <source>Walk me through what this project already contains</source>
        <translation>Gå igenom vad det här projektet redan innehåller</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="128"/>
        <source>What is a session database, and why would I use one?</source>
        <translation>Vad är en sessionsdatabas, och varför skulle jag använda en?</translation>
    </message>
    <message>
        <source>CSV vs MDF4 export — what is the difference?</source>
        <translation type="vanished">CSV vs MDF4-export — vad är skillnaden?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="130"/>
        <source>What is a frame parser, and when do I need one?</source>
        <translation>Vad är en ramparsare, och när behöver jag en?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="131"/>
        <source>When should I use Lua vs JavaScript for the parser?</source>
        <translation>När ska jag använda Lua vs JavaScript för parsaren?</translation>
    </message>
    <message>
        <source>Plot, Bar, and Gauge — when to use each?</source>
        <translation type="vanished">Plot, Bar och Gauge — när ska man använda varje?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="133"/>
        <source>What is the difference between a transform and a frame parser?</source>
        <translation>Vad är skillnaden mellan en transform och en ramparsare?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="136"/>
        <source>Add a UART source for an Arduino</source>
        <translation>Lägg till en UART-källa för en Arduino</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="137"/>
        <source>Set up an IMU project from scratch</source>
        <translation>Konfigurera ett IMU-projekt från grunden</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="138"/>
        <source>Configure an MQTT subscriber</source>
        <translation>Konfigurera en MQTT-prenumerant</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="139"/>
        <source>Add a CAN bus source</source>
        <translation>Lägg till en CAN-busskälla</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="140"/>
        <source>Set up a Modbus poller</source>
        <translation>Konfigurera en Modbus-pollare</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="141"/>
        <source>Add a network (TCP/UDP) source</source>
        <translation>Lägg till en nätverkskälla (TCP/UDP)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="142"/>
        <source>Write a CSV frame parser for me</source>
        <translation>Skriv en CSV frame-parser åt mig</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="143"/>
        <source>Help me parse a JSON frame</source>
        <translation>Hjälp mig tolka en JSON-frame</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="144"/>
        <source>Add an EMA smoothing transform to a dataset</source>
        <translation>Lägg till en EMA-utjämningstransform till en dataset</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="145"/>
        <source>Decode hexadecimal frames</source>
        <translation>Avkoda hexadecimala frames</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="146"/>
        <source>Calibrate a sensor with a linear transform</source>
        <translation>Kalibrera en sensor med en linjär transform</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="149"/>
        <source>Suggest dashboard widgets for my data</source>
        <translation>Föreslå instrumentpanelswidgetar för min data</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="150"/>
        <source>Build an executive overview workspace</source>
        <translation>Bygg en översiktsarbetsyta för ledning</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="151"/>
        <source>Add a painter widget for a custom visualization</source>
        <translation>Lägg till en painter-widget för en anpassad visualisering</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="152"/>
        <source>Show Plot, FFT, and Waterfall for one dataset</source>
        <translation>Visa Plot, FFT och Vattenfall för en Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="153"/>
        <source>Group my datasets into useful workspaces</source>
        <translation>Gruppera mina Datasets i användbara Arbetsytor</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="432"/>
        <source>Drop files or folders to let the assistant read them</source>
        <translation>Släpp filer eller mappar för att låta assistenten läsa dem</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="477"/>
        <source>Added folder "%1" - readable this session</source>
        <translation>Lade till mapp "%1" - läsbar denna session</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="478"/>
        <source>Added "%1" - readable this session</source>
        <translation>Lade till "%1" - läsbar denna session</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="562"/>
        <source>Ask Serial Studio anything…</source>
        <translation>Fråga Serial Studio vad som helst…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="582"/>
        <source>Clear conversation</source>
        <translation>Rensa konversation</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="626"/>
        <source>Stop generating</source>
        <translation>Stoppa generering</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="627"/>
        <source>Send message (Enter)</source>
        <translation>Skicka meddelande (Enter)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="669"/>
        <source>Provider</source>
        <translation>Leverantör</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="702"/>
        <source>Model selection</source>
        <translation>Modellval</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="748"/>
        <source>Run editing actions without asking each time. Blocked actions stay blocked.</source>
        <translation>Kör redigeringsåtgärder utan att fråga varje gång. Blockerade åtgärder förblir blockerade.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="750"/>
        <source>Auto-approve edits</source>
        <translation>Godkänn redigeringar automatiskt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="766"/>
        <source>Let the AI configure devices, connect/disconnect and send data. Each action still asks for your approval.</source>
        <translation>Låt AI:n konfigurera enheter, ansluta/koppla från och skicka data. Varje åtgärd kräver fortfarande ditt godkännande.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="768"/>
        <source>Allow device control</source>
        <translation>Tillåt enhetskontroll</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="794"/>
        <source>Manage API keys</source>
        <translation>Hantera API-nycklar</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="815"/>
        <source>Working</source>
        <translation>Arbetar</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="816"/>
        <source>Ready</source>
        <translation>Redo</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="817"/>
        <source>  •  cache %1k tok</source>
        <translation>•  cache %1k tok</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="818"/>
        <source>  •  cache write %1k tok</source>
        <translation>cache-skrivning %1k tok</translation>
    </message>
</context>
<context>
    <name>Audio</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="89"/>
        <source>No Microphone Detected</source>
        <translation>Ingen Mikrofon Upptäckt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="98"/>
        <source>Connect a mic or check your settings</source>
        <translation>Anslut en mikrofon eller kontrollera inställningarna</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="123"/>
        <source>Input Device</source>
        <translation>Ingångsenhet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="142"/>
        <source>Sample Rate</source>
        <translation>Samplingsfrekvens</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="230"/>
        <source>Sample Format</source>
        <translation>Samplingsformat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="180"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="249"/>
        <source>Channels</source>
        <translation>Kanaler</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="211"/>
        <source>Output Device</source>
        <translation>Utgångsenhet</translation>
    </message>
</context>
<context>
    <name>AuthenticateDialog</name>
    <message>
        <source>Dialog</source>
        <translation type="vanished">Dialog</translation>
    </message>
    <message>
        <source>Please provide the user name and password for the download location.</source>
        <translation type="vanished">Ange användarnamn och lösenord för nedladdningsplatsen.</translation>
    </message>
    <message>
        <source>&amp;User name:</source>
        <translation type="vanished">&amp;Användarnamn:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="vanished">&amp;Lösenord:</translation>
    </message>
</context>
<context>
    <name>AxisRangeDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="47"/>
        <source>Axis Range Configuration</source>
        <translation>Konfiguration av Axelområde</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="161"/>
        <source>Configure the visible range for the plot axes. Values update in real-time as you type.</source>
        <translation>Konfigurera det synliga området för plottaxlarna. Värdena uppdateras i realtid medan du skriver.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="169"/>
        <source>X Axis</source>
        <translation>X-axel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="194"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="265"/>
        <source>Minimum:</source>
        <translation>Minimum:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="206"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="277"/>
        <source>Enter min value</source>
        <translation>Ange minimivärde</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="215"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="286"/>
        <source>Maximum:</source>
        <translation>Maximum:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="227"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="298"/>
        <source>Enter max value</source>
        <translation>Ange maximivärde</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="242"/>
        <source>Y Axis</source>
        <translation>Y-axel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="317"/>
        <source>Reset</source>
        <translation>Återställ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="327"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
</context>
<context>
    <name>BackupRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="31"/>
        <source>Recover Backup</source>
        <translation>Återställ Säkerhetskopia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="94"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="165"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="166"/>
        <source>Untitled</source>
        <translation>Namnlös</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="97"/>
        <source>Project Loaded</source>
        <translation>Projekt Laddat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="98"/>
        <source>Auto-save</source>
        <translation>Autospara</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="99"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="119"/>
        <source>Before Restore</source>
        <translation>Före Återställning</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="100"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="106"/>
        <source>Before Delete Dataset</source>
        <translation>Före Radering av Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="101"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="107"/>
        <source>Before Delete Group</source>
        <translation>Före Radering av Grupp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="102"/>
        <source>Before New Project</source>
        <translation>Före Nytt Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="103"/>
        <source>Before Open Project</source>
        <translation>Före Öppna Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="104"/>
        <source>Before Load JSON</source>
        <translation>Före Ladda JSON</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="105"/>
        <source>Before Apply Template</source>
        <translation>Före Tillämpa Mall</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="108"/>
        <source>Before Delete Action</source>
        <translation>Före Radering av Åtgärd</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="109"/>
        <source>Before Delete Output Widget</source>
        <translation>Före Radering av Utmatningswidget</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="110"/>
        <source>Before Move Dataset</source>
        <translation>Före Flytta Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="111"/>
        <source>Before Move Group</source>
        <translation>Före Flytta Grupp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="112"/>
        <source>Before Delete Workspace</source>
        <translation>Före Ta Bort Arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="113"/>
        <source>Before Clear All Workspaces</source>
        <translation>Före Rensa Alla Arbetsytor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="114"/>
        <source>Before Remove Widget</source>
        <translation>Före Ta Bort Widget</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="115"/>
        <source>Before Reorder Workspaces</source>
        <translation>Före Omordna Arbetsytor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="116"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="117"/>
        <source>Before Batch Operation</source>
        <translation>Före Batchoperation</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="118"/>
        <source>Before Add Tile</source>
        <translation>Före Lägg till Panel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="142"/>
        <source>%1 (and %2 more)</source>
        <translation>%1 (och %2 till)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="159"/>
        <source> The frame parser code also differs and will be replaced.</source>
        <translation>Ramparserkoden skiljer sig också och kommer att ersättas.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="164"/>
        <source>Title changes from “%1” to “%2”. Group structure unchanged.</source>
        <translation>Titel ändras från "%1" till "%2". Gruppstruktur oförändrad.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="169"/>
        <source>Same groups and datasets, but the frame parser code differs — restoring will replace it.</source>
        <translation>Samma grupper och dataset, men ramparserkoden skiljer sig — återställning ersätter den.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="171"/>
        <source>Same groups and datasets as your current project. Restoring may still revert field-level edits.</source>
        <translation>Samma grupper och dataset som ditt nuvarande projekt. Återställning kan fortfarande återställa ändringar på fältnivå.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="178"/>
        <source>Restoring removes %1 and brings back %2.</source>
        <translation>Återställning tar bort %1 och återför %2.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="181"/>
        <source>Restoring removes %1.</source>
        <translation>Återställning tar bort %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="183"/>
        <source>Restoring brings back %1.</source>
        <translation>Återställning återför %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="209"/>
        <source>Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.</source>
        <translation>Välj en säkerhetskopia att återställa. Det nuvarande projektet sparas automatiskt först, så återställningen är reversibel.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="292"/>
        <source>No backups for this project yet. Edit or save the project to start the rolling backup.</source>
        <translation>Inga säkerhetskopior för detta projekt ännu. Redigera eller spara projektet för att starta rullande säkerhetskopiering.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="320"/>
        <source>Open Folder</source>
        <translation>Öppna Mapp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="328"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="334"/>
        <source>Restore</source>
        <translation>Återställ</translation>
    </message>
</context>
<context>
    <name>Benchmark</name>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="32"/>
        <source>Benchmark</source>
        <translation>Benchmark</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="79"/>
        <source>%1 frames/s</source>
        <translation>%1 ramar/s</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="83"/>
        <source>%1 s</source>
        <translation>%1 s</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="88"/>
        <source>n/a</source>
        <translation>ej tillgänglig</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Pass</source>
        <translation>Godkänd</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Fail</source>
        <translation>Underkänd</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="111"/>
        <source>Hotpath Benchmark</source>
        <translation>Hotpath Benchmark</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="122"/>
        <source>Measures how fast this computer can extract, parse, and visualize frames through Serial Studio's data pipeline.</source>
        <translation>Mäter hur snabbt denna dator kan extrahera, tolka och visualisera ramar genom Serial Studios datapipeline.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="168"/>
        <source>The interface will freeze while running</source>
        <translation>Gränssnittet fryser under körning</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="179"/>
        <source>Each phase runs flat-out on the main thread, so the window stops responding until it finishes. Your current project is reloaded automatically when the benchmark ends.</source>
        <translation>Varje fas körs på huvudtråden, så fönstret slutar svara tills den är klar. Ditt nuvarande projekt laddas om automatiskt när benchmarket avslutas.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="236"/>
        <source>Frames per phase:</source>
        <translation>Ramar per fas:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="250"/>
        <source>Minimum duration:</source>
        <translation>Minsta varaktighet:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="279"/>
        <source>Stages</source>
        <translation>Steg</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="287"/>
        <source>Parsers</source>
        <translation>Parsers</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="296"/>
        <source>Data export</source>
        <translation>Dataexport</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="304"/>
        <source>Dashboard</source>
        <translation>Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="316"/>
        <source>Data</source>
        <translation>Data</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="326"/>
        <source>Numeric only</source>
        <translation>Endast numeriska värden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="335"/>
        <source>Mixed (numeric + text)</source>
        <translation>Blandad (numerisk + text)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="351"/>
        <source>Select at least one stage and one data type to run a benchmark.</source>
        <translation>Välj minst ett steg och en datatyp för att köra ett prestandatest.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="210"/>
        <source>Running %1...</source>
        <translation>Kör %1...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="211"/>
        <source>Preparing...</source>
        <translation>Förbereder...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="383"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="395"/>
        <source>Throughput</source>
        <translation>Genomströmning</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="407"/>
        <source>Time</source>
        <translation>Tid</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="419"/>
        <source>Result</source>
        <translation>Resultat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="520"/>
        <source>Run a test to see results</source>
        <translation>Kör ett test för att se resultat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="539"/>
        <source>Pass/Fail applies to the data-pipeline and parser stages (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard stages are informational.</source>
        <translation>Godkänt/Underkänt gäller datapipeline- och parserstegen (datapipeline och inbyggd numerisk 1024 K ramar/s; inbyggd blandad 512 K; Lua numerisk 256 K; JavaScript numerisk och Lua blandad 128 K; JavaScript blandad 64 K). Export- och instrumentpanelsstegen är informativa.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Godkänt/Underkänt gäller datapipeline- och parserfaserna (datapipeline och Inbyggd numerisk 1024 K ramar/s; Inbyggd blandad 512 K; Lua numerisk 256 K; JavaScript numerisk och Lua blandad 128 K; JavaScript blandad 64 K). Export- och instrumentpanelsfaserna är informativa.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Native numeric 1024 K frames/s; Native mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Pass/Fail gäller data-pipeline- och parserfaserna (data-pipeline och Native numerisk 1024 K ramar/s; Native blandad 512 K; Lua numerisk 256 K; JavaScript numerisk och Lua blandad 128 K; JavaScript blandad 64 K). Export- och instrumentpanelsfaserna är informativa.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="554"/>
        <source>Copy</source>
        <translation>Kopiera</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline 1024 K frames/s; numeric: Lua 256 K, JavaScript 128 K; mixed: Lua 128 K, JavaScript 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Godkänt/Underkänt gäller datapipeline- och parserfaserna (datapipeline 1024 K ramar/s; numerisk: Lua 256 K, JavaScript 128 K; blandad: Lua 128 K, JavaScript 64 K). Export- och instrumentpanelsfaserna är informativa.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the parser phases only (Lua target 256 K frames/s, JavaScript 128 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Godkänt/Underkänt gäller endast parserfaserna (Lua-mål 256 K ramar/s, JavaScript 128 K). Export- och instrumentpanelsfaserna är informativa.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="561"/>
        <source>Clear</source>
        <translation>Rensa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="570"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Running...</source>
        <translation>Kör…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Run Benchmark</source>
        <translation>Kör Benchmark</translation>
    </message>
</context>
<context>
    <name>BenchmarkRunner</name>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="255"/>
        <source>Data pipeline</source>
        <translation>Datapipeline</translation>
    </message>
    <message>
        <source>Lua parser</source>
        <translation type="vanished">Lua-parser</translation>
    </message>
    <message>
        <source>JavaScript parser</source>
        <translation type="vanished">JavaScript-parser</translation>
    </message>
    <message>
        <source>Lua + data export</source>
        <translation type="vanished">Lua + dataexport</translation>
    </message>
    <message>
        <source>Lua + dashboard</source>
        <translation type="vanished">Lua + instrumentpanel</translation>
    </message>
    <message>
        <source>Native parser (numeric)</source>
        <translation type="vanished">Native-parser (numerisk)</translation>
    </message>
    <message>
        <source>Native parser (mixed)</source>
        <translation type="vanished">Native-parser (blandad)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="279"/>
        <source>Built-in parser (numeric)</source>
        <translation>Inbyggd parser (numerisk)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="297"/>
        <source>Built-in parser (mixed)</source>
        <translation>Inbyggd parser (blandad)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="281"/>
        <source>Lua parser (numeric)</source>
        <translation>Lua-parser (numerisk)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="288"/>
        <source>JavaScript parser (numeric)</source>
        <translation>JavaScript-parser (numerisk)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="299"/>
        <source>Lua parser (mixed)</source>
        <translation>Lua-parser (blandad)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="306"/>
        <source>JavaScript parser (mixed)</source>
        <translation>JavaScript-parser (blandad)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="322"/>
        <source>Built-in + data export (numeric)</source>
        <translation>Inbyggd + dataexport (numerisk)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="329"/>
        <source>Lua + data export (numeric)</source>
        <translation>Lua + dataexport (numerisk)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="336"/>
        <source>JavaScript + data export (numeric)</source>
        <translation>JavaScript + dataexport (numerisk)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="345"/>
        <source>Built-in + data export (mixed)</source>
        <translation>Inbyggd + dataexport (blandad)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="347"/>
        <source>Lua + data export (mixed)</source>
        <translation>Lua + dataexport (blandad)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="354"/>
        <source>JavaScript + data export (mixed)</source>
        <translation>JavaScript + dataexport (blandad)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="370"/>
        <source>Built-in + dashboard (numeric)</source>
        <translation>Inbyggd + instrumentpanel (numerisk)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="372"/>
        <source>Lua + dashboard (numeric)</source>
        <translation>Lua + instrumentpanel (numerisk)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>100 K frames</source>
        <translation>100 K ramar</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>250 K frames</source>
        <translation>250 K ramar</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>500 K frames</source>
        <translation>500 K ramar</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>1 M frames</source>
        <translation>1 M ramar</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>1 second</source>
        <translation>1 sekund</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>2 seconds</source>
        <translation>2 sekunder</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>5 seconds</source>
        <translation>5 sekunder</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>10 seconds</source>
        <translation>10 sekunder</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="200"/>
        <source>Serial Studio %1 - Hotpath Benchmark</source>
        <translation>Serial Studio %1 - Hotpath-benchmark</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="202"/>
        <source>%1 (%2), workload: %3 frames minimum, %4 s minimum</source>
        <translation>%1 (%2), arbetsbelastning: %3 ramar minimum, %4 s minimum</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Throughput</source>
        <translation>Genomströmning</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Target</source>
        <translation>Mål</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Time</source>
        <translation>Tid</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Result</source>
        <translation>Resultat</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="225"/>
        <source>%1 frames/s</source>
        <translation>%1 ramar/s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>n/a</source>
        <translation>ej tillämpligt</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Pass</source>
        <translation>Godkänd</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Fail</source>
        <translation>Underkänd</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="227"/>
        <source>%1 s</source>
        <translation>%1 s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="379"/>
        <source>JavaScript + dashboard (numeric)</source>
        <translation>JavaScript + instrumentpanel (numerisk)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="388"/>
        <source>Built-in + dashboard (mixed)</source>
        <translation>Inbyggd + instrumentpanel (blandad)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="390"/>
        <source>Lua + dashboard (mixed)</source>
        <translation>Lua + instrumentpanel (blandad)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="397"/>
        <source>JavaScript + dashboard (mixed)</source>
        <translation>JavaScript + instrumentpanel (blandad)</translation>
    </message>
    <message>
        <source>Showing dashboard...</source>
        <translation type="vanished">Visar instrumentpanel…</translation>
    </message>
</context>
<context>
    <name>BluetoothLE</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="54"/>
        <source>Device</source>
        <translation>Enhet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="106"/>
        <source>Service</source>
        <translation>Tjänst</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="142"/>
        <source>Characteristic</source>
        <translation>Karakteristik</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="200"/>
        <source>Scanning…</source>
        <translation>Söker…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="236"/>
        <source>No Bluetooth Adapter Detected</source>
        <translation>Ingen Bluetooth-adapter Hittades</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="247"/>
        <source>Connect a Bluetooth adapter or check your system settings</source>
        <translation>Anslut en Bluetooth-adapter eller kontrollera systeminställningarna</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="274"/>
        <source>This OS is not Supported Yet.</source>
        <translation>Detta operativsystem stöds inte ännu.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="285"/>
        <source>We'll update Serial Studio to work with this operating system as soon as Qt officially supports it</source>
        <translation>Vi uppdaterar Serial Studio för att fungera med detta operativsystem så snart QT officiellt stöder det</translation>
    </message>
</context>
<context>
    <name>CANBus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="57"/>
        <source>No CAN Drivers Found</source>
        <translation>Inga CAN-drivrutiner Hittades</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="70"/>
        <source>Install CAN hardware drivers for your system</source>
        <translation>Installera CAN-hårdvarudrivrutiner för ditt system</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="97"/>
        <source>CAN Driver</source>
        <translation>CAN-drivrutin</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="142"/>
        <source>Interface</source>
        <translation>Gränssnitt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="161"/>
        <source>Bitrate</source>
        <translation>Bithastighet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="225"/>
        <source>Flexible Data-Rate</source>
        <translation>Flexibel Datahastighet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="245"/>
        <source>Loopback</source>
        <translation>Loopback</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="265"/>
        <source>Listen-Only</source>
        <translation>Endast Lyssning</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="285"/>
        <source>DBC Database</source>
        <translation>DBC-databas</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="289"/>
        <source>Import DBC File…</source>
        <translation>Importera DBC-fil…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="322"/>
        <source>No CAN Interfaces Found</source>
        <translation>Inga CAN-gränssnitt Hittades</translation>
    </message>
</context>
<context>
    <name>CSV::Player</name>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="205"/>
        <source>Select CSV file</source>
        <translation>Välj CSV-fil</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="207"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-filer (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="369"/>
        <source>Device Connection Active</source>
        <translation>Enhetsanslutning Aktiv</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="370"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>För att använda denna funktion måste du koppla från enheten. Vill du fortsätta?</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="384"/>
        <source>Check file permissions and location</source>
        <translation>Kontrollera filbehörigheter och plats</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="414"/>
        <source>Insufficient Data in CSV File</source>
        <translation>Otillräcklig Data i CSV-Fil</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="384"/>
        <source>Cannot read CSV file</source>
        <translation>Kan inte läsa CSV-fil</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="415"/>
        <source>The CSV file must contain at least one data row to proceed. Check the file and try again.</source>
        <translation>CSV-filen måste innehålla minst en datarad för att fortsätta. Kontrollera filen och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="645"/>
        <source>Invalid CSV</source>
        <translation>Ogiltig CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="646"/>
        <source>The CSV file does not contain any data or headers.</source>
        <translation>CSV-filen innehåller inga data eller rubriker.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="655"/>
        <source>Select a date/time column</source>
        <translation>Välj en datum/tid-kolumn</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="655"/>
        <location filename="../../src/CSV/Player.cpp" line="667"/>
        <source>Set interval manually</source>
        <translation>Ställ in intervall manuellt</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="657"/>
        <source>CSV Date/Time Selection</source>
        <translation>Val av CSV-datum/tid</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="658"/>
        <source>Choose how to handle the date/time data:</source>
        <translation>Välj hur datum/tid-data ska hanteras:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="670"/>
        <source>Set Interval</source>
        <translation>Ställ In Intervall</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="671"/>
        <source>Please enter the interval between rows in milliseconds:</source>
        <translation>Ange intervallet mellan rader i millisekunder:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="687"/>
        <source>Select Date/Time Column</source>
        <translation>Välj Datum/tid-kolumn</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="688"/>
        <source>Please select the column that contains the date/time data:</source>
        <translation>Välj den kolumn som innehåller datum/tid-data:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="698"/>
        <source>Invalid Selection</source>
        <translation>Ogiltigt Val</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="698"/>
        <source>The selected column is not valid.</source>
        <translation>Den valda kolumnen är inte giltig.</translation>
    </message>
</context>
<context>
    <name>Console</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Console.qml" line="32"/>
        <source>Console</source>
        <translation>Konsol</translation>
    </message>
</context>
<context>
    <name>Console::Export</name>
    <message>
        <location filename="../../src/Console/Export.cpp" line="331"/>
        <source>Console Export is a Pro feature.</source>
        <translation>Konsolexport är en Pro-funktion.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="332"/>
        <source>This feature requires a license. Please purchase one to enable console export.</source>
        <translation>Denna funktion kräver en licens. Köp en för att aktivera konsolexport.</translation>
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
        <translation>Ingen Radslut</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="255"/>
        <source>New Line</source>
        <translation>Ny Rad</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="256"/>
        <source>Carriage Return</source>
        <translation>Vagnretur</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="257"/>
        <source>CR + NL</source>
        <translation>CR + NL</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="267"/>
        <source>Plain Text</source>
        <translation>Oformaterad Text</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="268"/>
        <source>Hexadecimal</source>
        <translation>Hexadecimal</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="290"/>
        <source>No Checksum</source>
        <translation>Ingen Kontrollsumma</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="926"/>
        <source>Device %1</source>
        <translation>Enhet %1</translation>
    </message>
</context>
<context>
    <name>ConstantsLibraryDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="44"/>
        <source>Insert Constant</source>
        <translation>Infoga Konstant</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Fundamental</source>
        <translation>Fundamental</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <source>Speed of light in vacuum</source>
        <translation>Ljusets hastighet i vakuum</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <source>Planck constant</source>
        <translation>Plancks konstant</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <source>Elementary charge</source>
        <translation>Elementarladdning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <source>Avogadro constant</source>
        <translation>Avogadros konstant</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <source>Boltzmann constant</source>
        <translation>Boltzmanns konstant</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Stefan-Boltzmann constant</source>
        <translation>Stefan-Boltzmanns konstant</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Mechanics</source>
        <translation>Mekanik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <source>Standard gravity</source>
        <translation>Standardgravitation</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Gravitational constant</source>
        <translation>Gravitationskonstant</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Pressure</source>
        <translation>Tryck</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <source>Standard atmosphere</source>
        <translation>Standardatmosfär</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Sea-level barometric pressure</source>
        <translation>Barometriskt lufttryck vid havsytan</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Temperature</source>
        <translation>Temperatur</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <source>Absolute zero (Celsius)</source>
        <translation>Absolut nollpunkt (Celsius)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <source>Water freezing point</source>
        <translation>Vattnets fryspunkt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Water boiling point (1 atm)</source>
        <translation>Vattnets kokpunkt (1 atm)</translation>
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
        <translation>Gaser &amp; Vätskor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <source>Universal gas constant</source>
        <translation>Universella gaskonstanten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <source>Specific gas constant (dry air)</source>
        <translation>Specifik gaskonstant (torr luft)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <source>Specific gas constant (water vapor)</source>
        <translation>Specifik gaskonstant (vattenånga)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <source>Air density (sea level, 15°C)</source>
        <translation>Luftdensitet (havsytan, 15°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <source>Water density (4°C)</source>
        <translation>Vattendensitet (4°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <source>Speed of sound in air (20°C)</source>
        <translation>Ljudhastighet i luft (20°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Heat capacity ratio (dry air)</source>
        <translation>Värmekapacitetskvot (torr luft)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Electromagnetism</source>
        <translation>Elektromagnetism</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <source>Vacuum permittivity</source>
        <translation>Vakuumpermittivitet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Vacuum permeability</source>
        <translation>Vakuumpermeabilitet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Math</source>
        <translation>Matematik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <source>Pi</source>
        <translation>Pi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <source>Euler's number</source>
        <translation>Eulers tal</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Golden ratio</source>
        <translation>Gyllene snittet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="212"/>
        <source>Physics Constants</source>
        <translation>Fysikkonstanter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="221"/>
        <source>SI-unit preset values. Click a row to insert it into %1.</source>
        <translation>SI-enhetsförinställda värden. Klicka på en rad för att infoga den i %1.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="231"/>
        <source>Search</source>
        <translation>Sök</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="250"/>
        <source>Symbol</source>
        <translation>Symbol</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="251"/>
        <source>Name</source>
        <translation>Namn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="252"/>
        <source>Value</source>
        <translation>Värde</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="253"/>
        <source>Category</source>
        <translation>Kategori</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="357"/>
        <source>No constants match.</source>
        <translation>Inga konstanter matchar.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="378"/>
        <source>%1 constants</source>
        <translation>%1 konstanter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="379"/>
        <source>%1 of %2 constants</source>
        <translation>%1 av %2 konstanter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="383"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
</context>
<context>
    <name>ControlScriptView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="33"/>
        <source>Control Script</source>
        <translation>Kontrollskript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="45"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="168"/>
        <source>Undo</source>
        <translation>Ångra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="52"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="179"/>
        <source>Redo</source>
        <translation>Gör Om</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="60"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="198"/>
        <source>Cut</source>
        <translation>Klipp Ut</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="61"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="208"/>
        <source>Copy</source>
        <translation>Kopiera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="62"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="218"/>
        <source>Paste</source>
        <translation>Klistra In</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="67"/>
        <source>Select All</source>
        <translation>Markera Allt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="77"/>
        <source>Format Document</source>
        <translation>Formatera Dokument</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="84"/>
        <source>Format Selection</source>
        <translation>Formatera Markering</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="148"/>
        <source>Reset</source>
        <translation>Återställ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="153"/>
        <source>Reset to the default control script</source>
        <translation>Återställ till standardkontrollskriptet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="158"/>
        <source>Open</source>
        <translation>Öppna</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="163"/>
        <source>Import a control script file</source>
        <translation>Importera en kontrollskriptfil</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="173"/>
        <source>Undo the last code edit</source>
        <translation>Ångra den senaste kodredigeringen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="185"/>
        <source>Redo the previously undone edit</source>
        <translation>Gör om den tidigare ångrade redigeringen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="203"/>
        <source>Cut selected code to clipboard</source>
        <translation>Klipp ut markerad kod till urklipp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="213"/>
        <source>Copy selected code to clipboard</source>
        <translation>Kopiera markerad kod till urklipp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="222"/>
        <source>Paste code from clipboard</source>
        <translation>Klistra in kod från urklipp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="236"/>
        <source>Help</source>
        <translation>Hjälp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="241"/>
        <source>Open the control script documentation</source>
        <translation>Öppna dokumentationen för kontrollskript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="251"/>
        <source>Validate</source>
        <translation>Validera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="255"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Verifiera att skriptet kompilerar korrekt</translation>
    </message>
</context>
<context>
    <name>CrashRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="31"/>
        <source>Recovery Options</source>
        <translation>Återställningsalternativ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="57"/>
        <source>Serial Studio has closed unexpectedly several times in a row.</source>
        <translation>Serial Studio har oväntat stängts flera gånger i rad.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="82"/>
        <source>Consecutive crashes: %1</source>
        <translation>På varandra följande krascher: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="90"/>
        <source>Last reported stage: %1</source>
        <translation>Senast rapporterade steg: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="98"/>
        <source>Detected at: %1</source>
        <translation>Upptäckt vid: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="113"/>
        <source>Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.</source>
        <translation>Välj en återställningsåtgärd. Serial Studio avslutas efter att åtgärden har tillämpats så att nästa start blir ren.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="127"/>
        <source>Reset Rendering Backend to Default</source>
        <translation>Återställ renderingsmotorn till standard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="135"/>
        <source>Skip Restoring the Last Opened Project</source>
        <translation>Hoppa över att återställa det senast öppnade projektet</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="142"/>
        <source>Reset all Preferences</source>
        <translation>Återställ alla inställningar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="160"/>
        <source>Continue Anyway</source>
        <translation>Fortsätt Ändå</translation>
    </message>
</context>
<context>
    <name>CsvPlayer</name>
    <message>
        <location filename="../../qml/Dialogs/CsvPlayer.qml" line="36"/>
        <source>CSV Player</source>
        <translation>CSV-spelare</translation>
    </message>
</context>
<context>
    <name>DBCPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="44"/>
        <source>DBC File Preview</source>
        <translation>Förhandsgranskning av DBC-fil</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="169"/>
        <source>DBC File: %1</source>
        <translation>DBC-fil: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="177"/>
        <source>Review the CAN messages and signals to import into a new Serial Studio project.</source>
        <translation>Granska CAN-meddelanden och signaler att importera till ett nytt Serial Studio-projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="185"/>
        <source>Messages</source>
        <translation>Meddelanden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="219"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="229"/>
        <source>Message Name</source>
        <translation>Meddelandenamn</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="235"/>
        <source>CAN ID</source>
        <translation>CAN-ID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="242"/>
        <source>Signals</source>
        <translation>Signaler</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="323"/>
        <source>No messages found in DBC file.</source>
        <translation>Inga meddelanden hittades i DBC-filen.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="341"/>
        <source>Total: %1 messages, %2 signals</source>
        <translation>Totalt: %1 meddelanden, %2 signaler</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="348"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="359"/>
        <source>Create Project</source>
        <translation>Skapa Projekt</translation>
    </message>
</context>
<context>
    <name>Dashboard</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard.qml" line="127"/>
        <source>Dashboard %1</source>
        <translation>Instrumentpanel %1</translation>
    </message>
</context>
<context>
    <name>DashboardButton</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="40"/>
        <source>Send</source>
        <translation>Skicka</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="64"/>
        <source>No transmit function defined</source>
        <translation>Ingen sändningsfunktion definierad</translation>
    </message>
</context>
<context>
    <name>DashboardCanvas</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="56"/>
        <source>Set Wallpaper…</source>
        <translation>Ställ In Bakgrundsbild…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="62"/>
        <source>Clear Wallpaper</source>
        <translation>Rensa Bakgrundsbild</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="72"/>
        <source>Tile Windows</source>
        <translation>Ordna Fönster</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="91"/>
        <source>Pro features detected in this project.</source>
        <translation>Pro-funktioner upptäckta i detta projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="93"/>
        <source>Fallback widgets are active. Purchase a license for full functionality.</source>
        <translation>Reservwidgetar är aktiva. Köp en licens för full funktionalitet.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="208"/>
        <source>Empty Workspace</source>
        <translation>Tom Arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="222"/>
        <source>Use the search bar to find and add widgets, or right-click a widget in another workspace to add it here.</source>
        <translation>Använd sökfältet för att hitta och lägga till widgetar, eller högerklicka på en widget i en annan arbetsyta för att lägga till den här.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="237"/>
        <source>Search Widgets</source>
        <translation>Sök Widgetar</translation>
    </message>
</context>
<context>
    <name>DashboardLayout</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="37"/>
        <source>Dashboard</source>
        <translation>Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="206"/>
        <source>API Server Active (%1)</source>
        <translation>API-server Aktiv (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="207"/>
        <source>API Server Ready</source>
        <translation>API-server Redo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="208"/>
        <source>API Server Off</source>
        <translation>API-server Av</translation>
    </message>
</context>
<context>
    <name>DashboardOutputPanel</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="123"/>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="275"/>
        <source>Send</source>
        <translation>Skicka</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="263"/>
        <source>Enter command…</source>
        <translation>Ange kommando…</translation>
    </message>
</context>
<context>
    <name>DashboardSlider</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardSlider.qml" line="90"/>
        <source>No transmit function defined</source>
        <translation>Ingen sändningsfunktion definierad</translation>
    </message>
</context>
<context>
    <name>DashboardTextField</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="47"/>
        <source>Enter command…</source>
        <translation>Ange kommando…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="57"/>
        <source>Send</source>
        <translation>Skicka</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="76"/>
        <source>No transmit function defined</source>
        <translation>Ingen sändningsfunktion definierad</translation>
    </message>
</context>
<context>
    <name>DashboardToggle</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="57"/>
        <source>ON</source>
        <translation>PÅ</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="59"/>
        <source>OFF</source>
        <translation>AV</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="70"/>
        <source>No transmit function defined</source>
        <translation>Ingen sändningsfunktion definierad</translation>
    </message>
</context>
<context>
    <name>DataGrid</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="86"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="87"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="86"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="87"/>
        <source>Resume</source>
        <translation>Återuppta</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="297"/>
        <source>Awaiting data…</source>
        <translation>Väntar på data…</translation>
    </message>
</context>
<context>
    <name>DataModel::ControlScriptEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="279"/>
        <source>Select Javascript file to import</source>
        <translation>Välj Javascript-fil att importera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="345"/>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="356"/>
        <source>Code Validation Failed</source>
        <translation>Kodvalidering Misslyckades</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="346"/>
        <source>Line %1: %2</source>
        <translation>Rad %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="357"/>
        <source>The script must define a setup() and/or loop() function.</source>
        <translation>Skriptet måste definiera en setup()- och/eller loop()-funktion.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="362"/>
        <source>Code Validation Successful</source>
        <translation>Kodvalidering Lyckades</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="363"/>
        <source>No syntax errors detected in the control script.</source>
        <translation>Inga syntaxfel upptäcktes i kontrollskriptet.</translation>
    </message>
</context>
<context>
    <name>DataModel::DBCImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="124"/>
        <source>Import DBC File</source>
        <translation>Importera DBC-fil</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="124"/>
        <source>DBC Files (*.dbc);;All Files (*)</source>
        <translation>DBC-filer (*.DBC);;Alla Filer (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="157"/>
        <source>Failed to parse DBC file: %1</source>
        <translation>Misslyckades att tolka DBC-fil: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="158"/>
        <source>Verify the file format and try again.</source>
        <translation>Verifiera filformatet och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="160"/>
        <source>DBC Import Error</source>
        <translation>DBC-importfel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="167"/>
        <source>DBC file contains no messages</source>
        <translation>DBC-filen innehåller inga meddelanden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="168"/>
        <source>The selected file does not contain any CAN message definitions.</source>
        <translation>Den valda filen innehåller inga CAN-meddelandedefinitioner.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="170"/>
        <source>DBC Import Warning</source>
        <translation>DBC-importvarning</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Misslyckades att skapa temporär projektfil</translation>
    </message>
    <message>
        <source>Check if the application has write permissions to the temporary directory.</source>
        <translation type="vanished">Kontrollera om applikationen har skrivbehörighet till den temporära katalogen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="221"/>
        <source>Successfully imported DBC file with %1 messages and %2 signals.</source>
        <translation>DBC-fil importerad med %1 meddelanden och %2 signaler.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="214"/>
        <source>The project editor is now open for customization.</source>
        <translation>Projektredigeraren är nu öppen för anpassning.</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Misslyckades med att ladda importerat projekt</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Den genererade projekt-JSON:en kunde inte laddas.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="216"/>
        <source> Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); only simple multiplexing is supported.</source>
        <translation>Hoppade över %1 signal(er) som använder utökad multiplexering (SG_MUL_VAL_); endast enkel multiplexering stöds.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="226"/>
        <source>DBC Import Complete</source>
        <translation>DBC-import Slutförd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="248"/>
        <source>CAN Bus</source>
        <translation>CAN-buss</translation>
    </message>
</context>
<context>
    <name>DataModel::DatasetTransformEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="59"/>
        <source>Dataset Value Transform</source>
        <translation>Datamängdsvärdetransform</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="96"/>
        <source>Lua</source>
        <translation>Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="96"/>
        <source>JavaScript</source>
        <translation>Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="120"/>
        <source>Language:</source>
        <translation>Språk:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="123"/>
        <source>Template:</source>
        <translation>Mall:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="102"/>
        <source>Enter raw value (e.g., 1024)</source>
        <translation>Ange råvärde (t.ex. 1024)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="107"/>
        <source>Test</source>
        <translation>Testa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="108"/>
        <source>Clear</source>
        <translation>Rensa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="134"/>
        <source>Input:</source>
        <translation>Ingång:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="137"/>
        <source>Output:</source>
        <translation>Utgång:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="110"/>
        <source>Apply</source>
        <translation>Tillämpa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="111"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="215"/>
        <source>Transform — %1</source>
        <translation>Transformering — %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="297"/>
        <source>Enter a value</source>
        <translation>Ange ett värde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="304"/>
        <source>Invalid number</source>
        <translation>Ogiltigt tal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="373"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Formatera Dokument	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="374"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Formatera Markering	ctrl+i</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="473"/>
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
-- Definiera en transform(value)-funktion som tar emot den
-- aktuella datamängdsavläsningen och returnerar ett transformerat
-- tal. Om ingen transform()-funktion definieras behålls råvärdet
-- och ingenting sparas med projektet.
--
-- Exempel:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- Globala variabler som deklareras på toppnivå kvarstår mellan
-- ramar, vilket är användbart för filter, ackumulatorer och andra
-- tillståndsberoende transformeringar, precis som i ett ramtolkningsskript:
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
-- Använd mallmenyn för färdiga exempel, eller klicka på
-- Testa för att prova din funktion.
--
</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="501"/>
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
 * Definiera en transform(value)-funktion som tar emot den
 * aktuella datamängdavläsningen och returnerar ett transformerat
 * tal. Om ingen transform()-funktion definieras behålls råvärdet
 * och inget sparas med projektet.
 *
 * Exempel:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Globala variabler deklarerade på toppnivå kvarstår mellan
 * ramar, vilket är användbart för filter, ackumulatorer och
 * andra tillståndsberoende transformationer -- precis som ett
 * ramtolkningsskript:
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
 * Använd rullgardinsmenyn Mall för färdiga exempel, eller
 * klicka på Testa för att prova din funktion.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="744"/>
        <source>Select Template…</source>
        <translation>Välj Mall…</translation>
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
 * Definiera en transform(value)-funktion som tar emot den
 * aktuella datamängdsavläsningen och returnerar ett
 * transformerat tal. Om ingen transform()-funktion definieras
 * behålls råvärdet och inget sparas med projektet.
 *
 * Exempel:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Globala variabler deklarerade på toppnivå kvarstår mellan
 * ramar, vilket är användbart för filter, ackumulatorer och
 * andra tillståndsberoende transformationer — precis som ett
 * ramtolkningsskript:
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
 * Använd mallrullgardinsmenyn för färdiga exempel, eller
 * klicka på Testa för att prova din funktion.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="635"/>
        <source>Engine error</source>
        <translation>Motorfel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="658"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="671"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="688"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="699"/>
        <source>Error: %1</source>
        <translation>Fel: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="664"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="692"/>
        <source>Error: transform() not defined</source>
        <translation>Fel: transform() ej definierad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="676"/>
        <source>Error: transform() must return a number</source>
        <translation>Fel: transform() måste returnera ett tal</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameBuilder</name>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1470"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1584"/>
        <source>Channel %1</source>
        <translation>Kanal %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1595"/>
        <source>Audio Input</source>
        <translation>Ljudingång</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1479"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1600"/>
        <source>Quick Plot</source>
        <translation>Snabbdiagram</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1253"/>
        <source>JavaScript transform exceeded budget</source>
        <translation>JavaScript-transform överskred budgeten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1254"/>
        <source>A dataset transform took longer than %1 ms; remaining datasets in the frame fell back to raw values until the next frame. Profile or simplify the transform code.</source>
        <translation>En datasettransform tog längre än %1 ms; återstående dataset i framen föll tillbaka till råvärden tills nästa frame. Profilera eller förenkla transformkoden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="199"/>
        <source>Frame pool exhausted</source>
        <translation>Frame-poolen är uttömd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="201"/>
        <source>A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API subscriber) is not draining frames fast enough. Serial Studio is falling back to per-frame allocations until the backlog clears. Disable a heavy consumer or reduce the data rate.</source>
        <translation>En nedströmskonsument (dashboard, CSV/MDF4-export, sessionsdatabas eller API-prenumerant) tömmer inte frames tillräckligt snabbt. Serial Studio faller tillbaka till per-frame-allokeringar tills kön är borta. Inaktivera en tung konsument eller sänk datahastigheten.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1431"/>
        <source>Device A</source>
        <translation>Enhet A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1486"/>
        <source>Quick Plot Data</source>
        <translation>Snabbdiagramdata</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1498"/>
        <source>Multiple Plots</source>
        <translation>Flera Diagram</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserModel</name>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Plain text (UTF-8)</source>
        <translation>Ren text (UTF-8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Hexadecimal</source>
        <translation>Hexadecimal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Binary (raw bytes)</source>
        <translation>Binär (råa byte)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="265"/>
        <source>End delimiter only</source>
        <translation>Endast slutavgränsare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="266"/>
        <source>Start + end delimiters</source>
        <translation>Start- + slutavgränsare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="267"/>
        <source>Start delimiter only</source>
        <translation>Endast startavgränsare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="268"/>
        <source>No delimiters (whole chunk)</source>
        <translation>Inga avgränsare (hela blocket)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="279"/>
        <source>No Checksum</source>
        <translation>Ingen Kontrollsumma</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="310"/>
        <source>Select Frame Parser Template</source>
        <translation>Välj Ramparsersmall</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="311"/>
        <source>Choose a template to load:</source>
        <translation>Välj en mall att ladda:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="494"/>
        <source>Invalid hexadecimal input.</source>
        <translation>Ogiltig hexadecimal inmatning.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="521"/>
        <source>No template selected.</source>
        <translation>Ingen mall vald.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="561"/>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation>%1 ram(ar) extraherade | %2 byte förbrukade | %3 byte buffrade | %4 borttappade</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="632"/>
        <source>Invalid JSON: %1</source>
        <translation>Ogiltig JSON: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="728"/>
        <source>Parameters</source>
        <translation>Parametrar</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserTestDialog</name>
    <message>
        <source>None</source>
        <translation type="vanished">Ingen</translation>
    </message>
    <message>
        <source>Invalid Hex Input</source>
        <translation type="vanished">Ogiltig Hex-ingång</translation>
    </message>
    <message>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation type="vanished">Ange giltiga hexadecimala byte.

Giltigt format: 01 A2 FF 3C</translation>
    </message>
    <message>
        <source>(no frames)</source>
        <translation type="vanished">(inga ramar)</translation>
    </message>
    <message>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation type="vanished">Extraktionen producerade inte en komplett ram. Kontrollera start-/slutavgränsarna och detekteringsläget.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 ram(ar) extraherade | %2 byte förbrukade | %3 byte buffrade | %4 borttappade</translation>
    </message>
    <message>
        <source>Pipeline Configuration</source>
        <translation type="vanished">Pipeline-konfiguration</translation>
    </message>
    <message>
        <source>Pipeline Results</source>
        <translation type="vanished">Pipelineresultat</translation>
    </message>
    <message>
        <source>Detection</source>
        <translation type="vanished">Detektering</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Avkodare</translation>
    </message>
    <message>
        <source>Checksum</source>
        <translation type="vanished">Kontrollsumma</translation>
    </message>
    <message>
        <source>Start Delimiter</source>
        <translation type="vanished">Startavgränsare</translation>
    </message>
    <message>
        <source>End Delimiter</source>
        <translation type="vanished">Slutavgränsare</translation>
    </message>
    <message>
        <source>Hex Delimiters</source>
        <translation type="vanished">Hexadecimala Avgränsare</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Endast slutavgränsare</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Start- + slutavgränsare</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Endast startavgränsare</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Inga avgränsare (hela blocket)</translation>
    </message>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">Oformaterad text (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Hexadecimal</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binär (råa byte)</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">Rensa</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Utvärdera</translation>
    </message>
    <message>
        <source>Enter raw stream bytes here...</source>
        <translation type="vanished">Ange råa strömbyte här...</translation>
    </message>
    <message>
        <source>Stage</source>
        <translation type="vanished">Förbered</translation>
    </message>
    <message>
        <source>Frame Data Input</source>
        <translation type="vanished">Ramdatainmatning</translation>
    </message>
    <message>
        <source>Frame Parser Results</source>
        <translation type="vanished">Ramtolkningsresultat</translation>
    </message>
    <message>
        <source>Enter frame data here…</source>
        <translation type="vanished">Ange ramdata här…</translation>
    </message>
    <message>
        <source>Dataset Index</source>
        <translation type="vanished">Datauppsättningsindex</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="vanished">Värde</translation>
    </message>
    <message>
        <source>Enter frame data above, enable HEX mode if needed, then click "Evaluate" to run the frame parser.

Example (Text): a,b,c,d,e,f
Example (HEX):  48 65 6C 6C 6F</source>
        <translation type="vanished">Ange ramdata ovan, aktivera HEX-läge vid behov och klicka sedan på "Utvärdera" för att köra ramtolken.

Exempel (Text): a,b,c,d,e,f
Exempel (HEX):  48 65 6C 6C 6F</translation>
    </message>
    <message>
        <source>Test Frame Parser</source>
        <translation type="vanished">Testa Ramtolk</translation>
    </message>
    <message>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation type="vanished">Ange hexadecimala bytes (t.ex. 01 A2 FF)</translation>
    </message>
    <message>
        <source>(empty)</source>
        <translation type="vanished">(tom)</translation>
    </message>
    <message>
        <source>No data returned</source>
        <translation type="vanished">Ingen data returnerad</translation>
    </message>
</context>
<context>
    <name>DataModel::JsCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="202"/>
        <source>Change Scripting Language?</source>
        <translation>Byt Skriptspråk?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="203"/>
        <source>Switching the scripting language replaces the current parser code with the equivalent template in the new language.

Any unsaved changes are lost. Continue?</source>
        <translation>Byte av skriptspråk ersätter den aktuella tolkningskoden med motsvarande mall i det nya språket.

Alla osparade ändringar går förlorade. Fortsätt?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="365"/>
        <source>Select Javascript file to import</source>
        <translation>Välj Javascript-fil att importera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="365"/>
        <source>Select Lua file to import</source>
        <translation>Välj Lua-fil att importera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="397"/>
        <source>Code Validation Successful</source>
        <translation>Kodvalidering Lyckades</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="398"/>
        <source>No syntax errors detected in the parser code.</source>
        <translation>Inga syntaxfel upptäcktes i parserkoden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="508"/>
        <source>Select Frame Parser Template</source>
        <translation>Välj Ramparsersmall</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="509"/>
        <source>Choose a template to load:</source>
        <translation>Välj en mall att ladda:</translation>
    </message>
</context>
<context>
    <name>DataModel::ModbusMapImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="296"/>
        <source>Import Modbus Register Map</source>
        <translation>Importera Modbus-registerkarta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="300"/>
        <source>Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files (*.xml);;JSON Files (*.json);;All Files (*)</source>
        <translation>Modbus-registerkartor (*.CSV *.XML *.JSON);;CSV-filer (*.CSV);;XML-filer (*.XML);;JSON-filer (*.JSON);;Alla filer (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="337"/>
        <source>No registers found</source>
        <translation>Inga register hittades</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="338"/>
        <source>The file could not be parsed or contains no register definitions.</source>
        <translation>Filen kunde inte tolkas eller innehåller inga registerdefinitioner.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="340"/>
        <source>Modbus Import</source>
        <translation>Modbus-import</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Misslyckades med att ladda importerat projekt</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Den genererade projekt-JSON:en kunde inte laddas.</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Misslyckades att skapa temporär projektfil</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Kontrollera skrivbehörigheter till den temporära katalogen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="385"/>
        <source>Successfully imported %1 registers in %2 groups.</source>
        <translation>Importerade %1 register i %2 grupper.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="387"/>
        <source>The project editor is now open for customization.</source>
        <translation>Projektredigeraren är nu öppen för anpassning.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="389"/>
        <source>Modbus Import Complete</source>
        <translation>Modbus-import Slutförd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="718"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
</context>
<context>
    <name>DataModel::NativeParserEditor</name>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">Ren text (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Hexadecimal</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binär (råa bytes)</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Endast slutavgränsare</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Start- + slutavgränsare</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Endast startavgränsare</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Inga avgränsare (hela blocket)</translation>
    </message>
    <message>
        <source>No Checksum</source>
        <translation type="vanished">Ingen Kontrollsumma</translation>
    </message>
    <message>
        <source>Invalid hexadecimal input.</source>
        <translation type="vanished">Ogiltig hexadecimal inmatning.</translation>
    </message>
    <message>
        <source>No template selected.</source>
        <translation type="vanished">Ingen mall vald.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 ram(ar) extraherade | %2 byte förbrukade | %3 byte buffrade | %4 borttappade</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation type="vanished">Parametrar</translation>
    </message>
</context>
<context>
    <name>DataModel::OutputCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="283"/>
        <source>Select Javascript file to import</source>
        <translation>Välj Javascript-fil att importera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="341"/>
        <source>Select Output Widget Template</source>
        <translation>Välj Mall för Utgångswidget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="342"/>
        <source>Choose a template to load:</source>
        <translation>Välj en mall att ladda:</translation>
    </message>
</context>
<context>
    <name>DataModel::PainterCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="295"/>
        <source>Select Javascript file to import</source>
        <translation>Välj Javascript-fil att importera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="382"/>
        <source>Select Painter Widget Template</source>
        <translation>Välj Mall för Painter-widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="383"/>
        <source>Choose a template to load:</source>
        <translation>Välj en mall att ladda:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="427"/>
        <source>Add datasets for this template?</source>
        <translation>Lägg till dataset för denna mall?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="428"/>
        <source>"%1" expects %2 dataset(s); the current group has %3.

Add %4 dataset(s) using the template's defaults?</source>
        <translation>"%1" förväntar sig %2 dataset; den aktuella gruppen har %3.

Lägg till %4 dataset med mallens standardvärden?</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectEditor</name>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2169"/>
        <source>Project Information</source>
        <translation>Projektinformation</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2179"/>
        <source>Project Title</source>
        <translation>Projekttitel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2180"/>
        <source>Untitled Project</source>
        <translation>Namnlöst Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2181"/>
        <source>Name or description of the project</source>
        <translation>Namn eller beskrivning av projektet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2404"/>
        <source>Frame Detection</source>
        <translation>Ramdetektering</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2418"/>
        <source>Frame Detection Method</source>
        <translation>Ramdetekteringsmetod</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2419"/>
        <source>Select how incoming data frames are identified</source>
        <translation>Välj hur inkommande dataramar identifieras</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2429"/>
        <source>Hexadecimal Delimiters</source>
        <translation>Hexadecimala Avgränsare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2430"/>
        <source>Enter frame start/end sequences as hexadecimal values</source>
        <translation>Ange ramstart-/slutsekvenser som hexadecimala värden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2446"/>
        <source>Frame Start Delimiter</source>
        <translation>Ramstartavgränsare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2447"/>
        <source>e.g. /*</source>
        <translation>t.ex. /*</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2448"/>
        <source>Sequence that marks the beginning of a data frame</source>
        <translation>Sekvens som markerar början av en dataram</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2460"/>
        <source>Frame End Delimiter</source>
        <translation>Ramslutavgränsare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2461"/>
        <source>e.g. */</source>
        <translation>t.ex. */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2462"/>
        <source>Sequence that marks the end of a data frame</source>
        <translation>Sekvens som markerar slutet på en dataram</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2468"/>
        <source>Payload Processing &amp; Validation</source>
        <translation>Nyttolastbearbetning &amp; Validering</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2479"/>
        <source>Data Conversion Method</source>
        <translation>Datakonverteringsmetod</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2480"/>
        <source>Select how incoming binary data is decoded before parsing</source>
        <translation>Välj hur inkommande binärdata avkodas före tolkning</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2496"/>
        <source>Checksum Algorithm</source>
        <translation>Kontrollsummealgoritm</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2497"/>
        <source>Select the checksum algorithm used to validate frames</source>
        <translation>Välj den kontrollsummealgoritm som används för att validera ramar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2199"/>
        <source>Group Information</source>
        <translation>Gruppinformation</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2209"/>
        <source>Group Title</source>
        <translation>Grupptitel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2210"/>
        <source>Untitled Group</source>
        <translation>Namnlös Grupp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2211"/>
        <source>Title or description of this dataset group</source>
        <translation>Titel eller beskrivning av denna datauppsättningsgrupp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2342"/>
        <source>Composite Widget</source>
        <translation>Sammansatt Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2343"/>
        <source>Select how this group of datasets should be visualized (optional)</source>
        <translation>Välj hur denna grupp av datauppsättningar ska visualiseras (valfritt)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2261"/>
        <source>Image Configuration</source>
        <translation>Bildkonfiguration</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3076"/>
        <source>Virtual Dataset</source>
        <translation>Virtuell Datauppsättning</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3077"/>
        <source>Virtual datasets compute their value from transforms and data tables, they do not require a frame index</source>
        <translation>Virtuella datauppsättningar beräknar sitt värde från transformeringar och datatabeller, de kräver inget ramindex</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3566"/>
        <source>Auto-detect</source>
        <translation>Autodetektera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3566"/>
        <source>Manual Delimiters</source>
        <translation>Manuella Avgränsare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2274"/>
        <source>Detection Mode</source>
        <translation>Detekteringsläge</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1289"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1292"/>
        <source>Frame Parser</source>
        <translation>Frame-parser</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1431"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1432"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1474"/>
        <source>Groups</source>
        <translation>Grupper</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1504"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1517"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1518"/>
        <source>Shared Memory</source>
        <translation>Delat Minne</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1504"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1523"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1524"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4929"/>
        <source>Dataset Values</source>
        <translation>Datamängdsvärden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1566"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1579"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1580"/>
        <source>Workspaces</source>
        <translation>Arbetsytor</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1618"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1621"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1622"/>
        <source>MQTT Publisher</source>
        <translation>MQTT-publicerare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1642"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1645"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1646"/>
        <source>Control Script</source>
        <translation>Kontrollskript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1695"/>
        <source>Publishing</source>
        <translation>Publicering</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1705"/>
        <source>Enable Publishing</source>
        <translation>Aktivera Publicering</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1706"/>
        <source>Broadcast frames, raw bytes and notifications to the broker</source>
        <translation>Sända ramar, råbytes och notifieringar till broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1717"/>
        <source>Payload</source>
        <translation>Nyttolast</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1718"/>
        <source>Selects what gets published: parsed dashboard data or raw RX bytes</source>
        <translation>Väljer vad som publiceras: tolkad instrumentpanelsdata eller råa RX-bytes</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1728"/>
        <source>Publish Rate (Hz)</source>
        <translation>Publiceringsfrekvens (Hz)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1729"/>
        <source>How many times per second to publish (1-30 Hz). Higher rates increase broker load; dashboard data is rate-limited so a slow broker never blocks frame parsing.</source>
        <translation>Hur många gånger per sekund som ska publiceras (1-30 Hz). Högre frekvenser ökar belastningen på broker; instrumentpanelens data är hastighetsbegränsad så en långsam broker blockerar aldrig frame-parsning.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1741"/>
        <source>Topic Base</source>
        <translation>Ämnesbas</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1742"/>
        <source>serial-studio/device</source>
        <translation>serial-studio/enhet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1743"/>
        <source>Base topic used for frame and raw-byte publishing</source>
        <translation>Basämne som används för publicering av ramar och råbytes</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1753"/>
        <source>Script Topic</source>
        <translation>Skriptämne</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1754"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1778"/>
        <source>Defaults to Topic Base when empty</source>
        <translation>Använder Basämne som standard när tomt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1755"/>
        <source>Topic the user script publishes to</source>
        <translation>Ämne som användarskriptet publicerar till</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1765"/>
        <source>Publish Notifications</source>
        <translation>Publicera Notifieringar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1766"/>
        <source>Mirror dashboard notifications to a dedicated topic</source>
        <translation>Spegla instrumentpanelens notifieringar till ett dedikerat ämne</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1777"/>
        <source>Notification Topic</source>
        <translation>Notifieringsämne</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1779"/>
        <source>Topic where dashboard notifications are mirrored</source>
        <translation>Ämne där instrumentpanelens notifieringar speglas</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1792"/>
        <source>Broker</source>
        <translation>Broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1802"/>
        <source>Hostname</source>
        <translation>Värdnamn</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1803"/>
        <source>broker.hivemq.com</source>
        <translation>broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1804"/>
        <source>Hostname or IP address of the MQTT broker</source>
        <translation>Värdnamn eller IP-adress för MQTT-broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1813"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1814"/>
        <source>TCP port exposed by the broker (1883 plain, 8883 TLS)</source>
        <translation>TCP-port som exponeras av broker (1883 okrypterad, 8883 TLS)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1824"/>
        <source>Custom Client ID</source>
        <translation>Anpassat Klient-ID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1826"/>
        <source>Off: a fresh random id is generated on every project load. On: use the id below.</source>
        <translation>Av: ett nytt slumpmässigt ID genereras vid varje projektladdning. På: använd ID:t nedan.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1837"/>
        <source>Client ID</source>
        <translation>Klient-ID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1838"/>
        <source>Identifier sent to the broker on CONNECT</source>
        <translation>Identifierare som skickas till broker vid CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1851"/>
        <source>Protocol Version</source>
        <translation>Protokollversion</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1852"/>
        <source>MQTT protocol revision used on CONNECT</source>
        <translation>MQTT-protokollrevision som används vid CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1861"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1862"/>
        <source>Seconds between PINGREQ packets when idle</source>
        <translation>Sekunder mellan PINGREQ-paket vid inaktivitet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1871"/>
        <source>Clean Session</source>
        <translation>Rensa Session</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1872"/>
        <source>Discard any persistent session state on CONNECT</source>
        <translation>Kassera eventuellt beständigt sessionstillstånd vid CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1887"/>
        <source>Username</source>
        <translation>Användarnamn</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1888"/>
        <source>Username for broker authentication (leave empty for anonymous)</source>
        <translation>Användarnamn för broker-autentisering (lämna tomt för anonym)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1898"/>
        <source>Password</source>
        <translation>Lösenord</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1899"/>
        <source>Password for broker authentication</source>
        <translation>Lösenord för broker-autentisering</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1910"/>
        <source>SSL / TLS</source>
        <translation>SSL / TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1920"/>
        <source>Use SSL/TLS</source>
        <translation>Använd SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1921"/>
        <source>Tunnel the broker connection over TLS</source>
        <translation>Tunnla broker-anslutningen över TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1934"/>
        <source>Protocol</source>
        <translation>Protokoll</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1935"/>
        <source>Negotiated TLS protocol family</source>
        <translation>Förhandlad TLS-protokollfamilj</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1945"/>
        <source>Peer Verify</source>
        <translation>Verifiera Motpart</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1946"/>
        <source>How strictly the broker's certificate chain is validated</source>
        <translation>Hur strikt brokerns certifikatkedja valideras</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1956"/>
        <source>Verify Depth</source>
        <translation>Verifieringsdjup</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1957"/>
        <source>Maximum certificate chain length accepted (0 = unlimited)</source>
        <translation>Maximal accepterad certifikatkedjelängd (0 = obegränsat)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2226"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Device %1</source>
        <translation>Enhet %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2244"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2363"/>
        <source>Input Device</source>
        <translation>Ingångsenhet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2245"/>
        <source>Select which connected device provides data for this group</source>
        <translation>Välj vilken ansluten enhet som tillhandahåller data för denna grupp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2276"/>
        <source>Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences</source>
        <translation>Auto-detektering läser JPEG/PNG magic bytes; Manuell använder explicita start-/slutsekvenser</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2286"/>
        <source>Start Sequence (Hex)</source>
        <translation>Startsekvens (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2287"/>
        <source>e.g. FF D8 FF</source>
        <translation>t.ex. FF D8 FF</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2288"/>
        <source>Hex bytes marking the start of an image frame</source>
        <translation>Hexbytes som markerar början av en bildram</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2297"/>
        <source>End Sequence (Hex)</source>
        <translation>Slutsekvens (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2298"/>
        <source>e.g. FF D9</source>
        <translation>t.ex. FF D9</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2299"/>
        <source>Hex bytes marking the end of an image frame</source>
        <translation>Hexbytes som markerar slutet av en bildram</translation>
    </message>
    <message>
        <source>Identity</source>
        <translation type="vanished">Identitet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2373"/>
        <source>Device Name</source>
        <translation>Enhetsnamn</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2374"/>
        <source>Device 1</source>
        <translation>Enhet 1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2375"/>
        <source>Human-readable name for this input device</source>
        <translation>Läsbart namn för denna ingångsenhet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2384"/>
        <source>Bus Type</source>
        <translation>Busstyp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2385"/>
        <source>Select the hardware interface for this input device</source>
        <translation>Välj hårdvarugränssnitt för denna ingångsenhet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2387"/>
        <source>Serial Port</source>
        <translation>Seriell Port</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2387"/>
        <source>Network Socket</source>
        <translation>Nätverksuttag</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2387"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2389"/>
        <source>Audio Input</source>
        <translation>Ljudingång</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2389"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2389"/>
        <source>CAN Bus</source>
        <translation>CAN-buss</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2389"/>
        <source>Raw USB</source>
        <translation>Rå USB</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2390"/>
        <source>HID Device</source>
        <translation>HID-enhet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2390"/>
        <source>Process</source>
        <translation>Process</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2390"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT-prenumerant</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2545"/>
        <source>Connection Settings</source>
        <translation>Anslutningsinställningar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2782"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3052"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4625"/>
        <source>General Information</source>
        <translation>Allmän Information</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2791"/>
        <source>Action Title</source>
        <translation>Åtgärdstitel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2793"/>
        <source>Untitled Action</source>
        <translation>Namnlös Åtgärd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2794"/>
        <source>Name or description of this action</source>
        <translation>Namn eller beskrivning av denna åtgärd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2803"/>
        <source>Action Icon</source>
        <translation>Åtgärdsikon</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2804"/>
        <source>Default Icon</source>
        <translation>Standardikon</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2805"/>
        <source>Icon displayed for this action in the dashboard</source>
        <translation>Ikon som visas för denna åtgärd i instrumentpanelen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2832"/>
        <source>Target Device</source>
        <translation>Målenhet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2833"/>
        <source>Select which connected device this action sends data to</source>
        <translation>Välj vilken ansluten enhet denna åtgärd skickar data till</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2845"/>
        <source>Data Payload</source>
        <translation>Datalast</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2856"/>
        <source>Send as Binary</source>
        <translation>Skicka som Binär</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2857"/>
        <source>Send raw binary data when this action is triggered</source>
        <translation>Skicka rå binärdata när denna åtgärd utlöses</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2868"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2880"/>
        <source>Command</source>
        <translation>Kommando</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2869"/>
        <source>Transmit Data (Hex)</source>
        <translation>Överför Data (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2870"/>
        <source>Hexadecimal payload to send when the action is triggered</source>
        <translation>Hexadecimal last att skicka när åtgärden utlöses</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2881"/>
        <source>Transmit Data</source>
        <translation>Överför Data</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2882"/>
        <source>Text payload to send when the action is triggered</source>
        <translation>Textlast att skicka när åtgärden utlöses</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2893"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4684"/>
        <source>Text Encoding</source>
        <translation>Textkodning</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2894"/>
        <source>Character encoding used to serialize the text payload</source>
        <translation>Teckenkodning som används för att serialisera textlasten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2918"/>
        <source>End-of-Line Sequence</source>
        <translation>Radslutsekvens</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2919"/>
        <source>EOL characters to append to the message (e.g. \n, \r\n)</source>
        <translation>Radsluttecken att lägga till meddelandet (t.ex. </translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2931"/>
        <source>Execution Behavior</source>
        <translation>Körningsbeteende</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2942"/>
        <source>Auto-Execute on Connect</source>
        <translation>Autokör vid Anslutning</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2943"/>
        <source>Automatically trigger this action when the device connects</source>
        <translation>Utlös denna åtgärd automatiskt när enheten ansluter</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2949"/>
        <source>Timer Behavior</source>
        <translation>Timerbeteende</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2958"/>
        <source>Timer Mode</source>
        <translation>Timerläge</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2961"/>
        <source>Choose when and how this action should repeat automatically</source>
        <translation>Välj när och hur denna åtgärd ska upprepas automatiskt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2968"/>
        <source>Interval (ms)</source>
        <translation>Intervall (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2972"/>
        <source>Timer Interval (ms)</source>
        <translation>Timerintervall (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2973"/>
        <source>Milliseconds between each repeated trigger of this action</source>
        <translation>Millisekunder mellan varje upprepad utlösning av denna åtgärd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2980"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2984"/>
        <source>Repeat Count</source>
        <translation>Upprepningsantal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2985"/>
        <source>Number of times to send the command on each trigger</source>
        <translation>Antal gånger kommandot ska skickas vid varje utlösning</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3062"/>
        <source>Untitled Dataset</source>
        <translation>Namnlös Datamängd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3063"/>
        <source>Dataset Title</source>
        <translation>Datamängdstitel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3064"/>
        <source>Name of the dataset, used for labeling and identification</source>
        <translation>Namn på datamängden, används för märkning och identifiering</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3093"/>
        <source>Hide on Dashboard</source>
        <translation>Dölj på Instrumentpanelen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3094"/>
        <source>Suppress this dataset's standalone dashboard tile; the painter widget can still read its values</source>
        <translation>Undertryck detta datasets fristående instrumentpanelspanel; painter-widgeten kan fortfarande läsa dess värden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3140"/>
        <source>Lower bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Nedre gräns för datauppsättningens värdeområde; widgets och FFT använder detta som standard när deras eget område inte är angivet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3153"/>
        <source>Upper bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Övre gräns för datauppsättningens värdeområde; widgets och FFT använder detta som standard när deras eget område inte är angivet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3211"/>
        <source>Choose Time or a dataset to drive the X-Axis in plots</source>
        <translation>Välj Tid eller en datauppsättning för att styra X-axeln i diagram</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3224"/>
        <source>Frequency Analysis</source>
        <translation>Frekvensanalys</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3271"/>
        <source>Choose Time (default) or any dataset whose value drives the Y axis -- produces a Campbell diagram when bound to e.g. RPM</source>
        <translation>Välj Tid (standard) eller valfri datamängd vars värde styr Y-axeln -- producerar ett Campbell-diagram när det binds till t.ex. RPM</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3321"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3410"/>
        <source>Minimum Value (optional)</source>
        <translation>Minimivärde (valfritt)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3322"/>
        <source>Lower bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Nedre gräns för datanormalisering; återgår till datasetets värdeområde om den lämnas odefinierad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3334"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3423"/>
        <source>Maximum Value (optional)</source>
        <translation>Maximivärde (valfritt)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3335"/>
        <source>Upper bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Övre gräns för datanormalisering; återgår till datasetets värdeområde om den lämnas odefinierad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3411"/>
        <source>Lower bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Nedre gräns för mätare- eller stapelområdet; återgår till datasetets värdeområde om den lämnas odefinierad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3424"/>
        <source>Upper bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Övre gräns för mätare- eller stapelområdet; återgår till datasetets värdeområde om den lämnas odefinierad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3581"/>
        <source>Painter Widget</source>
        <translation>Painter-widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4930"/>
        <source>Raw and transformed values for every dataset (read-only)</source>
        <translation>Råa och transformerade värden för varje datamängd (skrivskyddad)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4939"/>
        <source>Shared table defined in this project</source>
        <translation>Delad tabell definierad i detta projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5286"/>
        <source>Remove 1 widget reference whose target group or dataset no longer exists?</source>
        <translation>Ta bort 1 widgetreferens vars målgrupp eller dataset inte längre finns?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5287"/>
        <source>Remove %1 widget references whose target groups or datasets no longer exist?</source>
        <translation>Ta bort %1 widgetreferenser vars målgrupper eller dataset inte längre finns?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5292"/>
        <source>This will only affect workspace tile placement; no groups, datasets, or data are deleted.</source>
        <translation>Detta påverkar endast placering av arbetsytepaneler; inga grupper, dataset eller data tas bort.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5295"/>
        <source>Clean Up Workspaces</source>
        <translation>Rensa Arbetsytor</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3107"/>
        <source>Frame Index</source>
        <translation>Ramindex</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3108"/>
        <source>Frame position used for aligning datasets in time</source>
        <translation>Ramposition som används för att tidsjustera datamängder</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3117"/>
        <source>Measurement Unit</source>
        <translation>Mätenhet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3118"/>
        <source>Volts, Amps, etc.</source>
        <translation>Volt, Ampere, etc.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3119"/>
        <source>Unit of measurement, such as volts or amps (optional)</source>
        <translation>Mätenhet, såsom volt eller ampere (valfritt)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3166"/>
        <source>Plot Settings</source>
        <translation>Diagraminställningar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3189"/>
        <source>Enable Plot Widget</source>
        <translation>Aktivera Diagramwidget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3191"/>
        <source>Plot data in real-time</source>
        <translation>Rita data i realtid</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3210"/>
        <source>X-Axis Source</source>
        <translation>X-axelkälla</translation>
    </message>
    <message>
        <source>Choose which dataset to use for the X-Axis in plots</source>
        <translation type="vanished">Välj vilken dataset som ska användas för X-axeln i diagram</translation>
    </message>
    <message>
        <source>Minimum Plot Value (optional)</source>
        <translation type="vanished">Minsta Diagramvärde (valfritt)</translation>
    </message>
    <message>
        <source>Lower bound for plot display range</source>
        <translation type="vanished">Nedre gräns för diagrammets visningsområde</translation>
    </message>
    <message>
        <source>Maximum Plot Value (optional)</source>
        <translation type="vanished">Maximalt Plotvärde (valfritt)</translation>
    </message>
    <message>
        <source>Upper bound for plot display range</source>
        <translation type="vanished">Övre gräns för plottens visningsområde</translation>
    </message>
    <message>
        <source>FFT Configuration</source>
        <translation type="vanished">FFT-konfiguration</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3235"/>
        <source>Enable FFT Analysis</source>
        <translation>Aktivera FFT-analys</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3236"/>
        <source>Perform frequency-domain analysis of the dataset</source>
        <translation>Utför frekvensdomänanalys av datasetet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3246"/>
        <source>Enable Waterfall Plot</source>
        <translation>Aktivera Vattenfallsdiagram</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3247"/>
        <source>Show a scrolling spectrogram of frequency content over time (Pro)</source>
        <translation>Visa ett rullande spektrogram av frekvensinnehåll över tid (Pro)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3270"/>
        <source>Waterfall Y Axis</source>
        <translation>Vattenfall Y-axel</translation>
    </message>
    <message>
        <source>Choose Time (default) or any dataset whose value drives the Y axis — produces a Campbell diagram when bound to e.g. RPM</source>
        <translation type="vanished">Välj Tid (standard) eller valfri datamängd vars värde styr Y-axeln — producerar ett Campbell-diagram när det binds till t.ex. RPM</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3298"/>
        <source>FFT Window Size</source>
        <translation>FFT-fönsterstorlek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3299"/>
        <source>Number of samples used for each FFT calculation window</source>
        <translation>Antal sampel som används för varje FFT-beräkningsfönster</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3310"/>
        <source>FFT Sampling Rate (Hz, required)</source>
        <translation>FFT-samplingsfrekvens (Hz, krävs)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3311"/>
        <source>Sampling frequency used for FFT (in Hz)</source>
        <translation>Samplingsfrekvens som används för FFT (i Hz)</translation>
    </message>
    <message>
        <source>Minimum Value (recommended)</source>
        <translation type="vanished">Minimivärde (rekommenderas)</translation>
    </message>
    <message>
        <source>Lower bound for data normalization</source>
        <translation type="vanished">Nedre gräns för datanormalisering</translation>
    </message>
    <message>
        <source>Maximum Value (recommended)</source>
        <translation type="vanished">Maximalt Värde (rekommenderat)</translation>
    </message>
    <message>
        <source>Upper bound for data normalization</source>
        <translation type="vanished">Övre gräns för datanormalisering</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3360"/>
        <source>Widget Settings</source>
        <translation>Widget-inställningar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3383"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3384"/>
        <source>Select the visual widget used to display this dataset</source>
        <translation>Välj den visuella widget som används för att visa detta dataset</translation>
    </message>
    <message>
        <source>Minimum Display Value (required)</source>
        <translation type="vanished">Minsta Visningsvärde (obligatoriskt)</translation>
    </message>
    <message>
        <source>Lower bound of the gauge or bar display range</source>
        <translation type="vanished">Nedre gräns för mätarens eller stapelns visningsområde</translation>
    </message>
    <message>
        <source>Maximum Display Value (required)</source>
        <translation type="vanished">Högsta Visningsvärde (obligatoriskt)</translation>
    </message>
    <message>
        <source>Upper bound of the gauge or bar display range</source>
        <translation type="vanished">Övre gräns för mätarens eller stapelns visningsområde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3440"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3592"/>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3441"/>
        <source>Tick Count</source>
        <translation>Antal Markeringar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3445"/>
        <source>Major-tick count on the dial scale (0 = auto-fit to widget size)</source>
        <translation>Antal huvudmarkeringar på urtavlans skala (0 = anpassa automatiskt till widgetens storlek)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3464"/>
        <source>Label Format</source>
        <translation>Etikettformat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3465"/>
        <source>Decimal places or notation used on tick labels and the value display</source>
        <translation>Decimaler eller notation som används på markeringsetiketter och värdevisningen</translation>
    </message>
    <message>
        <source>Show Value Indicator</source>
        <translation type="vanished">Visa Värdeindikator</translation>
    </message>
    <message>
        <source>Toggle the boxed numeric readout that sits below or beside the widget</source>
        <translation type="vanished">Växla den inramade numeriska avläsningen som sitter under eller bredvid widgeten</translation>
    </message>
    <message>
        <source>Alarm Settings</source>
        <translation type="vanished">Larminställningar</translation>
    </message>
    <message>
        <source>Enable Alarms</source>
        <translation type="vanished">Aktivera Larm</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds alarm thresholds</source>
        <translation type="vanished">Utlöser ett visuellt larm när värdet överskrider larmtrösklarna</translation>
    </message>
    <message>
        <source>Low Threshold</source>
        <translation type="vanished">Lågt Tröskelvärde</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value drops below this threshold</source>
        <translation type="vanished">Utlöser ett visuellt larm när värdet sjunker under detta tröskelvärde</translation>
    </message>
    <message>
        <source>High Threshold</source>
        <translation type="vanished">Högt Tröskelvärde</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds this threshold</source>
        <translation type="vanished">Utlöser ett visuellt larm när värdet överskrider detta tröskelvärde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3501"/>
        <source>LED Display Settings</source>
        <translation>Inställningar för LED-display</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3512"/>
        <source>Show in LED Panel</source>
        <translation>Visa i LED-Panel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3513"/>
        <source>Enable visual status monitoring using an LED display</source>
        <translation>Aktivera visuell statusövervakning med en LED-display</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3524"/>
        <source>LED On Threshold (required)</source>
        <translation>LED-tröskelvärde för På (obligatoriskt)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3525"/>
        <source>LED lights up when value meets or exceeds this threshold</source>
        <translation>LED lyser när värdet når eller överskrider detta tröskelvärde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3544"/>
        <source>Off</source>
        <translation>Av</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3544"/>
        <source>Auto Start</source>
        <translation>Starta Automatiskt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3544"/>
        <source>Start on Trigger</source>
        <translation>Starta vid Trigger</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3544"/>
        <source>Toggle on Trigger</source>
        <translation>Växla vid Trigger</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3545"/>
        <source>Repeat N Times</source>
        <translation>Upprepa N Gånger</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3548"/>
        <source>Plain Text (UTF8)</source>
        <translation>Oformaterad Text (UTF8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3548"/>
        <source>Hexadecimal</source>
        <translation>Hexadecimal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3548"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3549"/>
        <source>Binary (Direct)</source>
        <translation>Binär (Direkt)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3554"/>
        <source>No Checksum</source>
        <translation>Ingen Kontrollsumma</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3558"/>
        <source>End Delimiter Only</source>
        <translation>Endast Slutavgränsare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3558"/>
        <source>Start Delimiter Only</source>
        <translation>Endast Startavgränsare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3559"/>
        <source>Start + End Delimiter</source>
        <translation>Start- + Slutavgränsare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3559"/>
        <source>No Delimiters</source>
        <translation>Inga Avgränsare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3569"/>
        <source>Button</source>
        <translation>Knapp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3569"/>
        <source>Slider</source>
        <translation>Skjutreglage</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3569"/>
        <source>Toggle</source>
        <translation>Växlare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3569"/>
        <source>Text Field</source>
        <translation>Textfält</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3570"/>
        <source>Knob</source>
        <translation>Ratt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3574"/>
        <source>Data Grid</source>
        <translation>Datarutnät</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3575"/>
        <source>GPS Map</source>
        <translation>GPS-karta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3576"/>
        <source>Gyroscope</source>
        <translation>Gyroskop</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3577"/>
        <source>Multiple Plot</source>
        <translation>Flerplott</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3578"/>
        <source>Accelerometer</source>
        <translation>Accelerometer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3579"/>
        <source>3D Plot</source>
        <translation>3D-plott</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3580"/>
        <source>Image View</source>
        <translation>Bildvisning</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3582"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3585"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3600"/>
        <source>None</source>
        <translation>Ingen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3586"/>
        <source>Bar</source>
        <translation>Stapel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3587"/>
        <source>Gauge</source>
        <translation>Mätare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3588"/>
        <source>Compass</source>
        <translation>Kompass</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3589"/>
        <source>Meter</source>
        <translation>Mätare</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3593"/>
        <source>Integer (0 decimals)</source>
        <translation>Heltal (0 decimaler)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3594"/>
        <source>1 decimal</source>
        <translation>1 decimal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3595"/>
        <source>2 decimals</source>
        <translation>2 decimaler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3596"/>
        <source>3 decimals</source>
        <translation>3 decimaler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3597"/>
        <source>Scientific</source>
        <translation>Vetenskaplig</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3601"/>
        <source>New Line (\n)</source>
        <translation>Ny Rad (</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3602"/>
        <source>Carriage Return (\r)</source>
        <translation>Vagnretur (\r)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3603"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3606"/>
        <source>No</source>
        <translation>Nej</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3607"/>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4635"/>
        <source>Label</source>
        <translation>Etikett</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4636"/>
        <source>Display label</source>
        <translation>Visa Etikett</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4646"/>
        <source>Button Icon</source>
        <translation>Knappikon</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4655"/>
        <source>Colorize Icon</source>
        <translation>Färglägg Ikon</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4656"/>
        <source>Tint the icon with the button color</source>
        <translation>Tona ikonen med knappfärgen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4673"/>
        <source>Initial Value</source>
        <translation>Initialt Värde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4685"/>
        <source>Character encoding used when transmit() returns a string value</source>
        <translation>Teckenkodning som används när transmit() returnerar ett strängvärde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4703"/>
        <source>Value Range</source>
        <translation>Värdeområde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3139"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4713"/>
        <source>Minimum Value</source>
        <translation>Minimivärde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3152"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4722"/>
        <source>Maximum Value</source>
        <translation>Maximivärde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4731"/>
        <source>Step Size</source>
        <translation>Stegstorlek</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectModel</name>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="517"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="526"/>
        <source>Lock Project</source>
        <translation>Lås Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="518"/>
        <source>Choose a password to lock the project:</source>
        <translation>Välj ett lösenord för att låsa projektet:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="526"/>
        <source>Confirm the password:</source>
        <translation>Bekräfta lösenordet:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="531"/>
        <source>Passwords do not match</source>
        <translation>Lösenorden stämmer inte överens</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="532"/>
        <source>The two passwords you entered do not match. The project was not locked.</source>
        <translation>De två lösenorden du angav stämmer inte överens. Projektet låstes inte.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="566"/>
        <source>Unlock Project</source>
        <translation>Lås Upp Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="567"/>
        <source>Enter the project password:</source>
        <translation>Ange projektlösenordet:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="577"/>
        <source>Incorrect password</source>
        <translation>Felaktigt lösenord</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="578"/>
        <source>The password you entered does not match the one stored in the project file.</source>
        <translation>Lösenordet du angav stämmer inte överens med det som finns lagrat i projektfilen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="609"/>
        <source>New Project</source>
        <translation>Nytt Projekt</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">Sampel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1211"/>
        <source>Multiple data sources require a Pro license</source>
        <translation>Flera datakällor kräver en Pro-licens</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1212"/>
        <source>Serial Studio Pro allows connecting to multiple devices simultaneously. Please upgrade to unlock this feature.</source>
        <translation>Serial Studio Pro tillåter anslutning till flera enheter samtidigt. Uppgradera för att låsa upp denna funktion.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1224"/>
        <source>Device %1</source>
        <translation>Enhet %1</translation>
    </message>
    <message>
        <source> (Copy)</source>
        <translation type="vanished">(Kopia)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1487"/>
        <source>Do you want to save your changes?</source>
        <translation>Vill du spara dina ändringar?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1488"/>
        <source>You have unsaved modifications in this project!</source>
        <translation>Du har osparade ändringar i detta projekt!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="389"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="398"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="411"/>
        <source>Project error</source>
        <translation>Projektfel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="389"/>
        <source>Project title cannot be empty!</source>
        <translation>Projekttitel kan inte vara tom!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="398"/>
        <source>You need to add at least one group!</source>
        <translation>Du måste lägga till minst en grupp!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="411"/>
        <source>You need to add at least one dataset!</source>
        <translation>Du måste lägga till minst en datamängd!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="458"/>
        <source>Your project needs a title</source>
        <translation>Ditt projekt behöver en titel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="460"/>
        <source>Add a group to get started</source>
        <translation>Lägg till en grupp för att komma igång</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="462"/>
        <source>Add a dataset to a group</source>
        <translation>Lägg till ett dataset till en grupp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="476"/>
        <source>Open the Project view at the top of the tree and enter a name. You can rename the project at any time.</source>
        <translation>Öppna Projektvyn högst upp i trädet och ange ett namn. Du kan byta namn på projektet när som helst.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="479"/>
        <source>Groups organize datasets into dashboard widgets. Use the Group button in the toolbar above to create one, then add datasets to it.</source>
        <translation>Grupper organiserar dataset i instrumentpanelswidgetar. Använd Grupp-knappen i verktygsfältet ovan för att skapa en, lägg sedan till dataset till den.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="483"/>
        <source>Datasets are the values that appear on the dashboard. Select a group in the tree and use the Dataset button in the toolbar to add one.</source>
        <translation>Dataset är de värden som visas på instrumentpanelen. Välj en grupp i trädet och använd Dataset-knappen i verktygsfältet för att lägga till ett.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="761"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="808"/>
        <source>Time</source>
        <translation>Tid</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1256"/>
        <source>Do you want to delete data source "%1"?</source>
        <translation>Vill du ta bort datakälla "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1257"/>
        <source>Groups using this source will move to the default source. This action cannot be undone.</source>
        <translation>Grupper som använder denna källa flyttas till standardkällan. Denna åtgärd kan inte ångras.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1527"/>
        <source>Save Serial Studio Project</source>
        <translation>Spara Serial Studio-projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1529"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2149"/>
        <source>Serial Studio Project Files (*.ssproj)</source>
        <translation>Serial Studio-projektfiler (*.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1550"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1769"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2140"/>
        <source>Untitled Project</source>
        <translation>Namnlöst Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1781"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2295"/>
        <source>Device A</source>
        <translation>Enhet A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1979"/>
        <source>Select Project File</source>
        <translation>Välj Projektfil</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1981"/>
        <source>Project Files (*.json *.ssproj)</source>
        <translation>Projektfiler (*.json *.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2025"/>
        <source>JSON validation error</source>
        <translation>JSON-valideringsfel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2114"/>
        <source>Project upgraded from an earlier file format</source>
        <translation>Projekt uppgraderat från ett tidigare filformat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2115"/>
        <source>This project was saved with schema version %1; the current version is %2. Defaults have been applied to any new fields. Save the project to lock in the upgrade.</source>
        <translation>Det här projektet sparades med schemaversion %1; den aktuella versionen är %2. Standardvärden har tillämpats på nya fält. Spara projektet för att låsa uppgraderingen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2147"/>
        <source>Save Imported Project</source>
        <translation>Spara Importerat Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2339"/>
        <source>Multi-source projects require a Pro license</source>
        <translation>Projekt med flera källor kräver Pro-licens</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2340"/>
        <source>This project contains multiple data sources. Only the first source has been loaded. A Serial Studio Pro license is required to use multi-source projects.</source>
        <translation>Detta projekt innehåller flera datakällor. Endast den första källan har laddats. En Serial Studio Pro-licens krävs för att använda projekt med flera källor.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2573"/>
        <source>Workspace IDs remapped on load</source>
        <translation>Arbetsyte-ID:n ommappades vid inläsning</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2574"/>
        <source>%1 custom workspace ID(s) overlapped the new reserved auto range and were moved into the user range. Save the project to make the remap permanent.</source>
        <translation>%1 anpassade arbetsyte-ID överlappade det nya reserverade automatiska intervallet och flyttades till användarintervallet. Spara projektet för att göra ommappningen permanent.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2716"/>
        <source>Legacy frame parser function updated</source>
        <translation>Äldre frame-parserfunktion uppdaterad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2717"/>
        <source>Your project used a legacy frame parser function with a 'separator' argument. It has been automatically migrated to the new format.</source>
        <translation>Ditt projekt använde en äldre frame-parser-funktion med ett 'separator'-argument. Den har automatiskt migrerats till det nya formatet.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2915"/>
        <source>Do you want to delete group "%1"?</source>
        <translation>Vill du ta bort grupp "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2916"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2961"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2993"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3731"/>
        <source>This action cannot be undone. Do you wish to proceed?</source>
        <translation>Denna åtgärd kan inte ångras. Vill du fortsätta?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2960"/>
        <source>Do you want to delete action "%1"?</source>
        <translation>Vill du ta bort åtgärd "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2992"/>
        <source>Do you want to delete dataset "%1"?</source>
        <translation>Vill du ta bort dataset "%1"?</translation>
    </message>
    <message>
        <source>%1 (Copy)</source>
        <translation type="vanished">%1 (Kopia)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3643"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3679"/>
        <source>Output Controls</source>
        <translation>Utgångskontroller</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3691"/>
        <source>New Button</source>
        <translation>Ny Knapp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3694"/>
        <source>New Slider</source>
        <translation>Ny Skjutreglage</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3697"/>
        <source>New Toggle</source>
        <translation>Ny Växlare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3700"/>
        <source>New Text Field</source>
        <translation>Nytt Textfält</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3703"/>
        <source>New Knob</source>
        <translation>Nytt Vred</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3730"/>
        <source>Do you want to delete output widget "%1"?</source>
        <translation>Vill du ta bort utgångswidget "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3895"/>
        <source>Group</source>
        <translation>Grupp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3913"/>
        <source>New Dataset</source>
        <translation>Ny Datauppsättning</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3916"/>
        <source>New Plot</source>
        <translation>Ny Graf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3920"/>
        <source>New FFT Plot</source>
        <translation>Ny FFT-graf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3924"/>
        <source>New Level Indicator</source>
        <translation>Ny Nivåindikator</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3928"/>
        <source>New Gauge</source>
        <translation>Ny Mätare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3932"/>
        <source>New Compass</source>
        <translation>Ny Kompass</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3938"/>
        <source>New Meter</source>
        <translation>Ny Mätare</translation>
    </message>
    <message>
        <source>New Thermometer</source>
        <translation type="vanished">Ny Termometer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3942"/>
        <source>New LED Indicator</source>
        <translation>Ny LED-indikator</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3946"/>
        <source>New Waterfall</source>
        <translation>Nytt Vattenfall</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4014"/>
        <source>Channel %1</source>
        <translation>Kanal %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4085"/>
        <source>New Action</source>
        <translation>Ny Åtgärd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4223"/>
        <source>Are you sure you want to change the group-level widget?</source>
        <translation>Är du säker på att du vill ändra gruppnivåwidgeten?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4225"/>
        <source>Existing datasets for this group are deleted</source>
        <translation>Befintliga dataset för denna grupp tas bort</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4288"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4289"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4290"/>
        <source>Accelerometer %1</source>
        <translation>Accelerometer %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4305"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4305"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4305"/>
        <source>Gyro %1</source>
        <translation>GYRO %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4320"/>
        <source>Latitude</source>
        <translation>Latitud</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4320"/>
        <source>Longitude</source>
        <translation>Longitud</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4320"/>
        <source>Altitude</source>
        <translation>Altitud</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4335"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4349"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4335"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4349"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4335"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4349"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4607"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5479"/>
        <source>Workspace</source>
        <translation>Arbetsyta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4772"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4974"/>
        <source>Shared Table</source>
        <translation>Delad Tabell</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4850"/>
        <source>register</source>
        <translation>register</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4974"/>
        <source>New Shared Table</source>
        <translation>Ny Delad Tabell</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4974"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4991"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5010"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5034"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5061"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5080"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5102"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5124"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5479"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5500"/>
        <source>Name:</source>
        <translation>Namn:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4991"/>
        <source>Rename Table</source>
        <translation>Byt Namn på Tabell</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5010"/>
        <source>Rename Group</source>
        <translation>Byt Namn på Grupp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5034"/>
        <source>Rename Dataset</source>
        <translation>Byt Namn på Dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5061"/>
        <source>Rename Data Source</source>
        <translation>Byt Namn på Datakälla</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5080"/>
        <source>Rename Action</source>
        <translation>Byt Namn på Åtgärd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5101"/>
        <source>New Register</source>
        <translation>Nytt Register</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5124"/>
        <source>Rename Register</source>
        <translation>Byt Namn på Register</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5161"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5186"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6002"/>
        <source>This action cannot be undone.</source>
        <translation>Denna åtgärd kan inte ångras.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5162"/>
        <source>This removes %1 register(s) along with the table. This action cannot be undone.</source>
        <translation>Detta tar bort %1 register tillsammans med tabellen. Denna åtgärd kan inte ångras.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5165"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5185"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6001"/>
        <source>Delete "%1"?</source>
        <translation>Ta Bort "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5168"/>
        <source>Delete Table</source>
        <translation>Ta Bort Tabell</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5188"/>
        <source>Delete Register</source>
        <translation>Ta Bort Register</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5210"/>
        <source>Export Table</source>
        <translation>Exportera Tabell</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5212"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5252"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-filer (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5250"/>
        <source>Import Table</source>
        <translation>Importera Tabell</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5479"/>
        <source>New Workspace</source>
        <translation>Ny Arbetsyta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5500"/>
        <source>Rename Workspace</source>
        <translation>Byt Namn på Arbetsyta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5583"/>
        <source>Overview</source>
        <translation>Översikt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5592"/>
        <source>All Data</source>
        <translation>All Data</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5770"/>
        <source>Discard workspace customisations?</source>
        <translation>Kassera anpassningar av arbetsytor?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5771"/>
        <source>Switching off Customize discards your edits and rebuilds the workspace list from the project's groups.</source>
        <translation>Att stänga av Anpassa kasserar dina ändringar och återskapar arbetsytelistan från projektets grupper.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5774"/>
        <source>Customize Workspaces</source>
        <translation>Anpassa Arbetsytor</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6004"/>
        <source>Delete Workspace</source>
        <translation>Ta Bort Arbetsyta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6331"/>
        <source>File save error</source>
        <translation>Filsparningsfel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2183"/>
        <source>File open error</source>
        <translation>Filöppningsfel</translation>
    </message>
</context>
<context>
    <name>DataModel::ProtoImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="902"/>
        <source>Import Protocol Buffers File</source>
        <translation>Importera Protocol Buffers-fil</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="904"/>
        <source>Proto Files (*.proto);;All Files (*)</source>
        <translation>Proto-filer (*.proto);;Alla Filer (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="938"/>
        <source>Failed to open proto file: %1</source>
        <translation>Misslyckades att öppna proto-fil: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="939"/>
        <source>Verify the file path and read permissions, then try again.</source>
        <translation>Verifiera filsökvägen och läsbehörigheter, försök sedan igen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="941"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="950"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="968"/>
        <source>Protobuf Import Error</source>
        <translation>Protobuf-importfel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="947"/>
        <source>Proto file is too large (the limit is 10 MB).</source>
        <translation>Proto-filen är för stor (gränsen är 10 MB).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="948"/>
        <source>Verify you selected the correct .proto definition file.</source>
        <translation>Verifiera att du valde rätt .proto-definitionsfil.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="965"/>
        <source>Failed to parse proto file at line %1: %2</source>
        <translation>Misslyckades att tolka proto-fil på rad %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="966"/>
        <source>Only proto3 syntax is supported. Verify the file format and try again.</source>
        <translation>Endast proto3-syntax stöds. Verifiera filformatet och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="973"/>
        <source>Proto file contains no message definitions</source>
        <translation>Proto-filen innehåller inga meddelandedefinitioner</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="974"/>
        <source>The selected file has no `message` blocks to import.</source>
        <translation>Den valda filen har inga `message`-block att importera.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="976"/>
        <source>Protobuf Import Warning</source>
        <translation>Protobuf-importvarning</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Misslyckades att ladda importerat projekt</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Den genererade projekt-JSON:en kunde inte laddas.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1014"/>
        <source>Successfully imported %1 message(s) and %2 field(s) from the proto file.</source>
        <translation>Importerade %1 meddelande(n) och %2 fält från proto-filen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1017"/>
        <source>The project editor is now open for customization.</source>
        <translation>Projektredigeraren är nu öppen för anpassning.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1019"/>
        <source>Protobuf Import Complete</source>
        <translation>Protobuf-import Slutförd</translation>
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
        <translation>Ogiltig Hex-inmatning</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="155"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>Ange giltiga hexadecimala bytes.

Giltigt format: 01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="160"/>
        <source>No transmit function code to evaluate.</source>
        <translation>Ingen sändfunktionskod att utvärdera.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="177"/>
        <source>transmit function is not callable</source>
        <translation>sändfunktionen är inte anropsbar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="238"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="239"/>
        <source>Clear</source>
        <translation>Rensa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="240"/>
        <source>Evaluate</source>
        <translation>Utvärdera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="241"/>
        <source>Input Value</source>
        <translation>Ingångsvärde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="242"/>
        <source>Transmit Function Output</source>
        <translation>Utdata från Överföringsfunktion</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="243"/>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="267"/>
        <source>Enter value to transmit…</source>
        <translation>Ange värde att överföra…</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="244"/>
        <source>Raw string output appears here</source>
        <translation>Rå strängutdata visas här</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="245"/>
        <source>Hex byte output appears here</source>
        <translation>Hex-byte-utdata visas här</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="248"/>
        <source>Test Transmit Function</source>
        <translation>Testa Överföringsfunktion</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="261"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>Ange hex-bytes (t.ex. 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="367"/>
        <source>(empty) No data returned</source>
        <translation>(tom) Ingen data returnerad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="369"/>
        <source>0 bytes</source>
        <translation>0 bytes</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="408"/>
        <source>%1 byte(s)</source>
        <translation>%1 byte</translation>
    </message>
</context>
<context>
    <name>DataTablesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="33"/>
        <source>Shared Memory</source>
        <translation>Delat Minne</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="149"/>
        <source>Add Shared Table</source>
        <translation>Lägg till Delad Tabell</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="151"/>
        <source>Add shared table</source>
        <translation>Lägg till delad tabell</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="160"/>
        <source>Help</source>
        <translation>Hjälp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="165"/>
        <source>Open help documentation for shared memory</source>
        <translation>Öppna hjälpdokumentation för delat minne</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="174"/>
        <source>Name</source>
        <translation>Namn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="175"/>
        <source>Description</source>
        <translation>Beskrivning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="176"/>
        <source>Entries</source>
        <translation>Poster</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="274"/>
        <source>No shared tables.</source>
        <translation>Inga delade tabeller.</translation>
    </message>
</context>
<context>
    <name>DatabaseExplorer</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="35"/>
        <source>Sessions</source>
        <translation>Sessioner</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="218"/>
        <source>Open</source>
        <translation>Öppna</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="220"/>
        <source>Open a session file</source>
        <translation>Öppna en sessionsfil</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="226"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="229"/>
        <source>Close session file</source>
        <translation>Stäng sessionsfil</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="242"/>
        <source>Replay</source>
        <translation>Spela Upp</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="246"/>
        <source>Replay selected session on the dashboard</source>
        <translation>Spela upp vald session på instrumentpanelen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="252"/>
        <source>Delete</source>
        <translation>Ta Bort</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="258"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Lås upp sessionsfilen för att radera sessioner</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="259"/>
        <source>Delete the selected session</source>
        <translation>Ta bort den valda sessionen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="276"/>
        <source>Unlock</source>
        <translation>Lås Upp</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="277"/>
        <source>Lock</source>
        <translation>Lås</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="282"/>
        <source>Unlock the session file to allow deletions</source>
        <translation>Lås upp sessionsfilen för att tillåta raderingar</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="283"/>
        <source>Set a password to prevent session deletions</source>
        <translation>Ange ett lösenord för att förhindra radering av sessioner</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="298"/>
        <source>Export CSV</source>
        <translation>Exportera CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="303"/>
        <source>Export selected session to CSV</source>
        <translation>Exportera vald session till CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="310"/>
        <source>Export PDF</source>
        <translation>Exportera PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="315"/>
        <source>Generate a PDF report for the selected session</source>
        <translation>Generera en PDF-rapport för den valda sessionen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="329"/>
        <source>Restore Project</source>
        <translation>Återställ Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="333"/>
        <source>Restore the project file from this session file</source>
        <translation>Återställ projektfilen från denna sessionsfil</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="402"/>
        <source>Loading session…</source>
        <translation>Laddar session…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="403"/>
        <source>Working…</source>
        <translation>Arbetar…</translation>
    </message>
</context>
<context>
    <name>DatasetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="109"/>
        <source>Pro features detected in this project.</source>
        <translation>Pro-funktioner upptäckta i detta projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="111"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Använder reservwidgetar. Köp en licens för att låsa upp full funktionalitet.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="139"/>
        <source>Plots</source>
        <translation>Diagram</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="144"/>
        <source>Plot</source>
        <translation>Diagram</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="148"/>
        <source>Toggle 2D plot visualization for this dataset</source>
        <translation>Växla 2D-diagramvisualisering för detta dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="160"/>
        <source>FFT Plot</source>
        <translation>FFT-diagram</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="163"/>
        <source>Toggle FFT plot to visualize frequency content</source>
        <translation>Växla FFT-diagram för att visualisera frekvensinnehåll</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="175"/>
        <source>Waterfall</source>
        <translation>Vattenfall</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="179"/>
        <source>Toggle waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Växla vattenfallsdiagram (spektrogram) — använder FFT-inställningarna</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="196"/>
        <source>Widgets</source>
        <translation>Widgetar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="202"/>
        <source>Bar/Level</source>
        <translation>Stapel/nivå</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="206"/>
        <source>Toggle bar/level indicator for this dataset</source>
        <translation>Växla stapel-/nivåindikator för detta dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="217"/>
        <source>Gauge</source>
        <translation>Mätare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="222"/>
        <source>Toggle gauge widget for analog-style display</source>
        <translation>Växla mätarwidget för analog visning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="234"/>
        <source>Compass</source>
        <translation>Kompass</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="238"/>
        <source>Toggle compass widget for directional data</source>
        <translation>Växla kompasswidget för riktningsdata</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="249"/>
        <source>Meter</source>
        <translation>Mätare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="254"/>
        <source>Toggle analog meter (half-arc) widget</source>
        <translation>Växla analog mätare (halvbåge) widget</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometer</translation>
    </message>
    <message>
        <source>Toggle thermometer widget for temperature data</source>
        <translation type="vanished">Växla termometerwidget för temperaturdata</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="265"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="270"/>
        <source>Toggle LED indicator for binary or thresholded values</source>
        <translation>Växla LED-indikator för binära eller tröskelvärden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="290"/>
        <source>Behavior</source>
        <translation>Beteende</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="295"/>
        <source>Alarm Bands</source>
        <translation>Larmband</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="304"/>
        <source>Define colored value ranges with severity tiers for this dataset's gauge.</source>
        <translation>Definiera färgade värdeintervall med allvarlighetsnivåer för detta datasets mätare.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="310"/>
        <source>Transform</source>
        <translation>Transformering</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="314"/>
        <source>Edit a value transform expression for calibration, filtering, or unit conversion</source>
        <translation>Redigera ett värdetransformationsuttryck för kalibrering, filtrering eller enhetskonvertering</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="327"/>
        <source>Duplicate</source>
        <translation>Duplicera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="332"/>
        <source>Duplicate this dataset with the same configuration</source>
        <translation>Duplicera denna datamängd med samma konfiguration</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="337"/>
        <source>Delete</source>
        <translation>Ta Bort</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="340"/>
        <source>Delete this dataset from the group</source>
        <translation>Ta bort denna datamängd från gruppen</translation>
    </message>
</context>
<context>
    <name>Donate</name>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="37"/>
        <source>Support Serial Studio</source>
        <translation>Stöd Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="86"/>
        <source>Support the development of %1!</source>
        <translation>Stöd utvecklingen av %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="97"/>
        <source>Serial Studio is free &amp; open-source software supported by volunteers. Consider donating or obtaining a Pro license to support development efforts :)</source>
        <translation>Serial Studio är fri programvara med öppen källkod som stöds av volontärer. Överväg att donera eller skaffa en Pro-licens för att stödja utvecklingsarbetet :)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="110"/>
        <source>You can also support this project by sharing it, reporting bugs and proposing new features!</source>
        <translation>Du kan också stödja detta projekt genom att dela det, rapportera buggar och föreslå nya funktioner!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="124"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="135"/>
        <source>Donate</source>
        <translation>Donera</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="150"/>
        <source>Get Serial Studio Pro</source>
        <translation>Skaffa Serial Studio Pro</translation>
    </message>
</context>
<context>
    <name>Downloader</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="127"/>
        <source>Stop</source>
        <translation>Stoppa</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="128"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="362"/>
        <source>Downloading updates</source>
        <translation>Laddar ner uppdateringar</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="129"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="455"/>
        <source>Time remaining</source>
        <translation>Återstående tid</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="129"/>
        <source>unknown</source>
        <translation>okänd</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="228"/>
        <source>Error</source>
        <translation>Fel</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="228"/>
        <source>Cannot find downloaded update!</source>
        <translation>Kan inte hitta nedladdad uppdatering!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="244"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="245"/>
        <source>Download complete!</source>
        <translation>Nedladdning klar!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="253"/>
        <source>Click "OK" to begin installing the update</source>
        <translation>Klicka på "OK" för att börja installera uppdateringen</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="255"/>
        <source>In order to install the update, you may need to quit the application.</source>
        <translation>För att installera uppdateringen kan du behöva avsluta applikationen.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="246"/>
        <source>The installer opens separately</source>
        <translation>Installationsprogrammet öppnas separat</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="259"/>
        <source>In order to install the update, you may need to quit the application. This is a mandatory update, exiting now will close the application.</source>
        <translation>För att installera uppdateringen kan du behöva avsluta applikationen. Detta är en obligatorisk uppdatering, att avsluta nu kommer att stänga applikationen.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="275"/>
        <source>Click the "Open" button to apply the update</source>
        <translation>Klicka på knappen "Öppna" för att tillämpa uppdateringen</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="288"/>
        <source>Updater</source>
        <translation>Uppdaterare</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="292"/>
        <source>Are you sure you want to cancel the download?</source>
        <translation>Är du säker på att du vill avbryta nedladdningen?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="294"/>
        <source>Are you sure you want to cancel the download? This is a mandatory update, exiting now will close the application</source>
        <translation>Är du säker på att du vill avbryta nedladdningen? Detta är en obligatorisk uppdatering, att avsluta nu kommer att stänga applikationen</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="349"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="356"/>
        <source>%1 bytes</source>
        <translation>%1 byte</translation>
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
        <translation>av</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="406"/>
        <source>Downloading Updates</source>
        <translation>Laddar Ner Uppdateringar</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="407"/>
        <source>Time Remaining</source>
        <translation>Återstående Tid</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="407"/>
        <source>Unknown</source>
        <translation>Okänd</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="431"/>
        <source>about %1 hours</source>
        <translation>cirka %1 timmar</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="433"/>
        <source>about one hour</source>
        <translation>cirka en timme</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="441"/>
        <source>%1 minutes</source>
        <translation>%1 minuter</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="443"/>
        <source>1 minute</source>
        <translation>1 minut</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="450"/>
        <source>%1 seconds</source>
        <translation>%1 sekunder</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="452"/>
        <source>1 second</source>
        <translation>1 sekund</translation>
    </message>
    <message>
        <source>Time remaining: 0 minutes</source>
        <translation type="vanished">Återstående tid: 0 minuter</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Öppna</translation>
    </message>
</context>
<context>
    <name>ExamplesBrowser</name>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="34"/>
        <source>Examples Browser</source>
        <translation>Exempelbläddrare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="120"/>
        <source>Back</source>
        <translation>Tillbaka</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="152"/>
        <source>Pro</source>
        <translation>Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="173"/>
        <source>Download &amp;&amp; Open</source>
        <translation>Ladda Ner &amp;&amp; Öppna</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="188"/>
        <source>View on GitHub</source>
        <translation>Visa på GitHub</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="91"/>
        <source>Search in Examples…</source>
        <translation>Sök i Exempel…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="245"/>
        <source>Fetching examples…</source>
        <translation>Hämtar exempel…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="568"/>
        <source>Loading...</source>
        <translation>Laddar...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="569"/>
        <source>No README available.</source>
        <translation>Ingen README tillgänglig.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="609"/>
        <source>Copied to Clipboard</source>
        <translation>Kopierat till Urklipp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="672"/>
        <source>No screenshot available</source>
        <translation>Ingen skärmbild tillgänglig</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="704"/>
        <source>Details</source>
        <translation>Detaljer</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="733"/>
        <source>Info</source>
        <translation>Info</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="756"/>
        <source>Category:</source>
        <translation>Kategori:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="769"/>
        <source>Difficulty:</source>
        <translation>Svårighetsgrad:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="787"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="829"/>
        <source>No Results Found</source>
        <translation>Inga Resultat Hittades</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="840"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Kontrollera stavningen eller prova ett annat sökord.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="855"/>
        <source>%1 examples</source>
        <translation>%1 exempel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="864"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
</context>
<context>
    <name>ExtensionManager</name>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="32"/>
        <source>Extension Manager</source>
        <translation>Tilläggshanterare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="119"/>
        <source>Refresh</source>
        <translation>Uppdatera</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="133"/>
        <source>Repos</source>
        <translation>Repos</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="163"/>
        <source>Repository Settings</source>
        <translation>Inställningar för Arkiv</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="175"/>
        <source>Back</source>
        <translation>Tillbaka</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="216"/>
        <source>Install</source>
        <translation>Installera</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="233"/>
        <source>Uninstall</source>
        <translation>Avinstallera</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="260"/>
        <source>Run</source>
        <translation>Kör</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="284"/>
        <source>Stop</source>
        <translation>Stoppa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="318"/>
        <source>Reset</source>
        <translation>Återställ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="78"/>
        <source>Search extensions…</source>
        <translation>Sök tillägg…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="368"/>
        <source>Fetching extensions…</source>
        <translation>Hämtar tillägg…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="605"/>
        <source>Running</source>
        <translation>Körs</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Update</source>
        <translation>Uppdatera</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Installed</source>
        <translation>Installerat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="644"/>
        <source>Unavailable</source>
        <translation>Ej Tillgängligt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="823"/>
        <source>No description available.</source>
        <translation>Ingen beskrivning tillgänglig.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="864"/>
        <source>Details</source>
        <translation>Detaljer</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="885"/>
        <source>Type:</source>
        <translation>Typ:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="898"/>
        <source>Author:</source>
        <translation>Författare:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="910"/>
        <source>Version:</source>
        <translation>Version:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="922"/>
        <source>License:</source>
        <translation>Licens:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="983"/>
        <source>No preview</source>
        <translation>Ingen förhandsgranskning</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1012"/>
        <source>  PLUGIN OUTPUT</source>
        <translation>PLUGIN-UTDATA</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1042"/>
        <source>No output yet. Run the plugin to see its log here.</source>
        <translation>Ingen utdata ännu. Kör pluginet för att se dess logg här.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1077"/>
        <source>No preview available</source>
        <translation>Ingen förhandsgranskning tillgänglig</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1121"/>
        <source>Repositories</source>
        <translation>Arkiv</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1134"/>
        <source>Add URLs to remote repositories or local folder paths.</source>
        <translation>Lägg till URL:er till fjärrarkiv eller lokala mappsökvägar.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1171"/>
        <source>LOCAL</source>
        <translation>LOKAL</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1228"/>
        <source>URL or local path…</source>
        <translation>URL eller lokal sökväg…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1259"/>
        <source>Browse…</source>
        <translation>Bläddra…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1236"/>
        <source>Add</source>
        <translation>Lägg Till</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1296"/>
        <source>No Results Found</source>
        <translation>Inga Resultat Hittades</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1307"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Kontrollera stavningen eller prova ett annat sökord.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1331"/>
        <source>No Extensions Available</source>
        <translation>Inga Tillägg Tillgängliga</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1342"/>
        <source>Add a repository URL or local path in the Repos settings, then refresh.</source>
        <translation>Lägg till en arkiv-URL eller lokal sökväg i Arkiv-inställningarna och uppdatera sedan.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1357"/>
        <source>%1 extensions</source>
        <translation>%1 tillägg</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1366"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
</context>
<context>
    <name>FFTPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="157"/>
        <source>Interpolation: %1</source>
        <translation>Interpolering: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="185"/>
        <source>Show Area Under Plot</source>
        <translation>Visa Area Under Kurva</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="203"/>
        <source>Show X Axis Label</source>
        <translation>Visa X-axeletikett</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="215"/>
        <source>Show Y Axis Label</source>
        <translation>Visa Y-axeletikett</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="233"/>
        <source>Show Crosshair</source>
        <translation>Visa Hårkors</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="240"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="240"/>
        <source>Resume</source>
        <translation>Återuppta</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="259"/>
        <source>Reset View</source>
        <translation>Återställ Vy</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="265"/>
        <source>Axis Range Settings</source>
        <translation>Inställningar för Axelområde</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="294"/>
        <source>Magnitude (dB)</source>
        <translation>Magnitud (dB)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="295"/>
        <source>Frequency (Hz)</source>
        <translation>Frekvens (Hz)</translation>
    </message>
</context>
<context>
    <name>FileDropArea</name>
    <message>
        <location filename="../../qml/Widgets/FileDropArea.qml" line="130"/>
        <source>Drop Projects and CSV files here</source>
        <translation>Släpp Projekt och CSV-filer här</translation>
    </message>
</context>
<context>
    <name>FileTransmission</name>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="34"/>
        <source>File Transmission</source>
        <translation>Filöverföring</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="102"/>
        <source>Transfer Protocol:</source>
        <translation>Överföringsprotokoll:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="135"/>
        <source>File Selection:</source>
        <translation>Filval:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="152"/>
        <source>Select File…</source>
        <translation>Välj Fil…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="170"/>
        <source>Transmission Interval:</source>
        <translation>Överföringsintervall:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="196"/>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="272"/>
        <source>msecs</source>
        <translation>ms</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="206"/>
        <source>Block Size:</source>
        <translation>Blockstorlek:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="234"/>
        <source>bytes</source>
        <translation>byte</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="244"/>
        <source>Timeout:</source>
        <translation>Timeout:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="282"/>
        <source>Max Retries:</source>
        <translation>Max Återförsök:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="340"/>
        <source>Progress: %1%</source>
        <translation>Förlopp: %1%</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="373"/>
        <source>%1 / %2 bytes</source>
        <translation>%1 / %2 byte</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="381"/>
        <source>Errors: %1</source>
        <translation>Fel: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="461"/>
        <source>Activity Log</source>
        <translation>Aktivitetslogg</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="465"/>
        <source>Clear</source>
        <translation>Rensa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="419"/>
        <source>Pause Transmission</source>
        <translation>Pausa Överföring</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="420"/>
        <source>Resume Transmission</source>
        <translation>Återuppta Överföring</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="423"/>
        <source>Stop Transmission</source>
        <translation>Stoppa Överföring</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="424"/>
        <source>Begin Transmission</source>
        <translation>Påbörja Överföring</translation>
    </message>
</context>
<context>
    <name>FlowDiagram</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="159"/>
        <source>Frame Parser</source>
        <translation>Frame-parser</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="166"/>
        <source>Device %1</source>
        <translation>Enhet %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="395"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1402"/>
        <source>Output Panel</source>
        <translation>Utdatapanel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="439"/>
        <source>Control</source>
        <translation>Kontroll</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="462"/>
        <source>Table</source>
        <translation>Tabell</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="478"/>
        <source>%1 regs</source>
        <translation>%1 reg</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="479"/>
        <source>empty</source>
        <translation>tom</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="504"/>
        <source>MQTT Publisher</source>
        <translation>MQTT-utgivare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="905"/>
        <source>Open the transform code editor for this dataset.</source>
        <translation>Öppna transformationskodredigeraren för denna datauppsättning.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1184"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1187"/>
        <source>Dataset Container</source>
        <translation>Datauppsättningscontainer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1196"/>
        <source>Multi-Plot</source>
        <translation>Flerplott</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1199"/>
        <source>Multiple Plot</source>
        <translation>Flerplott</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1208"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1211"/>
        <source>Accelerometer</source>
        <translation>Accelerometer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1220"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1223"/>
        <source>Gyroscope</source>
        <translation>Gyroskop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1232"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1235"/>
        <source>GPS Map</source>
        <translation>GPS-karta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1243"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1246"/>
        <source>3D Plot</source>
        <translation>3D-plott</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1254"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1257"/>
        <source>Image View</source>
        <translation>Bildvisning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1266"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1269"/>
        <source>Painter Widget</source>
        <translation>Painter-widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1278"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1281"/>
        <source>Data Grid</source>
        <translation>Datarutnät</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1294"/>
        <source>Generic</source>
        <translation>Generisk</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1307"/>
        <source>Plot</source>
        <translation>Plott</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1320"/>
        <source>FFT Plot</source>
        <translation>FFT-diagram</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1333"/>
        <source>Gauge</source>
        <translation>Mätare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1346"/>
        <source>Level Indicator</source>
        <translation>Nivåindikator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1359"/>
        <source>Compass</source>
        <translation>Kompass</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1372"/>
        <source>Meter</source>
        <translation>Mätare</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1385"/>
        <source>LED Indicator</source>
        <translation>LED-indikator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1414"/>
        <source>Slider</source>
        <translation>Skjutreglage</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1427"/>
        <source>Toggle</source>
        <translation>Växlare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1440"/>
        <source>Knob</source>
        <translation>Ratt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1453"/>
        <source>Text Field</source>
        <translation>Textfält</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1466"/>
        <source>Button</source>
        <translation>Knapp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1490"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1565"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1652"/>
        <source>Add Group</source>
        <translation>Lägg till Grupp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1505"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1580"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1667"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1712"/>
        <source>Add Dataset</source>
        <translation>Lägg till Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1519"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1594"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1681"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1726"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1933"/>
        <source>Add Output</source>
        <translation>Lägg till Utdata</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1535"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1607"/>
        <source>Add Action</source>
        <translation>Lägg till Åtgärd</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1544"/>
        <source>Add Data Source</source>
        <translation>Lägg till Datakälla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1551"/>
        <source>Add Data Table</source>
        <translation>Lägg till Datatabell</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1618"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1753"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1820"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1948"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1982"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2038"/>
        <source>Rename…</source>
        <translation>Byt Namn…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1626"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1783"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1853"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1905"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1956"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2012"/>
        <source>Duplicate</source>
        <translation>Duplicera</translation>
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
        <translation>Ta Bort…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1697"/>
        <source>Edit Frame Parser…</source>
        <translation>Redigera Frame-parser…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1739"/>
        <source>Edit Painter Code…</source>
        <translation>Redigera Painter-kod…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1761"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1829"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1881"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1990"/>
        <source>Move Up</source>
        <translation>Flytta Upp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1772"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1841"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1893"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2001"/>
        <source>Move Down</source>
        <translation>Flytta Ner</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1809"/>
        <source>Edit Transform Code…</source>
        <translation>Redigera Transform-kod…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2064"/>
        <source>Edit Code…</source>
        <translation>Redigera Kod…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="222"/>
        <source>Group</source>
        <translation>Grupp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="345"/>
        <source>Action</source>
        <translation>Åtgärd</translation>
    </message>
    <message>
        <source>No groups defined yet</source>
        <translation type="vanished">Inga grupper definierade ännu</translation>
    </message>
</context>
<context>
    <name>FrameParserTest</name>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="39"/>
        <source>Test Frame Parser</source>
        <translation>Testa Ramtolk</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="96"/>
        <source>Frame %1</source>
        <translation>Ram %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="98"/>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="209"/>
        <source>Decoder</source>
        <translation>Avkodare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="100"/>
        <source>Rows</source>
        <translation>Rader</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="101"/>
        <source>%1 row(s)</source>
        <translation>%1 rad(er)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="105"/>
        <source>Row %1</source>
        <translation>Rad %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="164"/>
        <source>Pipeline Configuration</source>
        <translation>Pipeline-konfiguration</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="186"/>
        <source>Detection</source>
        <translation>Detektering</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="232"/>
        <source>Start</source>
        <translation>Start</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="253"/>
        <source>End</source>
        <translation>Slut</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="274"/>
        <source>Checksum</source>
        <translation>Kontrollsumma</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="299"/>
        <source>Hex Delimiters</source>
        <translation>Hexadecimala Avgränsare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="315"/>
        <source>Frame Data Input</source>
        <translation>Ramdatainmatning</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="342"/>
        <source>Enter hex bytes (e.g. 01 A2 FF)</source>
        <translation>Ange hexadecimala byte (t.ex. 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="343"/>
        <source>Enter raw stream bytes here...</source>
        <translation>Ange råa strömbyte här...</translation>
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
        <translation>Exemplet innehåller inte de konfigurerade ramavgränsarna, så ingen ram kommer att extraheras. Skriv in dem i exemplet (t.ex. 
 för ny rad) eller justera detekteringsläget.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="407"/>
        <source>Pipeline Results</source>
        <translation>Pipelineresultat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="480"/>
        <source>Stage</source>
        <translation>Förbered</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="487"/>
        <source>Value</source>
        <translation>Värde</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="530"/>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation>Extraheringen producerade inte en komplett ram. Kontrollera start-/slutavgränsarna och detekteringsläget.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="532"/>
        <source>Enter sample data above and press Evaluate to preview the parsed output</source>
        <translation>Ange exempeldata ovan och tryck på Utvärdera för att förhandsgranska den tolkade utmatningen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="614"/>
        <source>Clear</source>
        <translation>Rensa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="625"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="632"/>
        <source>Evaluate</source>
        <translation>Utvärdera</translation>
    </message>
</context>
<context>
    <name>FrameParserView</name>
    <message>
        <source>Undo</source>
        <translation type="vanished">Ångra</translation>
    </message>
    <message>
        <source>Redo</source>
        <translation type="vanished">Gör Om</translation>
    </message>
    <message>
        <source>Cut</source>
        <translation type="vanished">Klipp Ut</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="vanished">Kopiera</translation>
    </message>
    <message>
        <source>Paste</source>
        <translation type="vanished">Klistra In</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation type="vanished">Markera Allt</translation>
    </message>
    <message>
        <source>Format Document</source>
        <translation type="vanished">Formatera Dokument</translation>
    </message>
    <message>
        <source>Format Selection</source>
        <translation type="vanished">Formatera Markering</translation>
    </message>
    <message>
        <source>Reset</source>
        <translation type="vanished">Återställ</translation>
    </message>
    <message>
        <source>Reset to the default parsing script</source>
        <translation type="vanished">Återställ till standardtolkningsskriptet</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Öppna</translation>
    </message>
    <message>
        <source>Import a script file for data parsing</source>
        <translation type="vanished">Importera en skriptfil för dataparsning</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Öppna hjälpdokumentation för dataparsning</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Språk:</translation>
    </message>
    <message>
        <source>Select Template…</source>
        <translation type="vanished">Välj Mall…</translation>
    </message>
    <message>
        <source>Undo the last code edit</source>
        <translation type="vanished">Ångra den senaste kodredigeringen</translation>
    </message>
    <message>
        <source>Redo the previously undone edit</source>
        <translation type="vanished">Gör om den tidigare ångrade redigeringen</translation>
    </message>
    <message>
        <source>Cut selected code to clipboard</source>
        <translation type="vanished">Klipp ut markerad kod till urklipp</translation>
    </message>
    <message>
        <source>Copy selected code to clipboard</source>
        <translation type="vanished">Kopiera markerad kod till urklipp</translation>
    </message>
    <message>
        <source>Paste code from clipboard</source>
        <translation type="vanished">Klistra in kod från urklipp</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="vanished">Hjälp</translation>
    </message>
    <message>
        <source>Test With Sample Data</source>
        <translation type="vanished">Testa med Exempeldata</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Utvärdera</translation>
    </message>
</context>
<context>
    <name>GPS</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="105"/>
        <source>Auto Center</source>
        <translation>Autocentrera</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="121"/>
        <source>Plot Trajectory</source>
        <translation>Rita Bana</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="138"/>
        <source>Zoom In</source>
        <translation>Zooma In</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="149"/>
        <source>Zoom Out</source>
        <translation>Zooma Ut</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="173"/>
        <source>Show Weather</source>
        <translation>Visa Väder</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="191"/>
        <source>NASA Weather Overlay</source>
        <translation>NASA Väderoverlägg</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="223"/>
        <source>Base Map: %1</source>
        <translation>Baskarta: %1</translation>
    </message>
</context>
<context>
    <name>GroupView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="97"/>
        <source>Pro features detected in this project.</source>
        <translation>Pro-funktioner upptäckta i detta projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="99"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Använder reservwidgets. Köp en licens för att låsa upp full funktionalitet.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="129"/>
        <source>Add Dataset</source>
        <translation>Lägg till Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="136"/>
        <source>Dataset</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="139"/>
        <source>Add a generic dataset to the current group</source>
        <translation>Lägg till ett generiskt dataset till den aktuella gruppen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="146"/>
        <source>Plot</source>
        <translation>Diagram</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="150"/>
        <source>Add a 2D plot to visualize numeric data</source>
        <translation>Lägg till ett 2D-diagram för att visualisera numerisk data</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="158"/>
        <source>FFT Plot</source>
        <translation>FFT-diagram</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="163"/>
        <source>Add an FFT plot for frequency domain visualization</source>
        <translation>Lägg till ett FFT-diagram för visualisering i frekvensdomänen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="169"/>
        <source>Waterfall</source>
        <translation>Vattenfall</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="174"/>
        <source>Add a waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Lägg till ett vattenfallsdiagram (spektrogram) — använder FFT-inställningarna</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="180"/>
        <source>Bar/Level</source>
        <translation>Stapel/nivå</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="184"/>
        <source>Add a bar or level indicator for scaled values</source>
        <translation>Lägg till en stapel- eller nivåindikator för skalade värden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="190"/>
        <source>Gauge</source>
        <translation>Mätare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="195"/>
        <source>Add a gauge widget for analog-style visualization</source>
        <translation>Lägg till en mätare-widget för analog-stil visualisering</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="202"/>
        <source>Compass</source>
        <translation>Kompass</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="206"/>
        <source>Add a compass to display directional or angular data</source>
        <translation>Lägg till en kompass för att visa riktnings- eller vinkeldata</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="212"/>
        <source>Meter</source>
        <translation>Mätare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="216"/>
        <source>Add an analog meter (half-arc) widget</source>
        <translation>Lägg till en analog mätarwidget (halvbåge)</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometer</translation>
    </message>
    <message>
        <source>Add a thermometer widget for temperature data</source>
        <translation type="vanished">Lägg till en termometerwidget för temperaturdata</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="223"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="228"/>
        <source>Add an LED indicator for binary status signals</source>
        <translation>Lägg till en LED-indikator för binära statussignaler</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="241"/>
        <source>Add Control</source>
        <translation>Lägg till Kontroll</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="247"/>
        <source>Button</source>
        <translation>Knapp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="251"/>
        <source>Add a button that sends a command on click</source>
        <translation>Lägg till en knapp som skickar ett kommando vid klick</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="257"/>
        <source>Slider</source>
        <translation>Skjutreglage</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="261"/>
        <source>Add a slider for sending scaled numeric values</source>
        <translation>Lägg till ett skjutreglage för att skicka skalade numeriska värden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="267"/>
        <source>Toggle</source>
        <translation>Växla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="271"/>
        <source>Add a toggle switch for on/off commands</source>
        <translation>Lägg till en växlingsknapp för på/av-kommandon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="278"/>
        <source>Text Field</source>
        <translation>Textfält</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="281"/>
        <source>Add a text field for typing and sending commands</source>
        <translation>Lägg till ett textfält för att skriva och skicka kommandon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="287"/>
        <source>Knob</source>
        <translation>Ratt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="291"/>
        <source>Add a rotary knob for setpoint control</source>
        <translation>Lägg till en vridratt för börvärdesstyrning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="310"/>
        <source>Edit Code</source>
        <translation>Redigera Kod</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="314"/>
        <source>Edit the JavaScript that draws this painter widget</source>
        <translation>Redigera JavaScript-koden som ritar denna painter-widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="327"/>
        <source>Duplicate</source>
        <translation>Duplicera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="331"/>
        <source>Duplicate the current group and its contents</source>
        <translation>Duplicera den aktuella gruppen och dess innehåll</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="336"/>
        <source>Delete</source>
        <translation>Ta Bort</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="341"/>
        <source>Delete the current group and all contained datasets</source>
        <translation>Ta bort den aktuella gruppen och alla innehållna dataset</translation>
    </message>
</context>
<context>
    <name>Gyroscope</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="376"/>
        <source>ROLL ↔</source>
        <translation>ROLL ↔</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="404"/>
        <source>YAW ↻</source>
        <translation>YAW ↻</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="432"/>
        <source>PITCH ↕</source>
        <translation>PITCH ↕</translation>
    </message>
</context>
<context>
    <name>HID</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="50"/>
        <source>HID Device</source>
        <translation>HID-enhet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="80"/>
        <source>Usage Page</source>
        <translation>Användningssida</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="96"/>
        <source>Usage</source>
        <translation>Användning</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="137"/>
        <source>Connect gamepads, joysticks, steering wheels, flight controllers, and other HID-class USB devices.</source>
        <translation>Anslut spelkontroller, joysticks, rattar, flygkontroller och andra HID-klass USB-enheter.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="145"/>
        <source>HID Usage Tables (USB.org)</source>
        <translation>HID-användningstabeller (USB.org)</translation>
    </message>
</context>
<context>
    <name>HelpCenter</name>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="33"/>
        <source>Help Center</source>
        <translation>Hjälpcenter</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="95"/>
        <source>Fetching help pages…</source>
        <translation>Hämtar hjälpsidor…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="129"/>
        <source>Search…</source>
        <translation>Sök…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="247"/>
        <source>Loading…</source>
        <translation>Laddar…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="291"/>
        <source>Select a page from the sidebar</source>
        <translation>Välj en sida från sidofältet</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="321"/>
        <source>Copied to Clipboard</source>
        <translation>Kopierat till Urklipp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="353"/>
        <source>View Online</source>
        <translation>Visa Online</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="372"/>
        <source>%1 pages</source>
        <translation>%1 sidor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="381"/>
        <source>Close</source>
        <translation>Stäng</translation>
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
        <translation>Nätverksuttag</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="269"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="271"/>
        <source>Audio</source>
        <translation>Ljud</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="272"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="273"/>
        <source>CAN Bus</source>
        <translation>CAN-buss</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="274"/>
        <source>USB Device</source>
        <translation>USB-enhet</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="275"/>
        <source>HID Device</source>
        <translation>HID-enhet</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="276"/>
        <source>Process</source>
        <translation>Process</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="277"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT-prenumerant</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="639"/>
        <source>Your trial period has ended.</source>
        <translation>Din provperiod har löpt ut.</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="640"/>
        <source>To continue using Serial Studio, please activate your license.</source>
        <translation>För att fortsätta använda Serial Studio, aktivera din licens.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Audio</name>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="743"/>
        <source>channels</source>
        <translation>kanaler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="796"/>
        <source> channels</source>
        <translation>kanaler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="944"/>
        <source>Unsigned 8-bit</source>
        <translation>Osignerad 8-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="945"/>
        <source>Signed 16-bit</source>
        <translation>Signerad 16-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="946"/>
        <source>Signed 24-bit</source>
        <translation>Signerad 24-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="947"/>
        <source>Signed 32-bit</source>
        <translation>Signerad 32-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="948"/>
        <source>Float 32-bit</source>
        <translation>Float 32-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="951"/>
        <source>Mono</source>
        <translation>Mono</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="952"/>
        <source>Stereo</source>
        <translation>Stereo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1332"/>
        <source>Input Device</source>
        <translation>Ingångsenhet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1340"/>
        <source>Sample Rate</source>
        <translation>Samplingsfrekvens</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1348"/>
        <source>Sample Format</source>
        <translation>Samplingsformat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1356"/>
        <source>Channels</source>
        <translation>Kanaler</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::BluetoothLE</name>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="456"/>
        <source>BLE I/O Module Error</source>
        <translation>BLE I/o-modulfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="751"/>
        <source>Select Device</source>
        <translation>Välj Enhet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="762"/>
        <source>Select Service</source>
        <translation>Välj Tjänst</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="773"/>
        <source>Select Characteristic</source>
        <translation>Välj Karakteristik</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="972"/>
        <source>Error while configuring BLE service</source>
        <translation>Fel vid konfigurering av BLE-tjänst</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1190"/>
        <source>Operation error</source>
        <translation>Operationsfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1193"/>
        <source>Characteristic write error</source>
        <translation>Fel vid skrivning av karakteristik</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1196"/>
        <source>Descriptor write error</source>
        <translation>Fel vid skrivning av deskriptor</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1199"/>
        <source>Unknown error</source>
        <translation>Okänt fel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1202"/>
        <source>Characteristic read error</source>
        <translation>Karakteristikläsningsfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1205"/>
        <source>Descriptor read error</source>
        <translation>Deskriptorläsningsfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1440"/>
        <source>BLE Device</source>
        <translation>BLE-enhet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1448"/>
        <source>Service</source>
        <translation>Tjänst</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1455"/>
        <source>Notify Characteristic</source>
        <translation>Notifiera Karakteristik</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1462"/>
        <source>Characteristic</source>
        <translation>Karakteristik</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::CANBus</name>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="302"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="308"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="314"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="319"/>
        <source>CAN Bus Not Available</source>
        <translation>CAN-buss Ej Tillgänglig</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="315"/>
        <source>No CAN bus plugins found on this system.

CAN bus support on macOS is limited and may require third-party hardware drivers.</source>
        <translation>Inga CAN-buss-plugins hittades på detta system.

CAN-buss-stöd på macOS är begränsat och kan kräva tredjepartshårdvarudrivrutiner.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="320"/>
        <source>No CAN bus plugins are available on this platform.</source>
        <translation>Inga CAN-buss-plugins är tillgängliga på denna plattform.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="332"/>
        <source>Invalid CAN Configuration</source>
        <translation>Ogiltig CAN-konfiguration</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="340"/>
        <source>Invalid Selection</source>
        <translation>Ogiltigt Val</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="349"/>
        <source>No Devices Available</source>
        <translation>Inga Enheter Tillgängliga</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="259"/>
        <source>CAN Device Creation Failed</source>
        <translation>Misslyckades Att Skapa CAN-enhet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="283"/>
        <source>CAN Connection Failed</source>
        <translation>CAN-anslutning Misslyckades</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="303"/>
        <source>No CAN bus plugins found on this system.

On Linux, ensure SocketCAN kernel modules are loaded.</source>
        <translation>Inga CAN-bus-plugins hittades på detta system.

På Linux, säkerställ att SOCKETCAN-kärnmoduler är laddade.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="309"/>
        <source>No CAN bus plugins found on this system.

On Windows, install CAN hardware drivers (PEAK, Vector, etc.).</source>
        <translation>Inga CAN-bus-plugins hittades på detta system.

På Windows, installera CAN-hårdvarudrivrutiner (PEAK, VECTOR, etc.).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="333"/>
        <source>The CAN bus configuration is incomplete. Select a valid plugin and interface.</source>
        <translation>CAN-bus-konfigurationen är ofullständig. Välj ett giltigt plugin och gränssnitt.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="341"/>
        <source>The selected plugin or interface is no longer available. Refresh the lists and try again.</source>
        <translation>Det valda pluginet eller gränssnittet är inte längre tillgängligt. Uppdatera listorna och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="350"/>
        <source>The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected.</source>
        <translation>Plugin- eller gränssnittslistan är tom. Uppdatera listorna och säkerställ att din CAN-hårdvara är ansluten.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="260"/>
        <source>Unable to create CAN bus device. Check your hardware and drivers.</source>
        <translation>Kan inte skapa CAN-bus-enhet. Kontrollera din hårdvara och drivrutiner.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="285"/>
        <source>Unable to connect to CAN bus device. Check your hardware connection and settings.</source>
        <translation>Kan inte ansluta till CAN-bus-enhet. Kontrollera din hårdvaruanslutning och inställningar.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="666"/>
        <source>CAN Bus Error</source>
        <translation>CAN-bussfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="667"/>
        <source>An error occurred but the CAN device is no longer available.</source>
        <translation>Ett fel uppstod men CAN-enheten är inte längre tillgänglig.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="674"/>
        <source>Error code: %1</source>
        <translation>Felkod: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="677"/>
        <source>CAN Bus Communication Error</source>
        <translation>CAN-busskommunikationsfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="734"/>
        <source>No CAN driver selected</source>
        <translation>Ingen CAN-drivrutin vald</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="696"/>
        <source>Load SocketCAN kernel modules first</source>
        <translation>Ladda SOCKETCAN-kärnmoduler först</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="692"/>
        <source>Connect a %1 adapter, then refresh</source>
        <translation>Anslut en %1-adapter och uppdatera sedan</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="699"/>
        <source>Set up a virtual CAN interface first</source>
        <translation>Konfigurera ett virtuellt CAN-gränssnitt först</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="701"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="721"/>
        <source>No interfaces found for %1</source>
        <translation>Inga gränssnitt hittades för %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="705"/>
        <source>Install &lt;a href='https://www.peak-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN drivers&lt;/a&gt;</source>
        <translation>Installera &lt;a href='https://www.PEAK-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN-drivrutiner&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="709"/>
        <source>Install &lt;a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'&gt;Vector CAN drivers&lt;/a&gt;</source>
        <translation>Installera &lt;a href='https://www.VECTOR.com/us/en/products/products-a-z/libraries-drivers/'&gt;VECTOR CAN-drivrutiner&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="713"/>
        <source>Install &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN drivers&lt;/a&gt;</source>
        <translation>Installera &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN-drivrutiner&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="716"/>
        <source>Install %1 drivers</source>
        <translation>Installera %1-drivrutiner</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="719"/>
        <source>Install %1 drivers for macOS</source>
        <translation>Installera %1-drivrutiner för macOS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="815"/>
        <source>Plugin</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="823"/>
        <source>Interface</source>
        <translation>Gränssnitt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="831"/>
        <source>Bitrate</source>
        <translation>Bithastighet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="840"/>
        <source>CAN FD</source>
        <translation>CAN-FD</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="847"/>
        <source>Loopback</source>
        <translation>Loopback</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="854"/>
        <source>Listen-Only</source>
        <translation>Endast Lyssning</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::GsUsbCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="381"/>
        <source>Failed to initialize libusb for the CANable adapter.</source>
        <translation>Misslyckades att initiera libusb för CANable-adaptern.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="409"/>
        <source>Unable to enumerate USB devices.</source>
        <translation>Kan inte räkna upp USB-enheter.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="429"/>
        <source>The selected CANable adapter is no longer connected, or another application has it open. On Windows the device must use the WinUSB driver (candleLight installs it automatically).</source>
        <translation>Den valda CANable-adaptern är inte längre ansluten, eller så har ett annat program öppnat den. På Windows måste enheten använda WinUSB-drivrutinen (candleLight installerar den automatiskt).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="442"/>
        <source>Could not claim the CANable USB interface.</source>
        <translation>Kunde inte ta kontroll över CANable USB-gränssnittet.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="509"/>
        <source>CANable adapter is not open for writing.</source>
        <translation>CANable-adaptern är inte öppen för skrivning.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="544"/>
        <source>Failed to transmit CAN frame to the adapter.</source>
        <translation>Misslyckades med att överföra CAN-frame till adaptern.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="560"/>
        <source>CAN bus error reported by the CANable adapter.</source>
        <translation>CAN-bussfel rapporterat av CANable-adaptern.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="615"/>
        <source>A CAN frame was not acknowledged on the bus.</source>
        <translation>En CAN-frame bekräftades inte på bussen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="707"/>
        <source>CANable adapter rejected the host-format handshake.</source>
        <translation>CANable-adaptern avvisade värdformatshandskakningen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="714"/>
        <source>Could not read CANable timing constants.</source>
        <translation>Kunde inte läsa CANable-timingkonstanter.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="720"/>
        <source>The bitrate %1 bps is not supported by this CANable adapter.</source>
        <translation>Bithastigheten %1 bps stöds inte av denna CANable-adapter.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="726"/>
        <source>CANable adapter rejected the requested bitrate.</source>
        <translation>CANable-adaptern avvisade den begärda bithastigheten.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="740"/>
        <source>Could not start the CANable channel.</source>
        <translation>Kunde inte starta CANable-kanalen.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::HID</name>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="173"/>
        <source>Unknown error</source>
        <translation>Okänt fel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="176"/>
        <source>

Check that your user is in the 'plugdev' group or that a udev rule grants access to this device.</source>
        <translation>Kontrollera att din användare är i gruppen 'plugdev' eller att en udev-regel ger åtkomst till denna enhet.

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="180"/>
        <source>Failed to open "%1"</source>
        <translation>Misslyckades att öppna "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="285"/>
        <source>HID Device Error</source>
        <translation>HID-enhetsfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="286"/>
        <source>The HID device was disconnected or encountered a fatal read error.</source>
        <translation>HID-enheten kopplades från eller påträffade ett allvarligt läsfel.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="395"/>
        <source>Select Device</source>
        <translation>Välj Enhet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="537"/>
        <source>HID Device</source>
        <translation>HID-enhet</translation>
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
        <translation>TLS 1.3 eller Senare</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="62"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 eller Senare</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="63"/>
        <source>Any Protocol</source>
        <translation>Valfritt Protokoll</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="64"/>
        <source>Secure Protocols Only</source>
        <translation>Endast Säkra Protokoll</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="66"/>
        <source>None</source>
        <translation>Ingen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="67"/>
        <source>Query Peer</source>
        <translation>Fråga Motpart</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="68"/>
        <source>Verify Peer</source>
        <translation>Verifiera Motpart</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="69"/>
        <source>Auto Verify Peer</source>
        <translation>Autoverifiera Motpart</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="167"/>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation>MQTT-Funktion Kräver Kommersiell Licens</translation>
    </message>
    <message>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio commercial license (Hobbyist tier or above).</source>
        <translation type="vanished">Prenumeration på en MQTT-broker är endast tillgänglig med en giltig kommersiell licens för Serial Studio (Hobbyist-nivå eller högre).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="168"/>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio license or an active trial.</source>
        <translation>Prenumeration på en MQTT-broker är endast tillgänglig med en giltig Serial Studio-licens eller en aktiv provperiod.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="393"/>
        <source>Use System Database</source>
        <translation>Använd Systemdatabas</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="394"/>
        <source>Load From Folder…</source>
        <translation>Ladda från Mapp…</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="427"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Välj Pem-certifikatkatalog</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="682"/>
        <source>Hostname</source>
        <translation>Värdnamn</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="689"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="698"/>
        <source>Topic Filter</source>
        <translation>Ämnesfilter</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="705"/>
        <source>Client ID</source>
        <translation>Klient-ID</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="712"/>
        <source>Username</source>
        <translation>Användarnamn</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="719"/>
        <source>Password</source>
        <translation>Lösenord</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="726"/>
        <source>MQTT Version</source>
        <translation>MQTT-version</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="734"/>
        <source>Clean Session</source>
        <translation>Rensa Session</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="741"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="750"/>
        <source>Auto Keep Alive</source>
        <translation>Automatisk Keep Alive</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="767"/>
        <source>SSL/TLS Enabled</source>
        <translation>SSL/TLS Aktiverad</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="777"/>
        <source>SSL Protocol</source>
        <translation>SSL-protokoll</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="785"/>
        <source>Peer Verify Mode</source>
        <translation>Motpartsverifieringsläge</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="793"/>
        <source>Peer Verify Depth</source>
        <translation>Motpartsverifieringsdjup</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="894"/>
        <source>MQTT Subscription Error</source>
        <translation>MQTT-prenumerationsfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="895"/>
        <source>Failed to subscribe to topic "%1".</source>
        <translation>Misslyckades prenumerera på ämne "%1".</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="922"/>
        <source>Invalid MQTT Protocol Version</source>
        <translation>Ogiltig MQTT-protokollversion</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="923"/>
        <source>The broker rejected the configured MQTT protocol version.</source>
        <translation>Mäklaren nekade den konfigurerade MQTT-protokollversionen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="926"/>
        <source>Client ID Rejected</source>
        <translation>Klient-ID Nekat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="927"/>
        <source>The broker rejected the client ID. Try a different identifier.</source>
        <translation>Mäklaren nekade klient-ID:t. Prova en annan identifierare.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="930"/>
        <source>MQTT Server Unavailable</source>
        <translation>MQTT-server Ej Tillgänglig</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="931"/>
        <source>The broker is currently unavailable. Retry later.</source>
        <translation>Brokern är för närvarande otillgänglig. Försök igen senare.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="934"/>
        <source>Authentication Error</source>
        <translation>Autentiseringsfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="935"/>
        <source>The credentials provided were rejected by the broker.</source>
        <translation>De angivna inloggningsuppgifterna avvisades av brokern.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="938"/>
        <source>Authorization Error</source>
        <translation>Auktoriseringsfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="939"/>
        <source>Account lacks permission for this operation.</source>
        <translation>Kontot saknar behörighet för denna åtgärd.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="942"/>
        <source>Network or Transport Error</source>
        <translation>Nätverks- eller transportfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="943"/>
        <source>Network/transport layer issue while connecting to the broker.</source>
        <translation>Problem med nätverks-/transportlagret vid anslutning till brokern.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="946"/>
        <source>MQTT Protocol Violation</source>
        <translation>MQTT-protokollöverträdelse</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="947"/>
        <source>The broker reported a protocol violation and closed the connection.</source>
        <translation>Brokern rapporterade en protokollöverträdelse och stängde anslutningen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="950"/>
        <source>MQTT 5 Error</source>
        <translation>MQTT 5-Fel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="951"/>
        <source>An MQTT 5 protocol-level error occurred.</source>
        <translation>Ett MQTT 5-protokollfel inträffade.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="954"/>
        <source>MQTT Error</source>
        <translation>MQTT-fel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="955"/>
        <source>An unexpected MQTT error occurred.</source>
        <translation>Ett oväntat MQTT-fel inträffade.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Modbus</name>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="364"/>
        <source>Invalid Serial Port</source>
        <translation>Ogiltig Serieport</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="409"/>
        <source>Modbus Initialization Failed</source>
        <translation>Modbus-initiering Misslyckades</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="435"/>
        <source>Modbus Connection Failed</source>
        <translation>Modbus-anslutning Misslyckades</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="365"/>
        <source>The selected serial port "%1" is no longer available. Refresh the port list and try again.</source>
        <translation>Den valda serieporten "%1" är inte längre tillgänglig. Uppdatera portlistan och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="410"/>
        <source>Unable to create Modbus device. Check your system configuration and try again.</source>
        <translation>Kan inte skapa Modbus-enhet. Kontrollera systemkonfigurationen och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="437"/>
        <source>Unable to connect to "%1". Check your connection settings.</source>
        <translation>Kan inte ansluta till "%1". Kontrollera anslutningsinställningarna.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="438"/>
        <source>"%1": %2</source>
        <translation>"%1": %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="559"/>
        <source>None</source>
        <translation>Ingen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="560"/>
        <source>Even</source>
        <translation>Jämn</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="561"/>
        <source>Odd</source>
        <translation>Udda</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="562"/>
        <source>Space</source>
        <translation>Space</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="563"/>
        <source>Mark</source>
        <translation>Mark</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="615"/>
        <source>Holding Registers (0x03)</source>
        <translation>Holding-register (0x03)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="616"/>
        <source>Input Registers (0x04)</source>
        <translation>Input-register (0x04)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="617"/>
        <source>Coils (0x01)</source>
        <translation>Coils (0x01)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="618"/>
        <source>Discrete Inputs (0x02)</source>
        <translation>Diskreta ingångar (0x02)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="800"/>
        <source>No register groups configured</source>
        <translation>Inga registergrupper konfigurerade</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="801"/>
        <source>Add at least one register group before generating a project.</source>
        <translation>Lägg till minst en registergrupp innan du genererar ett projekt.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="803"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="815"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="840"/>
        <source>Modbus Project Generator</source>
        <translation>Modbus-projektgenerator</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Misslyckades att skapa temporär projektfil</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Kontrollera skrivbehörigheter till den temporära katalogen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="812"/>
        <source>Failed to load generated project</source>
        <translation>Misslyckades att Ladda Genererat Projekt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="813"/>
        <source>The generated project JSON could not be loaded.</source>
        <translation>Det genererade projekt-JSON kunde inte laddas.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="835"/>
        <source>Successfully generated project with %1 groups and %2 datasets.</source>
        <translation>Projektet genererades framgångsrikt med %1 grupper och %2 datamängder.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="838"/>
        <source>The project editor is now open for customization.</source>
        <translation>Projektredigeraren är nu öppen för anpassning.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="853"/>
        <source>Modbus Project</source>
        <translation>Modbus-projekt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="858"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="878"/>
        <source>Holding Registers</source>
        <translation>Holding-register</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="879"/>
        <source>Input Registers</source>
        <translation>Ingångsregister</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="880"/>
        <source>Coils</source>
        <translation>Spolar</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="881"/>
        <source>Discrete Inputs</source>
        <translation>Diskreta Ingångar</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="895"/>
        <source>Unknown</source>
        <translation>Okänd</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="908"/>
        <source>Register %1</source>
        <translation>Register %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="916"/>
        <source>Coil %1</source>
        <translation>Spole %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="916"/>
        <source>Discrete %1</source>
        <translation>Diskret %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1322"/>
        <source>Error code: %1</source>
        <translation>Felkod: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1325"/>
        <source>Modbus Communication Error</source>
        <translation>Modbus-kommunikationsfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1337"/>
        <source>Select Port</source>
        <translation>Välj Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1490"/>
        <source>Protocol</source>
        <translation>Protokoll</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1498"/>
        <source>Slave Address</source>
        <translation>Slavadress</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1507"/>
        <source>Poll Interval (ms)</source>
        <translation>Pollintervall (ms)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1547"/>
        <source>Host / IP</source>
        <translation>Värd / IP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1554"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1569"/>
        <source>Serial Port</source>
        <translation>Serieport</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1577"/>
        <source>Baud Rate</source>
        <translation>Baudhastighet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1585"/>
        <source>Parity</source>
        <translation>Paritet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1593"/>
        <source>Data Bits</source>
        <translation>Databitar</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1601"/>
        <source>Stop Bits</source>
        <translation>Stoppbitar</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Network</name>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="473"/>
        <source>Network socket error</source>
        <translation>Nätverksuttag fel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="489"/>
        <source>Socket Type</source>
        <translation>Uttagstyp</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="497"/>
        <source>Remote Address</source>
        <translation>Fjärradress</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="505"/>
        <source>TCP Port</source>
        <translation>TCP-port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="514"/>
        <source>UDP Local Port</source>
        <translation>UDP Lokal Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="523"/>
        <source>UDP Remote Port</source>
        <translation>UDP Fjärrport</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="532"/>
        <source>UDP Multicast</source>
        <translation>UDP Multicast</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Process</name>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="183"/>
        <location filename="../../src/IO/Drivers/Process.cpp" line="224"/>
        <source>Failed to start process</source>
        <translation>Misslyckades att starta process</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="184"/>
        <source>Executable "%1" not found in PATH.</source>
        <translation>Körbar fil "%1" hittades inte i PATH.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="368"/>
        <source>Select Executable</source>
        <translation>Välj Körbar Fil</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="392"/>
        <source>Select Working Directory</source>
        <translation>Välj Arbetskatalog</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="417"/>
        <source>Select Named Pipe / FIFO</source>
        <translation>Välj Named Pipe / FIFO</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="514"/>
        <source>The process crashed.</source>
        <translation>Processen kraschade.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="515"/>
        <source>Exit code: %1</source>
        <translation>Avslutningskod: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="518"/>
        <source>Process "%1" stopped</source>
        <translation>Processen "%1" stoppades</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="536"/>
        <source>Unknown error</source>
        <translation>Okänt fel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="537"/>
        <source>Process Error</source>
        <translation>Processfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Pipe Error</source>
        <translation>Pipe-fel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Could not open named pipe: %1</source>
        <translation>Kunde inte öppna named pipe: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="763"/>
        <source>Mode</source>
        <translation>Läge</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Launch Process</source>
        <translation>Starta Process</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Named Pipe</source>
        <translation>Namngiven Pipe</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="771"/>
        <source>Executable</source>
        <translation>Körbar Fil</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="778"/>
        <source>Arguments</source>
        <translation>Argument</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="785"/>
        <source>Working Directory</source>
        <translation>Arbetskatalog</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="792"/>
        <source>Pipe Path</source>
        <translation>Pipe-sökväg</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SeeedCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="205"/>
        <source>The bitrate %1 bps is not supported by the USB-CAN Analyzer.</source>
        <translation>Bithastigheten %1 bps stöds inte av USB-CAN-analysatorn.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="215"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Kunde inte öppna seriell port %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="226"/>
        <source>Failed to initialize the USB-CAN Analyzer.</source>
        <translation>Misslyckades att initialisera USB-CAN-analysatorn.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="263"/>
        <source>USB-CAN Analyzer is not open for writing.</source>
        <translation>USB-CAN-analysatorn är inte öppen för skrivning.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="309"/>
        <source>CAN bus error reported by the USB-CAN Analyzer.</source>
        <translation>CAN-bussfel rapporterat av USB-CAN-analysatorn.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SlcanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="166"/>
        <source>The bitrate %1 bps is not a standard slcan rate.</source>
        <translation>Bithastigheten %1 bps är inte en standard slcan-hastighet.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="176"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Kunde inte öppna seriell port %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="189"/>
        <source>Failed to open the slcan channel.</source>
        <translation>Misslyckades att öppna slcan-kanalen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="228"/>
        <source>slcan adapter is not open for writing.</source>
        <translation>slcan-adaptern är inte öppen för skrivning.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="266"/>
        <source>CAN bus error reported by the slcan adapter.</source>
        <translation>CAN-bussfel rapporterat av slcan-adaptern.</translation>
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
        <translation>Ingen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="333"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="717"/>
        <source>Select Port</source>
        <translation>Välj Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="376"/>
        <source>Even</source>
        <translation>Jämn</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="377"/>
        <source>Odd</source>
        <translation>Udda</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="378"/>
        <source>Space</source>
        <translation>Mellanslag</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="379"/>
        <source>Mark</source>
        <translation>Markering</translation>
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
        <translation>"%1" är inte en giltig sökväg</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="547"/>
        <source>Please type another path to register a custom serial device</source>
        <translation>Ange en annan sökväg för att registrera en anpassad serieenhet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="807"/>
        <source>The specified device could not be found. Check the connection and try again.</source>
        <translation>Den angivna enheten kunde inte hittas. Kontrollera anslutningen och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="814"/>
        <source>An unknown error occurred. Check the device and try again.</source>
        <translation>Ett okänt fel uppstod. Kontrollera enheten och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="816"/>
        <source>The device is not open. Open the device before attempting this operation.</source>
        <translation>Enheten är inte öppen. Öppna enheten innan du försöker utföra denna åtgärd.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="245"/>
        <source>Failed to connect to serial port "%1"</source>
        <translation>Misslyckades att ansluta till serieport "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="782"/>
        <source>Unknown</source>
        <translation>Okänd</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="783"/>
        <source>Critical error on serial port "%1"</source>
        <translation>Kritiskt fel på serieporten "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="784"/>
        <source>Unknown error</source>
        <translation>Okänt fel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="806"/>
        <source>No error occurred.</source>
        <translation>Inget fel uppstod.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="808"/>
        <source>Permission denied. Ensure the application has the necessary access rights to the device.</source>
        <translation>Åtkomst nekad. Säkerställ att applikationen har nödvändiga åtkomsträttigheter till enheten.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="809"/>
        <source>Failed to open the device. It may already be in use or unavailable.</source>
        <translation>Misslyckades att öppna enheten. Den kan redan vara i bruk eller otillgänglig.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="810"/>
        <source>An error occurred while writing data to the device.</source>
        <translation>Ett fel uppstod vid skrivning av data till enheten.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="811"/>
        <source>An error occurred while reading data from the device.</source>
        <translation>Ett fel uppstod vid läsning av data från enheten.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="812"/>
        <source>A critical resource error occurred. The device may have been disconnected or is no longer accessible.</source>
        <translation>Ett kritiskt resursfel uppstod. Enheten kan ha kopplats från eller är inte längre tillgänglig.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="813"/>
        <source>The requested operation is not supported on this device.</source>
        <translation>Den begärda operationen stöds inte på denna enhet.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="815"/>
        <source>The operation timed out. The device may not be responding.</source>
        <translation>Operationen avbröts på grund av timeout. Enheten svarar eventuellt inte.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="978"/>
        <source>Serial Port</source>
        <translation>Seriell Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="986"/>
        <source>Baud Rate</source>
        <translation>Baudhastighet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="994"/>
        <source>Parity</source>
        <translation>Paritet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1002"/>
        <source>Data Bits</source>
        <translation>Databitar</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1010"/>
        <source>Stop Bits</source>
        <translation>Stoppbitar</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1018"/>
        <source>Flow Control</source>
        <translation>Flödeskontroll</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1026"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1033"/>
        <source>Auto-Reconnect</source>
        <translation>Återanslut Automatiskt</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::USB</name>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="169"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="177"/>
        <source>USB Error</source>
        <translation>USB-fel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="170"/>
        <source>Failed to initialize the USB subsystem. Check that libusb is available on your system.</source>
        <translation>Misslyckades att initiera USB-undersystemet. Kontrollera att libusb är tillgängligt på ditt system.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="212"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="227"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="612"/>
        <source>USB Device Error</source>
        <translation>USB-enhetsfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="190"/>
        <source>Could not open the USB device: %1.

On Linux, ensure you have read/write permission on the device node (add a udev rule or run as root). On macOS, the kernel driver may need to be detached first.</source>
        <translation>Kunde inte öppna USB-enheten: %1.

På Linux, säkerställ att du har läs-/skrivbehörighet på enhetsnoden (lägg till en udev-regel eller kör som root). På macOS kan kärndrivrutinen behöva kopplas bort först.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="178"/>
        <source>No USB device selected. Select a device and try again.</source>
        <translation>Ingen USB-enhet vald. Välj en enhet och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="184"/>
        <source>Unknown Device</source>
        <translation>Okänd Enhet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="189"/>
        <source>Failed to open "%1"</source>
        <translation>Misslyckades att öppna "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="228"/>
        <source>Could not claim interface %1 on the USB device.

Another driver or application may already have it open. On Linux, try unloading the kernel driver (e.g. cdc_acm) or adding a udev rule.</source>
        <translation>Kunde inte ta kontroll över gränssnitt %1 på USB-enheten.

En annan drivrutin eller applikation kan redan ha den öppen. På Linux, försök att ta bort kärndrivrutinen (t.ex. cdc_acm) eller lägg till en udev-regel.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="386"/>
        <source>Select Device</source>
        <translation>Välj Enhet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="405"/>
        <source>Select IN Endpoint</source>
        <translation>Välj In-slutpunkt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="416"/>
        <source>None (Read-only)</source>
        <translation>Ingen (Skrivskyddad)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="490"/>
        <source>Enable Advanced USB Control Transfers?</source>
        <translation>Aktivera Avancerade USB-kontrollöverföringar?</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="491"/>
        <source>This enables control transfers in addition to bulk transfers. Sending incorrect control requests can crash or damage connected hardware. Only enable this if you know what you are doing.</source>
        <translation>Detta aktiverar kontrollöverföringar utöver bulköverföringar. Att skicka felaktiga kontrollförfrågningar kan krascha eller skada ansluten hårdvara. Aktivera endast detta om du vet vad du gör.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="495"/>
        <source>Advanced USB Mode</source>
        <translation>Avancerat USB-läge</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="613"/>
        <source>The USB device was disconnected or encountered a fatal read error.</source>
        <translation>USB-enheten kopplades från eller påträffade ett allvarligt läsfel.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="750"/>
        <source>No isochronous IN endpoint was found on this device, but bulk endpoints are available.

Switch the Transfer Mode to "Bulk Stream" and try again.</source>
        <translation>Ingen isokron IN-ändpunkt hittades på denna enhet, men bulk-ändpunkter är tillgängliga.

Byt överföringsläge till "Bulk Stream" och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="755"/>
        <source>No bulk IN endpoint was found on this device, but isochronous endpoints are available.

Switch the Transfer Mode to "Isochronous" and try again.</source>
        <translation>Ingen bulk IN-ändpunkt hittades på denna enhet, men isokrona ändpunkter är tillgängliga.

Byt överföringsläge till "Isokron" och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="759"/>
        <source>No usable IN endpoint was found on this device.

The device may not expose data endpoints in its active configuration, or it may require a specific driver.</source>
        <translation>Ingen användbar IN-ändpunkt hittades på denna enhet.

Enheten kanske inte exponerar dataändpunkter i sin aktiva konfiguration, eller så kan den kräva en specifik drivrutin.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1185"/>
        <source>USB Device</source>
        <translation>USB-enhet</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1193"/>
        <source>Transfer Mode</source>
        <translation>Överföringsläge</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1196"/>
        <source>Bulk Stream</source>
        <translation>Bulk Stream</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1196"/>
        <source>Advanced Control</source>
        <translation>Avancerad Kontroll</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1196"/>
        <source>Isochronous</source>
        <translation>Isokron</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1201"/>
        <source>IN Endpoint</source>
        <translation>In-slutpunkt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1209"/>
        <source>OUT Endpoint</source>
        <translation>Out-slutpunkt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1217"/>
        <source>ISO Packet Size</source>
        <translation>ISO-paketstorlek</translation>
    </message>
</context>
<context>
    <name>IO::FileTransmission</name>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="211"/>
        <source>No file selected…</source>
        <translation>Ingen fil vald…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="246"/>
        <source>Plain Text</source>
        <translation>Oformaterad Text</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="247"/>
        <source>Raw Binary</source>
        <translation>Rå Binär</translation>
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
        <translation>Välj fil att överföra</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="291"/>
        <source>File selected: %1 (%2 bytes)</source>
        <translation>Fil vald: %1 (%2 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="294"/>
        <source>Error opening file: %1</source>
        <translation>Fel vid öppning av fil: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="380"/>
        <source>Starting %1 transfer…</source>
        <translation>Startar %1-överföring…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="608"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="628"/>
        <source>Transmission complete</source>
        <translation>Överföring slutförd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="610"/>
        <source>Plain text transmission complete</source>
        <translation>Överföring av oformaterad text slutförd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="630"/>
        <source>Raw binary transmission complete (%1 bytes)</source>
        <translation>Överföring av rå binärdata slutförd (%1 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="654"/>
        <source>Transfer complete</source>
        <translation>Överföring slutförd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="655"/>
        <source>Transfer completed successfully (%1 bytes)</source>
        <translation>Överföring slutförd (%1 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="657"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="658"/>
        <source>Transfer failed: %1</source>
        <translation>Överföring misslyckades: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="742"/>
        <source>%1 B/s</source>
        <translation>%1 B/s</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="744"/>
        <source>%1 KB/s</source>
        <translation>%1 Kb/s</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="746"/>
        <source>%1 MB/s</source>
        <translation>%1 Mb/s</translation>
    </message>
</context>
<context>
    <name>IO::FrameReader</name>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="350"/>
        <source>Frames dropped</source>
        <translation>Ramar borttappade</translation>
    </message>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="352"/>
        <source>Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) have been dropped. Reduce the data rate or disable a heavy consumer.</source>
        <translation>Inkommande data anländer snabbare än Serial Studio kan bearbeta den; %1 ram(ar) har tappats bort. Minska datahastigheten eller inaktivera en tung konsument.</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::XMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="83"/>
        <source>Cannot open file: %1</source>
        <translation>Kan inte öppna fil: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="112"/>
        <source>Transfer cancelled</source>
        <translation>Överföring avbruten</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="113"/>
        <source>Transfer cancelled by user</source>
        <translation>Överföring avbruten av användare</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="93"/>
        <source>Waiting for receiver…</source>
        <translation>Väntar på mottagare…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="207"/>
        <source>Receiver ready (CRC mode), sending data…</source>
        <translation>Mottagare redo (CRC-läge), skickar data…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="142"/>
        <source>Too many retries, transfer aborted</source>
        <translation>För många återförsök, överföring avbruten</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="143"/>
        <source>Maximum retries exceeded</source>
        <translation>Maximalt antal försök överskridet</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="147"/>
        <source>NAK received, retrying block %1 (%2/%3)</source>
        <translation>NAK mottagen, försöker block %1 igen (%2/%3)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="155"/>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="384"/>
        <source>Failed to seek in file</source>
        <translation>Misslyckades att söka i fil</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="165"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Överföring avbruten av mottagare</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="166"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Mottagare avbröt överföringen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="180"/>
        <source>Transfer complete</source>
        <translation>Överföring slutförd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="302"/>
        <source>File read returned more data than requested</source>
        <translation>Filläsning returnerade mer data än begärt</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="316"/>
        <source>Sending block %1 (%2 bytes)</source>
        <translation>Skickar block %1 (%2 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="329"/>
        <source>Sending EOT…</source>
        <translation>Skickar EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="375"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Timeout, försöker igen (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="370"/>
        <source>Transfer timed out</source>
        <translation>Överföringen avbröts på grund av timeout</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="371"/>
        <source>Timeout: no response from receiver</source>
        <translation>Timeout: inget svar från mottagare</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::YMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="62"/>
        <source>Cannot open file: %1</source>
        <translation>Kan inte öppna fil: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="96"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="173"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Överföring avbruten av mottagare</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="97"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="174"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Mottagaren avbröt överföringen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="74"/>
        <source>Waiting for receiver…</source>
        <translation>Väntar på mottagare…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="133"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="316"/>
        <source>Sending first EOT…</source>
        <translation>Skickar första EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="151"/>
        <source>Too many retries, transfer aborted</source>
        <translation>För många återförsök, överföring avbruten</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="152"/>
        <source>Maximum retries exceeded</source>
        <translation>Maximalt antal återförsök överskridet</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="156"/>
        <source>NAK received, retrying block %1</source>
        <translation>NAK mottagen, försöker igen med block %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="161"/>
        <source>Failed to seek in file</source>
        <translation>Misslyckades att söka i fil</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="220"/>
        <source>Sending second EOT…</source>
        <translation>Skickar andra EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="299"/>
        <source>Sending end-of-batch marker…</source>
        <translation>Skickar slutmarkör för batch…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="242"/>
        <source>Transfer complete</source>
        <translation>Överföring slutförd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="284"/>
        <source>Sending file header: %1 (%2 bytes)</source>
        <translation>Skickar filhuvud: %1 (%2 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="330"/>
        <source>Sending block %1 (%2/%3 bytes)</source>
        <translation>Skickar block %1 (%2/%3 byte)</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::ZMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="86"/>
        <source>Cannot open file: %1</source>
        <translation>Kan inte öppna fil: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="103"/>
        <source>File is too large for ZMODEM (%1 bytes, limit 4 GiB).</source>
        <translation>Filen är för stor för ZMODEM (%1 byte, gräns 4 GiB).</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="128"/>
        <source>Transfer cancelled</source>
        <translation>Överföring avbruten</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="129"/>
        <source>Transfer cancelled by user</source>
        <translation>Överföring avbruten av användare</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="269"/>
        <source>Hex header CRC mismatch, dropping frame</source>
        <translation>Hex-huvud CRC-felmatchning, släpper frame</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="444"/>
        <source>Sending file info: %1 (%2 bytes)</source>
        <translation>Skickar filinformation: %1 (%2 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="459"/>
        <source>Failed to seek to offset %1</source>
        <translation>Misslyckades att söka till offset %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="488"/>
        <source>File read returned more data than requested</source>
        <translation>Filläsning returnerade mer data än begärt</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="571"/>
        <source>Receiver requests data from offset %1</source>
        <translation>Mottagare begär data från offset %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="579"/>
        <source>Receiver skipped the file</source>
        <translation>Mottagare hoppade över filen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="591"/>
        <source>Too many errors, transfer aborted</source>
        <translation>För många fel, överföring avbruten</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="592"/>
        <source>Maximum retries exceeded</source>
        <translation>Maximalt antal återförsök överskridet</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="420"/>
        <source>Sending ZRQINIT, waiting for receiver…</source>
        <translation>Skickar ZRQINIT, väntar på mottagare…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="514"/>
        <source>File data sent, waiting for confirmation…</source>
        <translation>Fildata skickad, väntar på bekräftelse…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="525"/>
        <source>Sending ZFIN…</source>
        <translation>Skickar ZFIN…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="561"/>
        <source>Receiver ready, sending file info…</source>
        <translation>Mottagare redo, skickar filinformation…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="596"/>
        <source>NAK received, retrying (%1/%2)…</source>
        <translation>NAK mottagen, försöker igen (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="617"/>
        <source>Transfer complete</source>
        <translation>Överföring slutförd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="627"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Överföring avbruten av mottagare</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="628"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Mottagare avbröt överföringen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="636"/>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="637"/>
        <source>Receiver reported a file error</source>
        <translation>Mottagare rapporterade ett filfel</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="835"/>
        <source>Transfer timed out</source>
        <translation>Överföring avbröts på grund av timeout</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="836"/>
        <source>Timeout: no response from receiver</source>
        <translation>Timeout: inget svar från mottagare</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="840"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Timeout, försöker igen (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="42"/>
        <source>Select Icon</source>
        <translation>Välj Ikon</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="119"/>
        <source>Search Online…</source>
        <translation>Sök Online…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="132"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="144"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
</context>
<context>
    <name>ImageView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="68"/>
        <source>Normal</source>
        <translation>Normal</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="69"/>
        <source>Grayscale</source>
        <translation>Gråskala</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="70"/>
        <source>High Contrast</source>
        <translation>Hög Kontrast</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="71"/>
        <source>Vivid</source>
        <translation>Levande</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="72"/>
        <source>Night Vision</source>
        <translation>Mörkerseende</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="73"/>
        <source>Infrared</source>
        <translation>Infraröd</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="74"/>
        <source>Deep Blue</source>
        <translation>Djupblå</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="75"/>
        <source>Amber</source>
        <translation>Bärnsten</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="172"/>
        <source>Export Images</source>
        <translation>Exportera Bilder</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="182"/>
        <source>Open Export Folder</source>
        <translation>Öppna Exportmapp</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="198"/>
        <source>Zoom In</source>
        <translation>Zooma In</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="211"/>
        <source>Zoom Out</source>
        <translation>Zooma Ut</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="231"/>
        <source>Show Crosshair</source>
        <translation>Visa Hårkors</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="556"/>
        <source>Waiting for Image…</source>
        <translation>Väntar på Bild…</translation>
    </message>
</context>
<context>
    <name>KeyManagerDialog</name>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="23"/>
        <source>API Keys</source>
        <translation>API-nycklar</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="47"/>
        <source>Anthropic Claude. The default is Claude Haiku 4.5 ($1 input / $5 output per million tokens). Sonnet 4.6 and Opus 4.7 are also available. Supports streaming, tool use, extended thinking, and prompt caching.</source>
        <translation>Anthropic Claude. Standardvalet är Claude Haiku 4.5 ($1 indata / $5 utdata per miljon tokens). Sonnet 4.6 och OPUS 4.7 är också tillgängliga. Stöder strömning, verktygsanvändning, utökad bearbetning och prompt-cachning.</translation>
    </message>
    <message>
        <source>OpenAI Chat Completions. The default is GPT-4o mini ($0.15 input / $0.60 output per million tokens). GPT-4o, GPT-4 Turbo, and o1-mini are also available.</source>
        <translation type="vanished">OpenAI Chat Completions. Standardvalet är GPT-4o mini ($0,15 indata / $0,60 utdata per miljon tokens). GPT-4o, GPT-4 Turbo och o1-mini är också tillgängliga.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="57"/>
        <source>Google Gemini. The default is Gemini 2.0 Flash, which has a generous free tier (subject to rate limits). Gemini 1.5 Pro and Gemini 1.5 Flash are also available.</source>
        <translation>Google Gemini. Standardvalet är Gemini 2.0 Flash, som har en generös kostnadsfri nivå (med hastighetsbegränsningar). Gemini 1.5 Pro och Gemini 1.5 Flash är också tillgängliga.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="100"/>
        <source>Bring your own API keys. They are encrypted at rest with a per-machine key and never leave your computer except to communicate with the provider you select.</source>
        <translation>Ta med egna API-nycklar. De krypteras i vila med en maskinspecifik nyckel och lämnar aldrig datorn förutom för att kommunicera med den valda leverantören.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="166"/>
        <source>Key set</source>
        <translation>Nyckel inställd</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="167"/>
        <source>No key</source>
        <translation>Ingen nyckel</translation>
    </message>
    <message>
        <source>A key is on file — paste a new one to replace it</source>
        <translation type="vanished">En nyckel finns sparad — klistra in en ny för att ersätta den</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="52"/>
        <source>OpenAI Chat Completions. The default is GPT-5 mini for fast, cost-conscious agentic work. GPT-5.2 is the stronger general-purpose option, and GPT-5.2 Chat tracks the model currently used in ChatGPT.</source>
        <translation>OpenAI Chat Completions. Standardvalet är GPT-5 mini för snabbt, kostnadseffektivt agentarbete. GPT-5.2 är det starkare allmänna alternativet, och GPT-5.2 Chat följer modellen som för närvarande används i ChatGPT.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="61"/>
        <source>DeepSeek. OpenAI-compatible API. The default is deepseek-chat (V3). deepseek-reasoner (R1) is also available. Often the cheapest cloud option for tool use.</source>
        <translation>DeepSeek. OpenAI-kompatibelt API. Standardvalet är deepseek-chat (V3). deepseek-reasoner (R1) är också tillgänglig. Ofta det billigaste molnalternativet för verktygsanvändning.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="65"/>
        <source>OpenRouter. One key, ~200 models from Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier models (suffixed :free) are rate-limited but require no additional billing. Pay-as-you-go for the rest.</source>
        <translation>OpenRouter. En nyckel, ~200 modeller från Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen och andra. Gratismodeller (suffix :free) är hastighetsbegränsade men kräver ingen ytterligare fakturering. Betala per användning för resten.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="70"/>
        <source>Groq. Hardware-accelerated inference (LPUs) for very fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen models. Generous free tier with daily token limits.</source>
        <translation>Groq. Hårdvaruaccelererad inferens (LPU:er) för mycket snabba Llama-, Mixtral-, Gemma-, DeepSeek-R1 distill- och Qwen-modeller. Generös gratisplan med dagliga tokengränser.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="74"/>
        <source>Mistral. The default is Mistral Large. Codestral targets code completion, Pixtral handles vision, and the Ministral models are tuned for edge / low-latency use.</source>
        <translation>Mistral. Standardmodellen är Mistral Large. Codestral riktar sig mot kodkomplettering, Pixtral hanterar vision och Ministral-modellerna är anpassade för edge/låg latens.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="78"/>
        <source>Local model server. Works with any OpenAI-compatible endpoint -- Ollama, llama.cpp's llama-server, LM Studio, or vLLM. Nothing leaves your machine. The model list is queried live from the server.</source>
        <translation>Lokal modellserver. Fungerar med alla OpenAI-kompatibla slutpunkter -- Ollama, llama.cpp:s llama-server, LM Studio eller vLLM. Ingenting lämnar din maskin. Modelllistan hämtas live från servern.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="204"/>
        <source>A key is on file -- paste a new one to replace it</source>
        <translation>En nyckel finns sparad -- klistra in en ny för att ersätta den</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="205"/>
        <source>Paste your API key here</source>
        <translation>Klistra in din API-nyckel här</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="212"/>
        <source>Hide key</source>
        <translation>Dölj nyckel</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="213"/>
        <source>Show key while typing</source>
        <translation>Visa nyckel vid inmatning</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="224"/>
        <source>Get key</source>
        <translation>Hämta nyckel</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="225"/>
        <source>Open the provider's console to create a new key</source>
        <translation>Öppna leverantörens konsol för att skapa en ny nyckel</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="236"/>
        <source>Save</source>
        <translation>Spara</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="254"/>
        <source>Remove the stored key for %1</source>
        <translation>Ta bort den lagrade nyckeln för %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="278"/>
        <source>http://localhost:11434/v1</source>
        <translation>http://localhost:11434/v1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="282"/>
        <source>Install Ollama</source>
        <translation>Installera Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="283"/>
        <source>Open the Ollama download page</source>
        <translation>Öppna nedladdningssidan för Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="294"/>
        <source>Apply</source>
        <translation>Tillämpa</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="309"/>
        <source>Re-query the model list</source>
        <translation>Uppdatera modelllistan</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="357"/>
        <source>No API keys configured yet. Add a key to get started.</source>
        <translation>Inga API-nycklar konfigurerade ännu. Lägg till en nyckel för att komma igång.</translation>
    </message>
    <message>
        <source>No API keys configured yet. Add at least one above to get started.</source>
        <translation type="vanished">Inga API-nycklar konfigurerade ännu. Lägg till minst en ovan för att komma igång.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="360"/>
        <source>One provider is ready.</source>
        <translation>En leverantör är redo.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="362"/>
        <source>%1 providers are ready.</source>
        <translation>%1 leverantörer är redo.</translation>
    </message>
</context>
<context>
    <name>LicenseManagement</name>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="37"/>
        <source>Licensing</source>
        <translation>Licensiering</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="84"/>
        <source>Please wait…</source>
        <translation>Vänligen vänta…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="124"/>
        <source>Activate Serial Studio Pro</source>
        <translation>Aktivera Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="131"/>
        <source>Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.</source>
        <translation>Klistra in din licensnyckel nedan för att låsa upp Pro-funktioner som MQTT, 3D-plottning och mer.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="138"/>
        <source>Your license includes 5 device activations.
Plans include Monthly, Yearly, and Lifetime options.</source>
        <translation>Din licens inkluderar 5 enhetsaktivering.
Abonnemang inkluderar månads-, års- och livstidsalternativ.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="151"/>
        <source>Paste your license key here…</source>
        <translation>Klistra in din licensnyckel här…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="170"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="332"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="381"/>
        <source>Copy</source>
        <translation>Kopiera</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="176"/>
        <source>Paste</source>
        <translation>Klistra In</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="182"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="338"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="387"/>
        <source>Select All</source>
        <translation>Markera Allt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="234"/>
        <source>Product</source>
        <translation>Produkt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="241"/>
        <source>Serial Studio %1</source>
        <translation>Serial Studio %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="252"/>
        <source>Licensee</source>
        <translation>Licensinnehavare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="271"/>
        <source>Licensee E-Mail</source>
        <translation>Licensinnehavarens E-post</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="288"/>
        <source>Device Usage</source>
        <translation>Enhetsanvändning</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="296"/>
        <source>%1 devices in use (Unlimited plan)</source>
        <translation>%1 enheter i bruk (Obegränsad plan)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="297"/>
        <source>%1 of %2 devices used</source>
        <translation>%1 av %2 enheter använda</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="307"/>
        <source>Device ID</source>
        <translation>Enhets-ID</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="354"/>
        <source>License Key</source>
        <translation>Licensnyckel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="409"/>
        <source>Customer Portal</source>
        <translation>Kundportal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="420"/>
        <source>Buy License</source>
        <translation>Köp Licens</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="427"/>
        <source>Activate</source>
        <translation>Aktivera</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="437"/>
        <source>Deactivate</source>
        <translation>Avaktivera</translation>
    </message>
</context>
<context>
    <name>Licensing::LemonSqueezy</name>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="517"/>
        <source>There was an issue validating your license.</source>
        <translation>Det uppstod ett problem vid validering av din licens.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="535"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="716"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="835"/>
        <source>The license key you provided does not belong to Serial Studio.</source>
        <translation>Licensnyckeln du angav tillhör inte Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="536"/>
        <source>Please double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Kontrollera att du köpt din licens från den officiella Serial Studio-butiken.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="547"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="725"/>
        <source>This license key was activated on a different device.</source>
        <translation>Denna licensnyckel aktiverades på en annan enhet.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="548"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="726"/>
        <source>Deactivate it there first or contact support for help.</source>
        <translation>Avaktivera den där först eller kontakta support för hjälp.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="559"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="736"/>
        <source>This license is not currently active.</source>
        <translation>Denna licens är inte aktiv för närvarande.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="560"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="737"/>
        <source>It may have expired or been deactivated (status: %1).</source>
        <translation>Den kan ha gått ut eller avaktiverats (status: %1).</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="570"/>
        <source>Something went wrong on the server.</source>
        <translation>Något gick fel på servern.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="571"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="747"/>
        <source>No activation ID was returned.</source>
        <translation>Inget aktiverings-ID returnerades.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="581"/>
        <source>Could not validate your license at this time.</source>
        <translation>Kunde inte validera din licens just nu.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="582"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="756"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="845"/>
        <source>Try again later.</source>
        <translation>Försök igen senare.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="717"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="836"/>
        <source>Double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Dubbelkolla att du köpte din licens från den officiella Serial Studio-butiken.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="746"/>
        <source>Something went wrong on the server…</source>
        <translation>Något gick fel på servern…</translation>
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
        <translation>Din licens har aktiverats.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="654"/>
        <source>Thank you for supporting Serial Studio!
You now have access to all premium features.</source>
        <translation>Tack för att du stödjer Serial Studio!
Du har nu tillgång till alla premiumfunktioner.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="708"/>
        <source>There was an issue activating your license.</source>
        <translation>Ett problem uppstod vid aktivering av din licens.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="755"/>
        <source>Could not activate your license at this time.</source>
        <translation>Kunde inte aktivera din licens just nu.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="827"/>
        <source>There was an issue deactivating your license.</source>
        <translation>Ett problem uppstod vid avaktivering av din licens.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="844"/>
        <source>Could not deactivate your license at this time.</source>
        <translation>Kunde inte avaktivera din licens just nu.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="854"/>
        <source>Your license has been deactivated.</source>
        <translation>Din licens har avaktiverats.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="855"/>
        <source>Access to Pro features has been removed.
Thank you again for supporting Serial Studio!</source>
        <translation>Åtkomst till Pro-funktioner har tagits bort.
Tack återigen för att du stödjer Serial Studio!</translation>
    </message>
</context>
<context>
    <name>MDF4::Export</name>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="629"/>
        <source>MDF4 Export is a Pro feature.</source>
        <translation>MDF4-export är en Pro-funktion.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="630"/>
        <source>This feature requires a license. Please purchase one to enable MDF4 export.</source>
        <translation>Denna funktion kräver en licens. Köp en för att aktivera MDF4-export.</translation>
    </message>
</context>
<context>
    <name>MDF4::Player</name>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="357"/>
        <source>Select MDF4 file</source>
        <translation>Välj MDF4-fil</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="359"/>
        <source>MDF4 files (*.mf4 *.dat)</source>
        <translation>MDF4-filer (*.mf4 *.dat)</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="381"/>
        <source>Disconnect from device?</source>
        <translation>Koppla från enhet?</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="382"/>
        <source>You must disconnect from the current device before opening a MDF4 file.</source>
        <translation>Du måste koppla från den aktuella enheten innan du öppnar en MDF4-fil.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="398"/>
        <source>Cannot open MDF4 file</source>
        <translation>Kan inte öppna MDF4-fil</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="399"/>
        <source>The file may be corrupted or in an unsupported format.</source>
        <translation>Filen kan vara skadad eller i ett format som inte stöds.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="406"/>
        <source>Invalid MDF4 file</source>
        <translation>Ogiltig MDF4-fil</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="407"/>
        <source>Failed to read file structure. The file may be corrupted.</source>
        <translation>Misslyckades att läsa filstruktur. Filen kan vara skadad.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="422"/>
        <source>No data in file</source>
        <translation>Ingen data i fil</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="423"/>
        <source>The MDF4 file contains no measurement data.</source>
        <translation>MDF4-filen innehåller ingen mätdata.</translation>
    </message>
</context>
<context>
    <name>MQTT</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="51"/>
        <source>Hostname</source>
        <translation>Värdnamn</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="59"/>
        <source>e.g. broker.hivemq.com</source>
        <translation>t.ex. broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="67"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="87"/>
        <source>Topic Filter</source>
        <translation>Ämnesfilter</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="95"/>
        <source>e.g. sensors/#</source>
        <translation>t.ex. sensors/#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="103"/>
        <source>Client ID</source>
        <translation>Klient-ID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="120"/>
        <source>Regenerate</source>
        <translation>Generera Om</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="130"/>
        <source>Username</source>
        <translation>Användarnamn</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="145"/>
        <source>Password</source>
        <translation>Lösenord</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="161"/>
        <source>Version</source>
        <translation>Version</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="187"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="206"/>
        <source>Clean Session</source>
        <translation>Rensa Session</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="232"/>
        <source>Use SSL/TLS</source>
        <translation>Använd SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="258"/>
        <source>SSL Protocol</source>
        <translation>SSL-protokoll</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="289"/>
        <source>Peer Verify</source>
        <translation>Verifiera Motpart</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="320"/>
        <source>Verify Depth</source>
        <translation>Verifieringsdjup</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="341"/>
        <source>CA Certificates</source>
        <translation>Ca-certifikat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="349"/>
        <source>Load From Folder…</source>
        <translation>Ladda från Mapp…</translation>
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
        <translation type="vanished">TLS 1.3 eller Senare</translation>
    </message>
    <message>
        <source>DTLS 1.2 or Later</source>
        <translation type="vanished">DTLS 1.2 eller Senare</translation>
    </message>
    <message>
        <source>Any Protocol</source>
        <translation type="vanished">Valfritt Protokoll</translation>
    </message>
    <message>
        <source>Secure Protocols Only</source>
        <translation type="vanished">Endast Säkra Protokoll</translation>
    </message>
    <message>
        <source>None</source>
        <translation type="vanished">Ingen</translation>
    </message>
    <message>
        <source>Query Peer</source>
        <translation type="vanished">Fråga Motpart</translation>
    </message>
    <message>
        <source>Verify Peer</source>
        <translation type="vanished">Verifiera Motpart</translation>
    </message>
    <message>
        <source>Auto Verify Peer</source>
        <translation type="vanished">Autoverifiera Motpart</translation>
    </message>
    <message>
        <source>Use System Database</source>
        <translation type="vanished">Använd Systemdatabas</translation>
    </message>
    <message>
        <source>MQTT Subscriber</source>
        <translation type="vanished">MQTT-prenumerant</translation>
    </message>
    <message>
        <source>MQTT Publisher</source>
        <translation type="vanished">MQTT-utgivare</translation>
    </message>
    <message>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation type="vanished">MQTT-funktion kräver kommersiell licens</translation>
    </message>
    <message>
        <source>Load From Folder…</source>
        <translation type="vanished">Läs In från Mapp…</translation>
    </message>
    <message>
        <source>Connecting to MQTT brokers is only available with a valid Serial Studio commercial license (Hobbyist tier or above).

To unlock this feature, please activate your license or visit the store.</source>
        <translation type="vanished">Anslutning till MQTT-mäklare är endast tillgänglig med en giltig kommersiell licens för Serial Studio (nivå Hobbyist eller högre).

Aktivera din licens eller besök butiken för att låsa upp denna funktion.</translation>
    </message>
    <message>
        <source>Missing MQTT Topic</source>
        <translation type="vanished">MQTT-ämne Saknas</translation>
    </message>
    <message>
        <source>You must specify a topic before connecting as a publisher.</source>
        <translation type="vanished">Ett ämne måste anges innan anslutning som utgivare.</translation>
    </message>
    <message>
        <source>Configuration Error</source>
        <translation type="vanished">Konfigurationsfel</translation>
    </message>
    <message>
        <source>MQTT Topic Not Set</source>
        <translation type="vanished">MQTT-ämne Ej Angivet</translation>
    </message>
    <message>
        <source>You won't receive any messages until a topic is configured.</source>
        <translation type="vanished">Inga meddelanden tas emot förrän ett ämne har konfigurerats.</translation>
    </message>
    <message>
        <source>Configuration Warning</source>
        <translation type="vanished">Konfigurationsvarning</translation>
    </message>
    <message>
        <source>Invalid MQTT Topic</source>
        <translation type="vanished">Ogiltigt MQTT-ämne</translation>
    </message>
    <message>
        <source>The topic "%1" is not valid.</source>
        <translation type="vanished">Ämnet "%1" är inte giltigt.</translation>
    </message>
    <message>
        <source>Select PEM Certificates Directory</source>
        <translation type="vanished">Välj Pem-certifikatkatalog</translation>
    </message>
    <message>
        <source>Subscription Error</source>
        <translation type="vanished">Prenumerationsfel</translation>
    </message>
    <message>
        <source>Failed to subscribe to topic "%1".</source>
        <translation type="vanished">Misslyckades prenumerera på ämne "%1".</translation>
    </message>
    <message>
        <source>Invalid MQTT Protocol Version</source>
        <translation type="vanished">Ogiltig MQTT-protokollversion</translation>
    </message>
    <message>
        <source>The MQTT broker rejected the connection due to an unsupported protocol version. Ensure that your client and broker support the same protocol version.</source>
        <translation type="vanished">MQTT-brokern nekade anslutningen på grund av en protokollversion som inte stöds. Säkerställ att klienten och brokern stöder samma protokollversion.</translation>
    </message>
    <message>
        <source>Client ID Rejected</source>
        <translation type="vanished">Klient-ID Nekat</translation>
    </message>
    <message>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Try using a different client ID.</source>
        <translation type="vanished">Brokern nekade klient-ID:t. Det kan vara felaktigt formaterat, för långt eller redan används. Försök använda ett annat klient-ID.</translation>
    </message>
    <message>
        <source>MQTT Server Unavailable</source>
        <translation type="vanished">MQTT-server Ej Tillgänglig</translation>
    </message>
    <message>
        <source>The network connection was established, but the broker is currently unavailable. Verify the broker status and try again later.</source>
        <translation type="vanished">Nätverksanslutningen upprättades, men brokern är för närvarande otillgänglig. Verifiera brokerns status och försök igen senare.</translation>
    </message>
    <message>
        <source>Authentication Error</source>
        <translation type="vanished">Autentiseringsfel</translation>
    </message>
    <message>
        <source>The username or password provided is incorrect or malformed. Double-check your credentials and try again.</source>
        <translation type="vanished">Användarnamnet eller lösenordet är felaktigt eller felformaterat. Kontrollera dina inloggningsuppgifter och försök igen.</translation>
    </message>
    <message>
        <source>Authorization Error</source>
        <translation type="vanished">Auktoriseringsfel</translation>
    </message>
    <message>
        <source>The MQTT broker denied the connection due to insufficient permissions. Ensure that your account has the necessary access rights.</source>
        <translation type="vanished">MQTT-brokern nekade anslutningen på grund av otillräckliga behörigheter. Säkerställ att ditt konto har nödvändiga åtkomsträttigheter.</translation>
    </message>
    <message>
        <source>Network or Transport Error</source>
        <translation type="vanished">Nätverks- eller transportfel</translation>
    </message>
    <message>
        <source>A network or transport layer issue occurred, causing an unexpected connection failure. Check your network connection and broker settings.</source>
        <translation type="vanished">Ett nätverks- eller transportlagerproblem inträffade, vilket orsakade ett oväntat anslutningsfel. Kontrollera din nätverksanslutning och brokerinställningar.</translation>
    </message>
    <message>
        <source>MQTT Protocol Violation</source>
        <translation type="vanished">MQTT-protokollöverträdelse</translation>
    </message>
    <message>
        <source>The client detected a violation of the MQTT protocol and closed the connection. Check your MQTT implementation for compliance.</source>
        <translation type="vanished">Klienten upptäckte en överträdelse av MQTT-protokollet och stängde anslutningen. Kontrollera din MQTT-implementation för efterlevnad.</translation>
    </message>
    <message>
        <source>Unknown Error</source>
        <translation type="vanished">Okänt Fel</translation>
    </message>
    <message>
        <source>An unexpected error occurred. Check the logs for more details or restart the application.</source>
        <translation type="vanished">Ett oväntat fel inträffade. Kontrollera loggarna för mer information eller starta om applikationen.</translation>
    </message>
    <message>
        <source>MQTT 5 Error</source>
        <translation type="vanished">MQTT 5-Fel</translation>
    </message>
    <message>
        <source>An MQTT protocol level 5 error occurred. Check the broker logs or reason codes for more details.</source>
        <translation type="vanished">Ett MQTT-protokollnivå 5-fel inträffade. Kontrollera broker-loggarna eller orsakskoderna för mer information.</translation>
    </message>
    <message>
        <source>MQTT Authentication Failed</source>
        <translation type="vanished">MQTT-autentisering Misslyckades</translation>
    </message>
    <message>
        <source>Authentication failed: %1.</source>
        <translation type="vanished">Autentisering misslyckades: %1.</translation>
    </message>
    <message>
        <source>Extended authentication is required, but MQTT 5.0 is not enabled.</source>
        <translation type="vanished">Utökad autentisering krävs, men MQTT 5.0 är inte aktiverat.</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="vanished">Okänd</translation>
    </message>
    <message>
        <source>MQTT Authentication Required</source>
        <translation type="vanished">MQTT-autentisering Krävs</translation>
    </message>
    <message>
        <source>The MQTT broker requires authentication using method: "%1".

Please provide the necessary credentials.</source>
        <translation type="vanished">MQTT-brokern kräver autentisering med metod: "%1".

Ange nödvändiga inloggningsuppgifter.</translation>
    </message>
    <message>
        <source>Enter MQTT Username</source>
        <translation type="vanished">Ange MQTT-användarnamn</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation type="vanished">Användarnamn:</translation>
    </message>
    <message>
        <source>Enter MQTT Password</source>
        <translation type="vanished">Ange MQTT-lösenord</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation type="vanished">Lösenord:</translation>
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
        <translation>TLS 1.3 eller Senare</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="799"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 eller Senare</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="800"/>
        <source>Any Protocol</source>
        <translation>Valfritt Protokoll</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="801"/>
        <source>Secure Protocols Only</source>
        <translation>Endast Säkra Protokoll</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="803"/>
        <source>None</source>
        <translation>Ingen</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="804"/>
        <source>Query Peer</source>
        <translation>Fråga Motpart</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="805"/>
        <source>Verify Peer</source>
        <translation>Verifiera Motpart</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="806"/>
        <source>Auto Verify Peer</source>
        <translation>Autoverifiera Motpart</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1123"/>
        <source>Raw RX Data</source>
        <translation>Rå Rx-data</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1124"/>
        <source>Custom Script</source>
        <translation>Anpassat Skript</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1125"/>
        <source>Dashboard Data (CSV)</source>
        <translation>Instrumentpaneldata (CSV)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1126"/>
        <source>Dashboard Data (JSON)</source>
        <translation>Instrumentpaneldata (JSON)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1281"/>
        <source>MQTT publisher unavailable</source>
        <translation>MQTT-publicering otillgänglig</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1282"/>
        <source>A valid commercial license is required to use MQTT publishing.</source>
        <translation>En giltig kommersiell licens krävs för att använda MQTT-publicering.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1284"/>
        <location filename="../../src/MQTT/Publisher.cpp" line="1853"/>
        <source>MQTT Test Connection</source>
        <translation>MQTT Testa Anslutning</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1303"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Välj Pem-certifikatkatalog</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker reachable</source>
        <translation>MQTT-broker nåbar</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker unreachable</source>
        <translation>MQTT-broker onåbar</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT broker connection failed</source>
        <translation>MQTT-brokeranslutning misslyckades</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT Publisher</source>
        <translation>MQTT-utgivare</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherScriptEditor</name>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="49"/>
        <source>MQTT Publisher Script</source>
        <translation>MQTT-utgivarskript</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="86"/>
        <source>JavaScript</source>
        <translation>Javascript</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="86"/>
        <source>Lua</source>
        <translation>Lua</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="92"/>
        <source>Sample frame bytes (text or hex)</source>
        <translation>Exempelrambytes (text eller hex)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="97"/>
        <source>Hex</source>
        <translation>Hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="98"/>
        <source>Test</source>
        <translation>Testa</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="99"/>
        <source>Clear</source>
        <translation>Rensa</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="101"/>
        <source>Apply</source>
        <translation>Tillämpa</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="102"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="111"/>
        <source>Language:</source>
        <translation>Språk:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="114"/>
        <source>Template:</source>
        <translation>Mall:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="125"/>
        <source>Frame:</source>
        <translation>Ram:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="129"/>
        <source>Output:</source>
        <translation>Utgång:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="269"/>
        <source>Enter a frame</source>
        <translation>Ange en ram</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="276"/>
        <source>Invalid hex</source>
        <translation>Ogiltig hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="359"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Formatera Dokument	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="360"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Formatera Markering	ctrl+i</translation>
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
-- Definiera en mqtt(frame)-funktion som tar emot råbytes
-- från en tolkad frame och returnerar nyttolasten att publicera till
-- brokern, eller nil för att hoppa över denna frame.
--
-- Frame-argumentet matchar vad Frame Parser-skriptet
-- ser: en Lua-sträng som innehåller bytes mellan FrameReader-
-- avgränsare.
--
-- Exempel:
--   function mqtt(frame)
--     return frame    -- vidarebefordra som den är
--   end
--
-- Använd Template-rullgardinsmenyn för färdiga exempel.
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
 * Definiera en mqtt(frame)-funktion som tar emot råbytes
 * från en tolkad frame och returnerar nyttolasten att publicera till
 * brokern, eller null för att hoppa över denna frame.
 *
 * Frame-argumentet matchar vad Frame Parser-skriptet
 * ser: en sträng som innehåller bytes mellan FrameReader-
 * avgränsare.
 *
 * Exempel:
 *   function mqtt(frame) {
 *     return frame;   // vidarebefordra som den är
 *   }
 *
 * Använd Template-rullgardinsmenyn för färdiga exempel.
 */</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="601"/>
        <source>Script is empty</source>
        <translation>Skriptet är tomt</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="608"/>
        <source>Lua engine error</source>
        <translation>Lua-motorfel</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="630"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="644"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="668"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="682"/>
        <source>Error: %1</source>
        <translation>Fel: %1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="638"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="674"/>
        <source>mqtt() is not defined</source>
        <translation>mqtt() är inte definierad</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="655"/>
        <source>(nil -- frame skipped)</source>
        <translation>(nil -- frame överhoppad)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="657"/>
        <source>(non-string return)</source>
        <translation>(icke-sträng retur)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="687"/>
        <source>(null -- frame skipped)</source>
        <translation>(null -- ram överhoppad)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="765"/>
        <source>Select Template…</source>
        <translation>Välj Mall…</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherWorker</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="674"/>
        <source>Configure broker hostname and port before testing the connection.</source>
        <translation>Konfigurera mäklarens värdnamn och port innan anslutningen testas.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="710"/>
        <source>Successfully connected to %1:%2.</source>
        <translation>Ansluten till %1:%2.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="721"/>
        <source>Timed out after 5 seconds without reaching the broker.</source>
        <translation>Tidsgräns överskreds efter 5 sekunder utan att nå mäklaren.</translation>
    </message>
</context>
<context>
    <name>MQTTConfiguration</name>
    <message>
        <source>MQTT Setup</source>
        <translation type="vanished">MQTT-konfiguration</translation>
    </message>
    <message>
        <source>MQTT is a Pro Feature</source>
        <translation type="vanished">MQTT är en Pro-funktion</translation>
    </message>
    <message>
        <source>Activate your license or visit the store to unlock MQTT support.</source>
        <translation type="vanished">Aktivera din licens eller besök butiken för att låsa upp MQTT-stöd.</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="vanished">Allmänt</translation>
    </message>
    <message>
        <source>Authentication</source>
        <translation type="vanished">Autentisering</translation>
    </message>
    <message>
        <source>MQTT Options</source>
        <translation type="vanished">MQTT-alternativ</translation>
    </message>
    <message>
        <source>SSL Properties</source>
        <translation type="vanished">SSL-egenskaper</translation>
    </message>
    <message>
        <source>Host</source>
        <translation type="vanished">Värd</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="vanished">Port</translation>
    </message>
    <message>
        <source>Client ID</source>
        <translation type="vanished">Klient-ID</translation>
    </message>
    <message>
        <source>Keep Alive (s)</source>
        <translation type="vanished">Keep Alive (s)</translation>
    </message>
    <message>
        <source>Clean Session</source>
        <translation type="vanished">Rensa Session</translation>
    </message>
    <message>
        <source>Username</source>
        <translation type="vanished">Användarnamn</translation>
    </message>
    <message>
        <source>MQTT Username</source>
        <translation type="vanished">MQTT-användarnamn</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="vanished">Lösenord</translation>
    </message>
    <message>
        <source>MQTT Password</source>
        <translation type="vanished">MQTT-lösenord</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="vanished">Version</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="vanished">Läge</translation>
    </message>
    <message>
        <source>Topic</source>
        <translation type="vanished">Ämne</translation>
    </message>
    <message>
        <source>e.g. sensors/temperature or home/+/status</source>
        <translation type="vanished">t.ex. sensors/temperature eller home/+/status</translation>
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
        <translation type="vanished">Will-ämne</translation>
    </message>
    <message>
        <source>e.g. device/alerts/offline</source>
        <translation type="vanished">t.ex. device/alerts/offline</translation>
    </message>
    <message>
        <source>Will Message</source>
        <translation type="vanished">Will-meddelande</translation>
    </message>
    <message>
        <source>e.g. Device unexpectedly disconnected</source>
        <translation type="vanished">t.ex. Enhet oväntat frånkopplad</translation>
    </message>
    <message>
        <source>Enable SSL</source>
        <translation type="vanished">Aktivera SSL</translation>
    </message>
    <message>
        <source>SSL Protocol</source>
        <translation type="vanished">SSL-protokoll</translation>
    </message>
    <message>
        <source>Verify Depth</source>
        <translation type="vanished">Verifieringsdjup</translation>
    </message>
    <message>
        <source>Verify Mode</source>
        <translation type="vanished">Verifieringsläge</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Stäng</translation>
    </message>
    <message>
        <source>Disconnect</source>
        <translation type="vanished">Koppla Från</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="vanished">Anslut</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="191"/>
        <source>Console Only Mode</source>
        <translation>Endast Konsolläge</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="194"/>
        <source>Quick Plot Mode</source>
        <translation>Snabbdiagramläge</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="201"/>
        <source>Empty Project</source>
        <translation>Tomt Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="686"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="694"/>
        <source>Waiting for data…</source>
        <translation>Väntar på data…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="695"/>
        <source>Connecting to device…</source>
        <translation>Ansluter till enhet…</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="146"/>
        <source>Code sample</source>
        <translation>Kodexempel</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="148"/>
        <source>Completer</source>
        <translation>Kompletterare</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="150"/>
        <source>Highlighter</source>
        <translation>Syntaxmarkering</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="152"/>
        <source>Style</source>
        <translation>Stil</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="214"/>
        <source> spaces</source>
        <translation>mellanslag</translation>
    </message>
</context>
<context>
    <name>MarkdownWebView</name>
    <message>
        <location filename="../../qml/Widgets/MarkdownWebView.qml" line="36"/>
        <source>Copied to Clipboard</source>
        <translation>Kopierat till Urklipp</translation>
    </message>
</context>
<context>
    <name>Mdf4Player</name>
    <message>
        <location filename="../../qml/Dialogs/Mdf4Player.qml" line="24"/>
        <source>MDF4 Player</source>
        <translation>MDF4-spelare</translation>
    </message>
</context>
<context>
    <name>MessageBubble</name>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="97"/>
        <source>Error</source>
        <translation>Fel</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>You</source>
        <translation>Du</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>Assistant</source>
        <translation>Assistent</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="208"/>
        <source>Discovery</source>
        <translation>Upptäckt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="209"/>
        <source>Execution</source>
        <translation>Körning</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="239"/>
        <source>Approve %1 actions in %2?</source>
        <translation>Godkänn %1 åtgärder i %2?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="249"/>
        <source>These calls will run together. Expand each card below to inspect arguments.</source>
        <translation>Dessa anrop körs tillsammans. Expandera varje kort nedan för att inspektera argument.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="260"/>
        <source>Approve all</source>
        <translation>Godkänn alla</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="266"/>
        <source>Deny all</source>
        <translation>Neka alla</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="332"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="384"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="436"/>
        <source>Copy</source>
        <translation>Kopiera</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="337"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="389"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="441"/>
        <source>Copy All</source>
        <translation>Kopiera Alla</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="345"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="397"/>
        <source>Select All</source>
        <translation>Markera Allt</translation>
    </message>
</context>
<context>
    <name>MessageWebView</name>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="63"/>
        <source>You</source>
        <translation>Du</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="64"/>
        <source>Assistant</source>
        <translation>Assistent</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="65"/>
        <location filename="../../qml/AI/MessageWebView.qml" line="71"/>
        <source>Error</source>
        <translation>Fel</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="66"/>
        <source>Discovery</source>
        <translation>Upptäckt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="67"/>
        <source>Execution</source>
        <translation>Körning</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="68"/>
        <source>Running</source>
        <translation>Körs</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="69"/>
        <source>Awaiting approval</source>
        <translation>Väntar på godkännande</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="70"/>
        <source>Done</source>
        <translation>Klar</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="72"/>
        <source>Denied</source>
        <translation>Nekad</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="73"/>
        <source>Blocked</source>
        <translation>Blockerad</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="74"/>
        <source>Approve</source>
        <translation>Godkänn</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="75"/>
        <source>Deny</source>
        <translation>Neka</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="76"/>
        <source>Approve all</source>
        <translation>Godkänn alla</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="77"/>
        <source>Deny all</source>
        <translation>Neka alla</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="78"/>
        <source>Arguments</source>
        <translation>Argument</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="79"/>
        <source>Result</source>
        <translation>Resultat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="80"/>
        <source>Approve {n} actions in {family}?</source>
        <translation>Godkänn {n} åtgärder i {family}?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="81"/>
        <source>These calls will run together. Expand each card to inspect arguments.</source>
        <translation>Dessa anrop körs tillsammans. Expandera varje kort för att inspektera argument.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="82"/>
        <source>Copy</source>
        <translation>Kopiera</translation>
    </message>
</context>
<context>
    <name>Misc::Examples</name>
    <message>
        <location filename="../../src/Misc/Examples.cpp" line="282"/>
        <source>Failed to load README: %1</source>
        <translation>Kunde inte läsa in README: %1</translation>
    </message>
</context>
<context>
    <name>Misc::ExtensionManager</name>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="244"/>
        <source>Theme</source>
        <translation>Tema</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="247"/>
        <source>Frame Parser</source>
        <translation>Frame-parser</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="250"/>
        <source>Project Template</source>
        <translation>Projektmall</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="253"/>
        <source>Plugin</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="256"/>
        <source>All Types</source>
        <translation>Alla Typer</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="473"/>
        <source>Reset Extensions</source>
        <translation>Återställ Tillägg</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="474"/>
        <source>This uninstalls all extensions, removes all custom repositories, and restores the default settings. Continue?</source>
        <translation>Detta avinstallerar alla tillägg, tar bort alla anpassade arkiv och återställer standardinställningarna. Fortsätta?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="513"/>
        <source>Select Extension Repository Folder</source>
        <translation>Välj Mapp för Tilläggsarkiv</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1014"/>
        <source>Installed (repository no longer available)</source>
        <translation>Installerat (arkiv inte längre tillgängligt)</translation>
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
        <translation>Plugin-fel</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1321"/>
        <source>Plugin "%1" is not installed.</source>
        <translation>Plugin "%1" är inte installerat.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1332"/>
        <source>Extension "%1" is not a plugin (type: %2).</source>
        <translation>Tillägg "%1" är inte ett plugin (typ: %2).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1353"/>
        <source>Cannot read plugin metadata file:
%1/info.json</source>
        <translation>Kan inte läsa plugin-metadatafil:
%1/info.json</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1375"/>
        <source>Plugin "%1" requires gRPC but this build does not include gRPC support.</source>
        <translation>Plugin "%1" kräver GRPC men denna version inkluderar inte GRPC-stöd.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1384"/>
        <source>This plugin uses gRPC for high-performance data streaming. The API server needs to be enabled.

Would you like to enable it now?</source>
        <translation>Detta tillägg använder GRPC för högpresterande dataströmning. API-servern måste aktiveras.

Vill du aktivera den nu?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1390"/>
        <source>API Server Required</source>
        <translation>API-server Krävs</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1419"/>
        <source>Plugin "%1" has no 'entry' field in info.json.</source>
        <translation>Tillägget "%1" saknar fältet 'entry' i info.json.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1429"/>
        <source>Entry point not found:
%1</source>
        <translation>Startpunkt hittades inte:
%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1438"/>
        <source>Plugin "%1" has an invalid entry point path.</source>
        <translation>Tillägget "%1" har en ogiltig sökväg till startpunkt.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1481"/>
        <source>Missing Dependency</source>
        <translation>Saknat Beroende</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1482"/>
        <source>This plugin requires "%1" but it was not found on your system.

Would you like to open the download page?</source>
        <translation>Detta tillägg kräver "%1" men det hittades inte på ditt system.

Vill du öppna nedladdningssidan?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1387"/>
        <source>Plugins need the API server to communicate with Serial Studio. Would you like to enable it now?</source>
        <translation>Tillägg behöver API-servern för att kommunicera med Serial Studio. Vill du aktivera den nu?</translation>
    </message>
</context>
<context>
    <name>Misc::GraphicsBackend</name>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="246"/>
        <source>Restart Required</source>
        <translation>Omstart Krävs</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="247"/>
        <source>The new rendering backend will take effect after restarting Serial Studio. Restart now to apply the change?</source>
        <translation>Den nya renderingsmotorn träder i kraft efter omstart av Serial Studio. Starta om nu för att tillämpa ändringen?</translation>
    </message>
</context>
<context>
    <name>Misc::HelpCenter</name>
    <message>
        <location filename="../../src/Misc/HelpCenter.cpp" line="303"/>
        <source>Failed to load page: %1</source>
        <translation>Misslyckades att ladda sida: %1</translation>
    </message>
</context>
<context>
    <name>Misc::IconEngine</name>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="148"/>
        <source>Invalid icon identifier</source>
        <translation>Ogiltig ikonidentifierare</translation>
    </message>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="218"/>
        <source>Empty SVG data received</source>
        <translation>Tom SVG-data mottagen</translation>
    </message>
</context>
<context>
    <name>Misc::ShortcutGenerator</name>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="73"/>
        <source>Windows Shortcut (*.lnk)</source>
        <translation>Windows-genväg (*.lnk)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="75"/>
        <source>macOS Application (*.app)</source>
        <translation>macOS-program (*.app)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="77"/>
        <source>Desktop Entry (*.desktop)</source>
        <translation>Skrivbordspost (*.desktop)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="101"/>
        <source>Use a .icns icon for the sharpest result in Finder and the Dock.</source>
        <translation>Använd en .icns-ikon för skarpast resultat i Finder och Dock.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="103"/>
        <source>Leave the icon empty to inherit the Serial Studio executable icon.</source>
        <translation>Lämna ikonen tom för att ärva Serial Studios körbar fil-ikon.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="105"/>
        <source>Place the file under ~/.local/share/applications/ to expose it in your application launcher.</source>
        <translation>Placera filen under ~/.local/share/applications/ för att exponera den i din programstartare.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="116"/>
        <source>Apple Icon Image (*.icns)</source>
        <translation>Apple-ikonbild (*.icns)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="118"/>
        <source>Windows Icon (*.ico)</source>
        <translation>Windows-ikon (*.ico)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="120"/>
        <source>Vector or Raster Image (*.svg *.png)</source>
        <translation>Vektor- eller rasterbild (*.svg *.png)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="212"/>
        <source>A Pro license is required to generate shortcuts.</source>
        <translation>En Pro-licens krävs för att generera genvägar.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="217"/>
        <source>No output path was provided.</source>
        <translation>Ingen utmatningssökväg angavs.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="258"/>
        <source>Failed to write shortcut file.</source>
        <translation>Misslyckades att skriva genvägsfil.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="222"/>
        <source>Could not replace the existing shortcut at %1.</source>
        <translation>Kunde inte ersätta den befintliga genvägen vid %1.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="232"/>
        <source>Could not create the .app bundle directory layout.</source>
        <translation>Kunde inte skapa katalogstrukturen för .app-paketet.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="125"/>
        <source>Could not write the bundle launcher: %1</source>
        <translation>Kunde inte skriva paketets startfil: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="144"/>
        <source>Could not mark the bundle launcher as executable.</source>
        <translation>Kunde inte markera paketets startfil som körbar.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="164"/>
        <source>Could not write Info.plist: %1</source>
        <translation>Kunde inte skriva Info.plist: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="271"/>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="140"/>
        <source>Windows shortcut writer is not available on this platform.</source>
        <translation>Windows-genvägsverktyget är inte tillgängligt på denna plattform.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="285"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="199"/>
        <source>Linux shortcut writer is not available on this platform.</source>
        <translation>Linux-genvägsverktyget är inte tillgängligt på denna plattform.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="107"/>
        <source>Could not initialise COM (required to write .lnk shortcuts).</source>
        <translation>Kunde inte initiera COM (krävs för att skriva .lnk-genvägar).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="118"/>
        <source>CoCreateInstance(IShellLink) failed (HRESULT 0x%1).</source>
        <translation>CoCreateInstance(IShellLink) misslyckades (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="153"/>
        <source>QueryInterface(IPersistFile) failed (HRESULT 0x%1).</source>
        <translation>QueryInterface(IPersistFile) misslyckades (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="163"/>
        <source>Saving the .lnk file failed (HRESULT 0x%1).</source>
        <translation>Sparande av .lnk-filen misslyckades (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="185"/>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="154"/>
        <source>macOS shortcut writer is not available on this platform.</source>
        <translation>MacOS-genvägsverktyget är inte tillgängligt på denna plattform.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="86"/>
        <source>Could not open the shortcut path for writing: %1</source>
        <translation>Kunde inte öppna genvägssökvägen för skrivning: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="91"/>
        <source>Serial Studio shortcut</source>
        <translation>Serial Studio-genväg</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="112"/>
        <source>Could not mark the shortcut as executable.</source>
        <translation>Kunde inte markera genvägen som körbar.</translation>
    </message>
</context>
<context>
    <name>Misc::ThemeManager</name>
    <message>
        <location filename="../../src/Misc/ThemeManager.cpp" line="398"/>
        <source>System</source>
        <translation>System</translation>
    </message>
</context>
<context>
    <name>Misc::Utilities</name>
    <message>
        <source>Check for updates automatically?</source>
        <translation type="vanished">Sök efter uppdateringar automatiskt?</translation>
    </message>
    <message>
        <source>Should %1 automatically check for updates? You can always check for updates manually from the "About" dialog</source>
        <translation type="vanished">Ska %1 automatiskt söka efter uppdateringar? Du kan alltid söka efter uppdateringar manuellt från "Om"-dialogen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="161"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="163"/>
        <source>Save</source>
        <translation>Spara</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="165"/>
        <source>Save all</source>
        <translation>Spara alla</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="167"/>
        <source>Open</source>
        <translation>Öppna</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="169"/>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="171"/>
        <source>Yes to all</source>
        <translation>Ja till alla</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="173"/>
        <source>No</source>
        <translation>Nej</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="175"/>
        <source>No to all</source>
        <translation>Nej till alla</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="177"/>
        <source>Abort</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="179"/>
        <source>Retry</source>
        <translation>Försök Igen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="181"/>
        <source>Ignore</source>
        <translation>Ignorera</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="183"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="185"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="187"/>
        <source>Discard</source>
        <translation>Förkasta</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="189"/>
        <source>Help</source>
        <translation>Hjälp</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="191"/>
        <source>Apply</source>
        <translation>Tillämpa</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="193"/>
        <source>Reset</source>
        <translation>Återställ</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="195"/>
        <source>Restore defaults</source>
        <translation>Återställ standardvärden</translation>
    </message>
</context>
<context>
    <name>Misc::WorkspaceManager</name>
    <message>
        <location filename="../../src/Misc/WorkspaceManager.cpp" line="261"/>
        <source>Select Workspace Location</source>
        <translation>Välj Plats för Arbetsyta</translation>
    </message>
</context>
<context>
    <name>Modbus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="49"/>
        <source>Protocol</source>
        <translation>Protokoll</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="72"/>
        <source>Serial Port</source>
        <translation>Serieport</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="97"/>
        <source>Baud Rate</source>
        <translation>Baudhastighet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="201"/>
        <source>Parity</source>
        <translation>Paritet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="222"/>
        <source>Data Bits</source>
        <translation>Databitar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="245"/>
        <source>Stop Bits</source>
        <translation>Stoppbitar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="268"/>
        <source>Host</source>
        <translation>Värd</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="278"/>
        <source>IP Address</source>
        <translation>IP-adress</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="292"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="301"/>
        <source>TCP Port</source>
        <translation>TCP-port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="329"/>
        <source>Slave Address</source>
        <translation>Slavadress</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="334"/>
        <source>1-247</source>
        <translation>1-247</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="349"/>
        <source>Poll Interval (ms)</source>
        <translation>Pollintervall (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="354"/>
        <source>Polling interval</source>
        <translation>Pollintervall</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="382"/>
        <source>Configure Register Groups…</source>
        <translation>Konfigurera Registergrupper…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="392"/>
        <source>Import Register Map…</source>
        <translation>Importera Registerkarta…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="407"/>
        <source>%1 group(s) configured</source>
        <translation>%1 grupp(er) konfigurerad(e)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="408"/>
        <source>No groups configured</source>
        <translation>Inga grupper konfigurerade</translation>
    </message>
</context>
<context>
    <name>ModbusGroupsDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="45"/>
        <source>Modbus Register Groups</source>
        <translation>Modbus-registergrupper</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="166"/>
        <source>Configure multiple register groups to poll different register types in sequence.</source>
        <translation>Konfigurera flera registergrupper för att avläsa olika registertyper i sekvens.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="174"/>
        <source>Add New Group</source>
        <translation>Lägg till Ny Grupp</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="198"/>
        <source>Register Type:</source>
        <translation>Registertyp:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="210"/>
        <source>Start Address:</source>
        <translation>Startadress:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="217"/>
        <source>0-65535</source>
        <translation>0-65535</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="223"/>
        <source>Register Count:</source>
        <translation>Antal Register:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="234"/>
        <source>1-125</source>
        <translation>1-125</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="239"/>
        <source>Add Group</source>
        <translation>Lägg till Grupp</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="262"/>
        <source>Configured Groups</source>
        <translation>Konfigurerade Grupper</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="296"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="303"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="311"/>
        <source>Start</source>
        <translation>Start</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="318"/>
        <source>Count</source>
        <translation>Antal</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="325"/>
        <source>Action</source>
        <translation>Åtgärd</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="400"/>
        <source>Remove</source>
        <translation>Ta Bort</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="412"/>
        <source>No groups configured.
Add groups above to poll multiple register types.</source>
        <translation>Inga grupper konfigurerade.
Lägg till grupper ovan för att avläsa flera registertyper.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="430"/>
        <source>Total groups: %1</source>
        <translation>Totalt antal grupper: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="434"/>
        <source>Generate Project</source>
        <translation>Generera Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="440"/>
        <source>Clear All</source>
        <translation>Rensa Alla</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="446"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
</context>
<context>
    <name>ModbusPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="33"/>
        <source>Modbus Register Map Preview</source>
        <translation>Förhandsgranskning av Modbus-registerkarta</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="155"/>
        <source>File: %1</source>
        <translation>Fil: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="163"/>
        <source>Review the registers to import into a new Serial Studio project.</source>
        <translation>Granska registren som ska importeras till ett nytt Serial Studio-projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="171"/>
        <source>Registers</source>
        <translation>Register</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="205"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="212"/>
        <source>Name</source>
        <translation>Namn</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="221"/>
        <source>Address</source>
        <translation>Adress</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="227"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="235"/>
        <source>Data Type</source>
        <translation>Datatyp</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="242"/>
        <source>Units</source>
        <translation>Enheter</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="343"/>
        <source>No registers found in file.</source>
        <translation>Inga register hittades i filen.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="361"/>
        <source>Total: %1 registers in %2 groups</source>
        <translation>Totalt: %1 register i %2 grupper</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="367"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="379"/>
        <source>Create Project</source>
        <translation>Skapa Projekt</translation>
    </message>
</context>
<context>
    <name>MqttPublisherView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="33"/>
        <source>MQTT Publisher</source>
        <translation>MQTT-utgivare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="110"/>
        <source>Connected to broker</source>
        <translation>Ansluten till mäklare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="111"/>
        <source>Not connected</source>
        <translation>Ej ansluten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="124"/>
        <source>Test Connection</source>
        <translation>Testa Anslutning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="129"/>
        <source>Probe the broker with the current settings</source>
        <translation>Testa brokern med aktuella inställningar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="162"/>
        <source>Enable publishing first</source>
        <translation>Aktivera publicering först</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="140"/>
        <source>Edit Script</source>
        <translation>Redigera Skript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="146"/>
        <source>Edit the publisher script (Lua or JavaScript)</source>
        <translation>Redigera publiceringsskriptet (Lua eller JavaScript)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="158"/>
        <source>Load CA Certs</source>
        <translation>Ladda Ca-certifikat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="164"/>
        <source>Add PEM certificates from a folder</source>
        <translation>Lägg till PEM-certifikat från en mapp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="165"/>
        <source>Enable SSL/TLS first</source>
        <translation>Aktivera SSL/TLS först</translation>
    </message>
</context>
<context>
    <name>MultiPlot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpolera</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="261"/>
        <source>Interpolation: %1</source>
        <translation>Interpolering: %1</translation>
    </message>
    <message>
        <source>Show Legends</source>
        <translation type="vanished">Visa Förklaringar</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="283"/>
        <source>Show X Axis Label</source>
        <translation>Visa X-axeletikett</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="294"/>
        <source>Show Y Axis Label</source>
        <translation>Visa Y-axeletikett</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="306"/>
        <source>Show Crosshair</source>
        <translation>Visa Hårkors</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="313"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="313"/>
        <source>Resume</source>
        <translation>Återuppta</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="330"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Svep-/triggerläge</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="342"/>
        <source>Trigger Settings</source>
        <translation>Triggerinställningar</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="366"/>
        <source>Reset View</source>
        <translation>Återställ Vy</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="372"/>
        <source>Axis Range Settings</source>
        <translation>Inställningar för Axelområde</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">Sampel</translation>
    </message>
</context>
<context>
    <name>NativeTemplates</name>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="292"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="430"/>
        <source>Bytes per value</source>
        <translation>Byte per värde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="293"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="431"/>
        <source>Number of bytes combined into each channel value.</source>
        <translation>Antal byte som kombineras till varje kanalvärde.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="301"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="439"/>
        <source>Endianness</source>
        <translation>Endianness</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="302"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="440"/>
        <source>Byte order used when combining multi-byte values.</source>
        <translation>Byteordning som används vid kombination av flerbytevärden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="310"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="448"/>
        <source>Signed values</source>
        <translation>Signerade värden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="311"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="449"/>
        <source>Interprets each value as two's-complement signed.</source>
        <translation>Tolkar varje värde som tvåkomplementssignerat.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="651"/>
        <source>Tag routing table</source>
        <translation>Taggruttningstabell</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="652"/>
        <source>Comma-separated tag:index entries, e.g. 1:0,2:1,3:2. Tags may be decimal or 0x-prefixed hex.</source>
        <translation>Kommaseparerade tagg:index-poster, t.ex. 1:0,2:1,3:2. Taggar kan vara decimala eller 0x-prefixade hex.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1096"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1300"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1245"/>
        <source>Validate checksum</source>
        <translation>Validera kontrollsumma</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1097"/>
        <source>Rejects messages with an invalid Fletcher checksum.</source>
        <translation>Avvisar meddelanden med ogiltig Fletcher-kontrollsumma.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1301"/>
        <source>Rejects messages with an invalid additive checksum.</source>
        <translation>Avvisar meddelanden med ogiltig additiv kontrollsumma.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1454"/>
        <source>Protocol version</source>
        <translation>Protokollversion</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1455"/>
        <source>Selects the expected start marker (0xFE for v1, 0xFD for v2).</source>
        <translation>Väljer förväntad startmarkör (0xFE för v1, 0xFD för v2).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1883"/>
        <source>Validate CRC</source>
        <translation>Validera CRC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1884"/>
        <source>Rejects frames with an invalid CRC-24Q checksum.</source>
        <translation>Avvisar ramar med en ogiltig CRC-24Q kontrollsumma.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2059"/>
        <source>Channel count</source>
        <translation>Kanalantal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2060"/>
        <source>Number of output channels (registers or coils).</source>
        <translation>Antal utgångskanaler (register eller spolar).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2068"/>
        <source>Register offset</source>
        <translation>Registeroffset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2069"/>
        <source>Address offset subtracted from single-write echoes (40001 for legacy Modicon maps).</source>
        <translation>Adressoffset som subtraheras från enkel-skrivnings-ekon (40001 för äldre Modicon-kartor).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2079"/>
        <source>Signed registers</source>
        <translation>Signerade register</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2080"/>
        <source>Interprets 16-bit registers as two's-complement signed values.</source>
        <translation>Tolkar 16-bitars register som tvåkomplementssignerade värden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2386"/>
        <source>Payload layout</source>
        <translation>Nyttolastlayout</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2387"/>
        <source>Array emits every element in order; map routes keys through the key list.</source>
        <translation>Array skickar varje element i ordning; map dirigerar nycklar genom nyckellistan.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2397"/>
        <source>Keys (map mode)</source>
        <translation>Nycklar (kartläge)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2398"/>
        <source>Comma-separated map keys in channel order. Only used for the map layout.</source>
        <translation>Kommaseparerade kartnycklar i kanalordning. Används endast för kartlayouten.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="184"/>
        <source>Scalar fields</source>
        <translation>Skalära fält</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="185"/>
        <source>Comma-separated JSON fields repeated in every generated frame.</source>
        <translation>Kommaseparerade JSON-fält upprepade i varje genererad frame.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="192"/>
        <source>Sample array field</source>
        <translation>Sampel-arrayfält</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="193"/>
        <source>JSON field holding the batched sample array.</source>
        <translation>JSON-fält som innehåller den batchade sampel-arrayen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="334"/>
        <source>Records array field</source>
        <translation>Post-arrayfält</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="335"/>
        <source>JSON field holding the array of record objects.</source>
        <translation>JSON-fält som innehåller arrayen av postobjekt.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="341"/>
        <source>Record fields (in channel order)</source>
        <translation>Postfält (i kanalordning)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="342"/>
        <source>Comma-separated record fields. The position of each field sets its channel index.</source>
        <translation>Kommaseparerade postfält. Positionen för varje fält anger dess kanalindex.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="605"/>
        <source>Column widths</source>
        <translation>Kolumnbredder</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="606"/>
        <source>Comma-separated character counts per field. Leave empty to split on whitespace.</source>
        <translation>Kommaseparerade teckenlängder per fält. Lämna tomt för att dela på blanktecken.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="614"/>
        <source>Trim whitespace</source>
        <translation>Trimma blanktecken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="615"/>
        <source>Removes padding around every sliced field.</source>
        <translation>Tar bort utfyllnad runt varje uppdelat fält.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="744"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="893"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1360"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1787"/>
        <source>Keys (in channel order)</source>
        <translation>Nycklar (i kanalordning)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="745"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="894"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1788"/>
        <source>Comma-separated key names. The position of each key sets its channel index.</source>
        <translation>Kommaseparerade nyckelnamn. Positionen för varje nyckel anger dess kanalindex.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="753"/>
        <source>Pair separator</source>
        <translation>Parseparator</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="754"/>
        <source>Character between key=value pairs.</source>
        <translation>Tecken mellan nyckel=värde-par.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="760"/>
        <source>Key-value separator</source>
        <translation>Nyckel-värde-separator</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="761"/>
        <source>Character between a key and its value.</source>
        <translation>Tecken mellan en nyckel och dess värde.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="767"/>
        <source>Numeric values only</source>
        <translation>Endast numeriska värden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="768"/>
        <source>Ignores pairs whose value is not a number.</source>
        <translation>Ignorerar par vars värde inte är ett tal.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1010"/>
        <source>Command routing table</source>
        <translation>Kommandoruttningstabell</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1011"/>
        <source>Semicolon-separated entries of NAME:index list, e.g. CSQ:0,1;CREG:2,3;CGATT:4.</source>
        <translation>Semikolonseparerade poster i listan NAMN:index, t.ex. CSQ:0,1;CREG:2,3;CGATT:4.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1236"/>
        <source>Talker prefix</source>
        <translation>Talker-prefix</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1237"/>
        <source>Two-letter talker id, e.g. GP for GPS or GN for multi-constellation receivers.</source>
        <translation>Tvåbokstavs talker-id, t.ex. GP för GPS eller GN för multi-constellation-mottagare.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1246"/>
        <source>Rejects sentences whose *hh checksum does not match.</source>
        <translation>Avvisar meningar vars *hh-kontrollsumma inte stämmer.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1361"/>
        <source>Comma-separated parameter names. The position of each key sets its channel index.</source>
        <translation>Kommaseparerade parameternamn. Positionen för varje nyckel anger dess kanalindex.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1500"/>
        <source>Fields (in channel order)</source>
        <translation>Fält (i kanalordning)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1501"/>
        <source>Comma-separated field names. The position of each field sets its channel index.</source>
        <translation>Kommaseparerade fältnamn. Positionen för varje fält anger dess kanalindex.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1620"/>
        <source>Tags (in channel order)</source>
        <translation>Taggar (i kanalordning)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1621"/>
        <source>Comma-separated tag names. The position of each tag sets its channel index.</source>
        <translation>Kommaseparerade taggnamn. Positionen för varje tagg anger dess kanalindex.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MapTemplates.cpp" line="483"/>
        <source>Register blocks</source>
        <translation>Registerblock</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MapTemplates.cpp" line="484"/>
        <source>Polled register blocks with typed, scaled entries. Written by the Modbus register map importer.</source>
        <translation>Pollade registerblock med typade, skalade poster. Skrivna av Modbus-registerkartan importeraren.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MapTemplates.cpp" line="791"/>
        <source>Signal map</source>
        <translation>Signalkarta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MapTemplates.cpp" line="792"/>
        <source>CAN messages with their signal bit layouts and scaling. Written by the DBC importer.</source>
        <translation>CAN-meddelanden med sina signalbitlayouter och skalning. Skrivna av DBC-importeraren.</translation>
    </message>
</context>
<context>
    <name>Network</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="78"/>
        <source>Socket Type</source>
        <translation>Sockettyp</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="99"/>
        <source>Local Port</source>
        <translation>Lokal Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="106"/>
        <source>Type 0 for automatic port</source>
        <translation>Skriv 0 för automatisk port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="132"/>
        <source>Remote Address</source>
        <translation>Fjärradress</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="156"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="189"/>
        <source>Remote Port</source>
        <translation>Fjärrport</translation>
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
        <translation>Filtrera efter kanal…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="187"/>
        <source>Clear all notifications</source>
        <translation>Rensa alla notifikationer</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="271"/>
        <source>(no title)</source>
        <translation>(ingen titel)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="329"/>
        <source>No notifications yet</source>
        <translation>Inga notifikationer ännu</translation>
    </message>
    <message>
        <source>Dataset transforms and output widget scripts can post events here via notifyInfo / notifyWarning / notifyCritical.</source>
        <translation type="vanished">Datamängdstransformeringar och utdata-widgetskript kan publicera händelser här via notifyInfo / notifyWarning / notifyCritical.</translation>
    </message>
</context>
<context>
    <name>OnlineIconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="42"/>
        <source>Search Online Icons</source>
        <translation>Sök Onlineikoner</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="72"/>
        <source>Download failed: %1</source>
        <translation>Nedladdning misslyckades: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="97"/>
        <source>Search icons (e.g. temperature, arrow, play)…</source>
        <translation>Sök ikoner (t.ex. temperatur, pil, spela)…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="109"/>
        <source>Search</source>
        <translation>Sök</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="148"/>
        <source>Search for icons above to get started</source>
        <translation>Sök efter ikoner ovan för att komma igång</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="249"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="259"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
</context>
<context>
    <name>OutputWidgetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="91"/>
        <source>Output widgets require a Pro license.</source>
        <translation>Utdata-widgetar kräver en Pro-licens.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="125"/>
        <source>Button</source>
        <translation>Knapp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="129"/>
        <source>Send a command on click</source>
        <translation>Skicka ett kommando vid klick</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="134"/>
        <source>Slider</source>
        <translation>Skjutreglage</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="138"/>
        <source>Send scaled numeric values</source>
        <translation>Skicka skalade numeriska värden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="143"/>
        <source>Toggle</source>
        <translation>Växlare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="147"/>
        <source>Send on/off commands</source>
        <translation>Skicka på/av-kommandon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="152"/>
        <source>Text Field</source>
        <translation>Textfält</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="156"/>
        <source>Type and send arbitrary commands</source>
        <translation>Skriv och skicka godtyckliga kommandon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="160"/>
        <source>Knob</source>
        <translation>Ratt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="165"/>
        <source>Rotary input for setpoints</source>
        <translation>Roterande inmatning för börvärden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="93"/>
        <source>You can configure output widgets, but they only appear on the dashboard with a Pro license.</source>
        <translation>Du kan konfigurera utmatningswidgetar, men de visas endast på instrumentpanelen med en Pro-licens.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="182"/>
        <source>Duplicate</source>
        <translation>Duplicera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="185"/>
        <source>Duplicate this output widget</source>
        <translation>Duplicera denna utmatningswidget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="195"/>
        <source>Delete</source>
        <translation>Ta Bort</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="197"/>
        <source>Delete this output widget</source>
        <translation>Ta bort denna utmatningswidget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="274"/>
        <source>Transmit Function</source>
        <translation>Sändningsfunktion</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="284"/>
        <source>Import</source>
        <translation>Importera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="290"/>
        <source>Import transmit function from a .js file</source>
        <translation>Importera sändningsfunktion från en .js-fil</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="297"/>
        <source>Template</source>
        <translation>Mall</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="301"/>
        <source>Select a pre-built transmit function template</source>
        <translation>Välj en färdigbyggd mall för sändningsfunktion</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="306"/>
        <source>Test</source>
        <translation>Testa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="312"/>
        <source>Test the transmit function with sample input</source>
        <translation>Testa sändningsfunktionen med exempelinmatning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="353"/>
        <source>Undo</source>
        <translation>Ångra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="359"/>
        <source>Redo</source>
        <translation>Gör Om</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="367"/>
        <source>Cut</source>
        <translation>Klipp Ut</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="372"/>
        <source>Copy</source>
        <translation>Kopiera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="377"/>
        <source>Paste</source>
        <translation>Klistra In</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="384"/>
        <source>Select All</source>
        <translation>Markera Allt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="391"/>
        <source>Format Document</source>
        <translation>Formatera Dokument</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="396"/>
        <source>Format Selection</source>
        <translation>Formatera Markering</translation>
    </message>
</context>
<context>
    <name>Painter</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Painter.qml" line="56"/>
        <source>Painter Widget Error</source>
        <translation>Painter-widget-fel</translation>
    </message>
</context>
<context>
    <name>PainterCodeDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="30"/>
        <source>Painter Widget Code Editor</source>
        <translation>Kodredigerare för Painter-widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="76"/>
        <source>paint(ctx, w, h)</source>
        <translation>paint(ctx, w, h)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="86"/>
        <source>Import</source>
        <translation>Importera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="92"/>
        <source>Import painter code from a .js file</source>
        <translation>Importera painter-kod från en .js-fil</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="99"/>
        <source>Template</source>
        <translation>Mall</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="103"/>
        <source>Select a built-in painter template</source>
        <translation>Välj en inbyggd ritarmall</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="108"/>
        <source>Format</source>
        <translation>Formatera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="113"/>
        <source>Reformat the painter code</source>
        <translation>Formatera om ritarkoden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="119"/>
        <source>Test</source>
        <translation>Testa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="124"/>
        <source>Open a live preview with simulated dataset values</source>
        <translation>Öppna en förhandsvisning med simulerade datamängdsvärden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="127"/>
        <source>Preview</source>
        <translation>Förhandsvisning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="182"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="191"/>
        <source>Cut</source>
        <translation>Klipp Ut</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="192"/>
        <source>Copy</source>
        <translation>Kopiera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="193"/>
        <source>Paste</source>
        <translation>Klistra In</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="194"/>
        <source>Select All</source>
        <translation>Markera Allt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="196"/>
        <source>Undo</source>
        <translation>Ångra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="197"/>
        <source>Redo</source>
        <translation>Gör Om</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="199"/>
        <source>Format Document</source>
        <translation>Formatera Dokument</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="200"/>
        <source>Format Selection</source>
        <translation>Formatera Markering</translation>
    </message>
</context>
<context>
    <name>PainterTestDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="28"/>
        <source>Painter Live Preview</source>
        <translation>Förhandsgranskning av Painter Live</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="32"/>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="37"/>
        <source>Preview</source>
        <translation>Förhandsgranskning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="113"/>
        <source>Simulated datasets</source>
        <translation>Simulerade dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="180"/>
        <source>Runtime OK</source>
        <translation>Runtime OK</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="181"/>
        <source>Awaiting first frame...</source>
        <translation>Väntar på första frame...</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="194"/>
        <source>Console</source>
        <translation>Konsol</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="236"/>
        <source>Clear console</source>
        <translation>Rensa konsol</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="245"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
</context>
<context>
    <name>Plot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpolera</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="257"/>
        <source>Interpolation: %1</source>
        <translation>Interpolering: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="270"/>
        <source>Show Area Under Plot</source>
        <translation>Visa Area Under Kurva</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="288"/>
        <source>Show X Axis Label</source>
        <translation>Visa X-axeletikett</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="299"/>
        <source>Show Y Axis Label</source>
        <translation>Visa Y-axeletikett</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="311"/>
        <source>Show Crosshair</source>
        <translation>Visa Hårkors</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="318"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="318"/>
        <source>Resume</source>
        <translation>Återuppta</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="335"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Svep-/triggerläge</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="347"/>
        <source>Trigger Settings</source>
        <translation>Triggerinställningar</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="371"/>
        <source>Reset View</source>
        <translation>Återställ Vy</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="377"/>
        <source>Axis Range Settings</source>
        <translation>Inställningar för Axelområde</translation>
    </message>
</context>
<context>
    <name>Plot3D</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="203"/>
        <source>Interpolate</source>
        <translation>Interpolera</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="221"/>
        <source>Orbit Navigation</source>
        <translation>Orbitnavigering</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="231"/>
        <source>Pan Navigation</source>
        <translation>Panoreringsnavigering</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="242"/>
        <source>Orthogonal View</source>
        <translation>Ortogonal Vy</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="248"/>
        <source>Top View</source>
        <translation>Toppvy</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="254"/>
        <source>Left View</source>
        <translation>Vänster Vy</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="260"/>
        <source>Front View</source>
        <translation>Frontvy</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="277"/>
        <source>Auto Center</source>
        <translation>Autocentrera</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="293"/>
        <source>Anaglyph 3D</source>
        <translation>Anaglyf 3D</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="307"/>
        <source>Invert Eye Positions</source>
        <translation>Invertera Ögonpositioner</translation>
    </message>
</context>
<context>
    <name>PlotCommon</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="59"/>
        <source>None</source>
        <translation>Ingen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="62"/>
        <source>ZOH</source>
        <translation>ZOH</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="65"/>
        <source>Stem</source>
        <translation>Stam</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="67"/>
        <source>Linear</source>
        <translation>Linjär</translation>
    </message>
</context>
<context>
    <name>PlotWidget</name>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1326"/>
        <source>Time</source>
        <translation>Tid</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1349"/>
        <source>ΔX: %1  ΔY: %2 — Drag to move, right-click to clear</source>
        <translation>ΔX: %1  ΔY: %2 — Dra för att flytta, högerklicka för att rensa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1352"/>
        <source>Click to place cursor</source>
        <translation>Klicka för att placera markör</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1354"/>
        <source>Click to place second cursor — Drag to move</source>
        <translation>Klicka för att placera andra markören — Dra för att flytta</translation>
    </message>
</context>
<context>
    <name>ProNotice</name>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="119"/>
        <source>Visit Website</source>
        <translation>Besök Webbplats</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="127"/>
        <source>Buy License</source>
        <translation>Köp Licens</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="140"/>
        <source>Activate</source>
        <translation>Aktivera</translation>
    </message>
</context>
<context>
    <name>ProUpgradeNotice</name>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="26"/>
        <source>Assistant — Pro feature</source>
        <translation>Assistent — Pro-funktion</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="44"/>
        <source>The Assistant is a Serial Studio Pro feature. Activate your license to unlock it.</source>
        <translation>Assistenten är en Serial Studio Pro-funktion. Aktivera din licens för att låsa upp den.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="52"/>
        <source>Activate</source>
        <translation>Aktivera</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="66"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
</context>
<context>
    <name>Process</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="69"/>
        <source>Mode</source>
        <translation>Läge</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Launch Process</source>
        <translation>Starta Process</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Named Pipe</source>
        <translation>Namngiven Pipe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="101"/>
        <source>Executable</source>
        <translation>Körbar Fil</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="116"/>
        <source>/path/to/executable</source>
        <translation>/sökväg/till/körbar-fil</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="209"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="257"/>
        <source>Browse</source>
        <translation>Bläddra</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="145"/>
        <source>Arguments</source>
        <translation>Argument</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="156"/>
        <source>--arg1 value1 --arg2 value2</source>
        <translation>--arg1 värde1 --arg2 värde2</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="177"/>
        <source>Working Dir</source>
        <translation>Arbetskatalog</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="192"/>
        <source>(optional) /working/directory</source>
        <translation>(valfritt) /arbets/katalog</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="223"/>
        <source>Pipe Path</source>
        <translation>Pipe-sökväg</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="273"/>
        <source>Pick Running Process…</source>
        <translation>Välj Körande Process…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="311"/>
        <source>Launch a child process and capture its stdout, or connect to a named pipe written by an existing process.</source>
        <translation>Starta en underprocess och fånga dess stdout, eller anslut till en namngiven pipe som skrivs av en befintlig process.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="319"/>
        <source>Learn about named pipes</source>
        <translation>Läs mer om namngivna pipes</translation>
    </message>
</context>
<context>
    <name>ProcessPicker</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="60"/>
        <source>Select Running Process</source>
        <translation>Välj Körande Process</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="211"/>
        <source>Select a running process to derive a named-pipe path suggestion.</source>
        <translation>Välj en körande process för att härleda ett förslag på sökväg för namngiven pipe.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="217"/>
        <source>Filter Processes</source>
        <translation>Filtrera Processer</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="231"/>
        <source>Type to filter by name…</source>
        <translation>Skriv för att filtrera efter namn…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="235"/>
        <source>Refresh</source>
        <translation>Uppdatera</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="243"/>
        <source>Running Processes</source>
        <translation>Körande Processer</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="281"/>
        <source>Process</source>
        <translation>Process</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="288"/>
        <source>PID</source>
        <translation>PID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="383"/>
        <source>No processes match the filter.</source>
        <translation>Inga processer matchar filtret.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="384"/>
        <source>No running processes found.
Click Refresh to update the list.</source>
        <translation>Inga körande processer hittades.
Klicka på Uppdatera för att uppdatera listan.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="400"/>
        <source>%1 process(es)</source>
        <translation>%1 process(er)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="404"/>
        <source>Select</source>
        <translation>Välj</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="410"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
</context>
<context>
    <name>ProjectEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="43"/>
        <source>modified</source>
        <translation>ändrad</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="362"/>
        <source>This project is password protected</source>
        <translation>Detta projekt är lösenordsskyddat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="363"/>
        <source>Editing is available in Project mode</source>
        <translation>Redigering är tillgänglig i Projektläge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="374"/>
        <source>Enter the password to make changes, or open a different project.</source>
        <translation>Ange lösenordet för att göra ändringar, eller öppna ett annat projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="375"/>
        <source>Switch to Project mode to load and edit a project.</source>
        <translation>Växla till Projektläge för att ladda och redigera ett projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="397"/>
        <source>Unlock</source>
        <translation>Lås Upp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="398"/>
        <source>Switch to Project Mode</source>
        <translation>Växla till Projektläge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="417"/>
        <source>Open Other Project</source>
        <translation>Öppna Annat Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="418"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="434"/>
        <source>Create New Project</source>
        <translation>Skapa Nytt Projekt</translation>
    </message>
</context>
<context>
    <name>ProjectStructure</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="32"/>
        <source>Project Structure</source>
        <translation>Projektstruktur</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="71"/>
        <source>Search</source>
        <translation>Sök</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="169"/>
        <source>Move Up</source>
        <translation>Flytta Upp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="174"/>
        <source>Move Down</source>
        <translation>Flytta Ner</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="185"/>
        <source>Duplicate Selected (%1)</source>
        <translation>Duplicera Markerad (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="186"/>
        <source>Duplicate</source>
        <translation>Duplicera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="199"/>
        <source>Delete Selected (%1)</source>
        <translation>Ta Bort Markerad (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="200"/>
        <source>Delete</source>
        <translation>Ta Bort</translation>
    </message>
</context>
<context>
    <name>ProjectToolbar</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="142"/>
        <source>New</source>
        <translation>Ny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="145"/>
        <source>Create a new JSON project</source>
        <translation>Skapa ett nytt JSON-projekt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="158"/>
        <source>Open</source>
        <translation>Öppna</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="162"/>
        <source>Open an existing JSON project</source>
        <translation>Öppna ett befintligt JSON-projekt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="168"/>
        <source>Save</source>
        <translation>Spara</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="175"/>
        <source>Save the current project</source>
        <translation>Spara det aktuella projektet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="181"/>
        <source>Save As</source>
        <translation>Spara Som</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="188"/>
        <source>Save the current project under a new name</source>
        <translation>Spara det aktuella projektet under ett nytt namn</translation>
    </message>
    <message>
        <source>Unlock</source>
        <translation type="vanished">Lås Upp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="230"/>
        <source>Lock</source>
        <translation>Lås</translation>
    </message>
    <message>
        <source>Unlock the Project Editor with the project password</source>
        <translation type="vanished">Lås upp Projektredigeraren med projektlösenordet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="200"/>
        <source>Import</source>
        <translation>Importera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="204"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="208"/>
        <source>Generate a project from a Protocol Buffers (.proto) schema</source>
        <translation>Generera ett projekt från ett Protocol Buffers (.proto) schema</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="234"/>
        <source>Set a password and lock the Project Editor</source>
        <translation>Ange ett lösenord och lås Projektredigeraren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="245"/>
        <source>Add Device</source>
        <translation>Lägg till Enhet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="249"/>
        <source>Add a new data source (device) to the project</source>
        <translation>Lägg till en ny datakälla (enhet) till projektet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="272"/>
        <source>Action</source>
        <translation>Åtgärd</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="275"/>
        <source>Add a new action to the project</source>
        <translation>Lägg till en ny åtgärd till projektet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="260"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="264"/>
        <source>Output</source>
        <translation>Utgång</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="218"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="222"/>
        <source>Restore</source>
        <translation>Återställ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="226"/>
        <source>Restore a recent automatic snapshot of the current project</source>
        <translation>Återställ en nylig automatisk ögonblicksbild av det aktuella projektet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="267"/>
        <source>Add a new output control panel with a button</source>
        <translation>Lägg till en ny utgångskontrollpanel med en knapp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="288"/>
        <source>Slider</source>
        <translation>Skjutreglage</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="291"/>
        <source>Add an output slider control</source>
        <translation>Lägg till en utgångskontroll med skjutreglage</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="298"/>
        <source>Toggle</source>
        <translation>Växlare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="301"/>
        <source>Add an output toggle control</source>
        <translation>Lägg till en utgångskontroll med växlare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="308"/>
        <source>Knob</source>
        <translation>Ratt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="311"/>
        <source>Add an output knob control</source>
        <translation>Lägg till en utgångskontroll med ratt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="319"/>
        <source>Text Field</source>
        <translation>Textfält</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="321"/>
        <source>Add an output text field control</source>
        <translation>Lägg till en utgångskontroll med textfält</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="328"/>
        <source>Button</source>
        <translation>Knapp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="331"/>
        <source>Add an output button control</source>
        <translation>Lägg till en utdata-knappkontroll</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="344"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="348"/>
        <source>Dataset</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="350"/>
        <source>Add a generic dataset</source>
        <translation>Lägg till ett generiskt dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="364"/>
        <source>Plot</source>
        <translation>Diagram</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="367"/>
        <source>Add a 2D plot dataset</source>
        <translation>Lägg till ett 2D-diagramdataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="374"/>
        <source>FFT Plot</source>
        <translation>FFT-diagram</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="377"/>
        <source>Add a Fast Fourier Transform plot</source>
        <translation>Lägg till ett Fast Fourier Transform-diagram</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="384"/>
        <source>Gauge</source>
        <translation>Mätare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="387"/>
        <source>Add a gauge widget for numeric data</source>
        <translation>Lägg till en mätarwidget för numerisk data</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="395"/>
        <source>Level Indicator</source>
        <translation>Nivåindikator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="397"/>
        <source>Add a vertical bar level indicator</source>
        <translation>Lägg till en vertikal stapelnivåindikator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="404"/>
        <source>Compass</source>
        <translation>Kompass</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="407"/>
        <source>Add a compass widget for directional data</source>
        <translation>Lägg till en kompasswidget för riktningsdata</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="415"/>
        <source>LED Indicator</source>
        <translation>LED-indikator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="417"/>
        <source>Add an LED-style status indicator</source>
        <translation>Lägg till en LED-statusindikator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="430"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="434"/>
        <source>Group</source>
        <translation>Grupp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="436"/>
        <source>Add a dataset container group</source>
        <translation>Lägg till en datauppsättningscontainergrupp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="438"/>
        <source>Dataset Container</source>
        <translation>Datauppsättningscontainer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="442"/>
        <source>Image</source>
        <translation>Bild</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="444"/>
        <source>Add an image/video stream viewer</source>
        <translation>Lägg till en bild-/videoströmvisare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="446"/>
        <source>Image View</source>
        <translation>Bildvisning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="454"/>
        <source>Painter</source>
        <translation>Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="458"/>
        <source>Add a custom JavaScript-rendered painter widget</source>
        <translation>Lägg till en anpassad JavaScript-renderad painter-widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="459"/>
        <source>Painter widgets require a Pro license — adding one will fall back to a data grid</source>
        <translation>Painter-widgetar kräver en Pro-licens — att lägga till en kommer att falla tillbaka till ett datarutnät</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="460"/>
        <source>Painter Widget</source>
        <translation>Painter-widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="472"/>
        <source>Table</source>
        <translation>Tabell</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="475"/>
        <source>Add a data table view</source>
        <translation>Lägg till en datatabellvy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="477"/>
        <source>Data Grid</source>
        <translation>Datarutnät</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="483"/>
        <source>Multi-Plot</source>
        <translation>Flerplott</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="485"/>
        <source>Add a 2D plot with multiple signals</source>
        <translation>Lägg till ett 2D-diagram med flera signaler</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="487"/>
        <source>Multiple Plot</source>
        <translation>Flerplott</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="492"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="497"/>
        <source>3D Plot</source>
        <translation>3D-plott</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="495"/>
        <source>Add a 3D plot visualization</source>
        <translation>Lägg till en 3D-plottvisualisering</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="503"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="507"/>
        <source>Accelerometer</source>
        <translation>Accelerometer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="505"/>
        <source>Add a group for 3-axis accelerometer data</source>
        <translation>Lägg till en grupp för 3-axlig accelerometerdata</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="513"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="516"/>
        <source>Gyroscope</source>
        <translation>Gyroskop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="517"/>
        <source>Add a group for 3-axis gyroscope data</source>
        <translation>Lägg till en grupp för 3-axlig gyroskopdata</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="522"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="527"/>
        <source>GPS Map</source>
        <translation>GPS-karta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="525"/>
        <source>Add a map widget for GPS data</source>
        <translation>Lägg till en kartwidget för GPS-data</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="539"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="543"/>
        <source>Assistant</source>
        <translation>Assistent</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="546"/>
        <source>Open the Assistant</source>
        <translation>Öppna Assistenten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="552"/>
        <source>Help Center</source>
        <translation>Hjälpcenter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="556"/>
        <source>Open the Project Editor documentation</source>
        <translation>Öppna dokumentationen för projektredigeraren</translation>
    </message>
</context>
<context>
    <name>ProjectView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="34"/>
        <source>Project Summary</source>
        <translation>Projektsammanfattning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="81"/>
        <source>Pro features detected in this project.</source>
        <translation>Pro-funktioner upptäckta i detta projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="83"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Använder reservwidgetar. Köp en licens för att låsa upp full funktionalitet.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="118"/>
        <source>Project Title:</source>
        <translation>Projekttitel:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="129"/>
        <source>Untitled Project</source>
        <translation>Namnlöst Projekt</translation>
    </message>
    <message>
        <source>Points:</source>
        <translation type="vanished">Punkter:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="149"/>
        <source>Time Range:</source>
        <translation>Tidsområde:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="235"/>
        <source>Source</source>
        <translation>Källa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="236"/>
        <source>Sources</source>
        <translation>Källor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="241"/>
        <source>Group</source>
        <translation>Grupp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="242"/>
        <source>Groups</source>
        <translation>Grupper</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="247"/>
        <source>Dataset</source>
        <translation>Datamängd</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="248"/>
        <source>Datasets</source>
        <translation>Datamängder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="253"/>
        <source>Action</source>
        <translation>Åtgärd</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="254"/>
        <source>Actions</source>
        <translation>Åtgärder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="342"/>
        <source>Double-click a block to edit it. Right-click anywhere to add a group, dataset, action, data table, or device.</source>
        <translation>Dubbelklicka på ett block för att redigera det. Högerklicka var som helst för att lägga till en grupp, dataset, åtgärd, datatabell eller enhet.</translation>
    </message>
</context>
<context>
    <name>ProtoPreviewDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="41"/>
        <source>Protocol Buffers File Preview</source>
        <translation>Förhandsgranskning av Protocol Buffers-fil</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="165"/>
        <source>Proto File: %1</source>
        <translation>Proto-fil: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="173"/>
        <source>Browse the messages below, then create the project. Every message becomes a dashboard group; matching-type channel blocks get a MultiPlot and mixed-type messages get a DataGrid.</source>
        <translation>Bläddra bland meddelandena nedan och skapa sedan projektet. Varje meddelande blir en instrumentpanelsgrupp; kanaler med matchande typ får en MultiPlot och meddelanden med blandade typer får en DataGrid.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="183"/>
        <source>Show fields for</source>
        <translation>Visa fält för</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="209"/>
        <source>Fields</source>
        <translation>Fält</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="243"/>
        <source>Tag</source>
        <translation>Tagg</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="253"/>
        <source>Field Name</source>
        <translation>Fältnamn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="258"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="345"/>
        <source>No fields in the selected message.</source>
        <translation>Inga fält i det valda meddelandet.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="363"/>
        <source>Total: %1 messages, %2 fields</source>
        <translation>Totalt: %1 meddelanden, %2 fält</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="370"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="381"/>
        <source>Create Project</source>
        <translation>Skapa Projekt</translation>
    </message>
</context>
<context>
    <name>Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="71"/>
        <source>No error</source>
        <translation>Inget fel</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="73"/>
        <source>The broker rejected the connection due to an unsupported protocol version. Match the broker's MQTT version and try again.</source>
        <translation>Brokern nekade anslutningen på grund av en protokollversion som inte stöds. Matcha brokerns MQTT-version och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="76"/>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Regenerate it and try again.</source>
        <translation>Brokern nekade klient-ID:t. Det kan vara felaktigt formaterat, för långt eller redan används. Generera om det och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="79"/>
        <source>The network reached the broker, but the broker is currently unavailable. Verify its status and try again later.</source>
        <translation>Nätverket nådde brokern, men brokern är för närvarande otillgänglig. Verifiera dess status och försök igen senare.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="82"/>
        <source>The username or password is incorrect or malformed. Double-check the credentials and try again.</source>
        <translation>Användarnamnet eller lösenordet är felaktigt eller felformaterat. Kontrollera inloggningsuppgifterna och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="85"/>
        <source>The broker denied the connection due to insufficient permissions. Verify that the account has the required ACLs.</source>
        <translation>Brokern nekade anslutningen på grund av otillräckliga behörigheter. Verifiera att kontot har nödvändiga ACL:er.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="88"/>
        <source>A network or transport-layer issue prevented the connection. Check connectivity, ports, and TLS configuration.</source>
        <translation>Ett nätverks- eller transportlagerproblem förhindrade anslutningen. Kontrollera anslutning, portar och TLS-konfiguration.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="91"/>
        <source>The client detected an MQTT protocol violation and closed the connection. Verify broker and client compatibility.</source>
        <translation>Klienten upptäckte en överträdelse av MQTT-protokollet och stängde anslutningen. Verifiera broker- och klientkompatibilitet.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="94"/>
        <source>An unexpected error occurred. Check the broker logs and the application console for details.</source>
        <translation>Ett oväntat fel inträffade. Kontrollera broker-loggarna och applikationskonsolen för mer information.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="97"/>
        <source>An MQTT 5 protocol-level error occurred. Inspect the broker's reason code for details.</source>
        <translation>Ett MQTT 5-protokollnivåfel inträffade. Inspektera brokerns orsakskod för mer information.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="101"/>
        <source>Unspecified MQTT error (code %1).</source>
        <translation>Ospecificerat MQTT-fel (kod %1).</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../src/Misc/Translator.cpp" line="231"/>
        <source>Failed to load welcome text :(</source>
        <translation>Misslyckades att ladda välkomsttext :(</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="251"/>
        <source>Network error</source>
        <translation>Nätverksfel</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="254"/>
        <location filename="../../src/Licensing/Trial.cpp" line="270"/>
        <location filename="../../src/Licensing/Trial.cpp" line="302"/>
        <source>Trial Activation Error</source>
        <translation>Aktiveringsfel för Provperiod</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="267"/>
        <source>Invalid server response</source>
        <translation>Ogiltigt serversvar</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="268"/>
        <source>The server returned malformed data: %1</source>
        <translation>Servern returnerade felformaterad data: %1</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="299"/>
        <source>Unexpected server response</source>
        <translation>Oväntat serversvar</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="300"/>
        <source>The server response is missing required fields.</source>
        <translation>Serversvaret saknar obligatoriska fält.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="162"/>
        <source>Console Output File Error</source>
        <translation>Konsolens Utdatafilfel</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="163"/>
        <source>Cannot open file for writing!</source>
        <translation>Kan inte öppna fil för skrivning!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1311"/>
        <source>Invalid Bluetooth adapter!</source>
        <translation>Ogiltig Bluetooth-adapter!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1314"/>
        <source>Unsuported platform or operating system</source>
        <translation>Plattform eller operativsystem stöds ej</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1317"/>
        <source>Unsupported discovery method</source>
        <translation>Upptäcktsmetod stöds ej</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1320"/>
        <source>General I/O error</source>
        <translation>Allmänt I/O-fel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="271"/>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="252"/>
        <source>Frame Parser Disabled</source>
        <translation>Frame-parser Inaktiverad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="272"/>
        <source>The Lua frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>Lua frame-parsern för källa %1 överskred tidsgränsen %2 ramar i rad och har inaktiverats för att hålla Serial Studio responsivt.

Troligaste orsak: en oändlig loop eller extremt långsam operation i skriptkroppen. Åtgärda skriptet och ladda om projektet för att återaktivera parsning.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="320"/>
        <source>Lua Syntax Error</source>
        <translation>Lua-syntaxfel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="321"/>
        <source>The parser code contains an error:

%1</source>
        <translation>Parserkoden innehåller ett fel:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="369"/>
        <source>Lua Runtime Error</source>
        <translation>Lua-körningsfel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="370"/>
        <source>The parser code triggered an error:

%1</source>
        <translation>Parserkoden utlöste ett fel:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="391"/>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="478"/>
        <source>Missing Parse Function</source>
        <translation>Parse-funktion Saknas</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="392"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) ... end</source>
        <translation>Funktionen 'parse' är inte definierad i skriptet.

Säkerställ att din kod innehåller:
function parse(frame) ... end</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="454"/>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="530"/>
        <source>Parse Function Runtime Error</source>
        <translation>Körningsfel i Parse-funktionen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="455"/>
        <source>The parse function contains an error:

%1

Please fix the error in the function body.</source>
        <translation>Parse-funktionen innehåller ett fel:

%1

Åtgärda felet i funktionskroppen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="253"/>
        <source>The JavaScript frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>JavaScript frame-parsern för källa %1 överskred tidsgränsen %2 ramar i rad och har inaktiverats för att hålla Serial Studio responsivt.

Troligaste orsak: en oändlig loop eller extremt långsam operation i skriptkroppen. Åtgärda skriptet och ladda om projektet för att återaktivera parsning.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="419"/>
        <source>JavaScript Timed Out</source>
        <translation>Javascript Överskred Tidsgränsen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="420"/>
        <source>The parser code did not finish evaluating within %1 ms and was interrupted.

Most likely cause: an infinite loop at the top level of the script.</source>
        <translation>Tolkningskoden slutförde inte utvärderingen inom %1 ms och avbröts.

Troligaste orsak: en oändlig loop på toppnivån i skriptet.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="437"/>
        <source>JavaScript Syntax Error</source>
        <translation>Javascript-syntaxfel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="438"/>
        <source>The parser code contains a syntax error at line %1:

%2</source>
        <translation>Parserkoden innehåller ett syntaxfel på rad %1:

%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="452"/>
        <source>JavaScript Exception Occurred</source>
        <translation>Javascript-undantag Inträffade</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="453"/>
        <source>The parser code triggered the following exceptions:

%1</source>
        <translation>Parserkoden utlöste följande undantag:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="479"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) { ... }</source>
        <translation>Funktionen 'parse' är inte definierad i skriptet.

Säkerställ att din kod inkluderar:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="531"/>
        <source>The parse function contains an error at line %1:

%2

Please fix the error in the function body.</source>
        <translation>Parse-funktionen innehåller ett fel på rad %1:

%2

Åtgärda felet i funktionskroppen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="631"/>
        <source>Invalid Function Declaration</source>
        <translation>Ogiltig Funktionsdeklaration</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="632"/>
        <source>No callable 'parse' export found.

Define one of:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</source>
        <translation>Ingen anropbar 'parse'-export hittades.

Definiera en av:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="648"/>
        <source>The 'parse' function must accept at least one parameter (the frame payload).</source>
        <translation>Funktionen 'parse' måste acceptera minst en parameter (frame-payloaden).</translation>
    </message>
    <message>
        <source>No valid 'parse' function declaration found.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Ingen giltig 'parse'-funktionsdeklaration hittades.

Förväntad format:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="647"/>
        <source>Invalid Function Parameter</source>
        <translation>Ogiltig Funktionsparameter</translation>
    </message>
    <message>
        <source>The 'parse' function must have at least one parameter.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Funktionen 'parse' måste ha minst en parameter.

Förväntad format:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="613"/>
        <source>Deprecated Function Signature</source>
        <translation>Föråldrad Funktionssignatur</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="614"/>
        <source>The 'parse' function uses the old two-parameter format: parse(%1, %2)

This format is no longer supported. Please update to the new single-parameter format:
function parse(%1) { ... }

The separator parameter is no longer needed.</source>
        <translation>Funktionen 'parse' använder det gamla tvåparametersformatet: parse(%1, %2)

Detta format stöds inte längre. Uppdatera till det nya enparametersformatet:
function parse(%1) { ... }

Separatorparametern behövs inte längre.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="182"/>
        <source>Critical</source>
        <translation>Kritiskt</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="182"/>
        <source>Warning</source>
        <translation>Varning</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="572"/>
        <source>Project file not found</source>
        <translation>Projektfilen hittades inte</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="573"/>
        <source>The project file referenced by this shortcut could not be found:

%1</source>
        <translation>Projektfilen som denna genväg refererar till kunde inte hittas:

%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="576"/>
        <source>Would you like to delete this shortcut?</source>
        <translation>Vill du ta bort denna genväg?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="580"/>
        <source>Delete Shortcut</source>
        <translation>Ta Bort Genväg</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="582"/>
        <source>Quit</source>
        <translation>Avsluta</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1051"/>
        <source>Time (s)</source>
        <translation>Tid (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1109"/>
        <source>Freq: %1</source>
        <translation>Frekv: %1</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1112"/>
        <source>Time: −%1</source>
        <translation>Tid: −%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIProvider.cpp" line="362"/>
        <source>No OpenAI API key set. Open Manage Keys to add one.</source>
        <translation>Ingen OpenAI API-nyckel angiven. Öppna Hantera Nycklar för att lägga till en.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicProvider.cpp" line="234"/>
        <source>No Anthropic API key set. Open Manage Keys to add one.</source>
        <translation>Ingen Anthropic API-nyckel angiven. Öppna Hantera Nycklar för att lägga till en.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiProvider.cpp" line="285"/>
        <source>No Gemini API key set. Open Manage Keys to add one.</source>
        <translation>Ingen Gemini API-nyckel angiven. Öppna Hantera Nycklar för att lägga till en.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/LocalProvider.cpp" line="319"/>
        <source>No local model server URL configured. Open Manage Keys to set one.</source>
        <translation>Ingen lokal modellserver-URL konfigurerad. Öppna Hantera Nycklar för att ange en.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/DeepSeekProvider.cpp" line="146"/>
        <source>No DeepSeek API key set. Open Manage Keys to add one.</source>
        <translation>Ingen DeepSeek API-nyckel angiven. Öppna Hantera Nycklar för att lägga till en.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/MistralProvider.cpp" line="168"/>
        <source>No Mistral API key set. Open Manage Keys to add one.</source>
        <translation>Ingen Mistral API-nyckel angiven. Öppna Hantera Nycklar för att lägga till en.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenRouterProvider.cpp" line="181"/>
        <source>No OpenRouter API key set. Open Manage Keys to add one.</source>
        <translation>Ingen OpenRouter API-nyckel angiven. Öppna Hantera Nycklar för att lägga till en.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GroqProvider.cpp" line="152"/>
        <source>No Groq API key set. Open Manage Keys to add one.</source>
        <translation>Ingen Groq API-nyckel angiven. Öppna Hantera Nycklar för att lägga till en.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1011"/>
        <source>The frame parser is using more than %1% of CPU time.</source>
        <translation>Frame-parsern använder mer än %1% av CPU-tiden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1013"/>
        <source>Serial Studio is dropping frames to keep the application responsive. Please simplify or optimize the frame parser script to reduce its workload.</source>
        <translation>Serial Studio tappar ramar för att hålla applikationen responsiv. Förenkla eller optimera frame-parser-skriptet för att minska dess arbetsbelastning.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="386"/>
        <source>Expected %1, got '%2'</source>
        <translation>Förväntade %1, fick '%2'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="435"/>
        <source>Expected enum name after 'enum'</source>
        <translation>Förväntade enum-namn efter 'enum'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="449"/>
        <source>Expected oneof name</source>
        <translation>Förväntade oneof-namn</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="476"/>
        <source>Field tag '%1' out of range (1..%2)</source>
        <translation>Fälttagg '%1' utanför intervallet (1..%2)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="494"/>
        <source>Expected key type in map&lt;&gt;</source>
        <translation>Förväntade nyckeltyp i map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="502"/>
        <source>Expected value type in map&lt;&gt;</source>
        <translation>Förväntade värdetyp i map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="510"/>
        <source>Expected map field name</source>
        <translation>Förväntade map-fältnamn</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="522"/>
        <source>Expected map field tag</source>
        <translation>Förväntade map-fälttagg</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="554"/>
        <source>Expected field type, got '%1'</source>
        <translation>Förväntade fälttyp, fick '%1'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="573"/>
        <source>Expected field name after type</source>
        <translation>Förväntade fältnamn efter typ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="583"/>
        <source>Expected field tag number</source>
        <translation>Förväntade fälttaggnummer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="630"/>
        <source>Message nesting too deep (limit %1)</source>
        <translation>Meddelandenästling för djup (gräns %1)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="635"/>
        <source>Expected message name</source>
        <translation>Förväntade meddelandenamn</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="717"/>
        <source>Unexpected token '%1' at file scope</source>
        <translation>Oväntat token '%1' i filomfång</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="763"/>
        <source>Unsupported top-level keyword '%1'</source>
        <translation>Nyckelord '%1' på toppnivå stöds ej</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="294"/>
        <source>Automatic (Platform Default)</source>
        <translation>Automatisk (Plattformstandard)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="299"/>
        <source>Software (Fallback)</source>
        <translation>Mjukvara (Reserv)</translation>
    </message>
    <message>
        <source>Row %1</source>
        <translation type="vanished">Rad %1</translation>
    </message>
    <message>
        <source>[%1]</source>
        <translation type="vanished">[%1]</translation>
    </message>
    <message>
        <source>Frame %1</source>
        <translation type="vanished">Ram %1</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Avkodare</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation type="vanished">Rader</translation>
    </message>
    <message>
        <source>%1 row(s)</source>
        <translation type="vanished">%1 rad(er)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="186"/>
        <source>The native parser configuration is not a valid JSON object.</source>
        <translation>Den inbyggda parserkonfigurationen är inte ett giltigt JSON-objekt.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="196"/>
        <source>Unknown native parser template: "%1".</source>
        <translation>Okänd inbyggd parsermall: "%1".</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="330"/>
        <source>Built-In Parser Error</source>
        <translation>Inbyggt Parserfel</translation>
    </message>
    <message>
        <source>Native Parser Error</source>
        <translation type="vanished">Inbyggt Parserfel</translation>
    </message>
</context>
<context>
    <name>QuaGzipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="60"/>
        <source>QIODevice::Append is not supported for GZIP</source>
        <translation>QIODevice::Append stöds inte för GZIP</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="66"/>
        <source>Opening gzip for both reading and writing is not supported</source>
        <translation>Att öppna gzip för både läsning och skrivning stöds inte</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="75"/>
        <source>You can open a gzip either for reading or for writing. Which is it?</source>
        <translation>Du kan öppna en gzip antingen för läsning eller för skrivning. Vilket är det?</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="81"/>
        <source>Could not gzopen() file</source>
        <translation>Kunde inte köra gzopen() på filen</translation>
    </message>
</context>
<context>
    <name>QuaZIODevice</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="178"/>
        <source>QIODevice::Append is not supported for QuaZIODevice</source>
        <translation>QIODevice::Append stöds inte för QuaZIODevice</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="183"/>
        <source>QIODevice::ReadWrite is not supported for QuaZIODevice</source>
        <translation>QIODevice::ReadWrite stöds inte för QuaZIODevice</translation>
    </message>
</context>
<context>
    <name>QuaZipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quazipfile.cpp" line="251"/>
        <source>ZIP/UNZIP API error %1</source>
        <translation>ZIP/UNZIP API-fel %1</translation>
    </message>
</context>
<context>
    <name>ReportOptionsDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate PDF Report</source>
        <translation>Generera PDF-rapport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate Report</source>
        <translation>Generera Rapport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="61"/>
        <source>Solid</source>
        <translation>Heldragen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="62"/>
        <source>Dashed</source>
        <translation>Streckad</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="63"/>
        <source>Dotted</source>
        <translation>Prickad</translation>
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
        <translation>Letter (8,5 × 11 tum)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="90"/>
        <source>Legal (8.5 × 14 in)</source>
        <translation>Legal (8,5 × 14 tum)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="91"/>
        <source>Executive (7.25 × 10.5 in)</source>
        <translation>Executive (7,25 × 10,5 tum)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="92"/>
        <source>Tabloid (11 × 17 in)</source>
        <translation>Tabloid (11 × 17 tum)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="93"/>
        <source>Ledger (17 × 11 in)</source>
        <translation>Ledger (17 × 11 tum)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="103"/>
        <source>%1 — Session Report</source>
        <translation>%1 — Sessionsrapport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="105"/>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="279"/>
        <source>Session Report</source>
        <translation>Sessionsrapport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="187"/>
        <source>Branding</source>
        <translation>Varumärke</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="193"/>
        <source>Page</source>
        <translation>Sida</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="199"/>
        <source>Sections</source>
        <translation>Sektioner</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="247"/>
        <source>Identity</source>
        <translation>Identitet</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="261"/>
        <source>Company</source>
        <translation>Företag</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="268"/>
        <source>e.g. Acme Test Systems</source>
        <translation>t.ex. Acme Test Systems</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="272"/>
        <source>Document title</source>
        <translation>Dokumenttitel</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="283"/>
        <source>Author</source>
        <translation>Författare</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="290"/>
        <source>Prepared by (optional)</source>
        <translation>Förberedd av (valfritt)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="299"/>
        <source>Logo</source>
        <translation>Logotyp</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="312"/>
        <source>File</source>
        <translation>Fil</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="323"/>
        <source>PNG, JPG or SVG (optional)</source>
        <translation>PNG, JPG eller SVG (valfritt)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="325"/>
        <source>Browse…</source>
        <translation>Bläddra…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="328"/>
        <source>Clear</source>
        <translation>Rensa</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="369"/>
        <source>Paper</source>
        <translation>Papper</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="381"/>
        <source>Page size</source>
        <translation>Sidstorlek</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="509"/>
        <source>Annotate min, max, and mean values on plots</source>
        <translation>Annotera min-, max- och medelvärden på diagram</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="557"/>
        <source>Export HTML</source>
        <translation>Exportera HTML</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="497"/>
        <source>Cover page (logo, document title, test subtitle)</source>
        <translation>Omslagssida (logotyp, dokumenttitel, testunderrubrik)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="500"/>
        <source>Test information (project, timestamps, classification and notes)</source>
        <translation>Testinformation (projekt, tidsstämplar, klassificering och anteckningar)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="503"/>
        <source>Measurement summary (min, max, mean, std. deviation per parameter)</source>
        <translation>Mätsammanfattning (min, max, medel, std. avvikelse per parameter)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="506"/>
        <source>Parameter trends (time-series chart per numeric parameter)</source>
        <translation>Parametertrender (tidsseriegraf per numerisk parameter)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="396"/>
        <source>Plot appearance</source>
        <translation>Diagramutseende</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="410"/>
        <source>Line width</source>
        <translation>Linjebredd</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="442"/>
        <source>Line style</source>
        <translation>Linjestil</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="482"/>
        <source>Include</source>
        <translation>Inkludera</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="532"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="557"/>
        <source>Export PDF</source>
        <translation>Exportera PDF</translation>
    </message>
</context>
<context>
    <name>ReportProgressDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="20"/>
        <source>Generating Report</source>
        <translation>Genererar Rapport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="69"/>
        <source>Working…</source>
        <translation>Arbetar…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="86"/>
        <source>This can take a few seconds for sessions with many parameters. The window closes automatically when the report is ready.</source>
        <translation>Detta kan ta några sekunder för sessioner med många parametrar. Fönstret stängs automatiskt när rapporten är klar.</translation>
    </message>
</context>
<context>
    <name>RuntimeReconfigure</name>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="40"/>
        <source>Connection Lost</source>
        <translation>Anslutning Förlorad</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="41"/>
        <source>Device Unavailable</source>
        <translation>Enhet Otillgänglig</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="95"/>
        <source>The connection to your device was lost.</source>
        <translation>Anslutningen till enheten förlorades.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="96"/>
        <source>Serial Studio couldn't reach your device.</source>
        <translation>Serial Studio kunde inte nå enheten.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="104"/>
        <source>Check the cable, power, and that no other application has taken over the device. You can try reconnecting, switch to a different device, or quit.</source>
        <translation>Kontrollera kabeln, strömförsörjningen och att ingen annan applikation har tagit över enheten. Du kan försöka återansluta, byta till en annan enhet eller avsluta.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="107"/>
        <source>Make sure it's plugged in, powered on, and not already in use by another app. You can try again, pick a different device, or quit.</source>
        <translation>Kontrollera att den är inkopplad, påslagen och inte redan används av en annan app. Du kan försöka igen, välja en annan enhet eller avsluta.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="119"/>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="188"/>
        <source>Quit</source>
        <translation>Avsluta</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="129"/>
        <source>Pick Different Device</source>
        <translation>Välj Annan Enhet</translation>
    </message>
    <message>
        <source>Reconfigure</source>
        <translation type="vanished">Konfigurera Om</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="140"/>
        <source>Try Again</source>
        <translation>Försök Igen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="140"/>
        <source>Reconnect</source>
        <translation>Återanslut</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="156"/>
        <source>Pick the correct device, then press Connect.</source>
        <translation>Välj rätt enhet och tryck sedan på Anslut.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="165"/>
        <source>I/O Interface: %1</source>
        <translation>I/o-gränssnitt: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="198"/>
        <source>Connect</source>
        <translation>Anslut</translation>
    </message>
</context>
<context>
    <name>SerialStudio</name>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="334"/>
        <source>Data Grids</source>
        <translation>Datarutnät</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="337"/>
        <source>Multiple Data Plots</source>
        <translation>Flera Datadiagram</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="340"/>
        <source>Accelerometers</source>
        <translation>Accelerometrar</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="343"/>
        <source>Gyroscopes</source>
        <translation>Gyroskop</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="346"/>
        <source>GPS</source>
        <translation>GPS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="349"/>
        <source>FFT Plots</source>
        <translation>FFT-diagram</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="352"/>
        <source>LED Panels</source>
        <translation>LED-paneler</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="355"/>
        <source>Data Plots</source>
        <translation>Datadiagram</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="358"/>
        <source>Bars</source>
        <translation>Staplar</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="361"/>
        <source>Gauges</source>
        <translation>Mätare</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="364"/>
        <source>Terminal</source>
        <translation>Terminal</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="367"/>
        <source>Clock</source>
        <translation>Klocka</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="370"/>
        <source>Stopwatch</source>
        <translation>Stoppur</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="373"/>
        <source>Compasses</source>
        <translation>Kompasser</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="376"/>
        <source>Meters</source>
        <translation>Mätare</translation>
    </message>
    <message>
        <source>Thermometers</source>
        <translation type="vanished">Termometrar</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="379"/>
        <source>3D Plots</source>
        <translation>3D-diagram</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="383"/>
        <source>Image Views</source>
        <translation>Bildvisningar</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="386"/>
        <source>Output Panels</source>
        <translation>Utgångspaneler</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="389"/>
        <source>Notifications</source>
        <translation>Notifieringar</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="392"/>
        <source>Waterfalls</source>
        <translation>Vattenfallsdiagram</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="395"/>
        <source>Painter Widgets</source>
        <translation>Ritarwidgetar</translation>
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
        <translation>System</translation>
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
        <translation>Shift-jis</translation>
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
        <translation>Sessionsdetaljer</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="88"/>
        <source>Select a session to view details.</source>
        <translation>Välj en session för att visa detaljer.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="130"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="143"/>
        <source>Started:</source>
        <translation>Startad:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="156"/>
        <source>Ended:</source>
        <translation>Avslutad:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="162"/>
        <source>(in progress)</source>
        <translation>(pågår)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="169"/>
        <source>Frames:</source>
        <translation>Ramar:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="185"/>
        <source>Notes</source>
        <translation>Anteckningar</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="200"/>
        <source>Add session notes…</source>
        <translation>Lägg till sessionsanteckningar…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="201"/>
        <source>Notes are read-only for completed sessions.</source>
        <translation>Anteckningar är skrivskyddade för slutförda sessioner.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="286"/>
        <source>New tag…</source>
        <translation>Ny tagg…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="362"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Lås upp sessionsfilen för att radera sessioner</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="222"/>
        <source>Tags</source>
        <translation>Taggar</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="293"/>
        <source>Add</source>
        <translation>Lägg Till</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="330"/>
        <source>Replay</source>
        <translation>Spela Upp</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="338"/>
        <source>Export CSV</source>
        <translation>Exportera CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="345"/>
        <source>Generate Report</source>
        <translation>Generera Rapport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="356"/>
        <source>Delete</source>
        <translation>Ta Bort</translation>
    </message>
</context>
<context>
    <name>SessionList</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="19"/>
        <source>Sessions</source>
        <translation>Sessioner</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="71"/>
        <source>Search</source>
        <translation>Sök</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="91"/>
        <source>Date</source>
        <translation>Datum</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="92"/>
        <source>Frames</source>
        <translation>Ramar</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="93"/>
        <source>Tags</source>
        <translation>Taggar</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="193"/>
        <source>No sessions found.</source>
        <translation>Inga sessioner hittades.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="194"/>
        <source>No session file open.</source>
        <translation>Ingen sessionsfil öppen.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseManager</name>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1001"/>
        <source>Select logo image</source>
        <translation>Välj logotypbild</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1003"/>
        <source>Images (*.png *.jpg *.jpeg *.svg)</source>
        <translation>Bilder (*.png *.jpg *.jpeg *.svg)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="426"/>
        <source>Open Session File</source>
        <translation>Öppna Sessionsfil</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="428"/>
        <source>Session files (*.db)</source>
        <translation>Sessionsfiler (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1200"/>
        <source>Cannot open session file</source>
        <translation>Kan inte öppna sessionsfil</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="653"/>
        <source>Delete session from %1?</source>
        <translation>Ta bort session från %1?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="656"/>
        <source>Delete Session</source>
        <translation>Ta Bort Session</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1061"/>
        <source>No project data</source>
        <translation>Ingen projektdata</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="654"/>
        <source>All readings and raw data for this session are permanently removed.</source>
        <translation>Alla avläsningar och rådata för denna session tas bort permanent.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="484"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="493"/>
        <source>Lock Session File</source>
        <translation>Lås Sessionsfil</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="485"/>
        <source>Choose a password to lock the session file:</source>
        <translation>Välj ett lösenord för att låsa sessionsfilen:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="494"/>
        <source>Confirm the password:</source>
        <translation>Bekräfta lösenordet:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="502"/>
        <source>Passwords do not match</source>
        <translation>Lösenorden stämmer inte överens</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="503"/>
        <source>The two passwords you entered do not match. The session file was not locked.</source>
        <translation>De två lösenorden du angav stämmer inte överens. Sessionsfilen låstes inte.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="539"/>
        <source>Unlock Session File</source>
        <translation>Lås Upp Sessionsfil</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="540"/>
        <source>Enter the session file password:</source>
        <translation>Ange sessionsfilens lösenord:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="550"/>
        <source>Incorrect password</source>
        <translation>Felaktigt lösenord</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="551"/>
        <source>The password you entered does not match the one stored in the session file.</source>
        <translation>Lösenordet du angav stämmer inte överens med det som finns lagrat i sessionsfilen.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="643"/>
        <source>Session file locked</source>
        <translation>Sessionsfil låst</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="644"/>
        <source>Unlock the session file before deleting recorded sessions.</source>
        <translation>Lås upp sessionsfilen innan du tar bort inspelade sessioner.</translation>
    </message>
    <message>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation type="vanished">Denna session innehåller ingen inbäddad projektfil — instrumentpanelen använder en snabbdiagramlayout.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="778"/>
        <source>Export Session to CSV</source>
        <translation>Exportera Session till CSV</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="778"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-filer (*.CSV)</translation>
    </message>
    <message>
        <source>Export Complete</source>
        <translation type="vanished">Export Slutförd</translation>
    </message>
    <message>
        <source>Session exported to:
%1</source>
        <translation type="vanished">Session exporterad till:
%1</translation>
    </message>
    <message>
        <source>Preparing export…</source>
        <translation type="vanished">Förbereder export…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="973"/>
        <source>Done</source>
        <translation>Klar</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="937"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="973"/>
        <source>Failed</source>
        <translation>Misslyckades</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="943"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="983"/>
        <source>Report Failed</source>
        <translation>Rapport Misslyckades</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="945"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="984"/>
        <source>Could not generate the report. Check the output path and try again.</source>
        <translation>Kunde inte generera rapporten. Kontrollera utdatasökvägen och försök igen.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="873"/>
        <source>Save PDF Report</source>
        <translation>Spara PDF-rapport</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="853"/>
        <source>Loading session data…</source>
        <translation>Laddar sessionsdata…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="873"/>
        <source>Save HTML Report</source>
        <translation>Spara HTML-rapport</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="874"/>
        <source>PDF files (*.pdf)</source>
        <translation>PDF-filer (*.PDF)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="874"/>
        <source>HTML files (*.html)</source>
        <translation>HTML-filer (*.HTML)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1062"/>
        <source>This session file does not contain an embedded project.</source>
        <translation>Denna sessionsfil innehåller inget inbäddat projekt.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1071"/>
        <source>Invalid project data</source>
        <translation>Ogiltig projektdata</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1072"/>
        <source>The embedded project JSON is malformed and cannot be restored.</source>
        <translation>Den inbäddade projekt-JSON:en är felaktig och kan inte återställas.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1082"/>
        <source>Restore Project</source>
        <translation>Återställ Projekt</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1082"/>
        <source>Serial Studio projects (*.ssproj *.json)</source>
        <translation>Serial Studio-projekt (*.ssproj *.json)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1090"/>
        <source>Cannot write file</source>
        <translation>Kan inte skriva fil</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1090"/>
        <source>Check file permissions and try again.</source>
        <translation>Kontrollera filbehörigheter och försök igen.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseWorker</name>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="76"/>
        <source>Empty file path</source>
        <translation>Tom filsökväg</translation>
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
        <translation>Databasen är inte öppen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="262"/>
        <source>Database not open or empty label</source>
        <translation>Databasen är inte öppen eller etikett saknas</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="330"/>
        <source>Invalid label</source>
        <translation>Ogiltig etikett</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="597"/>
        <source>Cancelled</source>
        <translation>Avbruten</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="710"/>
        <source>Could not load session data</source>
        <translation>Kunde inte läsa in sessionsdata</translation>
    </message>
</context>
<context>
    <name>Sessions::HtmlReport</name>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="209"/>
        <source>Assembling report…</source>
        <translation>Sammanställer rapport…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="217"/>
        <source>Writing output…</source>
        <translation>Skriver utdata…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="282"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="342"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="700"/>
        <source>Session Report</source>
        <translation>Sessionsrapport</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="345"/>
        <source>Untitled project</source>
        <translation>Namnlöst projekt</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="352"/>
        <source>Prepared by</source>
        <translation>Förberedd av</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="355"/>
        <source>Generated on %1</source>
        <translation>Genererad den %1</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="377"/>
        <source>Test ID</source>
        <translation>Test-ID</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="379"/>
        <source>Duration</source>
        <translation>Varaktighet</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="381"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="493"/>
        <source>Samples</source>
        <translation>Sampel</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="383"/>
        <source>Parameters</source>
        <translation>Parametrar</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="385"/>
        <source>Started</source>
        <translation>Startad</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="387"/>
        <source>Ended</source>
        <translation>Avslutad</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="423"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="425"/>
        <source>Test identifier</source>
        <translation>Testidentifierare</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="426"/>
        <source>Start time</source>
        <translation>Starttid</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="427"/>
        <source>End time</source>
        <translation>Sluttid</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="428"/>
        <source>Total duration</source>
        <translation>Total varaktighet</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="429"/>
        <source>Samples acquired</source>
        <translation>Sampel insamlade</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="430"/>
        <source>Parameters logged</source>
        <translation>Parametrar loggade</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="446"/>
        <source>Classification</source>
        <translation>Klassificering</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="453"/>
        <source>Notes</source>
        <translation>Anteckningar</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="461"/>
        <source>Test Information</source>
        <translation>Testinformation</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="482"/>
        <source>Parameter</source>
        <translation>Parameter</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="485"/>
        <source>Units</source>
        <translation>Enheter</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="494"/>
        <source>Minimum</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="495"/>
        <source>Maximum</source>
        <translation>Maximum</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="496"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="652"/>
        <source>Mean</source>
        <translation>Medelvärde</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="497"/>
        <source>Std. Deviation</source>
        <translation>Standardavvikelse</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="542"/>
        <source>Measurement Summary</source>
        <translation>Mätsammanfattning</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="543"/>
        <source>click a column to sort</source>
        <translation>klicka på en kolumn för att sortera</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="568"/>
        <source>%1 samples over %2 seconds</source>
        <translation>%1 sampel över %2 sekunder</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="586"/>
        <source>Combined Parameter View</source>
        <translation>Kombinerad Parametervy</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="587"/>
        <source>click legend items to toggle signals</source>
        <translation>klicka på förklaringsobjekt för att växla signaler</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="595"/>
        <source>Parameter Trends</source>
        <translation>Parametertrender</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="650"/>
        <source>Min</source>
        <translation>Min</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="651"/>
        <source>Max</source>
        <translation>Max</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="724"/>
        <source>Page %1 of %2</source>
        <translation>Sida %1 av %2</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="794"/>
        <source>Loading rendering engine…</source>
        <translation>Laddar renderingsmotor…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="814"/>
        <source>Rendering charts…</source>
        <translation>Renderar diagram…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="858"/>
        <source>Generating PDF…</source>
        <translation>Genererar PDF…</translation>
    </message>
</context>
<context>
    <name>Sessions::Player</name>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="258"/>
        <source>Open Session File</source>
        <translation>Öppna Sessionsfil</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="260"/>
        <source>Session files (*.db)</source>
        <translation>Sessionsfiler (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="330"/>
        <source>Device Connection Active</source>
        <translation>Enhetsanslutning Aktiv</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="331"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>För att använda denna funktion måste du koppla från enheten. Vill du fortsätta?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="379"/>
        <location filename="../../src/Sessions/Player.cpp" line="458"/>
        <source>Cannot open session file</source>
        <translation>Kan inte öppna sessionsfil</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="380"/>
        <source>Unknown error</source>
        <translation>Okänt fel</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="396"/>
        <source>No project data</source>
        <translation>Ingen projektdata</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="397"/>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation>Denna session innehåller ingen inbäddad projektfil — instrumentpanelen använder en snabbdiagramlayout.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="459"/>
        <source>Check file permissions and try again.</source>
        <translation>Kontrollera filbehörigheter och försök igen.</translation>
    </message>
    <message>
        <source>No sessions found</source>
        <translation type="vanished">Inga sessioner hittades</translation>
    </message>
    <message>
        <source>This file does not contain any recording sessions.</source>
        <translation type="vanished">Denna fil innehåller inga inspelade sessioner.</translation>
    </message>
    <message>
        <source>Session has no columns</source>
        <translation type="vanished">Sessionen har inga kolumner</translation>
    </message>
    <message>
        <source>The selected session is missing its column definitions.</source>
        <translation type="vanished">Den valda sessionen saknar sina kolumndefinitioner.</translation>
    </message>
    <message>
        <source>Session has no readings</source>
        <translation type="vanished">Sessionen har inga avläsningar</translation>
    </message>
    <message>
        <source>The selected session does not contain any frames.</source>
        <translation type="vanished">Den valda sessionen innehåller inga ramar.</translation>
    </message>
</context>
<context>
    <name>Sessions::PlayerLoaderWorker</name>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="168"/>
        <source>Empty file path</source>
        <translation>Tom filsökväg</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="69"/>
        <source>This file does not contain any recording sessions.</source>
        <translation>Denna fil innehåller inga inspelade sessioner.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="144"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="205"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="224"/>
        <source>Cancelled</source>
        <translation>Avbruten</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="218"/>
        <source>The selected session is missing its column definitions.</source>
        <translation>Den valda sessionen saknar sina kolumndefinitioner.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="235"/>
        <source>The selected session does not contain any frames.</source>
        <translation>Den valda sessionen innehåller inga ramar.</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="34"/>
        <source>Preferences</source>
        <translation>Inställningar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="61"/>
        <source>General</source>
        <translation>Allmänt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="159"/>
        <source>Language</source>
        <translation>Språk</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="175"/>
        <source>Theme</source>
        <translation>Tema</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="210"/>
        <source>Workspace Folder</source>
        <translation>Arbetsytamapp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="320"/>
        <source>Enable API Server (Port 7777)</source>
        <translation>Aktivera API-server (Port 7777)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="237"/>
        <source>Automatically Check for Updates</source>
        <translation>Sök Automatiskt efter Uppdateringar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="67"/>
        <source>Dashboard</source>
        <translation>Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="386"/>
        <source>Export…</source>
        <translation>Exportera…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="435"/>
        <source>Data Plotting</source>
        <translation>Dataritning</translation>
    </message>
    <message>
        <source>Point Count</source>
        <translation type="vanished">Punktantal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="500"/>
        <source>UI Refresh Rate (Hz)</source>
        <translation>Ui-uppdateringsfrekvens (Hz)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="624"/>
        <source>Show Actions Panel</source>
        <translation>Visa Åtgärdspanel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="737"/>
        <source>Always Show Taskbar Buttons</source>
        <translation>Visa Alltid Aktivitetsfältsknappar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="79"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="849"/>
        <source>Console</source>
        <translation>Konsol</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="144"/>
        <source>Appearance</source>
        <translation>Utseende</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="194"/>
        <source>Files &amp; Updates</source>
        <translation>Filer &amp; Uppdateringar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="255"/>
        <source>Advanced</source>
        <translation>Avancerat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="338"/>
        <source>Allow External API Connections</source>
        <translation>Tillåt Externa API-anslutningar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="303"/>
        <source>Auto-Hide Toolbar</source>
        <translation>Dölj Verktygsfält Automatiskt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="73"/>
        <source>Taskbar</source>
        <translation>Aktivitetsfält</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="270"/>
        <source>Rendering Backend</source>
        <translation>Renderingsmotor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="354"/>
        <source>API Access Token</source>
        <translation>API-åtkomsttoken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="450"/>
        <source>Time Range</source>
        <translation>Tidsintervall</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Small</source>
        <translation>Liten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Normal</source>
        <translation>Normal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Large</source>
        <translation>Stor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Extra Large</source>
        <translation>Extra Stor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Custom</source>
        <translation>Anpassad</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="609"/>
        <source>Layout</source>
        <translation>Layout</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="647"/>
        <source>Video Export</source>
        <translation>Videoexport</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="665"/>
        <source>Save Videos by Default</source>
        <translation>Spara Videor som Standard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="716"/>
        <source>Behavior</source>
        <translation>Beteende</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="752"/>
        <source>Show Search Field</source>
        <translation>Visa Sökfält</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="767"/>
        <source>Auto-hide Taskbar</source>
        <translation>Dölj Aktivitetsfält Automatiskt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="785"/>
        <source>Hide Delay (ms)</source>
        <translation>Dölj Fördröjning (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="809"/>
        <source>Pinned Buttons</source>
        <translation>Fästa Knappar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="827"/>
        <source>Drag a pinned button on the taskbar to reorder it.</source>
        <translation>Dra en fäst knapp i aktivitetsfältet för att ändra ordning.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="848"/>
        <source>Settings</source>
        <translation>Inställningar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="851"/>
        <source>Clock</source>
        <translation>Klocka</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="852"/>
        <source>Stopwatch</source>
        <translation>Stoppur</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="853"/>
        <source>Pause / Resume</source>
        <translation>Pausa / Återuppta</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="854"/>
        <source>File Transmission</source>
        <translation>Filöverföring</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="855"/>
        <source>AI Assistant</source>
        <translation>Ai-assistent</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="984"/>
        <source>Display</source>
        <translation>Display</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="999"/>
        <source>Display Mode</source>
        <translation>Displayläge</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="537"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1012"/>
        <source>Font Family</source>
        <translation>Teckensnittsfamilj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="86"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="850"/>
        <source>Notifications</source>
        <translation>Notifikationer</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="384"/>
        <source>Export Protobuf File</source>
        <translation>Exportera Protobuf-fil</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="522"/>
        <source>Dashboard Font</source>
        <translation>Typsnitt för Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="552"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1027"/>
        <source>Font Size</source>
        <translation>Teckenstorlek</translation>
    </message>
    <message>
        <source>Image Export</source>
        <translation type="vanished">Bildexport</translation>
    </message>
    <message>
        <source>Save Images by Default</source>
        <translation type="vanished">Spara Bilder som Standard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1044"/>
        <source>Show Timestamps</source>
        <translation>Visa Tidsstämplar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1063"/>
        <source>Data Transmission</source>
        <translation>Dataöverföring</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1078"/>
        <source>Line Ending</source>
        <translation>Radslut</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1091"/>
        <source>Input Mode</source>
        <translation>Ingångsläge</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1104"/>
        <source>Text Encoding</source>
        <translation>Textkodning</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1117"/>
        <source>Checksum</source>
        <translation>Kontrollsumma</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1130"/>
        <source>Echo Sent Data</source>
        <translation>Eka Skickad Data</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1149"/>
        <source>Escape Codes</source>
        <translation>Escape-koder</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1164"/>
        <source>VT100 Emulation</source>
        <translation>Vt100-emulering</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1183"/>
        <source>ANSI Colors</source>
        <translation>Ansi-färger</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1241"/>
        <source>Delivery</source>
        <translation>Leverans</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1256"/>
        <source>System Notifications</source>
        <translation>Systemnotifikationer</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1277"/>
        <source>Show Warning/Critical events as OS desktop notifications when Serial Studio is not the foreground window.</source>
        <translation>Visa varnings-/kritiska händelser som skrivbordsnotifikationer när Serial Studio inte är det aktiva fönstret.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1287"/>
        <source>Application Logs</source>
        <translation>Applikationsloggar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1302"/>
        <source>Route Warnings to Notifications</source>
        <translation>Dirigera Varningar till Notifikationer</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1323"/>
        <source>Off by default — Qt and QML emit warnings frequently and enabling this can drown out real alarms. Critical messages are always routed regardless of this setting.</source>
        <translation>Av som standard — QT och QML genererar varningar frekvent och att aktivera detta kan överrösta verkliga larm. Kritiska meddelanden dirigeras alltid oavsett denna inställning.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1342"/>
        <source>Reset</source>
        <translation>Återställ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1377"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1385"/>
        <source>Apply</source>
        <translation>Tillämpa</translation>
    </message>
</context>
<context>
    <name>Setup</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="35"/>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="380"/>
        <source>Device Setup</source>
        <translation>Enhetskonfiguration</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="126"/>
        <source>API Server Active (%1)</source>
        <translation>API-server Aktiv (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="127"/>
        <source>API Server Ready</source>
        <translation>API-server Redo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="128"/>
        <source>API Server Off</source>
        <translation>API-server Av</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="188"/>
        <source>Frame Parsing</source>
        <translation>Frame-parsning</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="198"/>
        <source>Console Only (No Parsing)</source>
        <translation>Endast Konsol (Ingen Parsning)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="211"/>
        <source>Quick Plot (Comma Separated Values)</source>
        <translation>Snabbdiagram (Kommaseparerade Värden)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="222"/>
        <source>Parse via Project File</source>
        <translation>Tolka via Projektfil</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="245"/>
        <source>Change Project File (%1)</source>
        <translation>Ändra Projektfil (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="246"/>
        <source>Select Project File</source>
        <translation>Välj Projektfil</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="261"/>
        <source>Data Export</source>
        <translation>Dataexport</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="285"/>
        <source>CSV Spreadsheet</source>
        <translation>CSV-kalkylblad</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="303"/>
        <source>Session Recording</source>
        <translation>Sessionsinspelning</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="324"/>
        <source>MDF4 Recording</source>
        <translation>MDF4-inspelning</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="340"/>
        <source>Console Log</source>
        <translation>Konsolllogg</translation>
    </message>
    <message>
        <source>CSV File</source>
        <translation type="vanished">CSV-fil</translation>
    </message>
    <message>
        <source>Session Database</source>
        <translation type="vanished">Sessionsdatabas</translation>
    </message>
    <message>
        <source>MDF4 File</source>
        <translation type="vanished">MDF4-fil</translation>
    </message>
    <message>
        <source>Console Dump</source>
        <translation type="vanished">Konsolutskrift</translation>
    </message>
    <message>
        <source>Create CSV File</source>
        <translation type="vanished">Skapa CSV-fil</translation>
    </message>
    <message>
        <source>Create MDF4 File</source>
        <translation type="vanished">Skapa MDF4-fil</translation>
    </message>
    <message>
        <source>Create Session Log</source>
        <translation type="vanished">Skapa Sessionslogg</translation>
    </message>
    <message>
        <source>Export Console Data</source>
        <translation type="vanished">Exportera Konsoldata</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="392"/>
        <source>I/O Interface: %1</source>
        <translation>I/o-gränssnitt: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="461"/>
        <source>Multi-Device Project</source>
        <translation>Flerenhets-projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="474"/>
        <source>This project streams data from %1 independent devices.</source>
        <translation>Detta projekt strömmar data från %1 oberoende enheter.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="487"/>
        <source>Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.</source>
        <translation>Varje enhet har sina egna anslutningsinställningar. Konfigurera dem i Projektredigeraren under fliken Källor.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="506"/>
        <source>Open Project Editor</source>
        <translation>Öppna Projektredigeraren</translation>
    </message>
</context>
<context>
    <name>ShortcutGenerator</name>
    <message>
        <source>New Shortcut</source>
        <translation type="vanished">Ny Genväg</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="93"/>
        <source>Choose an Icon</source>
        <translation>Välj en Ikon</translation>
    </message>
    <message>
        <source>Save Shortcut</source>
        <translation type="vanished">Spara Genväg</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="22"/>
        <source>New Deployment</source>
        <translation>Ny Distribution</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="106"/>
        <source>Save Deployment</source>
        <translation>Spara Distribution</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="146"/>
        <source>General</source>
        <translation>Allmänt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="152"/>
        <source>Taskbar</source>
        <translation>Aktivitetsfält</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="158"/>
        <source>Logging</source>
        <translation>Loggning</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="215"/>
        <source>Identity</source>
        <translation>Identitet</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="271"/>
        <source>Click to choose an icon</source>
        <translation>Klicka för att välja en ikon</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="280"/>
        <source>Name:</source>
        <translation>Namn:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="289"/>
        <source>Deployment Name</source>
        <translation>Distributionsnamn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="380"/>
        <source>Theme</source>
        <translation>Tema</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="390"/>
        <source>Same as Serial Studio</source>
        <translation>Samma som Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="412"/>
        <source>Actions Panel</source>
        <translation>Åtgärdspanel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="423"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="632"/>
        <source>File Transmission</source>
        <translation>Filöverföring</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="439"/>
        <source>Double-clicking this deployment takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation>Dubbelklickning på denna distribution tar användaren direkt till den aktiva instrumentpanelen för detta projekt. Det finns ingen verktygsfält eller inställningspanel, bara data, och Serial Studio avslutas så snart enheten kopplas från.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="485"/>
        <source>Visibility</source>
        <translation>Synlighet</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="500"/>
        <source>Mode</source>
        <translation>Läge</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="509"/>
        <source>Always shown</source>
        <translation>Alltid synlig</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="510"/>
        <source>Auto-hide</source>
        <translation>Dölj Automatiskt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="511"/>
        <source>Hidden</source>
        <translation>Dold</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="526"/>
        <source>Hiding the taskbar removes window minimize/maximize/close buttons and forces auto-layout, so the dashboard always fills the available area.</source>
        <translation>Att dölja aktivitetsfältet tar bort fönstrets minimera/maximera/stäng-knappar och tvingar automatisk layout, så instrumentpanelen fyller alltid det tillgängliga området.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="530"/>
        <source>The taskbar slides in when the user moves the cursor near the bottom edge.</source>
        <translation>Aktivitetsfältet glider in när användaren flyttar markören nära den nedre kanten.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="532"/>
        <source>The taskbar is permanently visible at the bottom of the dashboard.</source>
        <translation>Aktivitetsfältet är permanent synligt längst ner på instrumentpanelen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="545"/>
        <source>Pinned Buttons</source>
        <translation>Fästa Knappar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="562"/>
        <source>Console</source>
        <translation>Konsol</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="576"/>
        <source>Notifications</source>
        <translation>Notifikationer</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="590"/>
        <source>Clock</source>
        <translation>Klocka</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="604"/>
        <source>Stopwatch</source>
        <translation>Stoppur</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="618"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="743"/>
        <source>Recordings are saved in the Serial Studio workspace folder</source>
        <translation>Inspelningar sparas i Serial Studios arbetsytamapp</translation>
    </message>
    <message>
        <source>Shortcut Name</source>
        <translation type="vanished">Genvägsnamn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="298"/>
        <source>Change Icon…</source>
        <translation>Ändra Ikon…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="315"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="333"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="343"/>
        <source>Choose a project file to begin</source>
        <translation>Välj en projektfil för att börja</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="365"/>
        <source>Behavior</source>
        <translation>Beteende</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="402"/>
        <source>Fullscreen</source>
        <translation>Helskärm</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation type="vanished">Dubbelklickning på denna genväg tar användaren direkt till den aktiva instrumentpanelen för detta projekt. Det finns ingen verktygsfält eller inställningspanel, bara data, och Serial Studio avslutas så snart enheten kopplas från.</translation>
    </message>
    <message>
        <source>Embed Project</source>
        <translation type="vanished">Bädda In Projekt</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.

Turn on Embed Project to bake the project into the shortcut, so it keeps working even if the original file is moved or deleted.</source>
        <translation type="vanished">Dubbelklickning på denna genväg tar användaren direkt till den aktiva instrumentpanelen för detta projekt. Det finns ingen verktygsfält eller inställningspanel, bara data, och Serial Studio avslutas så snart enheten kopplas från.

Aktivera Bädda in Projekt för att baka in projektet i genvägen, så att den fortsätter fungera även om originalfilen flyttas eller raderas.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="682"/>
        <source>Recorders</source>
        <translation>Inspelare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="697"/>
        <source>CSV File</source>
        <translation>CSV-fil</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="707"/>
        <source>MDF4 File</source>
        <translation>MDF4-fil</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="717"/>
        <source>Session Database</source>
        <translation>Sessionsdatabas</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="727"/>
        <source>Console Log</source>
        <translation>Konsolllogg</translation>
    </message>
    <message>
        <source>Recordings are saved to each module’s default location.</source>
        <translation type="vanished">Inspelningar sparas på varje moduls standardplats.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="772"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="781"/>
        <source>Save</source>
        <translation>Spara</translation>
    </message>
</context>
<context>
    <name>SourceFrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="116"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="353"/>
        <source>Undo</source>
        <translation>Ångra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="123"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="364"/>
        <source>Redo</source>
        <translation>Gör Om</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="132"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="383"/>
        <source>Cut</source>
        <translation>Klipp Ut</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="137"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="393"/>
        <source>Copy</source>
        <translation>Kopiera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="142"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="403"/>
        <source>Paste</source>
        <translation>Klistra In</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="149"/>
        <source>Select All</source>
        <translation>Markera Allt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="159"/>
        <source>Format Document</source>
        <translation>Formatera Dokument</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="166"/>
        <source>Format Selection</source>
        <translation>Formatera Markering</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="333"/>
        <source>Reset</source>
        <translation>Återställ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="418"/>
        <source>Validate</source>
        <translation>Validera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="422"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Verifiera att skriptet kompilerar korrekt</translation>
    </message>
    <message>
        <source>Reset template parameters to their defaults</source>
        <translation type="vanished">Återställ mallparametrar till standardvärden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="338"/>
        <source>Reset to the default parsing script</source>
        <translation>Återställ till standardtolkningsskriptet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="343"/>
        <source>Open</source>
        <translation>Öppna</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="348"/>
        <source>Import a script file for data parsing</source>
        <translation>Importera en skriptfil för datatolkning</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Öppna hjälpdokumentation för datatolkning</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Språk:</translation>
    </message>
    <message>
        <source>Native</source>
        <translation type="vanished">Inbyggd</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="258"/>
        <source>Select Template…</source>
        <translation>Välj Mall…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="358"/>
        <source>Undo the last code edit</source>
        <translation>Ångra senaste kodredigeringen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="370"/>
        <source>Redo the previously undone edit</source>
        <translation>Gör om tidigare ångrad redigering</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="388"/>
        <source>Cut selected code to clipboard</source>
        <translation>Klipp ut markerad kod till urklipp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="398"/>
        <source>Copy selected code to clipboard</source>
        <translation>Kopiera markerad kod till urklipp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="407"/>
        <source>Paste code from clipboard</source>
        <translation>Klistra in kod från urklipp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="283"/>
        <source>Help</source>
        <translation>Hjälp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="232"/>
        <source>Platform:</source>
        <translation>Plattform:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="240"/>
        <source>Built-In</source>
        <translation>Inbyggd</translation>
    </message>
    <message>
        <source>Template:</source>
        <translation type="vanished">Mall:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="274"/>
        <source>Test With Sample Data</source>
        <translation>Testa med Exempeldata</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Utvärdera</translation>
    </message>
</context>
<context>
    <name>SourceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="107"/>
        <source>Duplicate</source>
        <translation>Duplicera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="109"/>
        <source>Create a copy of this data source</source>
        <translation>Skapa en kopia av denna datakälla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="121"/>
        <source>Delete</source>
        <translation>Ta Bort</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="126"/>
        <source>Remove this data source</source>
        <translation>Ta bort denna datakälla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="127"/>
        <source>The primary data source cannot be removed</source>
        <translation>Den primära datakällan kan inte tas bort</translation>
    </message>
</context>
<context>
    <name>SqlitePlayer</name>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="25"/>
        <source>Session Player</source>
        <translation>Sessionsspelare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="92"/>
        <source>Loading session…</source>
        <translation>Laddar session…</translation>
    </message>
</context>
<context>
    <name>StartMenu</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="276"/>
        <source>Workspaces</source>
        <translation>Arbetsytor</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="372"/>
        <source>No Workspaces Available</source>
        <translation>Inga Arbetsytor Tillgängliga</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="406"/>
        <source>Actions</source>
        <translation>Åtgärder</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="429"/>
        <source>No Actions Available</source>
        <translation>Inga Åtgärder Tillgängliga</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="459"/>
        <source>Plugins</source>
        <translation>Tillägg</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="357"/>
        <source>New Workspace…</source>
        <translation>Ny Arbetsyta…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="497"/>
        <source>Manage Plugins…</source>
        <translation>Hantera Tillägg…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="507"/>
        <source>No Plugins Installed</source>
        <translation>Inga Tillägg Installerade</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="99"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="543"/>
        <source>Auto Layout</source>
        <translation>Automatisk Layout</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="107"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="555"/>
        <source>Full Screen</source>
        <translation>Helskärm</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="113"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="568"/>
        <source>Add External Window</source>
        <translation>Lägg till Externt Fönster</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="757"/>
        <source>Clock</source>
        <translation>Klocka</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="141"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="764"/>
        <source>Stopwatch</source>
        <translation>Stoppur</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="781"/>
        <source>Sessions</source>
        <translation>Sessioner</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="168"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="790"/>
        <source>File Transmission</source>
        <translation>Filöverföring</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="175"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="798"/>
        <source>AI Assistant</source>
        <translation>Ai-assistent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="343"/>
        <source>Show "%1"</source>
        <translation>Visa "%1"</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="348"/>
        <source>Show All Hidden Workspaces</source>
        <translation>Visa Alla Dolda Arbetsytor</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="588"/>
        <source>Export</source>
        <translation>Exportera</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="619"/>
        <source>CSV File</source>
        <translation>CSV-fil</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="625"/>
        <source>MDF4 File</source>
        <translation>MDF4-fil</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="631"/>
        <source>Console Transcript</source>
        <translation>Konsolutskrift</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="640"/>
        <source>Session Database</source>
        <translation>Sessionsdatabas</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="654"/>
        <source>No Export Formats Available</source>
        <translation>Inga Exportformat Tillgängliga</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="684"/>
        <source>Tools</source>
        <translation>Verktyg</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="813"/>
        <source>No Tools Available</source>
        <translation>Inga Verktyg Tillgängliga</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="881"/>
        <source>Reset</source>
        <translation>Återställ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="902"/>
        <source>Quit</source>
        <translation>Avsluta</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="928"/>
        <source>Edit…</source>
        <translation>Redigera…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="119"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="740"/>
        <source>Console</source>
        <translation>Konsol</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="125"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="749"/>
        <source>Notifications</source>
        <translation>Aviseringar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="149"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="772"/>
        <source>Preferences</source>
        <translation>Inställningar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="155"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="853"/>
        <source>Help Center</source>
        <translation>Hjälpcenter</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="939"/>
        <source>Delete</source>
        <translation>Ta Bort</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="940"/>
        <source>Hide</source>
        <translation>Dölj</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="874"/>
        <source>Resume</source>
        <translation>Återuppta</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="875"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="902"/>
        <source>Disconnect</source>
        <translation>Koppla Från</translation>
    </message>
</context>
<context>
    <name>Stopwatch</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Stop</source>
        <translation>Stoppa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Start</source>
        <translation>Start</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="210"/>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="288"/>
        <source>Lap</source>
        <translation>Varv</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="226"/>
        <source>Reset</source>
        <translation>Återställ</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="279"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="297"/>
        <source>Total</source>
        <translation>Totalt</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="393"/>
        <source>No laps recorded</source>
        <translation>Inga varv registrerade</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="401"/>
        <source>Press Lap while the stopwatch is running</source>
        <translation>Tryck på Varv medan stoppur är igång</translation>
    </message>
</context>
<context>
    <name>SubMenuCombo</name>
    <message>
        <location filename="../../qml/Widgets/SubMenuCombo.qml" line="86"/>
        <source>No Data Available</source>
        <translation>Ingen Data Tillgänglig</translation>
    </message>
</context>
<context>
    <name>SystemDatasetsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="33"/>
        <source>Dataset Values</source>
        <translation>Datamängdsvärden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="158"/>
        <source>Search</source>
        <translation>Sök</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="179"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="180"/>
        <source>Group</source>
        <translation>Grupp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="181"/>
        <source>Dataset</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="182"/>
        <source>Units</source>
        <translation>Enheter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="252"/>
        <source>(virtual)</source>
        <translation>(virtuell)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="298"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Kopiera åtkomstkod %1 till urklipp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="374"/>
        <source>Dataset access code copied</source>
        <translation>Datasetåtkomstkod kopierad</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="323"/>
        <source>No datasets defined in this project.</source>
        <translation>Inga dataset definierade i detta projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="324"/>
        <source>No datasets match your search.</source>
        <translation>Inga dataset matchar din sökning.</translation>
    </message>
</context>
<context>
    <name>TableDelegate</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="130"/>
        <source>Parameter</source>
        <translation>Parameter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="151"/>
        <source>Value</source>
        <translation>Värde</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="514"/>
        <source>(Custom Icon)</source>
        <translation>(Anpassad Ikon)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="639"/>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="645"/>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="811"/>
        <source>No</source>
        <translation>Nej</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="811"/>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
</context>
<context>
    <name>Taskbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="66"/>
        <source>Start Menu</source>
        <translation>Startmeny</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="194"/>
        <source>Menu</source>
        <translation>Meny</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="230"/>
        <source>Search…</source>
        <translation>Sök…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="496"/>
        <source>Settings</source>
        <translation>Inställningar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="497"/>
        <source>Console</source>
        <translation>Konsol</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="498"/>
        <source>Notifications</source>
        <translation>Notifikationer</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="499"/>
        <source>Clock</source>
        <translation>Klocka</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="500"/>
        <source>Stopwatch</source>
        <translation>Stoppur</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="502"/>
        <source>AI Assistant</source>
        <translation>Ai-assistent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Resume</source>
        <translation>Återuppta</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="949"/>
        <source>MQTT: Connected to %1</source>
        <translation>MQTT: Ansluten till %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="950"/>
        <source>MQTT: Not connected</source>
        <translation>MQTT: Ej ansluten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="974"/>
        <source>MQTT Publisher</source>
        <translation>MQTT-utgivare</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="984"/>
        <source>Status:</source>
        <translation>Status:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="992"/>
        <source>Connected</source>
        <translation>Ansluten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="993"/>
        <source>Disconnected</source>
        <translation>Frånkopplad</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1000"/>
        <source>Broker:</source>
        <translation>Broker:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1013"/>
        <source>Mode:</source>
        <translation>Läge:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1026"/>
        <source>Messages sent:</source>
        <translation>Meddelanden skickade:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1040"/>
        <source>Open MQTT Settings</source>
        <translation>Öppna MQTT-inställningar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="501"/>
        <source>File Transmission</source>
        <translation>Filöverföring</translation>
    </message>
    <message>
        <source>Search widgets…</source>
        <translation type="vanished">Sök widgetar…</translation>
    </message>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Ta Bort från Arbetsyta</translation>
    </message>
</context>
<context>
    <name>Terminal</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="141"/>
        <source>Copy</source>
        <translation>Kopiera</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="149"/>
        <source>Select all</source>
        <translation>Markera Allt</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="155"/>
        <source>Clear</source>
        <translation>Rensa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="253"/>
        <source>Send Data to Device</source>
        <translation>Skicka Data till Enhet</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="428"/>
        <source>Show Timestamp</source>
        <translation>Visa Tidsstämpel</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="436"/>
        <source>Echo</source>
        <translation>Eko</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="453"/>
        <source>Emulate VT-100</source>
        <translation>Emulera VT-100</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="466"/>
        <source>ANSI Colors</source>
        <translation>Ansi-färger</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="486"/>
        <source>Display: %1</source>
        <translation>Visning: %1</translation>
    </message>
</context>
<context>
    <name>Tips</name>
    <message>
        <source>Did You Know?</source>
        <translation type="vanished">Visste Du?</translation>
    </message>
    <message>
        <source>Keep your firmware simple by sending raw data and letting Serial Studio parse it in JavaScript, Lua, or code-free Built-In templates.</source>
        <translation type="vanished">Håll din firmware enkel genom att skicka rådata och låta Serial Studio tolka den i JavaScript, Lua eller kodfria inbyggda mallar.</translation>
    </message>
    <message>
        <source>Give each channel its own function to calibrate, filter, or convert units. Offload the math to Serial Studio and keep your firmware lean.</source>
        <translation type="vanished">Ge varje kanal sin egen funktion för att kalibrera, filtrera eller konvertera enheter. Flytta matematiken till Serial Studio och håll din firmware slimmad.</translation>
    </message>
    <message>
        <source>Need a value your device never sends? A virtual dataset computes its own channel, like power from voltage and current, plotted and logged as data.</source>
        <translation type="vanished">Behöver du ett värde som din enhet aldrig skickar? En virtuell dataset beräknar sin egen kanal, som effekt från spänning och ström, plottad och loggad som data.</translation>
    </message>
    <message>
        <source>Catch glitches like a bench scope. Time-axis plots have a sweep and trigger mode, and you can drag the trigger level right on the plot.</source>
        <translation type="vanished">Fånga störningar som ett oscilloskop. Tidsaxelplottar har ett svep- och triggerläge, och du kan dra triggernivån direkt på plotten.</translation>
    </message>
    <message>
        <source>Stop scrolling to find the right widget. Group them into your own workspaces and jump between them from the taskbar search.</source>
        <translation type="vanished">Sluta scrolla för att hitta rätt widget. Gruppera dem i dina egna arbetsytor och hoppa mellan dem från aktivitetsfältets sökfunktion.</translation>
    </message>
    <message>
        <source>Never lose a test run again. Record sessions to a local database, then browse, tag, and replay them whenever you need them.</source>
        <translation type="vanished">Förlora aldrig en testkörning igen. Spela in sessioner till en lokal databas, bläddra sedan, tagga och spela upp dem när du behöver dem.</translation>
    </message>
    <message>
        <source>Hand a polished report to your team in seconds. Export any session to HTML or PDF, complete with charts and min/max/mean stats.</source>
        <translation type="vanished">Leverera en polerad rapport till ditt team på sekunder. Exportera valfri session till HTML eller PDF, komplett med diagram och min/max/medelvärdesstatistik.</translation>
    </message>
    <message>
        <source>Close the loop without extra tooling. Output Controls let you send commands back to your device straight from the dashboard.</source>
        <translation type="vanished">Stäng loopen utan extra verktyg. Output Controls låter dig skicka kommandon tillbaka till din enhet direkt från dashboarden.</translation>
    </message>
    <message>
        <source>Build a visualization nobody else has. The Painter widget runs your own script to draw fully custom graphics from incoming data.</source>
        <translation type="vanished">Bygg en visualisering ingen annan har. Painter-widgeten kör ditt eget skript för att rita helt anpassad grafik från inkommande data.</translation>
    </message>
    <message>
        <source>One tool for every link. Serial Studio reads from UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT, and Process I/O.</source>
        <translation type="vanished">Ett verktyg för varje länk. Serial Studio läser från UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, ljud, USB, HID, MQTT och Process I/O.</translation>
    </message>
    <message>
        <source>Skip the terminal dance. Send and receive files over your serial link with the built-in XMODEM, YMODEM, and ZMODEM protocols.</source>
        <translation type="vanished">Hoppa över terminaldansen. Skicka och ta emot filer över din seriella länk med de inbyggda XMODEM-, YMODEM- och ZMODEM-protokollen.</translation>
    </message>
    <message>
        <source>Already have a Modbus register map or a DBC file? Generate a ready-to-use project from it automatically instead of building one by hand.</source>
        <translation type="vanished">Har du redan en Modbus-registerkarta eller en DBC-fil? Generera automatiskt ett färdigt projekt från den istället för att bygga ett för hand.</translation>
    </message>
    <message>
        <source>Describe what you want and let the AI Assistant build it. It can create and edit projects for you across eight model providers.</source>
        <translation type="vanished">Beskriv vad du vill och låt AI-assistenten bygga det. Den kan skapa och redigera projekt åt dig över åtta modellleverantörer.</translation>
    </message>
    <message>
        <source>Tip %1 of %2</source>
        <translation type="vanished">Tips %1 av %2</translation>
    </message>
    <message>
        <source>Learn More</source>
        <translation type="vanished">Läs Mer</translation>
    </message>
    <message>
        <source>Show Tips on Startup</source>
        <translation type="vanished">Visa tips vid start</translation>
    </message>
    <message>
        <source>Previous</source>
        <translation type="vanished">Föregående</translation>
    </message>
    <message>
        <source>Next</source>
        <translation type="vanished">Nästa</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Stäng</translation>
    </message>
</context>
<context>
    <name>ToolCallCard</name>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="47"/>
        <source>Awaiting approval</source>
        <translation>Väntar på godkännande</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="48"/>
        <source>Done</source>
        <translation>Klar</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="49"/>
        <source>Error</source>
        <translation>Fel</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="50"/>
        <source>Denied</source>
        <translation>Nekad</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="51"/>
        <source>Blocked</source>
        <translation>Blockerad</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="52"/>
        <source>Running</source>
        <translation>Körs</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="152"/>
        <source>Approve</source>
        <translation>Godkänn</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="158"/>
        <source>Deny</source>
        <translation>Neka</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="175"/>
        <source>Arguments</source>
        <translation>Argument</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="212"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="272"/>
        <source>Copy</source>
        <translation>Kopiera</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="217"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="277"/>
        <source>Copy All</source>
        <translation>Kopiera Allt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="225"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="285"/>
        <source>Select All</source>
        <translation>Markera Allt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="233"/>
        <source>Result</source>
        <translation>Resultat</translation>
    </message>
</context>
<context>
    <name>Toolbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="197"/>
        <source>Project Editor</source>
        <translation>Projektredigerare</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="200"/>
        <source>Open the Project Editor to create or modify your JSON layout</source>
        <translation>Öppna Projektredigeraren för att skapa eller ändra din JSON-layout</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="214"/>
        <source>Open Project</source>
        <translation>Öppna Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="217"/>
        <source>Open an existing JSON project</source>
        <translation>Öppna ett befintligt JSON-projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="226"/>
        <source>Open CSV</source>
        <translation>Öppna CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="232"/>
        <source>Play a CSV file as if it were live sensor data</source>
        <translation>Spela upp en CSV-fil som om den vore live sensordata</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="238"/>
        <source>Open MDF4</source>
        <translation>Öppna MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="243"/>
        <source>Play an MDF4 file as if it were live sensor data (Pro)</source>
        <translation>Spela upp en MDF4-fil som om den vore live sensordata (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="292"/>
        <source>Sessions</source>
        <translation>Sessioner</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="296"/>
        <source>Browse, replay, and export recorded sessions</source>
        <translation>Bläddra, spela upp och exportera inspelade sessioner</translation>
    </message>
    <message>
        <source>Shortcuts</source>
        <translation type="vanished">Genvägar</translation>
    </message>
    <message>
        <source>Create an operator shortcut for the current project</source>
        <translation type="vanished">Skapa en operatörsgenväg för det aktuella projektet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="302"/>
        <source>Extensions</source>
        <translation>Tillägg</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="265"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="305"/>
        <source>Browse and install extensions</source>
        <translation>Bläddra och installera tillägg</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <source>Assistant</source>
        <translation>Assistent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="264"/>
        <source>Chat with an AI to build and edit your project</source>
        <translation>Chatta med en AI för att bygga och redigera ditt projekt</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <source>Configure MQTT connection (publish or subscribe)</source>
        <translation type="vanished">Konfigurera MQTT-anslutning (publicera eller prenumerera)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="286"/>
        <source>Build an operator app for the current project</source>
        <translation>Bygg en operatörsapp för det aktuella projektet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="319"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="323"/>
        <source>Preferences</source>
        <translation>Inställningar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="327"/>
        <source>Open application settings and preferences</source>
        <translation>Öppna programinställningar och preferenser</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="345"/>
        <source>UART</source>
        <translation>UART</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="351"/>
        <source>Select Serial port (UART) communication</source>
        <translation>Välj seriell port (UART)-kommunikation</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="362"/>
        <source>Audio</source>
        <translation>Ljud</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="366"/>
        <source>Select audio input device (Pro)</source>
        <translation>Välj ljudingångsenhet (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="381"/>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="386"/>
        <source>Select raw USB communication (Pro)</source>
        <translation>Välj rå USB-kommunikation (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="395"/>
        <source>Network</source>
        <translation>Nätverk</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="400"/>
        <source>Select TCP/UDP network communication</source>
        <translation>Välj TCP/UDP-nätverkskommunikation</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="412"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="525"/>
        <source>Browse example projects</source>
        <translation>Bläddra bland exempelprojekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="533"/>
        <source>Help Center</source>
        <translation>Hjälpcenter</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="537"/>
        <source>Browse documentation, FAQ, and wiki</source>
        <translation>Bläddra i dokumentation, FAQ och wiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="417"/>
        <source>Select MODBUS communication (Pro)</source>
        <translation>Välj Modbus-kommunikation (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="281"/>
        <source>Deploy</source>
        <translation>Distribuera</translation>
    </message>
    <message>
        <source>Build an operator deployment for the current project</source>
        <translation type="vanished">Skapa en operatörsdistribution för det aktuella projektet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="431"/>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="436"/>
        <source>Select HID device communication (Pro)</source>
        <translation>Välj HID-enhetskommunikation (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="446"/>
        <source>Bluetooth</source>
        <translation>Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="450"/>
        <source>Select Bluetooth Low Energy communication</source>
        <translation>Välj Bluetooth Low Energy-kommunikation</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="462"/>
        <source>CAN Bus</source>
        <translation>CAN-buss</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="467"/>
        <source>Select CAN Bus communication (Pro)</source>
        <translation>Välj CAN-busskommunikation (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="481"/>
        <source>Process</source>
        <translation>Process</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="486"/>
        <source>Select process pipe communication (Pro)</source>
        <translation>Välj processpipe-kommunikation (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="502"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="506"/>
        <source>About</source>
        <translation>Om</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="510"/>
        <source>Show application info and license details</source>
        <translation>Visa applikationsinformation och licensdetaljer</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="522"/>
        <source>Examples</source>
        <translation>Exempel</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="543"/>
        <source>AI Wiki &amp; Chat</source>
        <translation>AI Wiki &amp; Chatt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="546"/>
        <source>View detailed documentation and ask questions on DeepWiki</source>
        <translation>Visa detaljerad dokumentation och ställ frågor på DeepWiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="586"/>
        <source>Activate</source>
        <translation>Aktivera</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="590"/>
        <source>Manage license and activate Serial Studio Pro</source>
        <translation>Hantera licens och aktivera Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="627"/>
        <source>Disconnect</source>
        <translation>Koppla Från</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <source>Connect</source>
        <translation>Anslut</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="607"/>
        <source>Connect or disconnect from the configured device</source>
        <translation>Anslut eller koppla från den konfigurerade enheten</translation>
    </message>
    <message>
        <source>Connect or disconnect from device or MQTT broker</source>
        <translation type="vanished">Anslut eller koppla från enhet eller MQTT-broker</translation>
    </message>
</context>
<context>
    <name>TriggerDialog</name>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="50"/>
        <source>Trigger Settings</source>
        <translation>Triggerinställningar</translation>
    </message>
    <message>
        <source>Hold the waveform stationary by aligning each sweep to a trigger event.</source>
        <translation type="vanished">Håll vågformen stilla genom att justera varje svep till en triggerhändelse.</translation>
    </message>
    <message>
        <source>Lock a repeating signal in place, like the Auto button on an oscilloscope. Each sweep starts at the same point on the waveform, so it holds still instead of scrolling past.</source>
        <translation type="vanished">Lås en upprepande signal på plats, som Auto-knappen på ett oscilloskop. Varje svep startar vid samma punkt på vågformen, så den står stilla istället för att rulla förbi.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="173"/>
        <source>Trigger</source>
        <translation>Trigger</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="vanished">Läge:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="110"/>
        <source>Mode</source>
        <translation>Läge</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Normal</source>
        <translation>Normal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Single</source>
        <translation>Enkel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="158"/>
        <source>Auto free-runs when nothing crosses the level.</source>
        <translation>Auto kör fritt när ingenting korsar nivån.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="159"/>
        <source>Normal waits for a crossing.</source>
        <translation>Normal väntar på en korsning.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="160"/>
        <source>Single captures one sweep, then stops.</source>
        <translation>Enkel fångar ett svep och stannar sedan.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="238"/>
        <source>Slope:</source>
        <translation>Lutning:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="270"/>
        <source>Trigger on a downward crossing</source>
        <translation>Trigga vid en nedåtgående korsning</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="316"/>
        <source>Timebase:</source>
        <translation>Tidsbas:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="381"/>
        <source>Leave timebase empty to use the plot's time range; lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each.</source>
        <translation>Lämna tidsbas tom för att använda plottens tidsintervall; sänk den för att zooma in på en snabb signal. Holdoff ignorerar nya triggers ett ögonblick efter varje trigger.</translation>
    </message>
    <message>
        <source>Signal:</source>
        <translation type="vanished">Signal:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="230"/>
        <source>Value to cross</source>
        <translation>Värde att korsa</translation>
    </message>
    <message>
        <source>Edge:</source>
        <translation type="vanished">Flank:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="251"/>
        <source>Rising</source>
        <translation>Stigande</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="255"/>
        <source>Trigger on an upward crossing</source>
        <translation>Trigga vid en uppåtgående korsning</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="266"/>
        <source>Falling</source>
        <translation>Fallande</translation>
    </message>
    <message>
        <source>A new sweep begins each time the signal crosses the level in the chosen direction. Auto also free-runs when no crossing is found.</source>
        <translation type="vanished">Ett nytt svep börjar varje gång signalen korsar nivån i den valda riktningen. Auto kör även fritt när ingen korsning hittas.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="289"/>
        <source>Timing</source>
        <translation>Timing</translation>
    </message>
    <message>
        <source>Timebase (ms):</source>
        <translation type="vanished">Tidsbas (ms):</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="329"/>
        <source>Match time range</source>
        <translation>Matcha tidsintervall</translation>
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
        <translation>Spärrfördröjning:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="360"/>
        <source>0</source>
        <translation>0</translation>
    </message>
    <message>
        <source>Timebase sets how much time one sweep shows; leave it empty to use the plot's time range. Lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each one.</source>
        <translation type="vanished">Tidsbas anger hur mycket tid ett svep visar; lämna tomt för att använda plottens tidsintervall. Sänk den för att zooma in på en snabb signal. Holdoff ignorerar nya triggers ett ögonblick efter varje trigger.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="396"/>
        <source>Capture Next</source>
        <translation>Fånga Nästa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="398"/>
        <source>Arm for one more single-shot capture</source>
        <translation>Aktivera för en Ytterligare Engångsfångst</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="217"/>
        <source>Level:</source>
        <translation>Nivå:</translation>
    </message>
    <message>
        <source>Trigger level</source>
        <translation type="vanished">Triggernivå</translation>
    </message>
    <message>
        <source>Holdoff (ms):</source>
        <translation type="vanished">Spärrfördröjning (ms):</translation>
    </message>
    <message>
        <source>Holdoff time</source>
        <translation type="vanished">Spärrfördröjningstid</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="197"/>
        <source>Source:</source>
        <translation>Källa:</translation>
    </message>
    <message>
        <source>Re-arm</source>
        <translation type="vanished">Återaktivera</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="411"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
</context>
<context>
    <name>UART</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="52"/>
        <source>COM Port</source>
        <translation>Com-port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="81"/>
        <source>Baud Rate</source>
        <translation>Baudhastighet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="174"/>
        <source>Data Bits</source>
        <translation>Databitar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="195"/>
        <source>Parity</source>
        <translation>Paritet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="216"/>
        <source>Stop Bits</source>
        <translation>Stoppbitar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="237"/>
        <source>Flow Control</source>
        <translation>Flödeskontroll</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="269"/>
        <source>Auto Reconnect</source>
        <translation>Återanslut Automatiskt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="287"/>
        <source>Send DTR Signal</source>
        <translation>Skicka Dtr-signal</translation>
    </message>
</context>
<context>
    <name>UI::Dashboard</name>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1192"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1903"/>
        <source>Console</source>
        <translation>Konsol</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1274"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1914"/>
        <source>Notifications</source>
        <translation>Notifikationer</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1355"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1925"/>
        <source>Clock</source>
        <translation>Klocka</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1435"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1935"/>
        <source>Stopwatch</source>
        <translation>Stoppur</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1983"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1998"/>
        <source>%1 (Fallback)</source>
        <translation>%1 (Reserv)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="2020"/>
        <source>LED Panel (%1)</source>
        <translation>LED-panel (%1)</translation>
    </message>
</context>
<context>
    <name>UI::DashboardWidget</name>
    <message>
        <location filename="../../src/UI/DashboardWidget.cpp" line="148"/>
        <source>Invalid</source>
        <translation>Ogiltig</translation>
    </message>
</context>
<context>
    <name>UI::WindowManager</name>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1057"/>
        <source>Select Background Image</source>
        <translation>Välj Bakgrundsbild</translation>
    </message>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1059"/>
        <source>Images (*.png *.jpg *.jpeg *.bmp)</source>
        <translation>Bilder (*.png *.jpg *.jpeg *.bmp)</translation>
    </message>
</context>
<context>
    <name>USB</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="50"/>
        <source>USB Device</source>
        <translation>USB-enhet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="80"/>
        <source>Transfer Mode</source>
        <translation>Överföringsläge</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="89"/>
        <source>Bulk Stream</source>
        <translation>Bulk Stream</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="90"/>
        <source>Advanced (Bulk + Control)</source>
        <translation>Avancerat (Bulk + Kontroll)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="91"/>
        <source>Isochronous</source>
        <translation>Isokron</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="143"/>
        <source>Connect to USB devices using bulk, control, or isochronous transfers. Suitable for data loggers, custom firmware devices, and USB instruments.</source>
        <translation>Anslut till USB-enheter med bulk-, kontroll- eller isokrona överföringar. Lämplig för dataloggrar, anpassade firmware-enheter och USB-instrument.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="152"/>
        <source>USB specifications (USB.org)</source>
        <translation>USB-specifikationer (USB.org)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="169"/>
        <source>IN Endpoint</source>
        <translation>In-slutpunkt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="205"/>
        <source>OUT Endpoint</source>
        <translation>Out-slutpunkt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="241"/>
        <source>Max Packet Size</source>
        <translation>Max Paketstorlek</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="301"/>
        <source>Control Transfers Enabled</source>
        <translation>Kontrollöverföringar Aktiverade</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="310"/>
        <source>Sending incorrect control requests may crash or damage connected hardware. Use with caution.</source>
        <translation>Att skicka felaktiga kontrollförfrågningar kan krascha eller skada ansluten hårdvara. Använd med försiktighet.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="317"/>
        <source>Learn about USB control transfers</source>
        <translation>Läs om USB-kontrollöverföringar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="351"/>
        <source>Packet size should match the maximum transfer size reported by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).</source>
        <translation>Paketstorlek bör matcha den maximala överföringsstorlek som rapporteras av slutpunkten. Typiska värden: 192 B (FS audio), 1024 B (HS).</translation>
    </message>
</context>
<context>
    <name>Updater</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="477"/>
        <source>Would you like to download the update now?</source>
        <translation>Vill du ladda ner uppdateringen nu?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="479"/>
        <source>Would you like to download the update now?&lt;br /&gt;This is a mandatory update, exiting now will close the application.</source>
        <translation>Vill du ladda ner uppdateringen nu?&lt;br /&gt;Detta är en obligatorisk uppdatering, att avsluta nu kommer att stänga applikationen.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="485"/>
        <source>&lt;strong&gt;Change log:&lt;/strong&gt;&lt;br/&gt;%1</source>
        <translation>&lt;strong&gt;Ändringslogg:&lt;/strong&gt;&lt;br/&gt;%1</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="488"/>
        <source>Version %1 of %2 has been released!</source>
        <translation>Version %1 av %2 har släppts!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="520"/>
        <source>No updates are available for the moment</source>
        <translation>Inga uppdateringar är tillgängliga för tillfället</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="522"/>
        <source>Congratulations! You are running the latest version of %1</source>
        <translation>Grattis! Du kör den senaste versionen av %1</translation>
    </message>
</context>
<context>
    <name>UserTableView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="166"/>
        <source>Add Register</source>
        <translation>Lägg till Register</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="169"/>
        <source>Add register</source>
        <translation>Lägg till register</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="176"/>
        <source>Insert Constant</source>
        <translation>Infoga Konstant</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="179"/>
        <source>Insert constant</source>
        <translation>Infoga konstant</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="186"/>
        <source>Import</source>
        <translation>Importera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="189"/>
        <source>Import registers from CSV</source>
        <translation>Importera register från CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="196"/>
        <source>Export</source>
        <translation>Exportera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="199"/>
        <source>Export registers to CSV</source>
        <translation>Exportera register till CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="211"/>
        <source>Rename</source>
        <translation>Byt Namn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="214"/>
        <source>Rename table</source>
        <translation>Byt namn på tabell</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="221"/>
        <source>Delete</source>
        <translation>Ta Bort</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="224"/>
        <source>Delete table</source>
        <translation>Ta bort tabell</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="238"/>
        <source>Help</source>
        <translation>Hjälp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="243"/>
        <source>Open help documentation for shared memory</source>
        <translation>Öppna hjälpdokumentation för delat minne</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="283"/>
        <source>Permissions</source>
        <translation>Behörigheter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="284"/>
        <source>Register Name</source>
        <translation>Registernamn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="285"/>
        <source>Default Value</source>
        <translation>Standardvärde</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="324"/>
        <source>Read-Only</source>
        <translation>Skrivskyddad</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="324"/>
        <source>Read/Write</source>
        <translation>Läs/skriv</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="462"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Kopiera åtkomstkod %1 till urklipp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="495"/>
        <source>Delete register</source>
        <translation>Ta Bort Register</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="512"/>
        <source>No registers.</source>
        <translation>Inga register.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="562"/>
        <source>Register access code copied</source>
        <translation>Registeråtkomstkod kopierad</translation>
    </message>
</context>
<context>
    <name>Waterfall</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="232"/>
        <source>Show Colorbar</source>
        <translation>Visa Färgskala</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="245"/>
        <source>Show Axes &amp; Grid</source>
        <translation>Visa Axlar &amp; Rutnät</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="258"/>
        <source>Show Crosshair</source>
        <translation>Visa Hårkors</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="272"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="272"/>
        <source>Resume</source>
        <translation>Återuppta</translation>
    </message>
    <message>
        <source>Clear History</source>
        <translation type="vanished">Rensa Historik</translation>
    </message>
</context>
<context>
    <name>Welcome</name>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="177"/>
        <source>Welcome to %1!</source>
        <translation>Välkommen till %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="188"/>
        <source>Serial Studio is a powerful real-time visualization tool, built for engineers, students, and makers.</source>
        <translation>Serial Studio är ett kraftfullt realtidsvisualiseringsverktyg, byggt för ingenjörer, studenter och makers.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="199"/>
        <source>You can start a fully-functional 14-day trial, activate it with your license key, or download and compile the GPLv3 source code yourself.</source>
        <translation>Du kan starta en fullt funktionell 14-dagars provperiod, aktivera den med din licensnyckel eller ladda ner och kompilera GPLv3-källkoden själv.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="209"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="394"/>
        <source>Buying Pro supports the author directly and helps fund future development.</source>
        <translation>Att köpa Pro stödjer författaren direkt och hjälper till att finansiera framtida utveckling.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="217"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="402"/>
        <source>Building the GPLv3 version yourself helps grow the community and encourages technical contributions.</source>
        <translation>Att bygga GPLv3-versionen själv hjälper till att växa communityn och uppmuntrar tekniska bidrag.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="239"/>
        <source>Please wait…</source>
        <translation>Vänligen vänta…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="275"/>
        <source>%1 days remaining in your trial.</source>
        <translation>%1 dagar kvar av din provperiod.</translation>
    </message>
    <message>
        <source>You’re currently using the fully-featured trial of %1 Pro. It’s valid for 14 days of personal, non-commercial use.</source>
        <translation type="vanished">Du använder för närvarande den fullt utrustade provversionen av %1 Pro. Den är giltig i 14 dagar för personligt, icke-kommersiellt bruk.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="285"/>
        <source>You're currently using the fully-featured trial of %1 Pro. It's valid for 14 days of personal, non-commercial use.</source>
        <translation>Du använder för närvarande den fullt utrustade provversionen av %1 Pro. Den är giltig i 14 dagar för personligt, icke-kommersiellt bruk.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="296"/>
        <source>Upgrade to a paid plan to keep using Serial Studio Pro.</source>
        <translation>Uppgradera till en betald plan för att fortsätta använda Serial Studio Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="304"/>
        <source>Or, compile the GPLv3 source code to use it for free.</source>
        <translation>Eller kompilera GPLv3-källkoden för att använda den gratis.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="312"/>
        <source>To see available subscription plans, click "Upgrade Now" below.</source>
        <translation>Klicka på "Uppgradera nu" nedan för att se tillgängliga prenumerationsplaner.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="333"/>
        <source>Don't nag me about the trial.
I understand that when it ends, I'll need to buy a license or build the GPLv3 version.</source>
        <translation>Påminn mig inte om provperioden.
Jag förstår att när den löper ut måste jag köpa en licens eller bygga GPLv3-versionen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="364"/>
        <source>Your %1 trial has expired.</source>
        <translation>Din %1-provperiod har löpt ut.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="374"/>
        <source>Your trial period has ended. To continue using %1 with all Pro features, please upgrade to a paid plan.</source>
        <translation>Din provperiod har löpt ut. För att fortsätta använda %1 med alla Pro-funktioner, uppgradera till en betald plan.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="385"/>
        <source>If you prefer, you can also compile the open-source version under the GPLv3 license.</source>
        <translation>Om du föredrar det kan du också kompilera open source-versionen under GPLv3-licensen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="413"/>
        <source>Thank you for trying %1!</source>
        <translation>Tack för att du provar %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="455"/>
        <source>Upgrade Now</source>
        <translation>Uppgradera Nu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="464"/>
        <source>Activate</source>
        <translation>Aktivera</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Open in Limited Mode</source>
        <translation>Öppna i Begränsat Läge</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Continue</source>
        <translation>Fortsätt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Start Trial</source>
        <translation>Starta Provperiod</translation>
    </message>
</context>
<context>
    <name>WhatsNew</name>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="31"/>
        <source>What's New in %1</source>
        <translation>Vad Är Nytt i %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="58"/>
        <source>Lua &amp; Built-In Parsers</source>
        <translation>Lua &amp; Inbyggda Tolkar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="59"/>
        <source>Parse frames in Lua 5.4 or with code-free Built-In templates, alongside JavaScript.</source>
        <translation>Tolka frames i Lua 5.4 eller med kodfria inbyggda mallar, tillsammans med JavaScript.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="67"/>
        <source>AI Assistant</source>
        <translation>Ai-assistent</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="68"/>
        <source>An in-app assistant across eight providers that can build and edit projects for you.</source>
        <translation>En inbyggd assistent med åtta leverantörer som kan skapa och redigera projekt åt dig.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="76"/>
        <source>Oscilloscope Sweep &amp; Trigger</source>
        <translation>Oscilloskop-svep &amp; Trigger</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="77"/>
        <source>Scope-style sweep with an animated trigger cursor you can drag on the plot.</source>
        <translation>Svep i oscilloskopstil med en animerad triggermarkör som kan dras på plottet.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="85"/>
        <source>Output Controls</source>
        <translation>Utgångskontroller</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="86"/>
        <source>Transmit back to your device with control widgets and a protocol-helper engine.</source>
        <translation>Skicka tillbaka till din enhet med kontrollwidgets och en protokollhjälpmotor.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="94"/>
        <source>Dashboard Workspaces</source>
        <translation>Dashboard-arbetsytor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="95"/>
        <source>Group widgets into your own dashboards and find them from the taskbar search.</source>
        <translation>Gruppera widgets i dina egna dashboards och hitta dem via aktivitetsfältets sökfunktion.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="103"/>
        <source>Session Database &amp; Reports</source>
        <translation>Sessionsdatabas och Rapporter</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="104"/>
        <source>Record sessions to SQLite, replay them, and export HTML or PDF reports.</source>
        <translation>Spela in sessioner till SQLITE, spela upp dem och exportera HTML- eller PDF-rapporter.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="112"/>
        <source>Operator Deployments</source>
        <translation>Operatörsdistributioner</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="113"/>
        <source>Ship a locked-down, single-purpose build to operators with a runtime-only mode.</source>
        <translation>Leverera en låst, specialanpassad version till operatörer med endast körläge.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="121"/>
        <source>New Dashboard Widgets</source>
        <translation>Nya Dashboard-widgets</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="122"/>
        <source>Gauge and Meter faces with live readouts, plus Clock, Stopwatch, and Waterfall.</source>
        <translation>Mätar- och visartavlor med livevisning, samt Klocka, Stoppur och Vattenfall.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="130"/>
        <source>Dataset Transforms</source>
        <translation>Datamängdstransformeringar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="131"/>
        <source>Calibrate, filter, and convert each channel with a per-dataset function, or add virtual datasets that compute new channels.</source>
        <translation>Kalibrera, filtrera och konvertera varje kanal med en funktion per dataset, eller lägg till virtuella dataset som beräknar nya kanaler.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="139"/>
        <source>Painter Widget</source>
        <translation>Painter-widget</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="140"/>
        <source>Draw fully custom graphics from incoming data with your own drawing script.</source>
        <translation>Rita helt anpassad grafik från inkommande data med ditt eget ritningsskript.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="148"/>
        <source>Data Tables</source>
        <translation>Datatabeller</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="149"/>
        <source>Live register-style tables with virtual datasets and editable cells.</source>
        <translation>Levande registerliknande tabeller med virtuella dataset och redigerbara celler.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="157"/>
        <source>Image Widget</source>
        <translation>Bild-widget</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="158"/>
        <source>Decode and display image frames streamed straight from your device.</source>
        <translation>Avkoda och visa bildramar som strömmas direkt från din enhet.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="166"/>
        <source>Notifications &amp; Alarms</source>
        <translation>Notiser &amp; Larm</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="167"/>
        <source>Multi-band dataset alarms with severity tiers, routed to the Notification Center.</source>
        <translation>Multibands-datasetlarm med allvarlighetsnivåer, dirigerade till Notiscenter.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="175"/>
        <source>Project Lock</source>
        <translation>Projektlås</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="176"/>
        <source>Lock a project to separate operator use from editing, with an access code.</source>
        <translation>Lås ett projekt för att separera operatörsanvändning från redigering, med en åtkomstkod.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="184"/>
        <source>MQTT, Protobuf &amp; File Transfer</source>
        <translation>MQTT, Protobuf &amp; Filöverföring</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="185"/>
        <source>MQTT input and publishing, Protobuf import, and XMODEM / YMODEM / ZMODEM transfers.</source>
        <translation>MQTT-inmatning och publicering, Protobuf-import och XMODEM / YMODEM / ZMODEM-överföringar.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="231"/>
        <source>Welcome to %1!</source>
        <translation>Välkommen till %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="241"/>
        <source>Here's what's new in version %1.</source>
        <translation>Här är vad som är nytt i version %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="414"/>
        <source>Show on Startup</source>
        <translation>Visa vid start</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="421"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
</context>
<context>
    <name>WidgetDelegate</name>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Ta bort från Arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml" line="335"/>
        <source>Device Disconnected</source>
        <translation>Enhet Frånkopplad</translation>
    </message>
</context>
<context>
    <name>Widgets::Bar</name>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="331"/>
        <source>Alarm</source>
        <translation>Larm</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="332"/>
        <source>critical</source>
        <translation>kritiskt</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="332"/>
        <source>warning</source>
        <translation>varning</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="336"/>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation>Värde %1%2 gick in i %3-bandet (%4–%5).</translation>
    </message>
    <message>
        <source>Value %1%2 reached the high alarm %3%2</source>
        <translation type="vanished">Värde %1%2 nådde det höga larmet %3%2</translation>
    </message>
    <message>
        <source>Value %1%2 reached the low alarm %3%2</source>
        <translation type="vanished">Värde %1%2 nådde det låga larmet %3%2</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="342"/>
        <source>Alarms</source>
        <translation>Larm</translation>
    </message>
</context>
<context>
    <name>Widgets::Compass</name>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="157"/>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="180"/>
        <source>N</source>
        <translation>N</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="160"/>
        <source>NE</source>
        <translation>NO</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="163"/>
        <source>E</source>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="166"/>
        <source>SE</source>
        <translation>SO</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="169"/>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="172"/>
        <source>SW</source>
        <translation>SV</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="175"/>
        <source>W</source>
        <translation>V</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="178"/>
        <source>NW</source>
        <translation>NV</translation>
    </message>
</context>
<context>
    <name>Widgets::DataGrid</name>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="129"/>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="130"/>
        <source>Value</source>
        <translation>Värde</translation>
    </message>
    <message>
        <source>Awaiting data…</source>
        <translation type="vanished">Väntar på data…</translation>
    </message>
</context>
<context>
    <name>Widgets::GPS</name>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery</source>
        <translation>Satellitbilder</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery with Labels</source>
        <translation>Satellitbilder med Etiketter</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Street Map</source>
        <translation>Gatukarta</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Topographic Map</source>
        <translation>Topografisk Karta</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Terrain</source>
        <translation>Terräng</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Light Gray Canvas</source>
        <translation>Ljusgrå Duk</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>Dark Gray Canvas</source>
        <translation>Mörkgrå Duk</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>National Geographic</source>
        <translation>National Geographic</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="373"/>
        <source>Additional map layers are available only for Pro users.</source>
        <translation>Ytterligare kartlager är endast tillgängliga för Pro-användare.</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="374"/>
        <source>We can't offer unrestricted access because the ArcGIS API key incurs real costs.</source>
        <translation>Vi kan inte erbjuda obegränsad åtkomst eftersom ArcGIS API-nyckeln medför verkliga kostnader.</translation>
    </message>
</context>
<context>
    <name>Widgets::MultiPlot</name>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="97"/>
        <source>Time (s)</source>
        <translation>Tid (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="97"/>
        <source>Samples</source>
        <translation>Sampel</translation>
    </message>
</context>
<context>
    <name>Widgets::Output::Base</name>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="168"/>
        <source>Transmit script timed out after %1 ms</source>
        <translation>Överföringsskript överskred tidsgränsen efter %1 ms</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="184"/>
        <source>Payload exceeds maximum size</source>
        <translation>Nyttolast överskrider maximal storlek</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="86"/>
        <source>Time (s)</source>
        <translation>Tid (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="112"/>
        <source>Samples</source>
        <translation>Sampel</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot3D</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot3D.cpp" line="959"/>
        <source>Grid Interval: %1 unit(s)</source>
        <translation>Rutnätsintervall: %1 enhet(er)</translation>
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
        <translation>Het</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="409"/>
        <source>Grayscale</source>
        <translation>Gråskala</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="411"/>
        <source>Unknown</source>
        <translation>Okänd</translation>
    </message>
</context>
<context>
    <name>WorkspaceDialog</name>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="51"/>
        <source>Edit Workspace</source>
        <translation>Redigera Arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="52"/>
        <source>New Workspace</source>
        <translation>Ny Arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="157"/>
        <source>Name:</source>
        <translation>Namn:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="166"/>
        <source>My Workspace</source>
        <translation>Min Arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="181"/>
        <source>Select widgets to include:</source>
        <translation>Välj widgets att inkludera:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="187"/>
        <source>Filter widgets…</source>
        <translation>Filtrera widgets…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="302"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="309"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
<context>
    <name>WorkspaceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="41"/>
        <source>Workspace</source>
        <translation>Arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="129"/>
        <source>Add Widget</source>
        <translation>Lägg till Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="131"/>
        <source>Add widget to workspace</source>
        <translation>Lägg till widget till arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="142"/>
        <source>Rename</source>
        <translation>Byt Namn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="144"/>
        <source>Rename workspace</source>
        <translation>Byt namn på arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="153"/>
        <source>Delete</source>
        <translation>Ta Bort</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="155"/>
        <source>Delete workspace</source>
        <translation>Ta bort arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="177"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="183"/>
        <source>Group</source>
        <translation>Grupp</translation>
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
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="229"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="267"/>
        <source>(unknown)</source>
        <translation>(okänd)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="247"/>
        <source>(group widget)</source>
        <translation>(gruppwidget)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="297"/>
        <source>Remove widget from workspace</source>
        <translation>Ta bort widget från arbetsytan</translation>
    </message>
    <message>
        <source>Remove from workspace</source>
        <translation type="vanished">Ta bort från arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="317"/>
        <source>No widgets in this workspace.</source>
        <translation>Inga widgetar i denna arbetsyta.</translation>
    </message>
</context>
<context>
    <name>WorkspacesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="33"/>
        <source>Workspaces</source>
        <translation>Arbetsytor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="127"/>
        <source>Customize</source>
        <translation>Anpassa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="129"/>
        <source>Edit workspaces manually</source>
        <translation>Redigera arbetsytor manuellt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="145"/>
        <source>Move Up</source>
        <translation>Flytta Upp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="147"/>
        <source>Move the selected workspace up</source>
        <translation>Flytta den valda arbetsytan uppåt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="158"/>
        <source>Move Down</source>
        <translation>Flytta Ner</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="160"/>
        <source>Move the selected workspace down</source>
        <translation>Flytta den valda arbetsytan nedåt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="171"/>
        <source>Add Workspace</source>
        <translation>Lägg till Arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="173"/>
        <source>Add workspace</source>
        <translation>Lägg till arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="182"/>
        <source>Cleanup</source>
        <translation>Rensa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="185"/>
        <source>Remove %1 widget reference(s) whose target group or dataset no longer exists</source>
        <translation>Ta bort %1 widgetreferens(er) vars målgrupp eller dataset inte längre finns</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="188"/>
        <source>No stale widget references in any workspace</source>
        <translation>Inga föråldrade widgetreferenser i någon arbetsyta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="203"/>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="204"/>
        <source>Widgets</source>
        <translation>Widgetar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="276"/>
        <source>No workspaces. Add one with the toolbar above, or reset to the auto layout.</source>
        <translation>Inga arbetsytor. Lägg till en med verktygsfältet ovan, eller återställ till automatisk layout.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="278"/>
        <source>Project has no eligible groups -- add a group with widgets to populate workspaces.</source>
        <translation>Projektet har inga lämpliga grupper -- lägg till en grupp med widgets för att fylla arbetsytor.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="284"/>
        <source>Reset to Auto Layout</source>
        <translation>Återställ till Automatisk Layout</translation>
    </message>
    <message>
        <source>No workspaces.</source>
        <translation type="vanished">Inga arbetsytor.</translation>
    </message>
</context>
</TS>